#pragma once

#include <boost/noncopyable.hpp>

//#include <Common/ProfileEvents.h>
#include <Common/Allocator.h>

#include <Common/Exception.h>
#include <Core/Defines.h>
#include <Common/ProfileEvents.h>

/*namespace ProfileEvents
{
    extern const Event IOBufferAllocs;
    extern const Event IOBufferAllocBytes;
} */


namespace DB
{


/** Replacement for std::vector<char> to use in buffers.
  * Differs in that is doesn't do unneeded memset. (And also tries to do as little as possible.)
  * Also allows to allocate aligned piece of memory (to use with O_DIRECT, for example).
  */
struct Memory : boost::noncopyable, Allocator<false>
{
    /// Padding is needed to allow usage of 'memcpySmallAllowReadWriteOverflow15' function with this buffer.
    static constexpr size_t pad_right = 15;

    size_t m_capacity = 0;  /// With padding.
    size_t m_size = 0;
    char * m_data = nullptr;
    size_t alignment = 0;

    Memory();

    /// If alignment != 0, then allocate memory aligned to specified value.
    Memory(size_t size_, size_t alignment_ = 0);
    ~Memory();

    Memory(Memory && rhs) noexcept;

    Memory & operator=(Memory && rhs) noexcept;

    size_t size() const;
    const char & operator[](size_t i) const;
    char & operator[](size_t i);
    const char * data() const;
    char * data();

    void resize(size_t new_size);

private:
    static size_t align(const size_t value, const size_t alignment);

    void alloc();

    void dealloc();
};


/** Buffer that could own its working memory.
  * Template parameter: ReadBuffer or WriteBuffer
  */
template <typename Base>
class BufferWithOwnMemory : public Base
{
protected:
    Memory memory;
public:
    /// If non-nullptr 'existing_memory' is passed, then buffer will not create its own memory and will use existing_memory without ownership.
    BufferWithOwnMemory(size_t size = DBMS_DEFAULT_BUFFER_SIZE, char * existing_memory = nullptr, size_t alignment = 0)
        : Base(nullptr, 0), memory(existing_memory ? 0 : size, alignment)
    {
        Base::set(existing_memory ? existing_memory : memory.data(), size);
        Base::padded = !existing_memory;
    }
};


}
