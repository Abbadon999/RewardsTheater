// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2023, Lev Leontev

#include "HttpUtil.h"

#include "BoostAsio.h"

namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace http = boost::beast::http;

namespace HttpUtil {

InternalServerErrorException::InternalServerErrorException(const std::string& message) : message(message) {}

const char* InternalServerErrorException::what() const noexcept {
    return message.c_str();
}

static asio::awaitable<ssl::stream<asio::ip::tcp::socket>> resolveHost(
    asio::io_context& ioContext,
    const std::string& host
);

static asio::awaitable<http::response<http::dynamic_body>> getResponse(
    const http::request<http::empty_body>& request,
    ssl::stream<asio::ip::tcp::socket>& stream
);

asio::awaitable<Response> request(
    const std::string& accessToken,
    const std::string& clientId,
    asio::io_context& ioContext,
    const std::string& host,
    const std::string& path,
    std::initializer_list<boost::urls::param_view> urlParams
) {
    ssl::stream<asio::ip::tcp::socket> stream = co_await resolveHost(ioContext, host);
    boost::urls::url pathWithParams = boost::urls::parse_origin_form(path).value();
    pathWithParams.set_params(urlParams);
    http::request<http::empty_body> request{http::verb::get, pathWithParams.buffer(), 11};
    request.set(http::field::host, host);
    request.set(http::field::authorization, "Bearer " + accessToken);
    request.set("Client-Id", clientId);

    http::response<http::dynamic_body> response = co_await getResponse(request, stream);
    std::string body = boost::beast::buffers_to_string(response.body().data());
    if (response.result() == http::status::internal_server_error) {
        throw InternalServerErrorException(body);
    }
    co_return Response{response.result(), boost::json::parse(body)};
}

asio::awaitable<Response> request(
    TwitchAuth& auth,
    asio::io_context& ioContext,
    const std::string& host,
    const std::string& path,
    std::initializer_list<boost::urls::param_view> urlParams
) {
    Response response =
        co_await request(auth.getAccessTokenOrThrow(), auth.getClientId(), ioContext, host, path, urlParams);
    if (response.status == http::status::unauthorized) {
        auth.logOutAndEmitAuthenticationFailure();
        throw TwitchAuth::UnauthenticatedException();
    }
    co_return response;
}

asio::awaitable<std::string> downloadFile(
    asio::io_context& ioContext,
    const std::string& host,
    const std::string& path
) {
    ssl::stream<asio::ip::tcp::socket> stream = co_await resolveHost(ioContext, host);
    http::request<http::empty_body> request{http::verb::get, path, 11};
    request.set(http::field::host, host);

    http::response<http::dynamic_body> response = co_await getResponse(request, stream);
    if (response.result() != http::status::ok) {
        throw TwitchAuth::UnauthenticatedException();
    }
    co_return boost::beast::buffers_to_string(response.body().data());
}

asio::awaitable<ssl::stream<asio::ip::tcp::socket>> resolveHost(asio::io_context& ioContext, const std::string& host) {
    ssl::context sslContext{ssl::context::tlsv12};
    sslContext.set_default_verify_paths();
    asio::ip::tcp::resolver resolver{ioContext};
    ssl::stream<asio::ip::tcp::socket> stream{ioContext, sslContext};

    const auto resolveResults = co_await resolver.async_resolve(host, "https", asio::use_awaitable);
    co_await asio::async_connect(
        stream.next_layer(), resolveResults.begin(), resolveResults.end(), asio::use_awaitable
    );
    co_await stream.async_handshake(ssl::stream_base::client, asio::use_awaitable);
    co_return stream;
}

asio::awaitable<http::response<http::dynamic_body>> getResponse(
    const http::request<http::empty_body>& request,
    ssl::stream<asio::ip::tcp::socket>& stream
) {
    co_await http::async_write(stream, request, asio::use_awaitable);
    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> response;
    co_await http::async_read(stream, buffer, response, asio::use_awaitable);
    co_return response;
}

}  // namespace HttpUtil
