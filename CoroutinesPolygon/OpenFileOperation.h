#pragma once
#include "BlockedOperationBase.h"
#include "ITask.h"
#include <string>
#include "File.h"

namespace AO
{
    struct OpenFileOperation : BlockedOperationBase<File>
    {
        OpenFileOperation(std::wstring path)
            : m_path(move(path))
        {
        }

        virtual TaskBlockingType GetBlockingType() override
        {
            return TaskBlockingType::Blocked;
        }


        virtual TaskExecutionResult Run() noexcept;

        //void Cancel() override
        //{
        //    // TODO: call cancel IO operation with thread that execute current operation
        //}

    private:
        std::wstring m_path;
    };

}
