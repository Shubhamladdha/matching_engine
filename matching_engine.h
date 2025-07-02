#pragma once
#include <unordered_map>
#include <queue>
#include <map>
#include <vector>
#include <mutex>
#include "order.h"
#include "safemsgqueue.h"

class MatchingEngine {
private:
    std::mutex mtx;
    std::unordered_map<int, Order> orderMap;
    std::priority_queue<std::pair<int, int>> buyOrders; // max-heap
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> sellOrders; // min-heap
    std::map<int, SafeMsgQueue*> clientMsgQueues; // clientId -> queue pointer
    void cleanHeap(std::priority_queue<std::pair<int, int>>& heap);
    void cleanHeap(std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>>& heap);
    void printOrderBook();
public:
    void registerClient(int clientId, SafeMsgQueue* q);
    void placeOrder(int clientId, const Order& orderIn);
    void cancelOrder(int clientId, int orderId);
    void matchOrders();
};
