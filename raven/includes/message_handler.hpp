#pragma once
#include <serialization/messages.hpp>
#include <subscription_manager.hpp>

namespace rvn
{
class MessageHandler
{
    class StreamState& streamState_;
    class SubscriptionManager& subscriptionManager_;

public:
    MessageHandler(StreamState& streamState, SubscriptionManager& subscriptionManager)
    : streamState_(streamState), subscriptionManager_(subscriptionManager) {};

    void operator()(ClientSetupMessage clientSetupMessage);
    void operator()(ServerSetupMessage serverSetupMessage);
    void operator()(SubscribeMessage subscribeMessage);
    void operator()(const auto& unknownMessage);
};
} // namespace rvn
