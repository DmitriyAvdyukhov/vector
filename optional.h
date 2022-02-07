#pragma once
#include <stdexcept>
#include <utility>

using namespace std::literals;
// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional 
{
public:
    Optional() = default;
    Optional(const T& value);
    Optional(T&& value);
    Optional(const Optional& other);   
    Optional(Optional&& other) noexcept;
    Optional& operator=(const T& value);   
    Optional& operator=(T&& rhs);    
    Optional& operator=(const Optional& rhs);    
    Optional& operator=(Optional&& rhs) noexcept;
    ~Optional();
    bool HasValue() const;    

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*();   
    const T& operator*() const;    
    T* operator->();    
    const T* operator->() const;   

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value();    
    const T& Value() const;   

    void Reset();
    
    template<typename ... Args>
    void Emplace(Args&&... args)
    {
        if (HasValue())
        {
           Reset();
        }
        value_ = new(&buf_[0]) T(std::forward<Args>(args) ...);
        is_initialized_ = true;
    }
    
private:
    // alignas нужен для правильного выравнивания блока памяти    
    alignas(T) char buf_[(sizeof(T))];
    T* value_ =  nullptr;
    bool is_initialized_ = false;
};

template<typename T>
inline Optional<T>::Optional(const T& value)
{
    value_ = new(&buf_[0]) T(value);
    is_initialized_ = true;
}

template<typename T>
inline Optional<T>::Optional(T&& value)
{
    value_ = new(&buf_[0]) T(std::move(value));
    is_initialized_ = true;
}

template<typename T>
inline Optional<T>::Optional(const Optional& other)
{
    if (this->HasValue() && other.HasValue())
    {
        *value_ = other.Value();
        is_initialized_ = true;
    }
    else if (!this->HasValue() && other.HasValue())
    {
        value_ = new(&buf_[0]) T(other.Value());
        is_initialized_ = true;
    }
    else if (!this->HasValue() && !other.HasValue())
    {
        Reset();
    }
    else if (this->HasValue() && !other.HasValue())
    {
        Reset();
    }

}

template<typename T>
inline Optional<T>::Optional(Optional&& other) noexcept
{
    if (this->HasValue() && other.HasValue())
    {
        *value_ = std::move(other.Value());
        is_initialized_ = true;
    }
    else if (!this->HasValue() && other.HasValue())
    {
        value_ = new(&buf_[0]) T(std::move(other.Value()));
        is_initialized_ = true;
    }
    else if (!this->HasValue() && !other.HasValue())
    {
        Reset();
    }
    else if (this->HasValue() && !other.HasValue())
    {
        Reset();
    }
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(const T& value)
{
    if (HasValue())
    {
        *value_ = value;
    }
    else
    {
        value_ = new(&buf_[0]) T(value);
        is_initialized_ = true;
    }
    return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(T&& rhs)
{
    if (HasValue())
    {
        *value_ = std::move(rhs);
    }
    else
    {
        value_ = new(&buf_[0]) T(std::move(rhs));
        is_initialized_ = true;
    }
    return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(const Optional& rhs)
{
    if (this->HasValue() && rhs.HasValue())
    {
        *value_ = rhs.Value();
        is_initialized_ = true;
    }
    else if (!this->HasValue() && rhs.HasValue())
    {
        value_ = new(&buf_[0]) T(rhs.Value());
        is_initialized_ = true;
    }
    else if (this->HasValue() && !rhs.HasValue())
    {
        Reset();
    }

    return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(Optional&& rhs) noexcept
{
    if (this->HasValue() && rhs.HasValue())
    {
        *value_ = std::move(rhs.Value());
        is_initialized_ = true;
    }
    else if (!this->HasValue() && rhs.HasValue())
    {
        value_ = new(&buf_[0]) T(std::move(rhs.Value()));
        is_initialized_ = true;
    }
    else if (this->HasValue() && !rhs.HasValue())
    {
        Reset();
    }

    return *this;
}

template<typename T>
inline Optional<T>::~Optional()
{
    if (is_initialized_)
    {
        value_->~T();
    }    
}

template<typename T>
inline bool Optional<T>::HasValue() const
{
    return is_initialized_;
}

template<typename T>
inline T& Optional<T>::operator*()
{
    return *value_;
}

template<typename T>
inline const T& Optional<T>::operator*() const
{
    return *value_;
}

template<typename T>
inline T* Optional<T>::operator->()
{
    return &Value();
}

template<typename T>
inline const T* Optional<T>::operator->() const
{
    return &Value();
}

template<typename T>
inline T& Optional<T>::Value()
{
    if (!is_initialized_)
    {
        throw BadOptionalAccess();
    }
    return *value_;
}

template<typename T>
inline const T& Optional<T>::Value() const
{
    if (!is_initialized_)
    {
        throw BadOptionalAccess();
    }
    return *value_;
}

template<typename T>
inline void Optional<T>::Reset()
{
    if (is_initialized_)
    {
        value_->~T();
        is_initialized_ = false;
    }
}

