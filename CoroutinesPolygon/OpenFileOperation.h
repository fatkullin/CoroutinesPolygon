#pragma once
#include "OperationBase.h"
#include "Task.h"
#include <string>
#include "File.h"

namespace AO
{
    struct OpenFileOperation : OperationBase<OpenFileOperation, File>
    {
        OpenFileOperation(std::wstring path)
            : m_path(move(path))
        {
        }

        virtual AO::TaskBlockingType GetBlockingType() override;

        AO::TaskExecutionResult Run() noexcept;

        void Cancel() override
        {
            // TODO: call cancel IO operation with thread that execute current operation
        }

    private:
        std::wstring m_path;
    };

}
