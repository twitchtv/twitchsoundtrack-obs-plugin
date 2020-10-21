
// Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <atomic>
#include <vector>

namespace Twitch::Utility {
template<typename ElementType>
class LockFreeList {
public:
    LockFreeList()
        : _head(nullptr)
    {
    }

    template<typename... ArgTypes>
    void push(ArgTypes &&... data)
    {
        Node *newNode = new Node(std::forward<ArgTypes>(data)...);
        newNode->next = _head.load(std::memory_order_relaxed);

        // Move the head to the new node
        while(!std::atomic_compare_exchange_weak_explicit(&_head,
            &newNode->next,
            newNode,
            std::memory_order_release,
            std::memory_order_relaxed)) {
        }
    }

    // Return false if no data, else return true
    template<typename ConsumerType>
    bool consumeData(ConsumerType &&consumer)
    {
        auto node = _head.exchange(nullptr, std::memory_order_consume);

        if(node == nullptr) {
            return false;
        }

        std::vector<ElementType> data;

        while(node != nullptr) {
            auto temp = node;
            data.emplace_back(std::move(node->data));
            node = node->next;
            delete temp;
        }

        for(auto iter = data.rbegin(); iter != data.rend(); ++iter) {
            consumer(std::move(*iter));
        }

        return true;
    }

private:
    struct Node {
        template<typename... ArgTypes>
        Node(ArgTypes &&... data)
            : data(std::forward<ArgTypes>(data)...)
        {
        }

        ElementType data;
        Node *next = nullptr;
    };

    std::atomic<Node *> _head;
};
} // namespace Twitch::Utility
