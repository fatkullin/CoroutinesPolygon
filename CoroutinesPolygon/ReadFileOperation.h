#pragma once
#include "AsyncOperation.h"

namespace AO
{
    using Data = std::vector<unsigned char>;

    class ReadFileOperation : public CoroAsyncOperation<Data>
    {
    public:
        static constexpr SIZE_T BufferSize = 4096;

        ReadFileOperation(HANDLE fileHandle, PLARGE_INTEGER offset);
        virtual ~ReadFileOperation();
        virtual HRESULT Run() noexcept override;

        virtual Data GetResult() override;

    private:
        std::vector<unsigned char> m_data;
        HANDLE m_fileHandle;
    };
}