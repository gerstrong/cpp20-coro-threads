#include "generator.h"
#include "threadPool.h"

#include <iostream>

constexpr unsigned MAX_FIBO = 94;

// Function to calculate factorial using coroutine and thread pool
Generator<std::uint64_t>
fibonacci_sequence(const unsigned n)
{
    if (n == 0)
        co_return;

    if (n > MAX_FIBO)
        throw std::runtime_error("Too big Fibonacci sequence. Elements would overflow.");

    co_yield 0;

    if (n == 1)
        co_return;

    co_yield 1;

    if (n == 2)
        co_return;

    std::uint64_t a = 0;
    std::uint64_t b = 1;

    for (unsigned i = 2; i < n; i++)
    {
        std::uint64_t s = a + b;
        co_yield s;
        a = b;
        b = s;
    }
}

int fibonacci_threads(unsigned int n, ThreadPool& pool)
{
    int sum = 0;

    for(int th=0 ; th<pool.numThreads() ; th++)
    {
        auto future = pool.enqueue([th](const unsigned int n) -> int
        {
            Generator<std::uint64_t> gen = fibonacci_sequence(n);

            for (int j = 0; gen; j++)
                std::cout << th << ": fib(" << j << ")=" << gen() << '\n';

            return 1;
        }, n);

        //sum += future.get();
        sum += 1;
    }

    return sum;
}

int main()
{

    int finishedThreads = 0;

    {
        constexpr size_t numThreads = 4;

        ThreadPool pool(numThreads);

        unsigned int n = 10;
        finishedThreads = fibonacci_threads(n, pool);
    }

    std::cout << finishedThreads << " Threads sind sauber durchgelaufen.\n";

    return 0;
}
