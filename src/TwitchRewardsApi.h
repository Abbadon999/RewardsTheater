// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2023, Lev Leontev

#pragma once

#include <boost/json.hpp>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "BoostAsio.h"
#include "Reward.h"
#include "TwitchAuth.h"

namespace detail {
class QObjectCallback;
}

class TwitchRewardsApi : public QObject {
    Q_OBJECT

public:
    TwitchRewardsApi(TwitchAuth& twitchAuth, boost::asio::io_context& ioContext);
    ~TwitchRewardsApi();

    /// Calls the receiver with rewards as std::variant<std::exception_ptr, std::vector<Reward>>.
    void getRewards(bool onlyManageableRewards, QObject* receiver, const char* member);

    /// Calls the receiver with the result as std::exception_ptr.
    void deleteReward(const std::string& rewardId, QObject* receiver, const char* member);

    /// Calls the receiver with the downloaded bytes as std::string upon success.
    void downloadImage(const Reward& reward, QObject* receiver, const char* member);

    class InvalidRewardParametersException : public std::exception {
    public:
        InvalidRewardParametersException(const boost::json::value& response);
        const char* what() const noexcept override;

    private:
        std::string message;
    };

    class NotAffiliateException : public std::exception {
    public:
        const char* what() const noexcept override;
    };

    class UnexpectedHttpStatusException : public std::exception {
    public:
        UnexpectedHttpStatusException(const boost::json::value& response);
        const char* what() const noexcept override;

    private:
        std::string message;
    };

private:
    boost::asio::awaitable<void> asyncGetRewards(bool onlyManageableRewards, detail::QObjectCallback& callback);
    boost::asio::awaitable<void> asyncDeleteReward(std::string rewardId, detail::QObjectCallback& callback);
    boost::asio::awaitable<void> asyncDownloadImage(boost::urls::url url, detail::QObjectCallback& callback);

    boost::asio::awaitable<std::vector<Reward>> asyncGetRewards(bool onlyManageableRewards);
    static Reward parseReward(const boost::json::value& reward);
    static Color hexColorToColor(const std::string& hexColor);
    static boost::urls::url getImageUrl(const boost::json::value& reward);
    static std::optional<std::int64_t> getOptionalSetting(const boost::json::value& setting, const std::string& key);

    boost::asio::awaitable<void> asyncDeleteReward(const std::string& rewardId);
    boost::asio::awaitable<std::string> asyncDownloadImage(const boost::urls::url& url);

    TwitchAuth& twitchAuth;
    boost::asio::io_context& ioContext;
};

namespace detail {

/// Calls a method on a QObject, or does nothing if the QObject no longer exists.
class QObjectCallback : public QObject {
    Q_OBJECT

public:
    QObjectCallback(TwitchRewardsApi* parent, QObject* receiver, const char* member);

    template <typename Result>
    void operator()(const char* typeName, const Result& result) {
        qRegisterMetaType<Result>(typeName);
        {
            std::lock_guard guard(receiverMutex);
            if (receiver) {
                QMetaObject::invokeMethod(
                    receiver, member, Qt::ConnectionType::QueuedConnection, QArgument(typeName, result)
                );
            }
        }
        deleteLater();
    }

private slots:
    void clearReceiver();

private:
    QObject* receiver;
    std::mutex receiverMutex;
    const char* member;
};

}  // namespace detail
