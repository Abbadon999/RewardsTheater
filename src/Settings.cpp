// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023, Lev Leontev

#include "Settings.h"

static const char* const PLUGIN_NAME = "RewardsTheater";
static const char* const REWARD_REDEMPTIONS_QUEUE_ENABLED_KEY = "REWARD_REDEMPTIONS_QUEUE_ENABLED_KEY";
static const char* const INTERVAL_BETWEEN_REWARDS_SECONDS_KEY = "INTERVAL_BETWEEN_REWARDS_SECONDS_KEY";
static const char* const TWITCH_ACCESS_TOKEN_KEY = "TWITCH_ACCESS_TOKEN_KEY";
static const char* const RANDOM_POSITION_ENABLED_KEY = "RANDOM_POSITION_ENABLED_KEY";
static const char* const LAST_OBS_SOURCE_NAME_KEY = "LAST_OBS_SOURCE_NAME_KEY";
static const char* const LAST_VIDEO_WIDTH_KEY = "LAST_VIDEO_WIDTH_KEY";
static const char* const LAST_VIDEO_HEIGHT_KEY = "LAST_VIDEO_HEIGHT_KEY";

Settings::Settings(config_t* config) : config(config) {}

bool Settings::isRewardRedemptionQueueEnabled() const {
    config_set_default_bool(config, PLUGIN_NAME, REWARD_REDEMPTIONS_QUEUE_ENABLED_KEY, true);
    return config_get_bool(config, PLUGIN_NAME, REWARD_REDEMPTIONS_QUEUE_ENABLED_KEY);
}

void Settings::setRewardRedemptionQueueEnabled(bool rewardRedemptionQueueEnabled) {
    config_set_bool(config, PLUGIN_NAME, REWARD_REDEMPTIONS_QUEUE_ENABLED_KEY, rewardRedemptionQueueEnabled);
}

double Settings::getIntervalBetweenRewardsSeconds() const {
    config_set_default_double(config, PLUGIN_NAME, INTERVAL_BETWEEN_REWARDS_SECONDS_KEY, 0);
    return config_get_double(config, PLUGIN_NAME, INTERVAL_BETWEEN_REWARDS_SECONDS_KEY);
}

void Settings::setIntervalBetweenRewardsSeconds(double intervalBetweenRewardsSeconds) {
    config_set_double(config, PLUGIN_NAME, INTERVAL_BETWEEN_REWARDS_SECONDS_KEY, intervalBetweenRewardsSeconds);
}

std::optional<std::string> Settings::getTwitchAccessToken() const {
    std::lock_guard lock(configMutex);
    config_set_default_string(config, PLUGIN_NAME, TWITCH_ACCESS_TOKEN_KEY, "");
    std::string result = config_get_string(config, PLUGIN_NAME, TWITCH_ACCESS_TOKEN_KEY);
    if (result.empty()) {
        return {};
    } else {
        return result;
    }
}

void Settings::setTwitchAccessToken(const std::optional<std::string>& accessToken) {
    std::lock_guard lock(configMutex);
    if (accessToken) {
        config_set_string(config, PLUGIN_NAME, TWITCH_ACCESS_TOKEN_KEY, accessToken.value().c_str());
    } else {
        config_remove_value(config, PLUGIN_NAME, TWITCH_ACCESS_TOKEN_KEY);
    }
}

std::optional<std::string> Settings::getObsSourceName(const std::string& rewardId) const {
    std::lock_guard lock(configMutex);
    config_set_default_string(config, PLUGIN_NAME, rewardId.c_str(), "");
    std::string result = config_get_string(config, PLUGIN_NAME, rewardId.c_str());
    if (result.empty()) {
        return {};
    } else {
        return result;
    }
}

void Settings::setObsSourceName(const std::string& rewardId, const std::optional<std::string>& obsSourceName) {
    std::lock_guard lock(configMutex);
    if (obsSourceName.has_value()) {
        config_set_string(config, PLUGIN_NAME, rewardId.c_str(), obsSourceName.value().c_str());
    } else {
        config_remove_value(config, PLUGIN_NAME, rewardId.c_str());
    }
}

static std::string getRandomPositionEnabledKey(const std::string& rewardId);

bool Settings::isRandomPositionEnabled(const std::string& rewardId) const {
    config_set_default_bool(config, PLUGIN_NAME, getRandomPositionEnabledKey(rewardId).c_str(), false);
    return config_get_bool(config, PLUGIN_NAME, getRandomPositionEnabledKey(rewardId).c_str());
}

void Settings::setRandomPositionEnabled(const std::string& rewardId, bool randomPositionEnabled) {
    config_set_bool(config, PLUGIN_NAME, getRandomPositionEnabledKey(rewardId).c_str(), randomPositionEnabled);
}

static std::string getLastVideoWidthKey(const std::string& rewardId);
static std::string getLastVideoHeightKey(const std::string& rewardId);

std::optional<std::pair<std::uint32_t, std::uint32_t>> Settings::getLastVideoSize(
    const std::string& rewardId,
    const std::string& obsSourceName
) const {
    std::lock_guard lock(configMutex);
    if (getLastObsSourceName(rewardId) != obsSourceName) {
        return {};
    }

    std::string lastVideoWidthKey = getLastVideoWidthKey(rewardId);
    std::string lastVideoHeightKey = getLastVideoHeightKey(rewardId);
    config_set_default_uint(config, PLUGIN_NAME, lastVideoWidthKey.c_str(), 0);
    config_set_default_uint(config, PLUGIN_NAME, lastVideoHeightKey.c_str(), 0);
    uint32_t lastVideoWidth = static_cast<uint32_t>(config_get_uint(config, PLUGIN_NAME, lastVideoWidthKey.c_str()));
    uint32_t lastVideoHeight = static_cast<uint32_t>(config_get_uint(config, PLUGIN_NAME, lastVideoHeightKey.c_str()));
    if (lastVideoWidth == 0 || lastVideoHeight == 0) {
        return {};
    } else {
        return std::make_pair(lastVideoWidth, lastVideoHeight);
    }
}

void Settings::setLastVideoSize(
    const std::string& rewardId,
    const std::string& obsSourceName,
    const std::optional<std::pair<std::uint32_t, std::uint32_t>>& lastVideoSize
) {
    std::lock_guard lock(configMutex);
    setLastObsSourceName(rewardId, obsSourceName);
    std::string lastVideoWidthKey = getLastVideoWidthKey(rewardId);
    std::string lastVideoHeightKey = getLastVideoHeightKey(rewardId);
    if (lastVideoSize.has_value()) {
        config_set_uint(config, PLUGIN_NAME, lastVideoWidthKey.c_str(), lastVideoSize.value().first);
        config_set_uint(config, PLUGIN_NAME, lastVideoHeightKey.c_str(), lastVideoSize.value().second);
    } else {
        config_remove_value(config, PLUGIN_NAME, lastVideoWidthKey.c_str());
        config_remove_value(config, PLUGIN_NAME, lastVideoHeightKey.c_str());
    }
}

static std::string getLastObsSourceKey(const std::string& rewardId);

void Settings::deleteReward(const std::string& rewardId) {
    std::lock_guard lock(configMutex);
    config_remove_value(config, PLUGIN_NAME, rewardId.c_str());
    config_remove_value(config, PLUGIN_NAME, getRandomPositionEnabledKey(rewardId).c_str());
    config_remove_value(config, PLUGIN_NAME, getLastObsSourceKey(rewardId).c_str());
    config_remove_value(config, PLUGIN_NAME, getLastVideoWidthKey(rewardId).c_str());
    config_remove_value(config, PLUGIN_NAME, getLastVideoHeightKey(rewardId).c_str());
}

std::string Settings::getLastObsSourceName(const std::string& rewardId) const {
    std::lock_guard lock(configMutex);
    std::string lastObsSourceKey = getLastObsSourceKey(rewardId);
    config_set_default_string(config, PLUGIN_NAME, lastObsSourceKey.c_str(), "");
    return config_get_string(config, PLUGIN_NAME, lastObsSourceKey.c_str());
}

void Settings::setLastObsSourceName(const std::string& rewardId, const std::string& obsSourceName) {
    std::lock_guard lock(configMutex);
    config_set_string(config, PLUGIN_NAME, getLastObsSourceKey(rewardId).c_str(), obsSourceName.c_str());
}

std::string getRandomPositionEnabledKey(const std::string& rewardId) {
    return rewardId + RANDOM_POSITION_ENABLED_KEY;
}

std::string getLastVideoWidthKey(const std::string& rewardId) {
    return rewardId + LAST_VIDEO_WIDTH_KEY;
}

std::string getLastVideoHeightKey(const std::string& rewardId) {
    return rewardId + LAST_VIDEO_HEIGHT_KEY;
}

std::string getLastObsSourceKey(const std::string& rewardId) {
    return rewardId + LAST_OBS_SOURCE_NAME_KEY;
}
