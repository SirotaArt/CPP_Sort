#include "prng.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using clk = std::chrono::high_resolution_clock;
using sec = std::chrono::duration<double>;

constexpr int N_SAMPLES   = 20;
constexpr int SAMPLE_SIZE = 1000;
constexpr int RANGE       = 5000;

static void writeSampleTxt(const std::string& path,
                           const std::vector<std::uint32_t>& v) {
    std::ofstream out(path);
    for (auto x : v) out << x << '\n';
}

static void writeBits(const std::string& path, prng::IGenerator& g,
                      std::size_t bytes) {
    std::ofstream out(path, std::ios::binary);
    constexpr std::size_t kBatch = 4096;
    std::vector<std::uint32_t> buf(kBatch);
    std::size_t produced = 0;
    while (produced < bytes) {
        for (auto& w : buf) w = g.next();
        std::size_t take = std::min(kBatch * 4, bytes - produced);
        out.write(reinterpret_cast<const char*>(buf.data()),
                  static_cast<std::streamsize>(take));
        produced += take;
    }
}

static int runSamples(const std::string& outDir) {
    fs::create_directories(outDir);

    auto makeGen = [](int idx) -> std::unique_ptr<prng::IGenerator> {
        if (idx == 0) return std::make_unique<prng::LcgXor>(0xC0FFEEull);
        if (idx == 1) return std::make_unique<prng::MidSquareImproved>(0xBEEFu);
        return                  std::make_unique<prng::XorShift32StarPlus>(0xCAFEu);
    };

    for (int g = 0; g < 3; ++g) {
        auto gen = makeGen(g);
        const std::string name = gen->name();
        const std::string dir  = outDir + "/" + name;
        fs::create_directories(dir);
        std::cout << "[samples] " << name << " -> " << dir << "\n";

        for (int i = 0; i < N_SAMPLES; ++i) {
            std::vector<std::uint32_t> sample(SAMPLE_SIZE);
            for (auto& x : sample) x = gen->next() % RANGE;
            char buf[64];
            std::snprintf(buf, sizeof(buf), "/sample_%02d.txt", i);
            writeSampleTxt(dir + buf, sample);
        }

        auto bitsGen = makeGen(g);
        writeBits(outDir + "/bits_" + name + ".bin", *bitsGen, 1024 * 1024);
        std::cout << "  bits saved (1 MiB)\n";
    }
    return 0;
}

template <class Gen>
static double benchGen(Gen& g, std::size_t n) {
    std::uint64_t sink = 0;
    auto t0 = clk::now();
    for (std::size_t i = 0; i < n; ++i) sink += g();
    auto t1 = clk::now();
    if (sink == 0xDEADBEEFull) std::cout << ""; 
    return sec(t1 - t0).count();
}

static int runBench(const std::string& outCsv) {
    const std::vector<std::size_t> sizes = {1000, 5000, 10000, 50000, 100000,
                                            500000, 1000000};
    FILE* fp = std::fopen(outCsv.c_str(), "w");
    if (!fp) { std::perror("fopen"); return 3; }
    std::fprintf(fp, "size,generator,time_sec\n");

    for (auto n : sizes) {
        std::cout << "[bench] n=" << n << "\n";

        prng::LcgXor              lcg;
        prng::MidSquareImproved   mid;
        prng::XorShift32StarPlus  xs;
        std::mt19937              mt(12345u);

        auto lcgFn = [&]{ return lcg.next(); };
        auto midFn = [&]{ return mid.next(); };
        auto xsFn  = [&]{ return xs.next();  };
        auto mtFn  = [&]{ return mt();       };

        double tLcg = benchGen(lcgFn, n);
        double tMid = benchGen(midFn, n);
        double tXs  = benchGen(xsFn,  n);
        double tMt  = benchGen(mtFn,  n);

        std::fprintf(fp, "%zu,LCG_Xor,%.9f\n",             n, tLcg);
        std::fprintf(fp, "%zu,MidSquare_Improved,%.9f\n",  n, tMid);
        std::fprintf(fp, "%zu,XorShift32_StarPlus,%.9f\n", n, tXs);
        std::fprintf(fp, "%zu,std_mt19937,%.9f\n",         n, tMt);
        std::cout << "  LCG=" << tLcg << "  MS=" << tMid
                  << "  XS=" << tXs  << "  MT=" << tMt << "  s\n";
        std::fflush(fp);
    }
    std::fclose(fp);
    return 0;
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "usage:\n  lr3 samples <out_dir>\n  lr3 bench <out_csv>\n";
            return 1;
        }
        const std::string cmd = argv[1];
        if (cmd == "samples") return runSamples(argc > 2 ? argv[2] : "results/samples");
        if (cmd == "bench")   return runBench  (argc > 2 ? argv[2] : "results/bench.csv");
        std::cerr << "unknown command: " << cmd << "\n";
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 10;
    }
}
