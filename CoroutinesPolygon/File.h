#pragma once

struct File
{
    File(CAtlFile&& file) noexcept
        : Handle(file)
    {
    }

    File() = default;

    File(File & other) = delete;

    File(File&& other) noexcept
        : Handle(other.Handle)
    {
    }

    File& operator=(File & other) = delete;

    File& operator=(File&& other) noexcept
    {
        if (this == &other)
            return *this;
        Handle.Attach(other.Handle.Detach());
        return *this;
    }

    CAtlFile Handle;
};

