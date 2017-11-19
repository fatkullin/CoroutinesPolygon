#pragma once
#include "OperationBase.h"
#include <experimental/coroutine>

template <typename... TArgs>
struct std::experimental::coroutine_traits <std::unique_ptr<AO::TypedTask<int>>, TArgs... >
{
    struct promise_type
    {
        std::promise<int> p;

        struct GetHandleAndSuspend
        {
            GetHandleAndSuspend(promise_type& taskPromise)
                : m_taskPromise(taskPromise)
            {
            }

            bool await_ready() _NOEXCEPT
            {
                return false;
            }

            bool await_suspend(coroutine_handle<> handle) _NOEXCEPT
            {
                m_taskPromise.m_task->m_coroHandle = handle;
                return true;
            }

            void await_resume() _NOEXCEPT
            {
            }

            promise_type& m_taskPromise;
        };


        auto initial_suspend()
        {
            return GetHandleAndSuspend(*this);
        }

        auto final_suspend()
        {
            return suspend_never{};
        }

        void return_value(int v)
        {
            m_task->m_executionResult = AO::TaskExecutionResult::Completed;
            p.set_value(v);
        }

        auto get_return_object()
        {
            auto result = std::make_unique<AO::TypedTask<int>>(p.get_future());
            m_task = result.get();
            return result;
        }

        void unhandled_exception()
        {
            p.set_exception(std::current_exception());
        }

        // TODO: check for lifetime
        AO::TypedTask<int>* m_task;
    };
};
