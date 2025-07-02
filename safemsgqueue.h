#pragma once
#include <queue>
#include <memory>
#include <mutex>
#include "order.h"

class SafeMsgQueue {
    std::queue<std::shared_ptr<OrderMessage>> q;
    std::mutex m;
public:
    void push(std::shared_ptr<OrderMessage> msg) {
        std::lock_guard<std::mutex> lock(m);
        q.push(msg);
    }
    bool pop_print(int clientId) {
        std::lock_guard<std::mutex> lock(m);
        if (q.empty()) return false;
        q.front()->print(clientId);
        q.pop();
        return true;
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(m);
        return q.empty();
    }
};
