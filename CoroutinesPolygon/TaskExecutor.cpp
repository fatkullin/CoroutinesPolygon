#include "stdafx.h"
#include "TaskExecutor.h"

#include "TaskManager.h"

namespace AO
{
    Task* TaskExecutor::Execute(Task*& task)
    {
		Task* nextTask = nullptr;
        task->Execute(&nextTask);

		task = nullptr;
		return nextTask;
    }

}
