#include "lottery_ticket.h"
#include "io.h"
#include "linear_search.h"
#include "bst.h"
#include "rb_tree.h"
#include "hash_table.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

using clk = std::chrono::high_resolution_clock;
using sec = std::chrono::duration<double>;

static volatile std::size_t gBenchmarkSink = 0;

static std::vector<std::string> sampleKeys(const std::vector<LotteryTicket>& data,std::size_t n, std::uint64_t seed) 
{
    std::set<std::string> uniq;
    for (const auto& t : data) uniq.insert(t.lotteryDate);
    std::vector<std::string> all(uniq.begin(), uniq.end());

    std::mt19937_64 rng(seed);
    std::shuffle(all.begin(), all.end(), rng);
    if (all.size() > n) all.resize(n);
    return all;
}

static bool sameResults(std::vector<const LotteryTicket*> a, std::vector<const LotteryTicket*> b) 
{
    if (a.size() != b.size()) return false;
    auto cmp = [](const LotteryTicket* x, const LotteryTicket* y){ return x < y; };
    std::sort(a.begin(), a.end(), cmp);
    std::sort(b.begin(), b.end(), cmp);
    return a == b;
}

using MultiMapIndex = std::multimap<std::string, LotteryTicket>;

static void buildMultiMap(const std::vector<LotteryTicket>& data, MultiMapIndex& index) 
{
    index.clear();
    for (const auto& t : data)
        index.emplace(t.lotteryDate, t);
}

static std::size_t multimapCount(const MultiMapIndex& index, const std::string& key) 
{
    auto range = index.equal_range(key);
    return static_cast<std::size_t>(std::distance(range.first, range.second));
}


static int runDemo() 
{
    const std::string inPath = "data/lottery_100000.csv";
    std::cout << "[LR2] reading " << inPath << " ...\n";
    auto data = io::readCsv(inPath);
    std::cout << "[LR2] loaded " << data.size() << " tickets\n";

    auto t0 = clk::now();
    search::BST       bst;     bst.build(data);
    auto t1 = clk::now();
    search::RBTree    rb;      rb.build(data);
    auto t2 = clk::now();
    search::HashTable ht;      ht.build(data);
    auto t3 = clk::now();
    MultiMapIndex mm;          buildMultiMap(data, mm);
    auto t4 = clk::now();

    std::cout << "  build BST       : " << sec(t1 - t0).count() << " s\n";
    std::cout << "  build RB-tree   : " << sec(t2 - t1).count() << " s\n";
    std::cout << "  build HashTable : " << sec(t3 - t2).count() << " s\n";
    std::cout << "  build multimap  : " << sec(t4 - t3).count() << " s\n";
    std::cout << "  hash collisions : " << ht.collisionCount()
              << " (buckets=" << ht.bucketCount()
              << ", max_chain=" << ht.maxChainLength() << ")\n";

    auto keys = sampleKeys(data, 5, 1);

    for (const auto& key : keys) 
    {
        std::cout << "\n--- key = " << key << " ---\n";
        auto rLin  = search::linearFindAll(data, key);
        auto rBst  = bst.findAll(key);
        auto rRb   = rb.findAll(key);
        auto rHash = ht.findAll(key);
        auto rMap  = multimapCount(mm, key);

        std::cout << "  linear : " << rLin.size()  << " matches\n";
        std::cout << "  BST    : " << rBst.size()  << " matches\n";
        std::cout << "  RB     : " << rRb.size()   << " matches\n";
        std::cout << "  hash   : " << rHash.size() << " matches\n";
        std::cout << "  mmap   : " << rMap         << " matches\n";

        bool ok = sameResults(rLin, rBst)
               && sameResults(rLin, rRb)
               && sameResults(rLin, rHash)
               && (rLin.size() == rMap);
        std::cout << "  consistency: " << (ok ? "OK" : "FAIL") << "\n";

        std::string safeKey = key; for (auto& c : safeKey) if (c == ':' || c == '/') c = '_';
        io::writeFound("results/found_" + safeKey + ".csv",
                       "linear", key, rLin);
    }

    return 0;
}

static int runBench(int argc, char** argv) 
{
    if (argc < 4) 
    {
        std::cerr << "usage: bench out.csv in1.csv [in2.csv ...]\n";
        return 2;
    }

    const std::string outPath = argv[2];

    FILE* fp = std::fopen(outPath.c_str(), "w");
    if (!fp) { std::perror("fopen"); return 3; }
    std::fprintf(fp,
        "size,n_queries,build_bst,build_rb,build_hash,build_multimap,"
        "hash_buckets,hash_non_empty,hash_collisions,hash_max_chain,"
        "search_linear_avg,search_bst_avg,search_rb_avg,search_hash_avg,"
        "search_multimap_avg\n");

    constexpr std::size_t kQueries = 100;

    for (int i = 3; i < argc; ++i) 
    {
        const std::string in = argv[i];
        std::cout << "[bench] " << in << " ...\n";
        auto data = io::readCsv(in);
        const std::size_t n = data.size();

        auto t0 = clk::now();
        search::BST bst;        bst.build(data);
        auto t1 = clk::now();
        search::RBTree rb;      rb.build(data);
        auto t2 = clk::now();
        search::HashTable ht;   ht.build(data);
        auto t3 = clk::now();
        MultiMapIndex mm;       buildMultiMap(data, mm);
        auto t4 = clk::now();

        const double tBuildBst  = sec(t1 - t0).count();
        const double tBuildRb   = sec(t2 - t1).count();
        const double tBuildHash = sec(t3 - t2).count();
        const double tBuildMap  = sec(t4 - t3).count();

        auto hits = sampleKeys(data, kQueries / 2, 42 + n);
        std::vector<std::string> queries = hits;

        for (std::size_t k = 0; queries.size() < kQueries; ++k) 
        {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "1900-01-%02zu", (k % 28) + 1);
            queries.emplace_back(buf);
        }

        auto runQueries = [&](auto fn) 
        {
            auto a = clk::now();
            std::size_t total = 0;
            for (const auto& q : queries) total += fn(q).size();
            auto b = clk::now();
            gBenchmarkSink = total;
            return sec(b - a).count() / queries.size();
        };

        double tLin  = runQueries([&](const std::string& q){ return search::linearFindAll(data, q); });
        double tBst  = runQueries([&](const std::string& q){ return bst.findAll(q); });
        double tRb   = runQueries([&](const std::string& q){ return rb.findAll(q); });
        double tHash = runQueries([&](const std::string& q){ return ht.findAll(q); });
        auto runCountQueries = [&](auto fn) 
        {
            auto a = clk::now();
            std::size_t total = 0;
            for (const auto& q : queries) total += fn(q);
            auto b = clk::now();
            gBenchmarkSink = total;
            return sec(b - a).count() / queries.size();
        };

        double tMap = runCountQueries([&](const std::string& q){ return multimapCount(mm, q); });

        std::cout << "  build  (B/R/H/M) = " << tBuildBst << " / " << tBuildRb
                  << " / " << tBuildHash << " / " << tBuildMap << " s\n";
        std::cout << "  hash collisions = " << ht.collisionCount()
                  << "  max_chain=" << ht.maxChainLength()
                  << "  buckets=" << ht.bucketCount() << "\n";
        std::cout << "  search avg       = " << tLin << " / " << tBst << " / "
                  << tRb << " / " << tHash << " / " << tMap << " s\n";

        std::fprintf(fp,
            "%zu,%zu,%.9f,%.9f,%.9f,%.9f,%zu,%zu,%zu,%zu,"
            "%.9f,%.9f,%.9f,%.9f,%.9f\n",
            n, queries.size(), tBuildBst, tBuildRb, tBuildHash, tBuildMap,
            ht.bucketCount(), ht.nonEmptyBuckets(), ht.collisionCount(), ht.maxChainLength(),
            tLin, tBst, tRb, tHash, tMap);
        std::fflush(fp);
    }
    std::fclose(fp);
    return 0;
}

int main(int argc, char** argv) 
{
    try 
    {
        if (argc >= 2 && std::string(argv[1]) == "bench") return runBench(argc, argv);
        return runDemo();
    } catch (const std::exception& e) 
    {
        std::cerr << "error: " << e.what() << "\n";
        return 10;
    }
}
