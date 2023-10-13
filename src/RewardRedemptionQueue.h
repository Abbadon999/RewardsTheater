// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2023, Lev Leontev

#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <obs.hpp>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "IoThreadPool.h"
#include "Reward.h"
#include "Settings.h"

class RewardRedemptionQueue {
public:
    RewardRedemptionQueue(const Settings& settings);
    ~RewardRedemptionQueue();

    std::vector<RewardRedemption> getRewardRedemptionQueue() const;
    void queueRewardRedemption(const RewardRedemption& rewardRedemption);
    void removeRewardRedemption(const RewardRedemption& rewardRedemption);

    void playObsSource(const std::string& obsSourceName);
    static std::vector<std::string> enumObsSources();

private:
    boost::asio::awaitable<void> asyncPlayRewardRedemptionsFromQueue();
    boost::asio::awaitable<RewardRedemption> asyncGetNextRewardRedemption();
    void playObsSource(OBSSourceAutoRelease source);
    boost::asio::awaitable<void> asyncPlayObsSource(OBSSourceAutoRelease source);
    boost::asio::deadline_timer createDeadlineTimer(obs_source_t* source);
    OBSSourceAutoRelease getObsSource(const RewardRedemption& rewardRedemption);
    OBSSourceAutoRelease getObsSource(const std::string& sourceName);

    static void startObsSource(obs_source_t* source);
    static void stopObsSource(obs_source_t* source);
    static void setSourceVisible(obs_source_t* source, bool visible);
    static bool isMediaSource(const obs_source_t* source);

    const Settings& settings;

    IoThreadPool rewardRedemptionQueueThread;
    mutable std::mutex rewardRedemptionQueueMutex;
    boost::asio::deadline_timer rewardRedemptionQueueCondVar;
    std::vector<RewardRedemption> rewardRedemptionQueue;

    unsigned playObsSourceState = 0;
    std::map<obs_source_t*, unsigned> sourcePlayedByState;
};