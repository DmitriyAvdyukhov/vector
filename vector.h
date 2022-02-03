#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>

template <typename T>
class RawMemory 
{
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
        : buffer_(Allocate(capacity))
        , capacity_(capacity) 
    {}

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory(RawMemory&& other) noexcept 
    {
        if (&buffer_ != &other.buffer_)
        {
            Swap(other);
        }
    }
    RawMemory& operator=(RawMemory&& rhs) noexcept 
    {
        if (&buffer_ != &rhs.buffer_)
        {
            Swap(rhs);
        }
        return *this;
    }
    ~RawMemory() 
    {
        if (buffer_ != nullptr)
        {
            Deallocate(buffer_);
        }
    }

    T* operator+(size_t offset) noexcept 
    {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T* operator+(size_t offset) const noexcept
    {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const T& operator[](size_t index) const noexcept 
    {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept
    {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept
    {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept
    {
        return buffer_;
    }

    T* GetAddress() noexcept 
    {
        return buffer_;
    }

    size_t Capacity() const
    {
        return capacity_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) 
    {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept 
    {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector
{
public:

    Vector() = default;

    explicit Vector(size_t size):data_(size), size_(size)
    {      
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }

    Vector(const Vector& other)
    {       
        RawMemory<T>  new_data(other.size_);       
        std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, new_data.GetAddress());
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
        size_ = other.size_;         
    }    

    Vector(Vector&& other) noexcept
    {
        if (this != &other)
        {
            Swap(other);
        }
    }

    ~Vector()
    {
        std::destroy_n(data_.GetAddress(), size_);
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity <= data_.Capacity())
        {
            return;
        }        
        RawMemory<T> new_data(new_capacity);
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        else
        {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);      
    }

    void Resize(size_t size)
    {
        if (Size() > size)
        {
            std::destroy_n(data_.GetAddress() + size, Size() - size);
            size_ = size;
        }
        else
        {
            if (Capacity() > size)
            {

            }
            else
            {
                Reserve(size);
                std::uninitialized_value_construct_n(data_.GetAddress() + Size(), size - Size());
                size_ = size;
            }
        }
    }

    Vector& operator=(const Vector& rhs)
    {
        if (this != &rhs)
        {
            if (rhs.size_ > data_.Capacity())
            {                
                Vector rhs_copy(rhs);
                Swap(rhs_copy);
            }
            else
            {   
                for (size_t i = 0; i < std::min(Size(), rhs.Size()); ++i) 
                {
                    data_[i] = rhs[i];
                }
                if (Size() > rhs.Size())
                {
                    std::destroy_n(data_ + rhs.Size(), Size() - rhs.Size());
                }
                else if (Size() < rhs.Size())
                { 
                    std::uninitialized_copy_n(rhs.data_.GetAddress() + Size(), rhs.Size() - Size(), data_.GetAddress() + Size());
                }
            }
        }
        size_ = rhs.size_;
        return *this;
    }

    Vector& operator=(Vector&& rhs)
    {
        if (this != &rhs)
        {
            Swap(rhs);
        }
        return *this;
    }

    void Swap(Vector& rhs)
    {
        data_.Swap(rhs.data_);
        std::swap(size_, rhs.size_);
    }

    size_t Size() const noexcept 
    {
        return size_;
    }

    size_t Capacity() const noexcept 
    {
        return data_.Capacity();
    }

    const T& operator[](size_t index) const noexcept
    {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept
    {
        assert(index < size_);
        return data_[index];
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};
