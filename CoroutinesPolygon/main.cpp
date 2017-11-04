
#include "stdafx.h"
#include "MyOperation.h"
#include "TaskManager.h"
#include <iostream>

AO::ResultFuture<int> GetFuture(std::shared_ptr<AO::TaskManager> taskManager)
{
    auto task = std::make_unique<AO::MyTask>(taskManager, L"d:\\tmp.dat");
    return taskManager->AddNewOperation(std::move(task));
}

int main()
{
    auto const taskManager = AO::TaskManager::Create(1);

    int r = 0;
    for (int j = 0; j < 1000; ++j)
    {
        std::vector<AO::ResultFuture<int>> vf;
        for (int i = 0; i < 100; ++i)
        {
            vf.emplace_back(GetFuture(taskManager));
        }

        for (auto& fut : vf)
        {
            auto const res = fut.Result.get();

            r += res;
        }
    }
    std::cout << r << " "  << "\n";

    return 0;
}

