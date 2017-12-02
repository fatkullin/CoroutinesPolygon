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
            auto readyNotifier = std::move(m_task->m_readyNotifier);
            m_task->Result.Exception = std::current_exception();

            Task* continuation;
            if (m_task->GetContinuation(&continuation))
            {
                m_task->NextTask = continuation;
            }

            if (readyNotifier)
            {
                readyNotifier->set_value();
            }
		}

		// TODO: check for lifetime
		AO::TypedTask<T>* m_task;
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

    template<class T>
    struct TaskResultOrException
    {
        T Result;
        std::exception_ptr Exception;

        T Get()
        {
            if (Exception)
                std::rethrow_exception(Exception);

            return std::move(Result);
        }
    };

    template<>
    struct TaskResultOrException<void>
    {
        std::exception_ptr Exception;

        void Get()
        {
            if (Exception)
                std::rethrow_exception(Exception);
        }
    };

	template <class T>
	struct TypedTask : public Task
	{
	private:
        std::atomic<Task*> Continuation = nullptr;

	public:
        bool SetContinuation(Task* task) noexcept
        {
            Task* expectedNull = nullptr;
            return Continuation.compare_exchange_strong(expectedNull, task);
        }

        bool GetContinuation(Task** task) noexcept
        {
            Task* noTask = nullptr;
            Task* invalidTask = (Task*)(-1);
            if (Continuation.compare_exchange_strong(noTask, invalidTask))
                return false;

            *task = Continuation;
            return true;
        }

        //Task* SetResultAndGetContinuation(T result)
        //{
        //    Task* cont;
        //    Result.Result = std::move(result);
        //    GetContinuation(&cont);
        //    return cont;
        //}

        //Task* SetExceptionAndGetContinuation(std::exception_ptr exception)
        //{
        //    Task* cont;
        //    Result.Exception = std::move(exception);
        //    GetContinuation(&cont);
        //    return cont;
        //}
        
	    using Result_t = T;
        TaskResultOrException<Result_t> Result;

		Task* NextTask = nullptr;							// childTask or continuation

        // TODO: create special class - ExecutionEnvironment
        HANDLE IoCompletionHandle = nullptr;

        TypedTask()
            : m_coroHandle(nullptr)
        {
        }

        // used only in AddNewOperation for taskmanager
        void SetPromise(std::unique_ptr<std::promise<void>> readyNotifier)
        {
            m_readyNotifier = std::move(readyNotifier);
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

        //virtual void Cancel() override
        //{
        //    // TODO: set cancelled exception
        //    throw std::runtime_error("Cancelled");
        //}

        // TODO: make interface for task promise
        std::experimental::coroutine_handle<> m_coroHandle;

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
		void await_suspend(std::experimental::coroutine_handle<AO::TaskPromiseType<TOuter>> awaiter) noexcept
		{
			TaskPromiseType<TOuter>& taskPromise = awaiter.promise();
            this->IoCompletionHandle = taskPromise.m_task->IoCompletionHandle;

		    taskPromise.m_task->NextTask = this;
			this->SetContinuation(taskPromise.m_task);
		}

		T await_resume()
		{
            return Result.Get();
		}

	public:
        std::unique_ptr<std::promise<void>> m_readyNotifier;
	};

    template <class TDerived, class T>
    struct OperationBase : public TypedTask<T>
    {
        OperationBase() 
            : m_state(TaskState::Creating)
        {
            m_step = &TDerived::Run;
        }

        virtual void Execute(Task** next) override
        {
            TDerived* ths = static_cast<TDerived*>(this);
            auto result = (ths->*m_step)();

			if (result == TaskExecutionResult::Completed)
			{
                auto futureMain = m_promiseMain.get_future();
                auto readyNotifier = std::move(this->m_readyNotifier);

                try
                {
                    this->Result.Result = std::move(futureMain.get());
                    this->GetContinuation(next);
                }
                catch (...)
                {
                    this->Result.Exception = std::move(std::current_exception());
                    this->GetContinuation(next);
                }

                if (readyNotifier)
                {
                    readyNotifier->set_value();
                }
			}
        }

    protected:

        AO::TaskExecutionResult CompletedWithError(Error_t&& result) noexcept
        {
            m_state = TaskState::Completed;
            m_promiseMain.set_exception(std::make_exception_ptr(ErrorException(std::move(result))));
            return AO::TaskExecutionResult::Completed;
        }

        AO::TaskExecutionResult CompletedWithSuccess(T&& result)
        {
            m_state = TaskState::Completed;
            m_promiseMain.set_value(std::move(result));
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
        std::promise<T> m_promiseMain;
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
	
		// now coroutine can be safely deleted
		return false;
	}
	
	template <class T>
	std::unique_ptr<AO::TypedTask<T>> TaskPromiseTypeBase<T>::get_return_object()
	{
		auto result = std::make_unique<AO::TypedTask<T>>();
		m_task = result.get();
		return result;
	}

    template <typename T>
    void TaskPromiseType<T>::return_value(T v)
    {
        auto readyNotifier = std::move(m_task->m_readyNotifier);

        m_task->Result.Result = std::move(v);
        Task* continuation;
        if (m_task->GetContinuation(&continuation))
        {
            m_task->NextTask = continuation;
        }

        if (readyNotifier)
        {
            readyNotifier->set_value();
        }
    }
}
