#ifndef CLLM_FEED_FORWARD_H
#define CLLM_FEED_FORWARD_H

#include "config.h"

namespace cllm {

class FeedForward {
public:
    explicit FeedForward(const ModelConfig& config);

    // Placeholder for the forward pass
    void forward();

private:
    ModelConfig config_;
    // Parameters for the two linear layers would be here
};

} // namespace cllm

#endif // CLLM_FEED_FORWARD_H
