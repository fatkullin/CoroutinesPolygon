#include "stdafx.h"
#include "OpenFileOperation.h"

namespace AO
{
    OpenFileOperation::TaskExecutionResult OpenFileOperation::Run() noexcept
    {
        CAtlFile file;
        auto hr = file.Create(m_path.c_str(),
                              GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              OPEN_EXISTING,
                              FILE_FLAG_OVERLAPPED);

        if (hr == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED))
        {
            return CompletedWithError(std::move(hr));
        }

        if (FAILED(hr))
        {
            return CompletedWithError(std::move(hr));
        }

        _ASSERTE(GetExecutionContext() != NULL);
        auto const ioCompletionHandle = GetExecutionContext();
        auto const code = CreateIoCompletionPort(HANDLE(file), ioCompletionHandle, 0, 0);

        if (code == nullptr)
            return CompletedWithError(std::move(HRESULT_FROM_WIN32(GetLastError())));

        return CompletedWithSuccess(File(std::move(file)));
    }
    }
