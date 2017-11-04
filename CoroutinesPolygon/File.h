#pragma once

struct File
{
    File(CAtlFile&& file) noexcept
        : m_file(file)
    {
    }

    File() = default;

    File(File & other) = delete;

    File(File&& other) noexcept
        : m_file(other.m_file)
    {
    }

    File& operator=(File & other) = delete;

    File& operator=(File&& other) noexcept
    {
        if (this == &other)
            return *this;
        m_file.Attach(other.m_file.Detach());
        return *this;
    }

    CAtlFile m_file;
};

