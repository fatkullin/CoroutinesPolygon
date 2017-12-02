#pragma once

//#include "TaskFuture.h"
#include <experimental/coroutine>
#include "TypedTaskCoroTraits.h"

namespace AO
{


	struct AwaiterTask : Task
	{
		AwaiterTask(std::experimental::coroutine_handle<> awaiter)
			: m_awaiter(awaiter)
		{

		}

		void Execute(Task**) override
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
			: Result(std::move(result))
            , m_task(std::move(task))
		{

		}

		T get()
		{
			Result.wait();
            return std::move(m_task->Result.Get());
		}
		
        ~ResultFuture()
		{
            Result.wait();
		}

		constexpr bool await_ready() const noexcept
		{
			return false;
		}

		// return true if continuation has been set
		constexpr bool await_suspend(std::experimental::coroutine_handle<> awaiter) noexcept
		{
			m_awaiter = awaiter;

			m_awaiterTask = std::make_unique<AwaiterTask>(m_awaiter);
            return  m_task->SetContinuation(m_awaiterTask.get());
		}

		T await_resume() noexcept
		{
            return std::move(m_task->Result.Get());
		}

		//std::unique_ptr<Future> TaskFuture;
		std::future<void> Result;

		std::unique_ptr<AwaiterTask> m_awaiterTask;

	private:
		std::experimental::coroutine_handle<> m_awaiter;
        std::unique_ptr<TypedTask<T>> m_task;
	};
}
