#pragma once
#include "OperationBase.h"
#include "AsyncOperation.h"
#include "File.h"

namespace AO
{
    using Data = std::vector<unsigned char>;

    struct ReadFileOperation
        : OperationBase<ReadFileOperation, Data>
    {
        class ReadFileResult : public AO::AsyncOperation
        {
        public:
            static constexpr SIZE_T BufferSize = 4096;
        
            ReadFileResult(HANDLE fileHandle, PLARGE_INTEGER offset);
            virtual ~ReadFileResult();
            virtual HRESULT Run() noexcept override;
            PVOID Data;

        private:
            HANDLE m_fileHandle;
        };

        ReadFileOperation(File&& file, HANDLE iocp);

        HANDLE GetHandle() const;

        TaskExecutionResult Run() noexcept;

        TaskExecutionResult OnCompleted();

        virtual void Cancel() override;

    private:
        File m_file;
        std::unique_ptr<ReadFileResult> m_asyncOperation;
        HANDLE m_iocp;
    };
}
