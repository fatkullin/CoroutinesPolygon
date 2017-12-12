
#include "stdafx.h"
#include "MyOperation.h"
#include "TaskManager.h"
#include <iostream>

//using FutureType = std::future<int>;
using FutureType = std::unique_ptr<AO::ResultFuture<int>>;

FutureType GetFuture(std::shared_ptr<AO::TaskManager> taskManager)
{
    //return AO::MyTaskAsync(taskManager, L"d:\\tmp.dat");

    //auto task = std::make_unique<AO::MyTask>(taskManager, L"d:\\tmp.dat");
    //return taskManager->AddNewOperation(std::move(task));

    auto task = AO::GetMyTask(L"d:\\tmp.dat");
    return taskManager->AddNewOperation(std::move(task));
}

int main()
{
	auto const taskManager = AO::TaskManager::Create(4, 40, 1);
	int r = 0;

    auto time = std::chrono::steady_clock::now();

	for (int j = 0; j < 1; ++j)
	{
        auto const iMax = 200000;
		std::vector<FutureType> vf;
        vf.reserve(iMax);

		for (int i = 0; i <iMax; ++i)
		{
			vf.emplace_back(GetFuture(taskManager));
		}

		for (int i = 0; i < iMax; ++i)
		{
			auto const res = vf[i]->get();
			r += res;
		}
	}

    auto diff = std::chrono::steady_clock::now() - time;

	std::cout << r << " " << "\n";
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " ms" << "\n";
    return 0;
}

