#include "stdafx.h"
#include "AsyncWaitInitialWorkerTask.h"
#include "AsyncOperation.h"

namespace AO
{
    AsyncWaitTask::AsyncWaitTask(HANDLE completionPort)
        : m_completionPort(completionPort)
    {
    }

    ITask* AsyncWaitTask::WaitForTask()
    {
        //BOOL WINAPI GetQueuedCompletionStatus(
        //    _In_  HANDLE       CompletionPort,
        //    _Out_ LPDWORD      lpNumberOfBytes,
        //    _Out_ PULONG_PTR   lpCompletionKey,
        //    _Out_ LPOVERLAPPED *lpOverlapped,
        //    _In_  DWORD        dwMilliseconds
        //);

        DWORD lpNumberOfBytes;
        ULONG_PTR lpCompletionKey;
        LPOVERLAPPED lpOverlapped;

        BOOL result = GetQueuedCompletionStatus(m_completionPort, &lpNumberOfBytes, &lpCompletionKey, &lpOverlapped,
                                                INFINITE);
        if (result == FALSE)
        {
            auto error = GetLastError();
            if (error == ERROR_ABANDONED_WAIT_0)
                return nullptr;
            // TODO: handle error
        }

        auto operation = static_cast<AsyncOperation*>(lpOverlapped);
        return operation->GetAttahedTask(lpNumberOfBytes, lpCompletionKey);
    }
}
