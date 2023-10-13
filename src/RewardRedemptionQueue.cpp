// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2023, Lev Leontev

#include "RewardRedemptionQueue.h"

#include <algorithm>
#include <boost/system/system_error.hpp>
#include <cstdint>
#include <cstring>
#include <utility>

namespace asio = boost::asio;

RewardRedemptionQueue::RewardRedemptionQueue(const Settings& settings)
    : settings(settings), rewardRedemptionQueueThread(1),
      rewardRedemptionQueueCondVar(rewardRedemptionQueueThread.ioContext, boost::posix_time::pos_infin) {
    asio::co_spawn(rewardRedemptionQueueThread.ioContext, asyncPlayRewardRedemptionsFromQueue(), asio::detached);
}

RewardRedemptionQueue::~RewardRedemptionQueue() {
    rewardRedemptionQueueThread.stop();
}

std::vector<RewardRedemption> RewardRedemptionQueue::getRewardRedemptionQueue() const {
    std::lock_guard<std::mutex> guard(rewardRedemptionQueueMutex);
    return rewardRedemptionQueue;
}

void RewardRedemptionQueue::queueRewardRedemption(const RewardRedemption& rewardRedemption) {
    if (!settings.isRewardRedemptionQueueEnabled()) {
        std::optional<std::string> obsSourceName = settings.getObsSourceName(rewardRedemption.reward.id);
        if (obsSourceName.has_value()) {
            playObsSource(obsSourceName.value());
        }
        return;
    }

    {
        std::lock_guard<std::mutex> guard(rewardRedemptionQueueMutex);
        rewardRedemptionQueue.push_back(rewardRedemption);
    }
    rewardRedemptionQueueThread.ioContext.post([this]() {
        rewardRedemptionQueueCondVar.cancel();  // Equivalent to notify_all() in a condition variable
    });
}

void RewardRedemptionQueue::removeRewardRedemption(const RewardRedemption& rewardRedemption) {
    stopObsSource(getObsSource(rewardRedemption));
    std::lock_guard<std::mutex> guard(rewardRedemptionQueueMutex);
    auto position = std::find(rewardRedemptionQueue.begin(), rewardRedemptionQueue.end(), rewardRedemption);
    if (position != rewardRedemptionQueue.end()) {
        rewardRedemptionQueue.erase(position);
    }
}

void RewardRedemptionQueue::playObsSource(const std::string& obsSourceName) {
    playObsSource(getObsSource(obsSourceName));
}

std::vector<std::string> RewardRedemptionQueue::enumObsSources() {
    std::vector<std::string> sources;

    struct AddToSourcesCallback {
        static bool addToSources(void* param, obs_source_t* source) {
            if (isMediaSource(source)) {
                auto& sources = *static_cast<std::vector<std::string>*>(param);
                sources.emplace_back(obs_source_get_name(source));
            }
            return true;
        }
    };

    obs_enum_sources(&AddToSourcesCallback::addToSources, &sources);
    return sources;
}

asio::awaitable<void> RewardRedemptionQueue::asyncPlayRewardRedemptionsFromQueue() {
    while (true) {
        RewardRedemption nextReward = co_await asyncGetNextRewardRedemption();
        co_await asyncPlayObsSource(getObsSource(nextReward));
        auto timeBeforeNextReward =
            std::chrono::milliseconds(static_cast<long long>(1000 * settings.getIntervalBetweenRewardsSeconds()));
        co_await asio::steady_timer(rewardRedemptionQueueThread.ioContext, timeBeforeNextReward)
            .async_wait(asio::use_awaitable);
    }
}

asio::awaitable<RewardRedemption> RewardRedemptionQueue::asyncGetNextRewardRedemption() {
    while (true) {
        {
            std::lock_guard guard(rewardRedemptionQueueMutex);
            if (!rewardRedemptionQueue.empty()) {
                RewardRedemption result = rewardRedemptionQueue.front();
                rewardRedemptionQueue.erase(rewardRedemptionQueue.begin());
                co_return result;
            }
        }
        try {
            co_await rewardRedemptionQueueCondVar.async_wait(asio::use_awaitable);
        } catch (const boost::system::system_error&) {
            // Condition variable signalled.
        }
    }
}

void RewardRedemptionQueue::playObsSource(OBSSourceAutoRelease source) {
    asio::co_spawn(rewardRedemptionQueueThread.ioContext, asyncPlayObsSource(std::move(source)), asio::detached);
}

asio::awaitable<void> RewardRedemptionQueue::asyncPlayObsSource(OBSSourceAutoRelease source) {
    if (!source) {
        co_return;
    }
    unsigned state = playObsSourceState++;
    sourcePlayedByState[source] = state;

    asio::deadline_timer deadlineTimer = createDeadlineTimer(source);

    struct StopDeadlineTimerCallback {
        asio::deadline_timer& deadlineTimer;
        asio::io_context& ioContext;

        static void stopDeadlineTimer(void* param, [[maybe_unused]] calldata_t* data) {
            auto& [deadlineTimer, ioContext] = *static_cast<StopDeadlineTimerCallback*>(param);
            // Posting to ioContext so that cancellation occurs after the async_wait (we have one thread).
            ioContext.post([&deadlineTimer]() {
                deadlineTimer.cancel();
            });
        }
    } callback(deadlineTimer, rewardRedemptionQueueThread.ioContext);

    signal_handler_t* signalHandler = obs_source_get_signal_handler(source);
    OBSSignal mediaEndedSignal(signalHandler, "media_ended", &StopDeadlineTimerCallback::stopDeadlineTimer, &callback);
    startObsSource(source);

    try {
        co_await deadlineTimer.async_wait(asio::use_awaitable);
    } catch (const boost::system::system_error&) {
        // Timer cancelled by the media_ended signal
    }

    if (sourcePlayedByState[source] == state) {
        sourcePlayedByState.erase(source);
        mediaEndedSignal.Disconnect();
        stopObsSource(source);
    }
}

asio::deadline_timer RewardRedemptionQueue::createDeadlineTimer(obs_source_t* source) {
    asio::deadline_timer deadlineTimer(rewardRedemptionQueueThread.ioContext);

    std::int64_t durationMilliseconds = obs_source_media_get_duration(source);
    if (durationMilliseconds != -1) {
        std::int64_t deadlineMilliseconds = durationMilliseconds + durationMilliseconds / 2;
        deadlineTimer.expires_from_now(boost::posix_time::milliseconds(deadlineMilliseconds));
    } else {
        deadlineTimer.expires_from_now(boost::posix_time::pos_infin);
    }

    return deadlineTimer;
}

OBSSourceAutoRelease RewardRedemptionQueue::getObsSource(const RewardRedemption& rewardRedemption) {
    std::optional<std::string> obsSourceName = settings.getObsSourceName(rewardRedemption.reward.id);
    if (!obsSourceName) {
        return {};
    }
    return getObsSource(obsSourceName.value());
}

OBSSourceAutoRelease RewardRedemptionQueue::getObsSource(const std::string& obsSourceName) {
    OBSSourceAutoRelease source = obs_get_source_by_name(obsSourceName.c_str());
    if (!isMediaSource(source)) {
        return {};
    }
    return source;
}

void RewardRedemptionQueue::startObsSource(obs_source_t* source) {
    if (!source) {
        return;
    }
    setSourceVisible(source, true);
    obs_source_media_restart(source);
}

void RewardRedemptionQueue::stopObsSource(obs_source_t* source) {
    if (!source) {
        return;
    }
    obs_source_media_stop(source);
    setSourceVisible(source, false);
}

void RewardRedemptionQueue::setSourceVisible(obs_source_t* source, bool visible) {
    struct SetSourceVisibleCallback {
        obs_source_t* source;
        bool visible;

        static bool setSourceVisibleOnScene(void* param, obs_source_t* sceneParam) {
            auto [source, visible] = *static_cast<SetSourceVisibleCallback*>(param);
            obs_scene_t* scene = obs_scene_from_source(sceneParam);
            obs_sceneitem_t* sceneItem = obs_scene_find_source(scene, obs_source_get_name(source));
            if (sceneItem) {
                obs_sceneitem_set_visible(sceneItem, visible);
            }
            return true;
        }
    } callback(source, visible);

    obs_enum_scenes(&SetSourceVisibleCallback::setSourceVisibleOnScene, &callback);
}

bool RewardRedemptionQueue::isMediaSource(const obs_source_t* source) {
    if (!source) {
        return false;
    }
    const char* sourceId = obs_source_get_id(source);
    return std::strcmp(sourceId, "ffmpeg_source") == 0 || std::strcmp(sourceId, "vlc_source") == 0;
}