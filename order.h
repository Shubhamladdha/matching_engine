#pragma once
#include <iostream>

enum Side { BUY, SELL };

class Order {
public:
    int orderId;
    int price;
    int amount;
    Side side;
    int clientId; // Track owner
};

class OrderMessage {
public:
    virtual ~OrderMessage() = default;
    virtual void print(int clientId) const = 0;
};

class OrderPlaced : public OrderMessage {
public:
    int orderId, price, amount;
    OrderPlaced(int orderId_, int price_, int amount_)
        : orderId(orderId_), price(price_), amount(amount_) {}
    void print(int clientId) const override {
        std::cout << "Client " << clientId << ": OrderPlaced {id=" << orderId << ", price=" << price << ", amount=" << amount << "}\n";
    }
};

class OrderCanceled : public OrderMessage {
public:
    int orderId;
    int reasonCode; // 0 = success, 1 = not found
    OrderCanceled(int orderId_, int reasonCode_) : orderId(orderId_), reasonCode(reasonCode_) {}
    void print(int clientId) const override {
        std::cout << "Client " << clientId << ": OrderCanceled {id=" << orderId << ", reasonCode=" << reasonCode << "}\n";
    }
};

class OrderTraded : public OrderMessage {
public:
    int orderId, tradedPrice, tradedAmount;
    OrderTraded(int orderId_, int tradedPrice_, int tradedAmount_)
        : orderId(orderId_), tradedPrice(tradedPrice_), tradedAmount(tradedAmount_) {}
    void print(int clientId) const override {
        std::cout << "Client " << clientId << ": OrderTraded {id=" << orderId << ", price=" << tradedPrice << ", amount=" << tradedAmount << "}\n";
    }
};
