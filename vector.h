#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
<<<<<<< Updated upstream
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
=======

template <typename T>
class Vector
{
private:
    static T* Allocate(size_t size)
    {
        return size != 0 ? static_cast<T*>(operator new(size * sizeof(T))) : nullptr;
    }

    static void Deallocate(T* buf) noexcept
>>>>>>> Stashed changes
    {
        operator delete(buf);
    }

<<<<<<< Updated upstream
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
                std::destroy_n(data_.GetAddress() + size, Size() - size);
                size_ = size;
            }
            else
            {
                Reserve(size);
                std::uninitialized_value_construct_n(data_.GetAddress() + Size(), size - Size());
                size_ = size;
            }
        }
    }

    void PushBack(const T& value)
    {
        if (Size() == Capacity())
        {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            new(new_data.GetAddress() + size_) T(value);
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
        else
        {
            new(data_.GetAddress() + size_) T(value);
        }
        ++size_;
    }

    void PushBack(T&& value) noexcept
    {
        if (Size() == Capacity())
        {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            new(new_data.GetAddress() + size_) T(std::move(value));            
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
        else
        {
            new(data_.GetAddress() + size_) T(std::move(value));
        }
        ++size_;
    }

    void PopBack()
    {
        assert(size_ != 0);
        --size_;
        std::destroy_at(data_.GetAddress() + size_);           
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
=======
    static void DestroyN(T* buf, size_t n)
    {
        for (size_t i = 0; i < n; ++i)
        {
            Destroy(buf + i);
        }
    }

    static void Destroy(T* buf)
    {
        buf->~T();
    }

    static void CopyConstruct(T* buf, const T& elem)
    {
        new(buf) T(elem);
    }

public:

    Vector() = default;

    explicit Vector(size_t size):data_(Allocate(size)), capacity_(size), size_(size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            new(data_ + i) T();
        }
    }

    explicit Vector(const Vector& v) : data_(Allocate(v.Size())), capacity_(v.Capacity()), size_(v.Size())
    {
        for (size_t i = 0; i < size_; ++i)
        {
            CopyConstruct(data_ + i, v[i]);
        }
    }

    ~Vector()
    {
        DestroyN(data_, size_);
        Deallocate(data_);
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity <= capacity_)
        {
            return;
        }
        
        T* new_data = Allocate(new_capacity);
        for (size_t i = 0; i != size_; ++i)
        {
            CopyConstruct(new_data + i, data_[i]);
        }
        DestroyN(data_, size_);
        Deallocate(data_);
         std::swap(data_, new_data);
        std::swap(capacity_, new_capacity);       
>>>>>>> Stashed changes
    }

    size_t Size() const noexcept 
    {
        return size_;
    }

    size_t Capacity() const noexcept 
    {
<<<<<<< Updated upstream
        return data_.Capacity();
=======
        return capacity_;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    RawMemory<T> data_;
    size_t size_ = 0;
};
=======
    T* data_ = nullptr;
    size_t capacity_ = 0;
    size_t size_ = 0;
};

>>>>>>> Stashed changes
