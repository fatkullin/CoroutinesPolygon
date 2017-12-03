#include "stdafx.h"
#include "SleepInitialWorkerTask.h"

namespace AO
{
    SleepTask::SleepTask()
    {
        NextTask = nullptr;
        Stop = false;
        TaskSet = false;
    }

    void SleepTask::SetTask(ITask* task)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        NextTask = task;
        TaskSet = true;
        m_cv.notify_all();
    }

    ITask* SleepTask::WaitForTask()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!Stop && !TaskSet)
            m_cv.wait(lock);

        if (Stop)
        {
            Stopped = true;
            lock.unlock();
            m_cv.notify_one();
            return nullptr;
        }

        TaskSet = false;
        return NextTask;
    }

    void SleepTask::Cancel()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        Stop = true;
        m_cv.notify_one();
        while (!Stopped)
            m_cv.wait(lock);
    }
}
