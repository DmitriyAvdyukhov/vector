#pragma once
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <iostream>
#include <utility>

#include "array_ptr.h"

using namespace std::literals;
struct ReserveCapacity 
{
    size_t capacity;
};

template <typename Type>
class SimpleVector 
{
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // ������ ������ �� size ���������, ������������������ ��������� �� ���������
    explicit SimpleVector(size_t size) : size_(size), capacity_(size), data_(new Type[capacity_])
    {
        if (size != 0)
        {           
            std::fill(begin(), begin() + size, Type());
        }
    }    

    // ������ ������ �� size ���������, ������������������ ��������� value
    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size), data_(new Type[capacity_])
    {
        if (size != 0)
        {            
            std::fill(begin(), begin() + size, value);
        }
    }

    // ������ ������ �� std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : data_(new Type[init.size()])
    {
        if (init.size() != 0)
        {           
            std::copy(init.begin(), init.end(), data_.Get());
            size_ = init.size();
            capacity_ = init.size();          
        }        
    }

    SimpleVector(const SimpleVector& other) 
    {
        SimpleVector tmp;
        tmp.Resize(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        swap(tmp);
    }

    SimpleVector(SimpleVector&& other) noexcept
    {    
        data_ = std::move(other.data_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

     SimpleVector(const ReserveCapacity& size) : size_(0), capacity_(size.capacity), data_(new Type[size.capacity])
    {}


     SimpleVector& operator=(const SimpleVector& rhs)
     {           
         if (this != &rhs)
         {
             SimpleVector temp;
             temp.Resize(rhs.size_);
             std::copy(rhs.begin(), rhs.end(), temp.begin());
             swap(temp);
         }

        return *this;
     }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept
    {   
        if (this != &rhs)
        {
            data_ = {};
            data_ = std::move(rhs.data_);
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);
        }

        return *this;
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity > capacity_)
        {            
            ArrayPtr<Type> temp (new_capacity);
            std::copy(data_.Get(), data_.Get() + size_, temp.Get());
            data_ = std::move(temp);
            capacity_ = new_capacity;
        }
    }

    // ���������� ���������� ��������� � �������
    size_t GetSize() const noexcept
    {
        return size_;
    }

    // ���������� ����������� �������
    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    // ��������, ������ �� ������
    bool IsEmpty() const noexcept
    {
        return !size_;
    }

    // ���������� ������ �� ������� � �������� index
    Type& operator[](size_t index) noexcept
    {        
        return data_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    const Type& operator[](size_t index) const noexcept
    {       
        return data_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    Type& At(size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("Item index is out of range"s);
        }        
        return data_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    const Type& At(size_t index) const
    {
        if (index > size_)
        {
            throw std::out_of_range("Item index is out of range"s);
        }
        return data_[index];
    }

    // �������� ������ �������, �� ������� ��� �����������
    void Clear() noexcept
    {
        size_ = 0;
    }

    // �������� ������ �������.
    // ��� ���������� ������� ����� �������� �������� �������� �� ��������� ��� ���� Type
    void Resize(size_t new_size) noexcept
    {
        if (new_size <= size_)
        {
            size_ = new_size;
        }
        else
        {
            ArrayPtr<Type> new_data(new_size);
            std::copy(std::make_move_iterator(data_.Get()), std::make_move_iterator(data_.Get() + size_), new_data.Get());           
            for (auto it = new_data.Get() + size_; it != new_data.Get() + new_size; ++it)
            {
                *it = Type{};
            }
            data_ = std::move(new_data);

            size_ = new_size;
            capacity_ = new_size;
        }
    }

    // ��������� ������� � ����� �������
    // ��� �������� ����� ����������� ����� ����������� �������
    void PushBack(const Type& item) noexcept
    {
        if (size_ == capacity_)
        {
            capacity_ == 0 ? capacity_ = 1 : capacity_ *= 2;
            
            ArrayPtr<Type> temp (capacity_);           
            size_t i = 0;
            for (auto it = begin(); it != end(); ++it)
            {
                *(temp.Get() + i) = std::move(*it);
                ++i;
            }
            data_ = std::move(temp);
            data_[size_] = item;            
            ++size_;
        }
        else
        {
            data_[size_] = item;
            ++size_;
        }
    }

    void PushBack(Type&& item) noexcept
    {
        if (size_ == capacity_)
        {
             capacity_ == 0 ? capacity_ = 1 : capacity_ *= 2;
            
            ArrayPtr<Type> temp(capacity_);           
            size_t i = 0;
            for (auto it = begin(); it != end(); ++it)
            {
                *(temp.Get() + i) = std::move(*it);
                ++i;
            }            
            data_ = std::move(temp);          
            data_[size_] = std::move(item);            
            ++size_;
        }
        else
        {
            data_[size_] = std::move(item);
            ++size_;
        }
    }

    // ��������� �������� value � ������� pos.
    // ���������� �������� �� ����������� ��������
    // ���� ����� �������� �������� ������ ��� �������� ���������,
    // ����������� ������� ������ ����������� �����, � ��� ������� ������������ 0 ����� ������ 1
    Iterator Insert(ConstIterator pos,   Type&& value) 
    {     
        assert(pos >= begin() || pos < end()); 

        const size_t in = pos - begin();
        if (capacity_ > size_)
        {
            std::copy_backward(std::make_move_iterator(begin() + in), std::make_move_iterator(begin() + size_), begin() + size_ +1);
            *(begin() + in) =  std::exchange(value, 0);
            ++size_;
            
            return data_.Get() + in;
        }
        
        capacity_ == 0 ? capacity_ = 1 : capacity_ *= 2;

        ArrayPtr<Type> new_data(capacity_);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(begin() + in), new_data.Get());
        *(new_data.Get() + in) = std::exchange(value, 0);
        std::copy(std::make_move_iterator(begin() + in), std::make_move_iterator(end()), new_data.Get() + in + 1);
        data_= std::move(new_data); 

        ++size_;         

        return data_.Get() + in; 
    }

    
    // ������� ������� ������� � ��������� �������
    Iterator Erase(ConstIterator pos)
    {
        assert(size_ != 0);
        assert(pos != end() || pos != begin() - 1);       

        const size_t er = pos - begin();
        ArrayPtr<Type> temp (capacity_);          
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(begin() + er), temp.Get());
        std::copy(std::make_move_iterator(begin() + er + 1), std::make_move_iterator(begin() + size_), temp.Get() + er);
        data_ = std::move(temp);
        --size_;
        
        return begin() + er;
    }

    // "�������" ��������� ������� �������. ������ �� ������ ���� ������
    void PopBack() 
    {
        assert(size_ != 0);
        --size_;       
    }

    // ���������� �������� � ������ ��������
    void swap(SimpleVector& other) noexcept
    {
        std::swap(size_, other.size_);
        data_.swap(other.data_);
        std::swap(capacity_, other.capacity_);
    }
    // ���������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator begin() noexcept
    {
        return data_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator end() noexcept
    {
        return begin() + size_;
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator begin() const noexcept
    {
        return data_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator end() const noexcept
    {
        return begin() + size_;
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cbegin() const noexcept
    {
        return data_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cend() const noexcept
    {
        return begin() + size_;
    }

private:
    size_t size_{};
    size_t capacity_{};
    ArrayPtr<Type> data_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return std::equal(lhs.begin(), lhs.begin() + lhs.GetSize(), rhs.begin(), rhs.begin() + rhs.GetSize());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.begin() + lhs.GetSize(),
        rhs.begin(), rhs.begin() + rhs.GetSize());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return rhs >= lhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs < rhs);
}

ReserveCapacity Reserve(size_t capacity) 
{
    return { static_cast<size_t>(capacity) };
}

