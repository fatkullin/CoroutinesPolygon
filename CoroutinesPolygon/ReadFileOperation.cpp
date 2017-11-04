#include "stdafx.h"
#include "ReadFileOperation.h"

namespace AO
{
    ReadFileOperation::ReadFileResult::
    ReadFileResult(HANDLE fileHandle, PLARGE_INTEGER offset): m_fileHandle(fileHandle)
    {
        Internal = 0;
        InternalHigh = 0;
        Offset = 0;
        OffsetHigh = 0;
        hEvent = NULL;
        Data = VirtualAlloc(NULL, BufferSize, MEM_COMMIT, PAGE_READWRITE);;

        if (offset != NULL)
        {
            Offset = offset->LowPart;
            OffsetHigh = offset->HighPart;
        }
    }

    ReadFileOperation::ReadFileResult::~ReadFileResult()
    {
        if (Data != NULL)
            VirtualFree(Data, 0, MEM_RELEASE);
    }

    HRESULT ReadFileOperation::ReadFileResult::Run() noexcept
    {
        return ReadFile(m_fileHandle, Data, BufferSize, nullptr, this);
    }

    ReadFileOperation::ReadFileOperation(File&& file, HANDLE iocp)
        : m_file(std::move(file))
        , m_iocp(iocp)
    {
    }

    HANDLE ReadFileOperation::GetHandle() const
    {
        return HANDLE(m_file.m_file);
    }

    AO::TaskExecutionResult ReadFileOperation::Run() noexcept
    {
        auto const code = CreateIoCompletionPort(GetHandle(), m_iocp, 0, 0);
        if (code == nullptr)
            return CompletedWithError(std::move(HRESULT_FROM_WIN32(GetLastError())));

        m_asyncOperation = std::make_unique<ReadFileResult>(GetHandle(), nullptr);

        return WaitForAsyncOperation(m_asyncOperation.get(), &ReadFileOperation::OnCompleted);
    }

    AO::TaskExecutionResult ReadFileOperation::OnCompleted()
    {
        // TODO: unnecessary copying of m_asyncOperation->Data
        return CompletedWithSuccess(Data((unsigned char*)(m_asyncOperation->Data),
                                         (unsigned char*)m_asyncOperation->Data + m_asyncOperation->BufferSize));
    }

    void ReadFileOperation::Cancel()
    {
        HRESULT res = CancelIoEx(GetHandle(), m_asyncOperation.get());
        // TODO: handle error
    }
}
