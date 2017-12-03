#include "stdafx.h"
#include "ReadFileOperation.h"

namespace AO
{
    ReadFileOperation::ReadFileOperation(HANDLE fileHandle, PLARGE_INTEGER offset)
        : m_fileHandle(fileHandle)
    {
        Internal = 0;
        InternalHigh = 0;
        Offset = 0;
        OffsetHigh = 0;
        hEvent = NULL;

        m_data.resize(BufferSize);

        if (offset != NULL)
        {
            Offset = offset->LowPart;
            OffsetHigh = offset->HighPart;
        }
    }

    ReadFileOperation::~ReadFileOperation()
    {
    }

    HRESULT ReadFileOperation::Run() noexcept
    {
        return ReadFile(m_fileHandle, m_data.data(), BufferSize, nullptr, this);
    }

    AO::Data ReadFileOperation::GetResult()
    {
        return std::move(m_data);
    }
}
