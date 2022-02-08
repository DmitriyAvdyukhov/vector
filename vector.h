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
   
    static T* Allocate(size_t n)
    {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }
    
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

    explicit Vector(size_t size) :data_(size), size_(size)
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

    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept
    {
        return data_.GetAddress();
    }

    iterator end() noexcept
    {
        return begin() + size_;
    }

    const_iterator begin() const noexcept
    {
        return data_.GetAddress();
    }

    const_iterator end() const noexcept
    {
        return begin() + size_;
    }

    const_iterator cbegin() const noexcept
    {
        return data_.GetAddress();
    }

    const_iterator cend() const noexcept
    {
        return begin() + size_;
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

    void PushBack(T&& value)
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
        std::destroy_at(data_.GetAddress() + size_ - 1);
        --size_;
    }

    template<typename ... Args>
    T& EmplaceBack(Args&&... args)
    {         
        if (Size() == Capacity())
        {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            new(new_data.GetAddress() + size_) T(std::forward<Args>(args)...);
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
            new(data_.GetAddress() + size_) T(std::forward<Args>(args)...);
        }
        ++size_;
        return data_[size_ - 1];
    }

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args)
    {
        iterator pos_emplace = const_cast<iterator>(pos); 
        size_t dis = std::distance(begin(), pos_emplace);
        iterator it_value = &data_[dis];
        if (Size() == Capacity())
        {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);            
            try
            {
                new(new_data.GetAddress() + dis) T(std::forward<Args>(args)...);
                if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
                {
                    std::uninitialized_move_n(data_.GetAddress(), dis , new_data.GetAddress());
                    std::uninitialized_move_n(data_.GetAddress() + dis, size_ - dis, new_data.GetAddress() + (dis + 1));
                }
                else
                {
                    std::uninitialized_copy_n(data_.GetAddress(), dis, new_data.GetAddress());
                    std::uninitialized_copy_n(data_.GetAddress() + dis, size_ - dis, new_data.GetAddress() + (dis + 1));
                }
            }
            catch(...)
            {
                new_data.~RawMemory();
            }
            
            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
            it_value = &data_[dis];
        }
        else
        {
            if (pos_emplace == end() || pos_emplace == nullptr)
            {
                new(end()) T(std::forward<Args>(args)...);
            }
            else
            {
                new(end()) T(std::forward<T>(*(end() - 1)));
                std::move_backward(it_value, end() - 1, end());
                *it_value = T(std::forward<Args>(args)...);
            }           
        }
        ++size_;
        return it_value;
    }

    iterator Erase(const_iterator pos) /*noexcept(std::is_nothrow_move_assignable_v<T>)*/
    {
        assert(size_ != 0);
        iterator pos_erase = const_cast<iterator>(pos);
        std::destroy_at(pos_erase);
        std::move(pos_erase + 1, end(), pos_erase);
        --size_;
        return pos_erase;
    }

    iterator Insert(const_iterator pos, const T& value)
    {        
        return Emplace(pos, value);
    }

    iterator Insert(const_iterator pos, T&& value)
    {
        return Emplace(pos, std::move(value));
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
        return data_[index];
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};
