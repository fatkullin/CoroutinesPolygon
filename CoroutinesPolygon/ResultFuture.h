#pragma once

#include "TaskFuture.h"
#include <experimental/coroutine>

namespace AO
{


	struct AwaiterTask : Task
	{
		AwaiterTask(std::experimental::coroutine_handle<> awaiter)
			: m_awaiter(awaiter)
		{

		}

		TaskExecutionResult Execute() override
		{
			m_awaiter.resume();
			return TaskExecutionResult::CompletedCoroutine;
		}

		void Cancel() override
		{

		}

	private:
		std::experimental::coroutine_handle<> m_awaiter;
	};



	///////////////////////////////////////////////////////////////////////////////////////////////

	template <class T>
	class ResultFuture
	{
	public:
		ResultFuture(std::unique_ptr<Future> taskFuture, std::future<T> result)
			: TaskFuture(std::move(taskFuture))
			, Result(std::move(result))
		{
		}

		T get()
		{
			return Result.get();
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
			return TaskFuture->SetContinuation(m_awaiterTask.get());
		}

		constexpr T await_resume() noexcept
		{
			return Result.get();
		}

		std::unique_ptr<Future> TaskFuture;
		std::future<T> Result;

		std::unique_ptr<AwaiterTask> m_awaiterTask;

	private:
		std::experimental::coroutine_handle<> m_awaiter;
	};
}