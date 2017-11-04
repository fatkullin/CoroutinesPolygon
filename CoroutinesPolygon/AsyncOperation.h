#pragma once
#include "Task.h"

namespace AO
{
    class AsyncOperation
        : public OVERLAPPED
    {
    public:
        DWORD LpNumberOfBytes;
        ULONG_PTR LpCompletionKey;

        virtual ~AsyncOperation() = default;

        Task* GetAttahedTask(DWORD lpNumberOfBytes,
            ULONG_PTR lpCompletionKey)
        {
            LpNumberOfBytes = lpNumberOfBytes;
            LpCompletionKey = lpCompletionKey;
            return m_task;
        }

        void SetTask(Task* task) noexcept
        {
            m_task = task;
        }

        virtual HRESULT Run() noexcept = 0;

    private:
        AO::Task* m_task;
    };
}
