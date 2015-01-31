#pragma once

#include <forward_list>

template <typename T>
class ObjectStorage
{
    typedef std::unique_ptr<T> TPtr;
    
public:
    ObjectStorage()
    {}
    
    T* NewObject()
    {
        TPtr ptr(new T());
        return Store(std::move(ptr));
    }
    template <typename P>
    T* NewObject(const P& p)
    {
        TPtr ptr(new T(p));
        return Store(std::move(ptr));
    }
    template <typename P1, typename P2>
    T* NewObject(const P1& p1, const P2& p2)
    {
        TPtr ptr(new T(p1, p2));
        return Store(std::move(ptr));
    }
    
    void swap(ObjectStorage& other)
    {
        m_objects.swap(other.m_objects);
    }
    
private:
    T* Store(TPtr&& ptr)
    {
        m_objects.push_front(std::move(ptr));
        return m_objects.front().get();
    }
    
    ObjectStorage(const ObjectStorage&);
    ObjectStorage& operator=(const ObjectStorage&);
    
private:
    std::forward_list<TPtr> m_objects;
};
