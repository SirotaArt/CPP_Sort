#pragma once

#include "lottery_ticket.h"
#include <string>
#include <vector>

namespace search
{

    class RBTree
    {
    public:
        RBTree()
        {
            nil_ = new Node{};
            nil_->color = Color::Black;
            nil_->left = nil_->right = nil_->parent = nil_;
            root_ = nil_;
        }

        RBTree(const RBTree&) = delete;
        RBTree& operator=(const RBTree&) = delete;

        ~RBTree() { destroy(root_); delete nil_; }

        void build(const std::vector<LotteryTicket>& data)
        {
            destroy(root_);
            root_ = nil_;
            for (const auto& t : data) insert(&t);
        }

        void insert(const LotteryTicket* p)
        {
            Node* parent = nil_;
            Node* cur = root_;
            while (cur != nil_) {
                parent = cur;
                if (p->lotteryDate == cur->key)
                {
                    cur->bucket.push_back(p);
                    return;
                }
                cur = (p->lotteryDate < cur->key) ? cur->left : cur->right;
            }

            Node* z = new Node{ p->lotteryDate, {p}, Color::Red, nil_, nil_, parent };
            if (parent == nil_)                     root_ = z;
            else if (z->key < parent->key)          parent->left = z;
            else                                    parent->right = z;
            fixInsert(z);
        }

        std::vector<const LotteryTicket*> findAll(const std::string& key) const
        {
            const Node* cur = root_;
            while (cur != nil_)
            {
                if (key == cur->key) return cur->bucket;
                cur = (key < cur->key) ? cur->left : cur->right;
            }

            return {};
        }

    private:
        enum class Color { Red, Black };

        struct Node
        {
            std::string                       key;
            std::vector<const LotteryTicket*> bucket;
            Color  color = Color::Black;
            Node* left = nullptr;
            Node* right = nullptr;
            Node* parent = nullptr;
        };

        Node* root_ = nullptr;
        Node* nil_ = nullptr;

        void destroy(Node* n)
        {
            if (!n || n == nil_) return;
            destroy(n->left); destroy(n->right); delete n;
        }

        void rotateLeft(Node* x)
        {
            Node* y = x->right;
            x->right = y->left;
            if (y->left != nil_) y->left->parent = x;
            y->parent = x->parent;
            if (x->parent == nil_)       root_ = y;
            else if (x == x->parent->left)    x->parent->left = y;
            else                              x->parent->right = y;
            y->left = x; x->parent = y;
        }

        void rotateRight(Node* x)
        {
            Node* y = x->left;
            x->left = y->right;
            if (y->right != nil_) y->right->parent = x;
            y->parent = x->parent;
            if (x->parent == nil_)       root_ = y;
            else if (x == x->parent->right)   x->parent->right = y;
            else                              x->parent->left = y;
            y->right = x; x->parent = y;
        }

        void fixInsert(Node* z)
        {
            while (z->parent->color == Color::Red)
            {
                if (z->parent == z->parent->parent->left)
                {
                    Node* uncle = z->parent->parent->right;
                    if (uncle->color == Color::Red)
                    {

                        z->parent->color = Color::Black;
                        uncle->color = Color::Black;
                        z->parent->parent->color = Color::Red;
                        z = z->parent->parent;
                    }
                    else
                    {
                        if (z == z->parent->right)
                        {
                            z = z->parent; rotateLeft(z);
                        }
                        z->parent->color = Color::Black;
                        z->parent->parent->color = Color::Red;
                        rotateRight(z->parent->parent);
                    }
                }
                else {

                    Node* uncle = z->parent->parent->left;
                    if (uncle->color == Color::Red)
                    {
                        z->parent->color = Color::Black;
                        uncle->color = Color::Black;
                        z->parent->parent->color = Color::Red;
                        z = z->parent->parent;
                    }
                    else {
                        if (z == z->parent->left)
                        {
                            z = z->parent; rotateRight(z);
                        }
                        z->parent->color = Color::Black;
                        z->parent->parent->color = Color::Red;
                        rotateLeft(z->parent->parent);
                    }
                }
            }
            root_->color = Color::Black;
        }
    };

}
