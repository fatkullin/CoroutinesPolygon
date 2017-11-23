#pragma once
#include "TaskManager.h"
#include "OperationBase.h"
#include "File.h"

namespace AO
{
    std::future<int> MyTaskAsync(std::shared_ptr<AO::TaskManager> taskManager,
        std::wstring filePath);

    std::unique_ptr<TypedTask<int>> GetMyTask(std::wstring filePath);


    struct MyTask : AO::OperationBase<MyTask, int>
    {
        MyTask(std::shared_ptr<AO::TaskManager> taskManager,
               std::wstring filePath);

        virtual ~MyTask();

        virtual void Cancel() override;

        AO::TaskExecutionResult Run();

        AO::TaskExecutionResult Next() noexcept;

        AO::TaskExecutionResult Last() noexcept;


    private:
        bool m_cancelled = false;
        std::shared_ptr<AO::TaskManager> m_taskManager;

        std::future<File> m_openFile;
        std::future<std::vector<unsigned char>> m_readFile;
        std::wstring m_filePath;
    };
}
