#pragma once
#include "ITask.h"
#include "BlockedOperationBase.h"
#include <experimental/coroutine>

namespace AO
{
    class AsyncOperation
        : public OVERLAPPED
    {
    public:
        DWORD NumberOfBytes;
        ULONG_PTR CompletionKey;

    public:
        // this method is executed when operation has been completed 
        ITask* GetAttahedTask(DWORD lpNumberOfBytes, ULONG_PTR lpCompletionKey)
        {
            NumberOfBytes = lpNumberOfBytes;
            CompletionKey = lpCompletionKey;
            return m_task;
        }

        virtual ~AsyncOperation() = default;
        virtual HRESULT Run() noexcept = 0;

    protected:
        // used by CoroAsyncOperation
        void SetTask(ITask* task) noexcept
        {
            m_task = task;
        }

    private:
        AO::ITask* m_task;
    };

    template <class T>
    struct CoroAsyncOperation : public AsyncOperation
    {
        constexpr bool await_ready() const noexcept
        {
            return false;
        }

        template<class TOuter>
        bool await_suspend(std::experimental::coroutine_handle<TaskPromiseType<TOuter>> awaiter) noexcept
        {
            TaskPromiseType<TOuter>& taskPromise = awaiter.promise();

            SetTask(taskPromise.GetTask());

            auto hr = Run();
            if (FAILED(hr))
            {
                m_error = hr;
                return false;
            }

            return true;
        }

        T await_resume()
        {
            if (FAILED(m_error))
            {
                throw ErrorException(std::move(m_error));
            }

            return GetResult();
        }

        virtual T GetResult() = 0;

    private:
        HRESULT m_error = S_OK;
    };
}
