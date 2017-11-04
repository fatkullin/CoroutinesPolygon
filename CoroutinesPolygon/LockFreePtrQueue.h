#pragma once
#include <concurrent_queue.h>

namespace AO
{
    template<class T>
    class LockFreePtrQueue
    {
    public:
        LockFreePtrQueue()
        {
        }

        T* TryPop() noexcept
        {
            T* result = nullptr;
            if (m_queue.try_pop(result))
                return result;
            return nullptr;
        }

        void Push(T* value)
        {
            m_queue.push(value);
        }

    private:
        concurrency::concurrent_queue<T*> m_queue;
    };
}
