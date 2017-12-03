#pragma once
#include "TaskManager.h"
#include "BlockedOperationBase.h"

namespace AO
{
    std::unique_ptr<TypedTask<int>> GetMyTask(std::wstring filePath);
}
