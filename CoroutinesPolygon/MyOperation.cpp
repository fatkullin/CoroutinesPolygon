﻿#include "stdafx.h"
#include "MyOperation.h"
#include "OpenFileOperation.h"
#include "ReadFileOperation.h"

namespace AO
{
    static void DoSyncWork()
    {
        double a = 1;
        for (int i = 0; i < 10000; ++i)
        {
            a *= sqrt(i);
        }
    }

    MyTask::MyTask(std::shared_ptr<AO::TaskManager> taskManager,
                   std::wstring filePath)
        : m_taskManager(std::move(taskManager))
        , m_filePath(filePath)
    {
    }

    MyTask::~MyTask()
    {
    }

    void MyTask::Cancel()
    {
        m_cancelled = true;
    }

    AO::TaskExecutionResult MyTask::Run()
    {
        DoSyncWork();

        if (m_cancelled)
            return CompletedWithError(std::move(HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED)));

        auto fileOperation = std::make_unique<OpenFileOperation>(m_filePath);
        m_openFile = fileOperation->GetFuture();

        auto taskFuture = m_taskManager->AddNewTask(std::move(fileOperation));
        return Wait(std::move(taskFuture), &MyTask::Next);
    }

    AO::TaskExecutionResult MyTask::Next() noexcept
    {
        if (m_cancelled)
            return CompletedWithError(std::move(HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED)));

        try
        {
            auto file = m_openFile.get();

            auto readFileOperation = std::make_unique<ReadFileOperation>(std::move(file), m_taskManager->CompletionPort);

            m_readFile = readFileOperation->GetFuture();

            auto taskFuture = m_taskManager->AddNewTask(std::move(readFileOperation));
            return Wait(std::move(taskFuture), &MyTask::Last);
        }
        catch (ErrorException& err)
        {
            return CompletedWithError(std::move(err.Error));
        }
    }

    AO::TaskExecutionResult MyTask::Last() noexcept
    {
        if (m_cancelled)
            return CompletedWithError(std::move(HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED)));

        try
        {
            auto data = m_readFile.get();
            return CompletedWithSuccess((int)data[0]);
        }
        catch (ErrorException& err)
        {
            return CompletedWithError(std::move(err.Error));
        }
    }
}