#include "errorresponsehandler.h"

// esp-idf includes
#include <asio.hpp>
#include <esp_log.h>

// 3rdparty lib includes
#include <fmt/core.h>
#include <asio_webserver/clientconnection.h>
#include <asio_webserver/webserver.h>

namespace {
constexpr const char * const TAG = "ASIO_WEBSERVER";
} // namespace

ErrorResponseHandler::ErrorResponseHandler(ClientConnection &clientConnection, std::string_view path) :
    m_clientConnection{clientConnection},
    m_path{path}
{
    ESP_LOGI(TAG, "constructed for %.*s (%s:%hi)", path.size(), path.data(),
             m_clientConnection.remote_endpoint().address().to_string().c_str(), m_clientConnection.remote_endpoint().port());
}

ErrorResponseHandler::~ErrorResponseHandler()
{
    ESP_LOGI(TAG, "destructed for %.*s (%s:%hi)", m_path.size(), m_path.data(),
             m_clientConnection.remote_endpoint().address().to_string().c_str(), m_clientConnection.remote_endpoint().port());
}

void ErrorResponseHandler::requestHeaderReceived(std::string_view key, std::string_view value)
{
}

void ErrorResponseHandler::requestBodyReceived(std::string_view body)
{
}

void ErrorResponseHandler::sendResponse()
{
    ESP_LOGI(TAG, "sending response for %.*s (%s:%hi)", m_path.size(), m_path.data(),
             m_clientConnection.remote_endpoint().address().to_string().c_str(), m_clientConnection.remote_endpoint().port());

    m_response = fmt::format("Error 404 Not Found: {}", m_path);

    m_response = fmt::format("HTTP/1.1 404 Not Found\r\n"
                             "Connection: {}\r\n"
                             "Content-Type: text/plain\r\n"
                             "Content-Length: {}\r\n"
                             "\r\n"
                             "{}",
                             m_clientConnection.webserver().connectionKeepAlive() ? "keep-alive" : "close",
                             m_response.size(), m_response);

    asio::async_write(m_clientConnection.socket(),
                      asio::buffer(m_response.data(), m_response.size()),
                      [this, self=m_clientConnection.shared_from_this()](std::error_code ec, std::size_t length)
                      { written(ec, length); });
}

void ErrorResponseHandler::written(std::error_code ec, std::size_t length)
{
    if (ec)
    {
        ESP_LOGW(TAG, "error: %i (%s:%hi)", ec.value(),
                 m_clientConnection.remote_endpoint().address().to_string().c_str(), m_clientConnection.remote_endpoint().port());
        m_clientConnection.responseFinished(ec);
        return;
    }

    ESP_LOGI(TAG, "expected=%zd actual=%zd for %.*s (%s:%hi)", m_response.size(), length, m_path.size(), m_path.data(),
             m_clientConnection.remote_endpoint().address().to_string().c_str(), m_clientConnection.remote_endpoint().port());

    m_clientConnection.responseFinished(ec);
}
