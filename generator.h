#include <coroutine>
#include <exception>

// A simple coroutine generator that asynchronously calculates factorial
template<typename T>
struct Generator
{
    struct promise_type
    {
        T result_;
        std::exception_ptr exception_;

        Generator get_return_object()
        {
            return Generator{handle_type::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { exception_ = std::current_exception(); }

        void return_void() { } // for co_return with void

        template <std::convertible_to<T> From>
        std::suspend_always yield_value(From&& from) // for co_yield
        {
            result_ = std::forward<From>(from);
            return {};
        }

    };
    using handle_type = std::coroutine_handle<promise_type>;

    Generator(handle_type handle) : coro(handle) {}

    ~Generator()
    {
        if (coro) coro.destroy();
    }

    handle_type coro;

    explicit operator bool()
    {
        fill();
        return !coro.done();
    }

    T operator()()
    {
        fill();
        full_ = false;
        return std::move(coro.promise().result_);
    }

private:

    bool full_ = false;

    void fill()
    {
        if(!full_)
        {
            coro();
            if(coro.promise().exception_)
                std::rethrow_exception(coro.promise().exception_);

            full_ = true;
        }
    }
};
