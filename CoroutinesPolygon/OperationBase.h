#pragma once
#include "Task.h"
#include "AsyncOperation.h"

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

    template <class TDerived, class T>
    struct OperationBase : public Task
    {
        using Result_t = T;

        OperationBase() : m_state(TaskState::Creating)
        {
            m_step = &TDerived::Run;
        }

        std::future<T> GetFuture()
        {
            return m_promise.get_future();
        }

        virtual AO::TaskExecutionResult Execute() override
        {
            TDerived* ths = static_cast<TDerived*>(this);
            return (ths->*m_step)();
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
}
