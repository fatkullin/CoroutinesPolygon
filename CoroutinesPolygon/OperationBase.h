#pragma once
#include "Task.h"
#include "AsyncOperation.h"
#include <experimental/coroutine>

namespace AO
{
    using Error_t = HRESULT;

    struct ErrorException
    {
        explicit ErrorException(Error_t&& error) noexcept
            : Error(std::move(error))
        {}

        Error_t Error;
    };

    enum class TaskState
    {
        Creating,
        Completed,
    };

	template <class T>
	struct TypedTask;

	template <class T>
	struct TaskPromiseTypeBase
	{
		std::promise<T> p;

		struct GetHandleAndSuspend
		{
			GetHandleAndSuspend(TaskPromiseTypeBase& taskPromise)
				: m_taskPromise(taskPromise)
			{
			}

			bool await_ready() noexcept
			{
				return false;
			}

			bool await_suspend(std::experimental::coroutine_handle<> handle) noexcept;

			void await_resume() noexcept
			{
			}

			TaskPromiseTypeBase& m_taskPromise;
		};

		struct ReturnContinuation
		{
			ReturnContinuation(TaskPromiseTypeBase& taskPromise)
				: m_taskPromise(taskPromise)
			{
			}

			bool await_ready() const noexcept
			{
				return false;
			}

			bool await_suspend(std::experimental::coroutine_handle<>);

			void await_resume() noexcept
			{

			}

			TaskPromiseTypeBase& m_taskPromise;
		};


		auto initial_suspend()
		{
			return GetHandleAndSuspend(*this);
		}

		auto final_suspend() noexcept
		{
			return ReturnContinuation{ *this };
		}

		std::unique_ptr<AO::TypedTask<T>> get_return_object();

		void unhandled_exception()
		{
			p.set_exception(std::current_exception());
		}

		// TODO: check for lifetime
		AO::TypedTask<T>* m_task;
	};

	template<typename T>
	class TaskPromiseType : public TaskPromiseTypeBase<T>
	{
	public:
		void return_value(T v)
		{
			p.set_value(std::move(v));
		}
	};

	template<>
	class TaskPromiseType<void> : public TaskPromiseTypeBase<void>
	{
	public:
		void return_void() noexcept
		{
			p.set_value();
		}
	};

	template <class T>
	struct TypedTask : public Task
	{
        using Result_t = T;

		Task* NextTask = nullptr;							// childTask or continuation

        TypedTask()
            : m_coroHandle(nullptr)
        {
            
        }

	    TypedTask(std::future<T> value)
            : m_future(std::move(value))
        {
            
        }

		std::future<T> GetFuture()
		{
            return std::move(m_future);
		}

        virtual void Execute(Task** nextTask) override
        {
			NextTask = nullptr;
            m_coroHandle.resume();
			*nextTask = NextTask;
        }

        virtual void Cancel() override
        {
            // TODO: set cancelled exception
            throw std::runtime_error("Cancelled");
        }

        // TODO: make interface for task promise
        std::experimental::coroutine_handle<> m_coroHandle;

		constexpr bool await_ready() const noexcept
		{
			return false;
		}

		template<class TOuter>
		void await_suspend(std::experimental::coroutine_handle<AO::TaskPromiseType<TOuter>> awaiter) noexcept
		{
			TaskPromiseType<TOuter>& taskPromise = awaiter.promise();
			taskPromise.m_task->NextTask = this;

			this->Promise.SetContinuation(taskPromise.m_task);
		}

		T await_resume()
		{
			return m_future.get();
		}

	protected:
        void SetFuture(std::future<T> value)
        {
            m_future = std::move(value);
        }

	private:
        std::future<T> m_future;
	};

    template <class TDerived, class T>
    struct OperationBase : public TypedTask<T>
    {
        OperationBase() 
            : m_state(TaskState::Creating)
        {
            SetFuture(m_promise.get_future());
            m_step = &TDerived::Run;
        }

        virtual void Execute(Task** next) override
        {
            TDerived* ths = static_cast<TDerived*>(this);
            auto result = (ths->*m_step)();

			if (result == TaskExecutionResult::Completed)
			{
				this->Promise.GetContinuation(next);
				this->Promise.SetReady();
			}
        }

    protected:
        AO::TaskExecutionResult Wait(std::unique_ptr<AO::Future> future, AO::TaskExecutionResult(TDerived::*nextStep)())
        {
            WaitingFuture = std::move(future);
            m_step = nextStep;
            return AO::TaskExecutionResult::WaitForOtherTask;
        }

        AO::TaskExecutionResult CompletedWithError(Error_t&& result) noexcept
        {
            m_state = TaskState::Completed;
            m_promise.set_exception(std::make_exception_ptr(ErrorException(std::move(result))));
            return AO::TaskExecutionResult::Completed;
        }

        AO::TaskExecutionResult CompletedWithSuccess(T&& result)
        {
            m_state = TaskState::Completed;
            m_promise.set_value(std::move(result));
            return AO::TaskExecutionResult::Completed;
        }

        AO::TaskExecutionResult WaitForAsyncOperation(AO::AsyncOperation* asyncOperation, AO::TaskExecutionResult(TDerived::*nextStep)())
        {
            asyncOperation->SetTask(this);
            m_step = nextStep;

            auto hr = asyncOperation->Run();
            if (FAILED(hr))
            {
                return CompletedWithError(std::move(hr));
            }

            return AO::TaskExecutionResult::AsyncOperationRun;
        }

    private:
        AO::TaskExecutionResult(TDerived::*m_step)();
        std::promise<T> m_promise;
        TaskState m_state;
    };

	template <class T>
	bool TaskPromiseTypeBase<T>::GetHandleAndSuspend::await_suspend(std::experimental::coroutine_handle<> handle) noexcept
	{
		m_taskPromise.m_task->m_coroHandle = handle;
		return true;
	}
	
	template <class T>
	bool TaskPromiseTypeBase<T>::ReturnContinuation::await_suspend(std::experimental::coroutine_handle<>)
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
	
	template <class T>
	std::unique_ptr<AO::TypedTask<T>> TaskPromiseTypeBase<T>::get_return_object()
	{
		auto result = std::make_unique<AO::TypedTask<T>>(p.get_future());
		m_task = result.get();
		return result;
	}
}
