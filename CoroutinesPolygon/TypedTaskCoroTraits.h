#pragma once
#include "OperationBase.h"
#include <experimental/coroutine>

template <typename... TArgs>
struct std::experimental::coroutine_traits <std::unique_ptr<AO::TypedTask<int>>, TArgs... >
{
    struct promise_type
    {
		~promise_type()
		{
			
		}

        std::promise<int> p;

        struct GetHandleAndSuspend
        {
            GetHandleAndSuspend(promise_type& taskPromise)
                : m_taskPromise(taskPromise)
            {
            }

            bool await_ready() noexcept
            {
                return false;
            }

            bool await_suspend(coroutine_handle<> handle) noexcept
            {
                m_taskPromise.m_task->m_coroHandle = handle;
                return true;
            }

            void await_resume() noexcept
            {
            }

            promise_type& m_taskPromise;
        };

		struct ReturnContinuation
		{
			ReturnContinuation(promise_type& taskPromise)
				: m_taskPromise(taskPromise)
			{
			}

			bool await_ready() const noexcept
			{
				return false;
			}

			bool await_suspend(std::experimental::coroutine_handle<>)
			{
				AO::Task* continuation;
				if (m_taskPromise.m_task->Promise.GetContinuation(&continuation))
				{
					m_taskPromise.m_task->NextTask = continuation;
				}

				m_taskPromise.m_task->Promise.SetReady();

				// now coroutine can be safely deleted
				return false;
			}

			void await_resume() noexcept
			{
				
			}

			promise_type& m_taskPromise;
		};


        auto initial_suspend()
        {
            return GetHandleAndSuspend(*this);
        }

		auto final_suspend() noexcept
		{
			return ReturnContinuation{ *this };
		}

        void return_value(int v)
        {
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
