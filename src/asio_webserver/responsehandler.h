#pragma once

#include <string_view>

class ResponseHandler
{
public:
    virtual ~ResponseHandler() = default;

    virtual void requestHeaderReceived(std::string_view key, std::string_view value) = 0;
    virtual void sendResponse() = 0;
};
