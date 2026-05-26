#pragma once

#include "lottery_ticket.h"
#include <vector>
#include <string>

namespace search
{

    class BST
    {
    public:
        BST() = default;
        BST(const BST&) = delete;
        BST& operator=(const BST&) = delete;
        ~BST() { destroy(root_); }

        void build(const std::vector<LotteryTicket>& data)
        {
            destroy(root_); root_ = nullptr;
            for (const auto& t : data) insert(&t);
        }

        void insert(const LotteryTicket* p)
        {
            Node** cur = &root_;
            while (*cur) {
                if (p->lotteryDate == (*cur)->key)
                {
                    (*cur)->bucket.push_back(p);
                    return;
                }
                cur = (p->lotteryDate < (*cur)->key) ? &(*cur)->left : &(*cur)->right;
            }
            *cur = new Node{ p->lotteryDate, {p}, nullptr, nullptr };
        }

        std::vector<const LotteryTicket*> findAll(const std::string& key) const
        {
            const Node* cur = root_;
            while (cur)
            {
                if (key == cur->key) return cur->bucket;
                cur = (key < cur->key) ? cur->left : cur->right;
            }
            return {};
        }

    private:
        struct Node
        {
            std::string                       key;
            std::vector<const LotteryTicket*> bucket;
            Node* left;
            Node* right;
        };

        static void destroy(Node* n)
        {
            if (!n) return;
            destroy(n->left); destroy(n->right); delete n;
        }

        Node* root_ = nullptr;
    };
}
