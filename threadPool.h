#include <vector>
#include <thread>
#include <future>
#include <functional>
#include <queue>

// Thread pool with a specified number of threads
class [[nodiscard]] ThreadPool
{
public:
    ThreadPool(const size_t numThreads)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            threads.emplace_back([this]
            {
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !taskQueue.empty(); });
                        if (stop && taskQueue.empty())
                            return;
                        task = std::move(taskQueue.front());
                        taskQueue.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& thread : threads)
            thread.join();
    }

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        using ReturnType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
            {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            taskQueue.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return result;
    }

    auto numThreads()
    {
        return threads.size();
    }


private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;
};

