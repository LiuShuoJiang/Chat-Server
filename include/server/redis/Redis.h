#ifndef CHAT_PROJECT_REDIS_H
#define CHAT_PROJECT_REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
#include <string>

class Redis {
public:
    Redis();

    ~Redis();

    // Connect to redis server
    bool connect();

    // Publish messages to the channel specified by redis
    bool publish(int channel, std::string message);

    // Subscribe messages to the channel specified by redis
    bool subscribe(int channel);

    // Unsubscribe messages to the channel specified by redis
    bool unsubscribe(int channel);

    // Receive messages in subscription channels in a separate thread
    void observerChannelMessage();

    // Initialize the callback object that reports channel messages to the service layer
    void initNotifyHandler(std::function<void(int, std::string)> fn);

private:
    // hiredis synchronization context object: responsible for publish messages
    redisContext *_publishContext;

    // hiredis synchronization context object: responsible for subscribe messages
    redisContext *_subscribeContext;

    // Callback operation: receive the subscribed message, report to the service layer
    std::function<void(int, std::string)> _notifyMessageHandler;
};

#endif//CHAT_PROJECT_REDIS_H
