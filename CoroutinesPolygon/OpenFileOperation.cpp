#include "stdafx.h"
#include "OpenFileOperation.h"

namespace AO
{
    AO::TaskBlockingType OpenFileOperation::GetBlockingType()
    {
        return AO::TaskBlockingType::Blocked;
    }

    AO::TaskExecutionResult OpenFileOperation::Run() noexcept
    {
        CAtlFile file;
        HRESULT hr = file.Create(m_path.c_str(),
                                 GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, FILE_FLAG_OVERLAPPED);

        if (hr == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED))
        {
            return CompletedWithError(std::move(hr));
        }
        else if (FAILED(hr))
        {
            return CompletedWithError(std::move(hr));
        }
        else
        {
            return CompletedWithSuccess(File(std::move(file)));
        }
    }
}
