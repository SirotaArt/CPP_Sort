#include "lottery_ticket.h"
#include "io.h"
#include "sortings.h"
#include "data_gen.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using clk = std::chrono::high_resolution_clock;

template <class Sorter>

static double timeOnce(std::vector<LotteryTicket>& a, Sorter sortFn) 
{
    auto t0 = clk::now();
    sortFn(a);
    auto t1 = clk::now();
    return std::chrono::duration<double>(t1 - t0).count();
}

static void sortByStd(std::vector<LotteryTicket>& v) 
{
    std::sort(v.begin(), v.end());
}

static int runDefault() 
{
    const std::string inPath  = "data/lottery_100000.csv";
    const std::string outDir  = "results/";
    constexpr std::size_t kDefaultSize = 100000;

    std::filesystem::create_directories("data");
    std::filesystem::create_directories(outDir);

    std::vector<LotteryTicket> src;

    if (std::ifstream(inPath).good()) 
    {
        std::cout << "[LR1] reading " << inPath << " ...\n";
        src = io::readCsv(inPath);
    } 

    else 
    {
        std::cout << "[LR1] " << inPath << " not found — generating "
                  << kDefaultSize << " records in-process\n";
        src = gen::synthesize(kDefaultSize);
        try 
        {
            io::writeCsv(inPath, src);
            std::cout << "[LR1] saved " << inPath << "\n";
        } 
        
        catch (const std::exception& e) 
        {
            std::cerr << "[LR1] warning: could not save " << inPath
                      << ": " << e.what() << "\n";
        }
    }

    std::cout << "[LR1] loaded " << src.size() << " records\n";

    auto a1 = src; double t1 = timeOnce(a1, [](auto& v){ sortings::selectionSort(v); });
    std::cout << "  selection : " << t1 << " s\n";
    io::writeCsv(outDir + "sorted_selection.csv", a1);

    auto a2 = src; double t2 = timeOnce(a2, [](auto& v){ sortings::bubbleSort(v);    });
    std::cout << "  bubble    : " << t2 << " s\n";
    io::writeCsv(outDir + "sorted_bubble.csv", a2);

    auto a3 = src; double t3 = timeOnce(a3, [](auto& v){ sortings::heapSort(v);      });
    std::cout << "  heap      : " << t3 << " s\n";
    io::writeCsv(outDir + "sorted_heap.csv", a3);

    auto a4 = src; double t4 = timeOnce(a4, sortByStd);
    std::cout << "  std::sort : " << t4 << " s\n";
    io::writeCsv(outDir + "sorted_std.csv", a4);

    bool ok = (a1 == a4) && (a2 == a4) && (a3 == a4);
    std::cout << "[LR1] consistency check: " << (ok ? "OK" : "FAIL") << "\n";
    return ok ? 0 : 1;
}

static int runBench(int argc, char** argv) {

    if (argc < 4) 
    {
        std::cerr << "usage: bench <out_csv> [--no-quadratic] <in.csv> [...]\n";
        return 2;
    }

    std::string outCsv = argv[2];
    bool skipQuadratic = false;
    int firstInput = 3;

    if (std::string(argv[3]) == "--no-quadratic") 
    {
        skipQuadratic = true;
        firstInput = 4;
    }

    FILE* fp = std::fopen(outCsv.c_str(), "w");

    if (!fp) 
    { 
        std::perror("fopen"); return 3; 
    }

    std::fprintf(fp, "size,selection,bubble,heap,std_sort\n");

    for (int i = firstInput; i < argc; ++i) 
    {
        std::string in = argv[i];
        std::cout << "[bench] " << in << " ...\n";
        auto src = io::readCsv(in);
        const std::size_t n = src.size();

        double t_sel = -1, t_bub = -1, t_heap = -1, t_std = -1;

        if (!skipQuadratic || n <= 50000) 
        {
            auto a = src;
            t_sel = timeOnce(a, [](auto& v){ sortings::selectionSort(v); });
            std::cout << "  selection : " << t_sel << " s\n";
        }

        if (!skipQuadratic || n <= 50000) 
        {
            auto a = src;
            t_bub = timeOnce(a, [](auto& v){ sortings::bubbleSort(v); });
            std::cout << "  bubble    : " << t_bub << " s\n";
        }

        {
            auto a = src;
            t_heap = timeOnce(a, [](auto& v){ sortings::heapSort(v); });
            std::cout << "  heap      : " << t_heap << " s\n";
        }

        {
            auto a = src;
            t_std = timeOnce(a, sortByStd);
            std::cout << "  std::sort : " << t_std << " s\n";
        }

        std::fprintf(fp, "%zu,%.9f,%.9f,%.9f,%.9f\n", n, t_sel, t_bub, t_heap, t_std);
        std::fflush(fp);
    }

    std::fclose(fp);
    std::cout << "[bench] written to " << outCsv << "\n";
    return 0;
}

int main(int argc, char** argv) 
{
    try 
    {
        if (argc >= 2 && std::string(argv[1]) == "bench")
            return runBench(argc, argv);
        return runDefault();
    } 

    catch (const std::exception& e) 
    {
        std::cerr << "error: " << e.what() << "\n";
        return 10;
    }
}
