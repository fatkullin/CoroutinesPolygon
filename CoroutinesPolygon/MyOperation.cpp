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

	std::unique_ptr<TypedTask<int>> GetMyTask(std::wstring filePath)
    {
        auto someWork = DoSyncWork();
		co_await *someWork;

		auto fileOperation = std::make_unique<OpenFileOperation>(filePath);

		auto file = co_await *fileOperation;

        auto readFile = ReadFileOperation(file.Handle, nullptr);
        auto data = co_await readFile;

		co_return (int)data[0];
    }
}
