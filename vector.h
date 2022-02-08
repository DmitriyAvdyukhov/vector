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

    explicit RawMemory(size_t capacity);       

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;

    RawMemory(RawMemory&& other) noexcept;  
    RawMemory& operator=(RawMemory&& rhs) noexcept;    

    ~RawMemory() noexcept;   

    T* operator+(size_t offset) noexcept;
    const T* operator+(size_t offset) const noexcept; 
    const T& operator[](size_t index) const noexcept; 
    T& operator[](size_t index) noexcept;   

    void Swap(RawMemory& other) noexcept;    

    const T* GetAddress() const noexcept;    

    T* GetAddress() noexcept;    

    size_t Capacity() const;   

private:
   
    static T* Allocate(size_t n) noexcept;      
    static void Deallocate(T* buf) noexcept;  

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector
{
public:

    Vector() = default;
    explicit Vector(size_t size);

    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;    

    ~Vector() noexcept;
    

    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;  

    void Reserve(size_t new_capacity) noexcept;    

    void Resize(size_t size) noexcept;    

    void PushBack(const T& value) noexcept;    

    void PushBack(T&& value) noexcept;   

    void PopBack();

    template<typename ... Args>
    T& EmplaceBack(Args&&... args) noexcept;   

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args);    

    iterator Erase(const_iterator pos);    

    iterator Insert(const_iterator pos, const T& value);    

    iterator Insert(const_iterator pos, T&& value);    

    Vector& operator=(const Vector& rhs) noexcept;
    Vector& operator=(Vector&& rhs) noexcept;    

    void Swap(Vector& rhs) noexcept;   

    size_t Size() const noexcept;  

    size_t Capacity() const noexcept;    

    const T& operator[](size_t index) const noexcept;   

    T& operator[](size_t index) noexcept;
    

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};

//----------------------------RawMemory------------------------------------------------
//------Costructer and destructor-----

template<typename T>
inline RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity))
    , capacity_(capacity)
{}

template<typename T>
inline RawMemory<T>::RawMemory(RawMemory && other) noexcept
{
    if (&buffer_ != &other.buffer_)
    {
        Swap(other);
    }
}

template<typename T>
inline RawMemory<T>::~RawMemory() noexcept
{
    if (buffer_ != nullptr)
    {
        Deallocate(buffer_);
    }
}

//------------Methods----------------

template<typename T>
inline void RawMemory<T>::Swap(RawMemory& other) noexcept
{
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template<typename T>
inline const T* RawMemory<T>::GetAddress() const noexcept
{
    return buffer_;
}

template<typename T>
inline T* RawMemory<T>::GetAddress() noexcept
{
    return buffer_;
}

template<typename T>
inline size_t RawMemory<T>::Capacity() const
{
    return capacity_;
}

template<typename T>
inline T* RawMemory<T>::Allocate(size_t n) noexcept
{
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

template<typename T>
inline void RawMemory<T>::Deallocate(T* buf) noexcept
{
    operator delete(buf);
}

//--------Operators-------

template<typename T>
inline RawMemory<T>& RawMemory<T>::operator=(RawMemory&& rhs) noexcept
{
    if (&buffer_ != &rhs.buffer_)
    {
        Swap(rhs);
    }
    return *this;
}

template<typename T>
inline T* RawMemory<T>::operator+(size_t offset) noexcept
{
    assert(offset <= capacity_);
    return buffer_ + offset;
}

template<typename T>
inline const T* RawMemory<T>::operator+(size_t offset) const noexcept
{
    return const_cast<RawMemory&>(*this) + offset;
}

template<typename T>
inline const T& RawMemory<T>::operator[](size_t index) const noexcept
{
    return const_cast<RawMemory&>(*this)[index];
}

template<typename T>
inline T& RawMemory<T>::operator[](size_t index) noexcept
{
    return buffer_[index];
}

//---------------------------------------Vector-----------------------------
//------Costructer and destructor-----

template<typename T>
inline Vector<T>::Vector(size_t size) : data_(size), size_(size)
{
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template<typename T>
inline Vector<T>::Vector(const Vector& other)
{
    RawMemory<T>  new_data(other.size_);
    std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, new_data.GetAddress());
    std::destroy_n(data_.GetAddress(), size_);
    data_.Swap(new_data);
    size_ = other.size_;
}

template<typename T>
inline Vector<T>::Vector(Vector&& other) noexcept
{
    if (this != &other)
    {
        Swap(other);
    }
}

template<typename T>
inline Vector<T>::~Vector() noexcept
{
    std::destroy_n(data_.GetAddress(), size_);
}

//-----------Iterators--------

template<typename T>
inline T* Vector<T>::begin() noexcept
{
    return data_.GetAddress();
}

template<typename T>
inline T* Vector<T>::end() noexcept
{
    return begin() + size_;
}

template<typename T>
inline const T* Vector<T>::begin() const noexcept
{
    return data_.GetAddress();
}

template<typename T>
inline const T* Vector<T>::end() const noexcept
{
    return begin() + size_;
}

template<typename T>
inline const T* Vector<T>::cbegin() const noexcept
{
    return data_.GetAddress();
}

template<typename T>
inline const T* Vector<T>::cend() const noexcept
{
    return begin() + size_;
}

//------------Methods--------------

template<typename T>
inline void Vector<T>::Reserve(size_t new_capacity) noexcept
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

template<typename T>
inline void Vector<T>::Resize(size_t size) noexcept
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

template<typename T>
inline void Vector<T>::PushBack(const T& value) noexcept
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

template<typename T>
inline void Vector<T>::PushBack(T&& value) noexcept
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

template<typename T>
inline void Vector<T>::PopBack()
{
    assert(size_ != 0);
    std::destroy_at(data_.GetAddress() + size_ - 1);
    --size_;
}

template<typename T>
template<typename ...Args>
inline T& Vector<T>::EmplaceBack(Args && ...args) noexcept
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

template<typename T>
template<typename ...Args>
inline T* Vector<T>::Emplace(const_iterator pos, Args && ...args)
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
                std::uninitialized_move_n(data_.GetAddress(), dis, new_data.GetAddress());
                std::uninitialized_move_n(data_.GetAddress() + dis, size_ - dis, new_data.GetAddress() + (dis + 1));
            }
            else
            {
                std::uninitialized_copy_n(data_.GetAddress(), dis, new_data.GetAddress());
                std::uninitialized_copy_n(data_.GetAddress() + dis, size_ - dis, new_data.GetAddress() + (dis + 1));
            }
        }
        catch (...)
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

template<typename T>
inline T* Vector<T>::Erase(const_iterator pos)
{
    assert(size_ != 0);
    iterator pos_erase = const_cast<iterator>(pos);
    std::destroy_at(pos_erase);
    std::move(pos_erase + 1, end(), pos_erase);
    --size_;
    return pos_erase;
}

template<typename T>
inline T* Vector<T>::Insert(const_iterator pos, const T& value)
{
    return Emplace(pos, value);
}

template<typename T>
inline T* Vector<T>::Insert(const_iterator pos, T&& value)
{
    return Emplace(pos, std::move(value));
}

template<typename T>
inline void Vector<T>::Swap(Vector& rhs) noexcept
{
    data_.Swap(rhs.data_);
    std::swap(size_, rhs.size_);
}

template<typename T>
inline size_t Vector<T>::Size() const noexcept
{
    return size_;
}

template<typename T>
inline size_t Vector<T>::Capacity() const noexcept
{
    return data_.Capacity();
}

//------------Operators-------------

template<typename T>
inline Vector<T>& Vector<T>::operator=(const Vector& rhs) noexcept
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

template<typename T>
inline Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept
{
    if (this != &rhs)
    {
        Swap(rhs);
    }
    return *this;
}

template<typename T>
inline const T& Vector<T>::operator[](size_t index) const noexcept
{
    return const_cast<Vector&>(*this)[index];
}

template<typename T>
inline T& Vector<T>::operator[](size_t index) noexcept
{
    return data_[index];
}


