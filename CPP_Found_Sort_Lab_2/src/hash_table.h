#pragma once

#include "lottery_ticket.h"
#include <cstdint>
#include <string>
#include <vector>

namespace search
{

    class HashTable
    {
    public:
        explicit HashTable(std::size_t initialBuckets = 16)
        {
            buckets_.resize(roundPow2(initialBuckets));
            mask_ = buckets_.size() - 1;
        }

        void build(const std::vector<LotteryTicket>& data)
        {
            buckets_.assign(buckets_.size(), {});
            size_ = 0;
            for (const auto& t : data) insert(&t);
        }

        void insert(const LotteryTicket* p)
        {
            if (size_ * 4 >= buckets_.size() * 3) rehash(buckets_.size() * 2);

            std::uint64_t h = fnv1a(p->lotteryDate);
            std::size_t idx = static_cast<std::size_t>(h) & mask_;
            for (auto& e : buckets_[idx])
            {
                if (e.key == p->lotteryDate)
                {
                    e.bucket.push_back(p);
                    return;
                }
            }

            buckets_[idx].push_back(Entry{ p->lotteryDate, {p} });
            ++size_;
        }

        std::vector<const LotteryTicket*> findAll(const std::string& key) const
        {
            std::uint64_t h = fnv1a(key);
            std::size_t idx = static_cast<std::size_t>(h) & mask_;
            for (const auto& e : buckets_[idx])
                if (e.key == key) return e.bucket;
            return {};
        }

        std::size_t uniqueKeys() const { return size_; }
        std::size_t bucketCount() const { return buckets_.size(); }

        std::size_t collisionCount() const
        {
            std::size_t collisions = 0;
            for (const auto& bucket : buckets_)
                if (bucket.size() > 1) collisions += bucket.size() - 1;
            return collisions;
        }

        std::size_t maxChainLength() const
        {
            std::size_t best = 0;
            for (const auto& bucket : buckets_)
                if (bucket.size() > best) best = bucket.size();
            return best;
        }

        std::size_t nonEmptyBuckets() const
        {
            std::size_t count = 0;
            for (const auto& bucket : buckets_)
                if (!bucket.empty()) ++count;
            return count;
        }

    private:
        struct Entry
        {
            std::string                       key;
            std::vector<const LotteryTicket*> bucket;
        };

        static std::size_t roundPow2(std::size_t v)
        {
            std::size_t p = 1; while (p < v) p <<= 1; return p;
        }


        static std::uint64_t fnv1a(const std::string& s)
        {
            std::uint64_t h = 1469598103934665603ull;
            for (unsigned char c : s) {
                h ^= c;
                h *= 1099511628211ull;
            }
            return h;
        }

        void rehash(std::size_t newCount)
        {
            std::vector<std::vector<Entry>> old;
            old.swap(buckets_);
            buckets_.assign(newCount, {});
            mask_ = newCount - 1;
            for (auto& bucket : old)
                for (auto& e : bucket)
                {
                    std::uint64_t h = fnv1a(e.key);
                    std::size_t idx = static_cast<std::size_t>(h) & mask_;
                    buckets_[idx].push_back(std::move(e));
                }
        }

        std::vector<std::vector<Entry>> buckets_;
        std::size_t                     mask_ = 0;
        std::size_t                     size_ = 0;
    };
}
