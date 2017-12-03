#include "stdafx.h"
#include "TaskExecutor.h"

#include "TaskManager.h"

namespace AO
{
    ITask* TaskExecutor::Execute(ITask*& task)
    {
		ITask* nextTask = nullptr;
        task->Execute(&nextTask);

		task = nullptr;
		return nextTask;
    }

}
