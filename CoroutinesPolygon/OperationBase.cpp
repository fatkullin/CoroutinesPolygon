#include "stdafx.h"
#include "OperationBase.h"

//bool AO::TaskPromiseType::GetHandleAndSuspend::await_suspend(std::experimental::coroutine_handle<> handle) noexcept
//{
//	m_taskPromise.m_task->m_coroHandle = handle;
//	return true;
//}
//
//bool AO::TaskPromiseType::ReturnContinuation::await_suspend(std::experimental::coroutine_handle<>)
//{
//	AO::Task* continuation;
//	if (m_taskPromise.m_task->Promise.GetContinuation(&continuation))
//	{
//		m_taskPromise.m_task->NextTask = continuation;
//	}
//
//	m_taskPromise.m_task->Promise.SetReady();
//
//	// now coroutine can be safely deleted
//	return false;
//}
//
//std::unique_ptr<AO::TypedTask<int>> AO::TaskPromiseType::get_return_object()
//{
//	auto result = std::make_unique<AO::TypedTask<int>>(p.get_future());
//	m_task = result.get();
//	return result;
//}
void AO::TaskPromiseType<void>::return_void() noexcept
{
    auto readyNotifier = std::move(m_task->m_readyNotifier);
    Task* continuation;
    if (m_task->GetContinuation(&continuation))
    {
        m_task->NextTask = continuation;
    }

    if (readyNotifier)
    {
        readyNotifier->set_value();
    }
}
