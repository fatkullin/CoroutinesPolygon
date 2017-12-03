#include "stdafx.h"
#include "BlockedOperationBase.h"


void AO::TaskPromiseType<void>::return_void() noexcept
{
    auto continuation = GetTask()->SetResultAndGetContinuation(
        TaskResultOrException<void>());

    if (continuation)
    {
        // not null continuation means some other task waits for 'm_task'
        // so 'm_task' is alive and we can make the following assigment

        GetTask()->SetNextTask(continuation);
    }
}
