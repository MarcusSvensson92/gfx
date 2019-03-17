#ifndef GFX_UTIL_H
#define GFX_UTIL_H

#include <new>
#include <math.h>
#include <stdio.h>

static void Print(const char* message, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);
    strcat(buffer, "\n");
    printf(buffer);
}
static void Abort()
{
#ifdef _WIN32
    DebugBreak();
#endif
    Print("Press a key to exit...");
    getchar();
    exit(EXIT_FAILURE);
}

#define ARRAY_COUNT(arr) static_cast<uint32_t>(sizeof(arr) / sizeof(*arr))
#define ASSERT(cond) if (!(cond)) { Print("Assertion failed: %s", #cond); Abort(); }
#define VK(func) if (func != VK_SUCCESS) { Print("Function returned with erroneous result code: %s", #func); Abort(); }

inline void* Alloc(size_t size)
{
	return malloc(size);
}
inline void* Realloc(void* ptr, size_t size)
{
	return realloc(ptr, size);
}
inline void Free(void* ptr)
{
	free(ptr);
}
template<typename T>
inline T* New()
{
	return new(Alloc(sizeof(T))) T();
}
template<typename T>
inline void Delete(T* ptr)
{
	ptr->~T();
	Free(ptr);
}

struct Blob
{
    void*   m_Data = NULL;
    size_t  m_Size = 0;
};
inline void DestroyBlob(const Blob& blob)
{
    if (blob.m_Data)
        Free(blob.m_Data);
}

template <typename T>
class Array
{
private:
	T*		    m_Array;
	uint32_t    m_Count;
	uint32_t    m_Capacity;

public:
	Array(uint32_t count = 0)
		: m_Array(count > 0 ? static_cast<T*>(Alloc(count * sizeof(T))) : NULL)
		, m_Count(count)
		, m_Capacity(count)
	{
	}
	~Array()
	{
        for (uint32_t i = 0; i < m_Count; ++i)
        {
            m_Array[i].~T();
        }
        if (m_Array)
        {
            Free(m_Array);
        }
	}
	void Reserve(uint32_t capacity)
	{
		m_Array = static_cast<T*>(Realloc(m_Array, capacity * sizeof(T)));
		m_Capacity = capacity;
	}
	void Resize(uint32_t count)
	{
        m_Count = count;
        if (m_Count > m_Capacity)
        {
            Reserve(m_Count);
        }
	}
    void Grow(uint32_t count)
    {
        m_Count += count;
        if (m_Count > m_Capacity)
        {
            Reserve(m_Count);
        }
    }
	void Push(const T& entry)
	{
        if (m_Count + 1 > m_Capacity)
        {
            Reserve(m_Capacity == 0 ? 1 : m_Capacity * 2);
        }
		m_Array[m_Count++] = entry;
	}
    void EraseSwap(uint32_t index)
    {
        m_Array[index] = m_Array[--m_Count];
    }
	void Clear()
	{
		m_Count = 0;
	}
	uint32_t Count() const
	{
		return m_Count;
	}
	T* Data() const
	{
		return m_Array;
	}
	T& operator[](uint32_t index)
	{
		return m_Array[index];
	}
	const T& operator[](uint32_t index) const
	{
		return m_Array[index];
	}
};

template <typename T>
class HashTable
{
private:
	uint64_t*	m_Hashes;
	T*		    m_Entries;
	uint32_t	m_Capacity;

public:
	HashTable(uint32_t capacity)
		: m_Capacity(capacity)
	{
		size_t size = capacity * (sizeof(uint64_t) + sizeof(T));
		void* mem = Alloc(size);
		m_Hashes = static_cast<uint64_t*>(mem);
        m_Entries = reinterpret_cast<T*>(m_Hashes + capacity);
		memset(m_Hashes, 0xff, capacity * sizeof(uint64_t));
	}
	~HashTable()
	{
		Free(m_Hashes);
	}
    void Put(uint64_t hash, const T& entry)
    {
        uint32_t i = static_cast<uint32_t>(hash % static_cast<uint64_t>(m_Capacity));
        while (m_Hashes[i] != hash && m_Hashes[i] != ~0ULL)
            i = (i + 1) % m_Capacity;
        m_Hashes[i] = hash;
        m_Entries[i] = entry;
    }
    void Clear()
    {
        memset(m_Hashes, 0xff, m_Capacity * sizeof(uint64_t));
    }
    T* Find(uint64_t hash) const
    {
        uint32_t i = static_cast<uint32_t>(hash % static_cast<uint64_t>(m_Capacity));
        while (m_Hashes[i] != hash && m_Hashes[i] != ~0ULL)
            i = (i + 1) % m_Capacity;
        return m_Hashes[i] != ~0ULL ? &m_Entries[i] : NULL;
    }
    T* Get(uint32_t index) const
    {
        return m_Hashes[index] != ~0ULL ? &m_Entries[index] : NULL;
    }
    uint32_t Capacity() const
    {
        return m_Capacity;
    }
};

class String
{
private:
    char*   m_String;

public:
    String()
        : m_String(NULL)
    {
    }
    String(const char* str, ...)
        : m_String(NULL)
    {
        char buffer[1024];
        va_list args;
        va_start(args, str);
        vsnprintf(buffer, sizeof(buffer), str, args);
        va_end(args);
        Append(buffer);
    }
    String(const String& str)
        : m_String(NULL)
    {
        Append(str.m_String);
    }
    String& operator=(const char* str)
    {
        m_String = NULL;
        Append(str);
        return *this;
    }
    String& operator=(const String& str)
    {
        m_String = NULL;
        Append(str.m_String);
        return *this;
    }
    ~String()
    {
        if (m_String)
            Free(m_String);
    }
    void Append(const char* str, size_t len)
    {
        size_t old_len = m_String ? strlen(m_String) : 0;
        size_t new_len = old_len + len;
        m_String = static_cast<char*>(Realloc(m_String, new_len + 1));
        strncpy(m_String + old_len, str, len);
        m_String[new_len] = '\0';
    }
    void Append(const char* str)
    {
        Append(str, strlen(str));
    }
    void Append(const String& str)
    {
        Append(str.m_String);
    }
    void AppendFormat(const char* str, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, str);
        vsnprintf(buffer, sizeof(buffer), str, args);
        va_end(args);
        Append(buffer);
    }
    const char* Data() const
    {
        return m_String;
    }
    size_t Length() const
    {
        return strlen(m_String);
    }
};

class ReadStream
{
private:
    const void* m_Ptr;
    const void* m_End;

public:
    ReadStream(const void* ptr, size_t size)
        : m_Ptr(ptr)
        , m_End(static_cast<const uint8_t*>(ptr) + size)
    {
    }

    void IncrPtr(size_t size)
    {
        m_Ptr = static_cast<const void*>(static_cast<const uint8_t*>(m_Ptr) + size);
    }
    const void* GetPtr() const
    {
        return m_Ptr;
    }

    bool IsEndOfStream() const
    {
        return m_Ptr == m_End;
    }

    const void* Read(size_t* out_size = NULL)
    {
        uint64_t size = ReadUint64();
        const void* data = m_Ptr;
        m_Ptr = static_cast<const void*>(static_cast<const uint8_t*>(m_Ptr) + size);
        if (out_size)
        {
            *out_size = size;
        }
        return data;
    }
    uint8_t ReadUint8()
    {
        const uint8_t* ptr = static_cast<const uint8_t*>(m_Ptr);
        uint8_t n = *ptr;
        m_Ptr = static_cast<const void*>(++ptr);
        return n;
    }
    uint16_t ReadUint16()
    {
        const uint16_t* ptr = static_cast<const uint16_t*>(m_Ptr);
        uint16_t n = *ptr;
        m_Ptr = static_cast<const void*>(++ptr);
        return n;
    }
    uint32_t ReadUint32()
    {
        const uint32_t* ptr = static_cast<const uint32_t*>(m_Ptr);
        uint32_t n = *ptr;
        m_Ptr = static_cast<const void*>(++ptr);
        return n;
    }
    uint64_t ReadUint64()
    {
        const uint64_t* ptr = static_cast<const uint64_t*>(m_Ptr);
        uint64_t n = *ptr;
        m_Ptr = static_cast<const void*>(++ptr);
        return n;
    }
    int32_t ReadInt32()
    {
        const int32_t* ptr = static_cast<const int32_t*>(m_Ptr);
        int32_t n = *ptr;
        m_Ptr = static_cast<const void*>(++ptr);
        return n;
    }
    float ReadFloat()
    {
        const float* ptr = static_cast<const float*>(m_Ptr);
        float n = *ptr;
        m_Ptr = static_cast<const void*>(++ptr);
        return n;
    }
    const float* ReadFloat3()
    {
        const float* ptr = static_cast<const float*>(m_Ptr);
        m_Ptr = static_cast<const void*>(static_cast<const float*>(m_Ptr) + 3);
        return ptr;
    }
};
class WriteStream
{
private:
    void*   m_Ptr;
    void*   m_End;

public:
    WriteStream(void* ptr, size_t size)
        : m_Ptr(ptr)
        , m_End(static_cast<uint8_t*>(ptr) + size)
    {
    }

    void IncrPtr(size_t size)
    {
        m_Ptr = static_cast<void*>(static_cast<uint8_t*>(m_Ptr) + size);
    }
    const void* GetPtr() const
    {
        return m_Ptr;
    }

    bool IsEndOfStream() const
    {
        return m_Ptr == m_End;
    }

    void Write(const void* data, size_t size)
    {
        WriteUint64(size);
        if (size)
        {
            memcpy(m_Ptr, data, size);
        }
        m_Ptr = static_cast<void*>(static_cast<uint8_t*>(m_Ptr) + size);
    }
    void WriteUint8(uint8_t n)
    {
        uint8_t* ptr = static_cast<uint8_t*>(m_Ptr);
        *ptr = n;
        m_Ptr = static_cast<void*>(++ptr);
    }
    void WriteUint16(uint16_t n)
    {
        uint16_t* ptr = static_cast<uint16_t*>(m_Ptr);
        *ptr = n;
        m_Ptr = static_cast<void*>(++ptr);
    }
    void WriteUint32(uint32_t n)
    {
        uint32_t* ptr = static_cast<uint32_t*>(m_Ptr);
        *ptr = n;
        m_Ptr = static_cast<void*>(++ptr);
    }
    void WriteUint64(uint64_t n)
    {
        uint64_t* ptr = static_cast<uint64_t*>(m_Ptr);
        *ptr = n;
        m_Ptr = static_cast<void*>(++ptr);
    }
    void WriteInt32(int32_t n)
    {
        int32_t* ptr = static_cast<int32_t*>(m_Ptr);
        *ptr = n;
        m_Ptr = static_cast<void*>(++ptr);
    }
    void WriteFloat(float n)
    {
        float* ptr = static_cast<float*>(m_Ptr);
        *ptr = n;
        m_Ptr = static_cast<void*>(++ptr);
    }
    void WriteFloat3(const float* a)
    {
        memcpy(m_Ptr, a, sizeof(float) * 3);
        m_Ptr = static_cast<void*>(static_cast<float*>(m_Ptr) + 3);
    }
};

inline uint32_t Min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}
inline uint32_t Max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}
inline uint32_t Clamp(uint32_t n, uint32_t low, uint32_t high)
{
    return Min(Max(n, low), high);
}
inline uint32_t MipCount(uint32_t w, uint32_t h, uint32_t d)
{
    return static_cast<uint32_t>(log(static_cast<double>(Max(Max(w, h), d))) / log(2.0)) + 1;
}

static bool FileExists(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (file == NULL)
        return false;
    fclose(file);
    return true;
}
static bool ReadFile(const char* path, const char* mode, void** data, size_t* size)
{
    *data = NULL;
    *size = 0;

    FILE* file = fopen(path, mode);
    if (file == NULL)
        return false;

    fseek(file, 0L, SEEK_END);
    size_t alloc_size = static_cast<size_t>(ftell(file));
    fseek(file, 0L, SEEK_SET);

    *data = Alloc(alloc_size);

    uint8_t chunk[256 * 1024];
    size_t chunk_size;
    while ((chunk_size = fread(chunk, 1, sizeof(chunk), file)) > 0)
    {
        size_t new_size = *size + chunk_size;
        if (new_size > alloc_size)
        {
            *data = Realloc(*data, new_size);
            alloc_size = new_size;
        }
        memcpy(static_cast<uint8_t*>(*data) + *size, chunk, chunk_size);
        *size = new_size;
    }

    fclose(file);
    return true;
}
static bool WriteFile(const char* path, const char* mode, const void* data, size_t size)
{
    char path_buf[2048];
    strncpy(path_buf, path, sizeof(path_buf));

    char* path_curr = path_buf;
    while (path_curr = strchr(path_curr, '\\'))
    {
        *path_curr = '/';
        ++path_curr;
    }

    path_curr = path_buf;
    while (path_curr = strchr(path_curr, '/'))
    {
        *path_curr = '\0';
#ifdef _WIN32
        if (!CreateDirectory(path_buf, 0x0) && GetLastError() != ERROR_ALREADY_EXISTS)
            return false;
#else
        if (mkdir(path, 0777) != 0 && errno != EEXIST)
            return false;
#endif
        *path_curr = '/';
        ++path_curr;
    }

    FILE* file = fopen(path_buf, mode);
    if (file == NULL)
        return false;
    fwrite(data, 1, size, file);
    fclose(file);
    return true;
}

#endif
