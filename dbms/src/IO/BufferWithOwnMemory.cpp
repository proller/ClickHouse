#include "BufferWithOwnMemory.h"

//#include <Common/ProfileEvents.h>
//#include <Common/Allocator.h>

//#include <Common/Exception.h>
#include <Core/Defines.h>


namespace ProfileEvents
{
    extern const Event IOBufferAllocs;
    extern const Event IOBufferAllocBytes;
}


namespace DB
{

    Memory::Memory() {}

    /// If alignment != 0, then allocate memory aligned to specified value.
    Memory::Memory(size_t size_, size_t alignment_) : m_capacity(size_), m_size(m_capacity), alignment(alignment_)
    {
        alloc();
    }

    Memory::~Memory()
    {
        dealloc();
    }

    Memory::Memory(Memory && rhs) noexcept
    {
        *this = std::move(rhs);
    }

    Memory & Memory::operator=(Memory && rhs) noexcept
    {
        std::swap(m_capacity, rhs.m_capacity);
        std::swap(m_size, rhs.m_size);
        std::swap(m_data, rhs.m_data);
        std::swap(alignment, rhs.alignment);

        return *this;
    }

    size_t Memory::size() const { return m_size; }
    const char & Memory::operator[](size_t i) const { return m_data[i]; }
    char & Memory::operator[](size_t i) { return m_data[i]; }
    const char * Memory::data() const { return m_data; }
    char * Memory::data() { return m_data; }

    void Memory::resize(size_t new_size)
    {
        if (0 == m_capacity)
        {
            m_size = new_size;
            m_capacity = new_size;
            alloc();
        }
        else if (new_size <= m_size)
        {
            m_size = new_size;
            return;
        }
        else
        {
            size_t new_capacity = align(new_size + pad_right, alignment);
            m_data = static_cast<char *>(Allocator::realloc(m_data, m_capacity, new_capacity, alignment));
            m_capacity = new_capacity;
            m_size = m_capacity - pad_right;
        }
    }

    /*static*/ size_t Memory::align(const size_t value, const size_t alignment)
    {
        if (!alignment)
            return value;

        return (value + alignment - 1) / alignment * alignment;
    }

    void Memory::alloc()
    {
        if (!m_capacity)
        {
            m_data = nullptr;
            return;
        }

        size_t padded_capacity = m_capacity + pad_right;

        ProfileEvents::increment(ProfileEvents::IOBufferAllocs);
        ProfileEvents::increment(ProfileEvents::IOBufferAllocBytes, padded_capacity);

        size_t new_capacity = align(padded_capacity, alignment);
        m_data = static_cast<char *>(Allocator::alloc(new_capacity, alignment));
        m_capacity = new_capacity;
        m_size = m_capacity - pad_right;
    }

    void Memory::dealloc()
    {
        if (!m_data)
            return;

        Allocator::free(m_data, m_capacity);
        m_data = nullptr;    /// To avoid double free if next alloc will throw an exception.
    }

}
