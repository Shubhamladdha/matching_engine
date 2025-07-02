#include "matching_engine.h"
#include <algorithm>
#include <iostream>
#include <thread>

void MatchingEngine::cleanHeap(std::priority_queue<std::pair<int, int>>& heap) {
    while (!heap.empty() && !orderMap.count(heap.top().second)) {
        heap.pop();
    }
}
void MatchingEngine::cleanHeap(std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>>& heap) {
    while (!heap.empty() && !orderMap.count(heap.top().second)) {
        heap.pop();
    }
}

void MatchingEngine::printOrderBook() {
    // Assumes mtx is already locked by caller
    std::vector<const Order*> buys, sells;
    buys.reserve(orderMap.size());
    sells.reserve(orderMap.size());
    for (const auto& [id, order] : orderMap) {
        if (order.side == BUY)
            buys.push_back(&order);
        else
            sells.push_back(&order);
    }
    std::sort(buys.begin(), buys.end(), [](const Order* a, const Order* b) { return a->price > b->price; });
    std::sort(sells.begin(), sells.end(), [](const Order* a, const Order* b) { return a->price < b->price; });
    std::cout << "Order Book Snapshot (thread " << std::this_thread::get_id() << "):\n";
    std::cout << "Buy Orders:\n";
    for (const auto* order : buys)
        std::cout << "  ID: " << order->orderId << ", Price: " << order->price << ", Amount: " << order->amount << ", Client: " << order->clientId << "\n";
    std::cout << "Sell Orders:\n";
    for (const auto* order : sells)
        std::cout << "  ID: " << order->orderId << ", Price: " << order->price << ", Amount: " << order->amount << ", Client: " << order->clientId << "\n";
    std::cout << std::endl;
}

void MatchingEngine::registerClient(int clientId, SafeMsgQueue* q) {
    std::lock_guard<std::mutex> lock(mtx);
    clientMsgQueues[clientId] = q;
}

void MatchingEngine::placeOrder(int clientId, const Order& orderIn) {
    std::lock_guard<std::mutex> lock(mtx);
    if (orderMap.count(orderIn.orderId)) {
        auto msg = std::make_shared<OrderCanceled>(OrderCanceled{orderIn.orderId, 1});
        clientMsgQueues[clientId]->push(msg);
        return;
    }
    Order order = orderIn;
    order.clientId = clientId;
    orderMap[order.orderId] = order;
    if (order.side == BUY)
        buyOrders.emplace(order.price, order.orderId);
    else
        sellOrders.emplace(order.price, order.orderId);
    auto msg = std::make_shared<OrderPlaced>(OrderPlaced{order.orderId, order.price, order.amount});
    clientMsgQueues[clientId]->push(msg);
    matchOrders();
}

void MatchingEngine::cancelOrder(int clientId, int orderId) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!orderMap.count(orderId)) {
        auto msg = std::make_shared<OrderCanceled>(OrderCanceled{orderId, 1});
        clientMsgQueues[clientId]->push(msg);
        return;
    }
    orderMap.erase(orderId);
    auto msg = std::make_shared<OrderCanceled>(OrderCanceled{orderId, 0});
    clientMsgQueues[clientId]->push(msg);
}

void MatchingEngine::matchOrders() {
    printOrderBook();
    while (true) {
        cleanHeap(buyOrders);
        cleanHeap(sellOrders);
        if (buyOrders.empty() || sellOrders.empty()) break;
        auto [buyPrice, buyId] = buyOrders.top();
        auto [sellPrice, sellId] = sellOrders.top();
        if (buyPrice < sellPrice) break;
        Order& buyOrder = orderMap[buyId];
        Order& sellOrder = orderMap[sellId];
        int tradedAmount = std::min(buyOrder.amount, sellOrder.amount);
        int tradedPrice = sellPrice;
        if (clientMsgQueues.count(buyOrder.clientId)) {
            auto msg = std::make_shared<OrderTraded>(OrderTraded{buyOrder.orderId, tradedPrice, tradedAmount});
            clientMsgQueues[buyOrder.clientId]->push(msg);
        }
        if (clientMsgQueues.count(sellOrder.clientId)) {
            auto msg = std::make_shared<OrderTraded>(OrderTraded{sellOrder.orderId, tradedPrice, tradedAmount});
            clientMsgQueues[sellOrder.clientId]->push(msg);
        }
        buyOrder.amount -= tradedAmount;
        sellOrder.amount -= tradedAmount;
        if (buyOrder.amount == 0) {
            buyOrders.pop();
            orderMap.erase(buyId);
        }
        if (sellOrder.amount == 0) {
            sellOrders.pop();
            orderMap.erase(sellId);
        }
    }
}
