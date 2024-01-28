#include "Redis.h"
#include <iostream>


Redis::Redis() : _publishContext(nullptr), _subscribeContext(nullptr) {}

Redis::~Redis() {
    if (_publishContext != nullptr) {
        redisFree(_publishContext);
    }

    if (_subscribeContext != nullptr) {
        redisFree(_subscribeContext);
    }
}

bool Redis::connect() {
    // Contextual connection responsible for publishing messages
    _publishContext = redisConnect("127.0.0.1", 6379);
    if (_publishContext == nullptr) {
        std::cerr << "Connect to Redis failed!" << std::endl;
        return false;
    }

    // Contextual connection responsible for subscribing to messages
    _subscribeContext = redisConnect("127.0.0.1", 6379);
    if (_subscribeContext == nullptr) {
        std::cerr << "Connect to Redis failed!" << std::endl;
        return false;
    }

    // In a separate thread, listen for events on the channel
    // If there is a message it is reported to the service layer
    std::thread t([&]() {
        observerChannelMessage();
    });
    t.detach();

    std::cout << "Connect to Redis server successful!" << std::endl;

    return true;
}

bool Redis::publish(int channel, std::string message) {
    auto *reply = (redisReply *) redisCommand(_publishContext, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply) {
        std::cerr << "publish command failed!" << std::endl;
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel) {
    if (redisAppendCommand(this->_subscribeContext, "SUBSCRIBE %d", channel) == REDIS_ERR) {
        std::cerr << "Subscribe command failed!" << std::endl;
        return false;
    }

    int done = 0;
    while (!done) {
        if (redisBufferWrite(this->_subscribeContext, &done) == REDIS_ERR) {
            std::cerr << "Subscribe command failed!" << std::endl;
            return false;
        }
    }

    return true;
}

bool Redis::unsubscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(this->_subscribeContext, "UNSUBSCRIBE %d", channel)) {
        std::cerr << "Unsubscribe command failed!" << std::endl;
        return false;
    }

    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subscribeContext, &done)) {
            std::cerr << "Unsubscribe command failed!" << std::endl;
            return false;
        }
    }

    return true;
}

void Redis::observerChannelMessage() {
    redisReply *reply = nullptr;
    while (redisGetReply(this->_subscribeContext, (void **) &reply) == REDIS_OK) {
        // The message received by the subscription is an array with three elements
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            // Report messages occurring on the channel to the service layer
            _notifyMessageHandler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }

    std::cerr << ">>>>>>>>>>>>> Observer Channel Message Quit <<<<<<<<<<<<<" << std::endl;
}

void Redis::initNotifyHandler(std::function<void(int, std::string)> fn) {
    this->_notifyMessageHandler = fn;
}
