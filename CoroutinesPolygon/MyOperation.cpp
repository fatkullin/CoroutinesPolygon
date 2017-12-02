#include "stdafx.h"
#include "MyOperation.h"
#include "OpenFileOperation.h"
#include "ReadFileOperation.h"
#include "TypedTaskCoroTraits.h"

namespace AO
{
    static std::unique_ptr<TypedTask<void>> DoSyncWork()
    {
        double a = 1;
        for (int i = 0; i < 10000; ++i)
        {
            a *= sqrt(i);
        }

		co_return;
    }

    std::future<int> MyTaskAsync(std::shared_ptr<AO::TaskManager> taskManager, std::wstring filePath)
    {
        CAtlFile cAtlFile;
        HRESULT hr = cAtlFile.Create(filePath.c_str(),
            GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, FILE_FLAG_OVERLAPPED);
        
        auto file = File(std::move(cAtlFile));

        auto readFileOperation = std::make_unique<ReadFileOperation>(std::move(file));

        auto readFuture = taskManager->AddNewOperation(std::move(readFileOperation));

        auto data = co_await *readFuture;

        co_return (int)data[0];
    }

	std::unique_ptr<TypedTask<int>> GetMyTask(std::wstring filePath)
    {
        auto someWork = DoSyncWork();
		co_await *someWork;

		auto fileOperation = std::make_unique<OpenFileOperation>(filePath);

		auto file = co_await *fileOperation;

		auto readFileOperation = std::make_unique<ReadFileOperation>(std::move(file));
		auto data = co_await *readFileOperation;

		co_return (int)data[0];
    }
}
