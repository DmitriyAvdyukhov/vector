#pragma once
#include <cassert>
#include <cstdlib>

template <typename Type>
class ArrayPtr 
{
public:
    // �������������� ArrayPtr ������� ����������
    ArrayPtr() = default;

    // ������ � ���� ������ �� size ��������� ���� Type.
    // ���� size == 0, ���� raw_ptr_ ������ ���� ����� nullptr
    explicit ArrayPtr(size_t size) noexcept
    {
        if (size == 0)
        {
            raw_ptr_ = nullptr;
        }
        else
        {
            raw_ptr_ = new Type[size];
        }
    }

    // ����������� �� ������ ���������, ��������� ����� ������� � ���� ���� nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept
    {
        raw_ptr_ = raw_ptr;
    }

    // ��������� �����������
    ArrayPtr(const ArrayPtr&) = delete;
    ArrayPtr(ArrayPtr&& other) noexcept
    {     
        if (this != &other)
        {
            raw_ptr_ = std::move(other.raw_ptr_);
            other.raw_ptr_ = {};
        }
    }

    ~ArrayPtr()
    {
        delete[] raw_ptr_;
    }

    // ��������� ������������
    ArrayPtr& operator=(const ArrayPtr&) = delete;
    ArrayPtr& operator=(ArrayPtr&& rhs) noexcept
    {
        if (this != &rhs)
        {
            raw_ptr_ = {};
            raw_ptr_ = std::move(rhs.raw_ptr_);
            rhs.raw_ptr_ = {};
        }

        return *this;
    }

    // ���������� ��������� �������� � ������, ���������� �������� ������ �������
    // ����� ������ ������ ��������� �� ������ ������ ����������
    [[nodiscard]] Type* Release() noexcept
    {
        Type* temp_ptr = raw_ptr_;
        raw_ptr_ = nullptr;

        return temp_ptr;
    }

    // ���������� ������ �� ������� ������� � �������� index
    Type& operator[](size_t index) noexcept
    {
        return raw_ptr_ [index];
    }

    // ���������� ����������� ������ �� ������� ������� � �������� index
    const Type& operator[](size_t index) const noexcept
    {
        return raw_ptr_ [index];
    }

    // ���������� true, ���� ��������� ���������, � false � ��������� ������
    explicit operator bool() const noexcept
    {
        return raw_ptr_ != nullptr;
    }

    // ���������� �������� ������ ���������, ��������� ����� ������ �������
    Type* Get() const noexcept
    {
        return raw_ptr_;
    }

    // ������������ ��������� ��������� �� ������ � �������� other
    void swap(ArrayPtr& other) noexcept
    {     
        if (this != &other)
        {
            Type* temp = std::move(other.raw_ptr_);
            other.raw_ptr_ = nullptr;
            other.raw_ptr_ = std::move(raw_ptr_);
            raw_ptr_ = nullptr;
            raw_ptr_ = std::move(temp);
            temp = nullptr;
        }
    }

private:
    Type* raw_ptr_ = nullptr;
};