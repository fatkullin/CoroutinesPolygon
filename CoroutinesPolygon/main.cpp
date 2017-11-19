
#include "stdafx.h"
#include "MyOperation.h"
#include "TaskManager.h"
#include <iostream>

//using FutureType = std::future<int>;
using FutureType = AO::ResultFuture<int>;

FutureType GetFuture(std::shared_ptr<AO::TaskManager> taskManager)
{
    //return AO::MyTaskAsync(taskManager, L"d:\\tmp.dat");

    //auto task = std::make_unique<AO::MyTask>(taskManager, L"d:\\tmp.dat");
    //return taskManager->AddNewOperation(std::move(task));

    auto task = AO::GetMyTask(taskManager, L"d:\\tmp.dat");
    return taskManager->AddNewOperation(std::move(task));
}

int main()
{
    auto const taskManager = AO::TaskManager::Create(1);
    int r = 0;
    for (int j = 0; j < 1000; ++j)
    {
        std::vector<FutureType> vf;
        for (int i = 0; i < 100; ++i)
        {
            vf.emplace_back(GetFuture(taskManager));
        }

        for (auto& fut : vf)
        {
            auto const res = fut.get();

            r += res;
        }
    }
	std::cout << r << " "  << "\n";
    return 0;
}

