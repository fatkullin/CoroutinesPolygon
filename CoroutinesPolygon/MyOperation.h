#pragma once
#include "TaskManager.h"
#include "OperationBase.h"
#include "File.h"

namespace AO
{
    std::future<int> MyTaskAsync(std::shared_ptr<AO::TaskManager> taskManager,
        std::wstring filePath);

    std::unique_ptr<TypedTask<int>> GetMyTask(std::wstring filePath);

}
