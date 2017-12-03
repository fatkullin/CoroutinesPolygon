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

    template<class T>
    struct TaskResultOrException
    {
        TaskResultOrException(T result)
            : Result(std::move(result))
        {
        }

        TaskResultOrException(std::exception_ptr exception)
            : Exception(exception)
        {
        }

        T Get()
        {
            if (Exception)
                rethrow_exception(Exception);

            return std::move(Result);
        }

    private:
        T Result;
        std::exception_ptr Exception;
    };

    template<>
    struct TaskResultOrException<void>
    {
        TaskResultOrException() = default;

        TaskResultOrException(std::exception_ptr exception)
            : Exception(exception)
        {
        }

        void Get()
        {
            if (Exception)
                rethrow_exception(Exception);
        }

    private:
        std::exception_ptr Exception;

    };


	template <class T>
	struct CoroTask;
    
    template <class T>
    struct TypedTask;
    
	template <class T>
	struct TaskPromiseTypeBase
	{
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

		auto initial_suspend()
		{
			return GetHandleAndSuspend(*this);
		}

		auto final_suspend() noexcept
		{
            return std::experimental::suspend_never();
		}

		std::unique_ptr<TypedTask<T>> get_return_object();

		void unhandled_exception()
		{
            auto continuation = m_task->SetResultAndGetContinuation(
                TaskResultOrException<T>(std::current_exception()));
            if (continuation)
            {
                // not null continuation means some other task waits for 'm_task'
                // so 'm_task' is alive and we can make the following assigment
                m_task->SetNextTask(continuation);
            }
		}

		// TODO: check for lifetime
		CoroTask<T>* m_task;
	};

	template<typename T>
	class TaskPromiseType : public TaskPromiseTypeBase<T>
	{
	public:
	    void return_value(T v);
	};

	template<>
	class TaskPromiseType<void> : public TaskPromiseTypeBase<void>
	{
	public:
	    void return_void() noexcept;
	};

	template <class T>
	struct TypedTask : public Task
	{
        using Result_t = T;
        using IoCompletionHandle_t = HANDLE;
        using ExecutionContext_t = IoCompletionHandle_t;
        
    public:
        // This method is called by TaskManager before adding to task queue
        // It sets ExecutionContext and returns object that signal about task ending
	    std::future<void> PrepareToRun(ExecutionContext_t context)
        {
            IoCompletionHandle = context;
            m_readyNotifier = std::make_unique<std::promise<void>>();
            return m_readyNotifier->get_future();
        }

        // Set the continuation task that will be runned by execution environment
	    // after finishing of the current task. Continuation task should not be destroyed
	    // until the execution ending.
        // 
        bool SetContinuation(Task* task) noexcept
        {
            Task* expectedNull = nullptr;
            return m_continuation.compare_exchange_strong(expectedNull, task);
        }

        TaskResultOrException<Result_t>* GetResult()
	    {
            return TaskResult.get();
	    }

		constexpr bool await_ready() const noexcept
		{
			return false;
		}

        // Some outer task await for 'this' task.
        // We should setup execution context for 'this' task (IoCompletionHandle),
	    // then return 'this' task as 'NextTask' for outer task - so working thread be able to run this task.
        // The outer task should be continued after 'this' task becomes ready.
        // 
		template<class TOuter>
		void await_suspend(std::experimental::coroutine_handle<TaskPromiseType<TOuter>> awaiter) noexcept
		{
			TaskPromiseType<TOuter>& taskPromise = awaiter.promise();
            this->IoCompletionHandle = taskPromise.m_task->IoCompletionHandle;

            taskPromise.m_task->SetNextTask(this);
		    //taskPromise.m_task->NextTask = this;
			this->SetContinuation(taskPromise.m_task);
		}

		T await_resume()
		{
            return TaskResult->Get();
		}

	protected:
        ExecutionContext_t GetExecutionContext() const
        {
            return IoCompletionHandle;
        }

        // ReSharper disable once CppHiddenFunction
        Task* SetResultAndGetContinuation(TaskResultOrException<Result_t> result)
        {
            auto readyNotifier = move(m_readyNotifier);

            Task* cont = nullptr;
            TaskResult = std::make_unique<TaskResultOrException<Result_t>>(std::move(result));

            // destructor for task might be run just after GetContinuation completion
            GetContinuation(&cont);

            // now 'this' object can be deleted so use only local variables
            if (readyNotifier)
            {
                readyNotifier->set_value();
            }

            return cont;
        }

        std::unique_ptr<std::promise<void>> m_readyNotifier;

	private:
        bool GetContinuation(Task** task) noexcept
        {
            Task* noTask = nullptr;
            Task* invalidTask = (Task*)(-1);
            if (m_continuation.compare_exchange_strong(noTask, invalidTask))
                return false;

            *task = m_continuation;
            return true;
        }

    private:
        std::atomic<Task*> m_continuation = nullptr;
        IoCompletionHandle_t IoCompletionHandle = nullptr;
        std::unique_ptr<TaskResultOrException<Result_t>> TaskResult;

        template<class V> friend struct TypedTask;
	};

    template <class T>
    struct CoroTask : public TypedTask<T>
    {
        virtual void Execute(Task** nextTask) override
        {
            NextTask = nullptr;
            m_coroHandle.resume();
            *nextTask = NextTask;
        }

        // public morozov
        // ReSharper disable once CppHidingFunction
        Task* SetResultAndGetContinuation(TaskResultOrException<T> result)
        {
            return TypedTask<T>::SetResultAndGetContinuation(std::move(result));
        }

        std::experimental::coroutine_handle<> m_coroHandle;

        void SetNextTask(Task* next)
        {
            NextTask = next;
        }

    private:
        Task* NextTask = nullptr;							// childTask or continuation
    };

    template <class TDerived, class T>
    struct OperationBase : public TypedTask<T>
    {
        OperationBase() 
        {
            m_step = &TDerived::Run;
        }

        virtual void Execute(Task** next) override
        {
            TDerived* ths = static_cast<TDerived*>(this);
            auto result = (ths->*m_step)();
            *next = result.Continuation;
        }

    protected:

        TaskExecutionResult CompletedWithError(Error_t&& result) noexcept
        {
            auto next = this->SetResultAndGetContinuation(
                TaskResultOrException<T>(std::make_exception_ptr(ErrorException(std::move(result)))));

            return TaskExecutionResult(TaskExecutionResult::Completed, next);
        }

        TaskExecutionResult CompletedWithSuccess(T&& result)
        {
            auto next = this->SetResultAndGetContinuation(
                TaskResultOrException<T>(std::move(result)));

            return TaskExecutionResult(TaskExecutionResult::Completed, next);
        }

        TaskExecutionResult WaitForAsyncOperation(AsyncOperation* asyncOperation, TaskExecutionResult(TDerived::*nextStep)())
        {
            asyncOperation->SetTask(this);
            m_step = nextStep;

            auto hr = asyncOperation->Run();
            if (FAILED(hr))
            {
                return CompletedWithError(std::move(hr));
            }

            return TaskExecutionResult(TaskExecutionResult::AsyncOperationRun, nullptr);
        }

    private:
        TaskExecutionResult(TDerived::*m_step)();
        std::promise<T> m_promiseMain;
    };

	template <class T>
	bool TaskPromiseTypeBase<T>::GetHandleAndSuspend::await_suspend(std::experimental::coroutine_handle<> handle) noexcept
	{
		m_taskPromise.m_task->m_coroHandle = handle;
		return true;
	}
	
	template <class T>
	std::unique_ptr<TypedTask<T>> TaskPromiseTypeBase<T>::get_return_object()
	{
		auto result = std::make_unique<CoroTask<T>>();
		m_task = result.get();
		return result;
	}

    template <typename T>
    void TaskPromiseType<T>::return_value(T v)
    {
        auto continuation = m_task->SetResultAndGetContinuation(
            TaskResultOrException<T>(std::move(v)));

	    if (continuation)
        {
            // not null continuation means some other task waits for 'm_task'
            // so 'm_task' is alive and we can make the following assigment

            m_task->SetNextTask(continuation);
        }
    }
}
