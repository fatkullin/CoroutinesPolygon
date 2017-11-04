#include "stdafx.h"
#include "TaskExecutor.h"

#include "TaskManager.h"

namespace AO
{
    Task* TaskExecutor::Execute(Task*& task)
    {
        auto const executionResult = task->Execute();

        if (executionResult == ChildTaskCreated)
        {
            auto const child = task->Child;

            return child;
        }
        else if (executionResult == CompletedWithChildTask)
        {
            auto const child = task->Child;
            task->Promise.SetReady();

            task = nullptr;
            return child;
        }
        else if (executionResult == AsyncOperationRun)
        {
            task = nullptr;
            return nullptr;
        }

        // TODO: the following "if" logic should be placed right into Execute() realization for task
        else if (executionResult == WaitForOtherTask)
        {
            auto& future = task->WaitingFuture;
            if (future->SetContinuation(task))
            {
                task = nullptr;
                return nullptr;
            }

            return nullptr;
        }
        else if (executionResult == Completed)
        {
            auto& promise = task->Promise;
            Task* continuation;
            if (promise.GetContinuation(&continuation)) //similar to completedWithChildTask
            {
                task->Promise.SetReady();
                task = nullptr;
                return continuation;
            }
            else
            {
                task->Promise.SetReady();
                task = nullptr;
                return nullptr;
            }
        }
        else // state = yielded
        {
            auto const result = task;
            task = nullptr;
            return result;
        }
    }

}
