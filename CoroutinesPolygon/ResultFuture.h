#pragma once

#include <experimental/coroutine>
#include "TypedTaskCoroTraits.h"

namespace AO
{
	struct AwaiterTask : ITask
	{
		AwaiterTask(std::experimental::coroutine_handle<> awaiter)
			: m_awaiter(awaiter)
		{

		}

		void Execute(ITask**) override
		{
			m_awaiter.resume();
		}

		//void Cancel() override
		//{

		//}

	private:
		std::experimental::coroutine_handle<> m_awaiter;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////

	template <class T>
	class ResultFuture
	{
	public:
		ResultFuture(std::future<void> result, std::unique_ptr<TypedTask<T>> task)
			: m_futureAboutCompletion(std::move(result))
            , m_task(std::move(task))
		{

		}

		T get()
		{
			m_futureAboutCompletion.wait();
            return std::move(m_task->GetResult()->Get());
		}
		
        ~ResultFuture()
		{
            m_futureAboutCompletion.wait();
		}

		constexpr bool await_ready() const noexcept
		{
			return false;
		}

		// return true if continuation has been set
		bool await_suspend(std::experimental::coroutine_handle<> awaiter) noexcept
		{
			m_awaiter = awaiter;

			m_awaiterTask = std::make_unique<AwaiterTask>(m_awaiter);
            return m_task->SetContinuation(m_awaiterTask.get());
		}

		T await_resume() noexcept
		{
            return std::move(m_task->GetResult()->Get());
		}

	private:
		std::unique_ptr<AwaiterTask> m_awaiterTask;
		std::future<void> m_futureAboutCompletion;
		std::experimental::coroutine_handle<> m_awaiter;
        std::unique_ptr<TypedTask<T>> m_task;
	};
}
