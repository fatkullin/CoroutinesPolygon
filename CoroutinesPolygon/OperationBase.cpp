#include "stdafx.h"
#include "OperationBase.h"


void AO::TaskPromiseType<void>::return_void() noexcept
{
    auto continuation = m_task->SetResultAndGetContinuation(
        TaskResultOrException<void>());

    if (continuation)
    {
        // not null continuation means some other task waits for 'm_task'
        // so 'm_task' is alive and we can make the following assigment

        m_task->SetNextTask(continuation);
    }
}
