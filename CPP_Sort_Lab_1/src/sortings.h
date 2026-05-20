#pragma once
#include <vector>
#include <cstddef>
#include <utility>

namespace sortings
{

    template <class T>
    void selectionSort(std::vector<T>& a)
    {
        const std::size_t n = a.size();
        for (std::size_t i = 0; i + 1 < n; ++i)
        {
            std::size_t minIdx = i;
            for (std::size_t j = i + 1; j < n; ++j)
                if (a[j] < a[minIdx]) minIdx = j;
            if (minIdx != i) std::swap(a[i], a[minIdx]);
        }
    }

    template <class T>
    void bubbleSort(std::vector<T>& a)
    {
        const std::size_t n = a.size();
        if (n < 2) return;
        for (std::size_t i = 0; i + 1 < n; ++i)
        {
            bool swapped = false;
            for (std::size_t j = 0; j + 1 < n - i; ++j)
            {
                if (a[j] > a[j + 1])
                {
                    std::swap(a[j], a[j + 1]);
                    swapped = true;
                }
            }

            if (!swapped) break;
        }
    }

    namespace detail
    {

        template <class T>
        void siftDown(std::vector<T>& a, std::size_t root, std::size_t heapSize) {
            while (true)
            {
                std::size_t left = 2 * root + 1;
                std::size_t right = 2 * root + 2;
                std::size_t largest = root;

                if (left  < heapSize && a[left]  > a[largest]) largest = left;
                if (right < heapSize && a[right] > a[largest]) largest = right;
                if (largest == root) break;

                std::swap(a[root], a[largest]);
                root = largest;
            }
        }
    }

    template <class T>
    void heapSort(std::vector<T>& a) 
    {
        const std::size_t n = a.size();
        if (n < 2) return;

        for (std::size_t i = n / 2; i-- > 0; )
            detail::siftDown(a, i, n);

        for (std::size_t end = n - 1; end > 0; --end) 
        {
            std::swap(a[0], a[end]);
            detail::siftDown(a, 0, end);
        }
    }

}
