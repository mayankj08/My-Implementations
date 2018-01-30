#ifndef CS540_SMART_PTR_HPP
#define CS540_SMART_PTR_HPP

#include<mutex>
#include<algorithm>
#include<iostream>
#include<atomic>
#include<thread>

namespace cs540 
{

struct delete_deleter_base 
{
    virtual void deleter()=0;
    virtual void destroy()=0;
    virtual ~delete_deleter_base()
    {}
};

template <typename T>
struct delete_deleter : public delete_deleter_base 
{
    void deleter()
    { 
        if(pointerToDelete != nullptr)
        {
            delete pointerToDelete; 
            pointerToDelete = nullptr;   
        }
       
    }

    void destroy()
    { 
        delete this; 
    } 
    
    static delete_deleter<T>* get(T* p)
    { 
        return new delete_deleter<T>(p);
    } 

private:
    delete_deleter(T* p):pointerToDelete(p)
    {}
    
    T* pointerToDelete;
};

struct simple_dealloc 
{
    template<typename T> 
    struct type : public delete_deleter<T> 
    {
        typedef delete_deleter<T> name;
    };
};

struct counted_ref 
{

    counted_ref():refCounter(0)
    {}
    
    counted_ref(const counted_ref& r):refCounter(r.refCounter)
    { 
        if (refCounter) 
            ++refCounter->counter;
    }

    template<typename S, typename D>
    explicit counted_ref(S* p, D d):
        refCounter(new impl(p,d))
    {}
    
    ~counted_ref()
    { 
        release(); 
    }
    
    void release() 
    {
        static std::mutex mtx;
        mtx.lock();

        if (refCounter && ((--refCounter->counter) == 0))
        {
            refCounter->dellocatorObj->deleter();
            if(refCounter)
            {
                delete refCounter;
                refCounter=nullptr;
            }
        }

        mtx.unlock();
    }

    void swap(counted_ref& r)
    { 
        std::swap(refCounter, r.refCounter); 
    }
    
    counted_ref& operator=(const counted_ref& r) 
    {
        impl* tmp = r.refCounter;
        if (tmp != refCounter) 
        {
            if (tmp) (++tmp->counter);
                release();
            refCounter = tmp;
        }
        return *this;
    }

private:
    struct impl 
    {
        template<typename S, typename D> 
        explicit impl(S* p, D)
            :counter(1),dellocatorObj(D::template type<S>::name::get(p))
        {}
        
        ~impl()
        { 
            dellocatorObj->destroy(); 
        }
        
        std::atomic<int> counter;
        delete_deleter_base* dellocatorObj;

    impl& operator=(const impl&)
    { 
        return *this; 
    }
};

    impl*  refCounter;
};

template<typename T> struct SharedPtr 
{
    typedef T element_type;

    // default constructor
    SharedPtr():refPointer(0),refObject()
    {}

    SharedPtr(const SharedPtr<T> & s):refPointer(s.refPointer),refObject(s.refObject)
    {}
    
    template<typename S>
    SharedPtr(const SharedPtr<S>& s):refPointer(s.refPointer),refObject(s.refObject)
    {}

    template<typename S>
    SharedPtr(const SharedPtr<S>& s,T* p):refPointer(p),refObject(s.refObject)
    {}

    template<typename S>
    explicit SharedPtr(S* p):refPointer(p),refObject(p,simple_dealloc())
    {}

    ~SharedPtr()
    {}

    void reset()
    { 
        SharedPtr<T> ().swap(*this);
    }
    
    template<typename S> 
    void reset(S* p)
    { 
        SharedPtr<T> (p).swap(*this);  
    }
    
    
    void swap(SharedPtr<T> & other) 
    {
        std::swap(refPointer, other.refPointer);
        refObject.swap(other.refObject);
    }

    SharedPtr<T> & operator=(const SharedPtr<T> & s) 
    {
        refPointer = s.refPointer;
        refObject = s.refObject;
        return *this;
    }
    
    template<typename S>
    SharedPtr<T> & operator=(const SharedPtr<S>& s) 
    {
        refPointer = s.refPointer;
        refObject = s.refObject;
        return *this;
    }

    T& operator*()const
    { 
        return *get(); 
    }
    
    T* operator->()const
    { 
        return get(); 
    }
    
    T* get()const
    { 
        return refPointer; 
    }

    // operators
    explicit operator bool()const
    { 
        return !!refPointer; 
    }
    

private:
    
    template<typename S> friend struct SharedPtr;
    T* refPointer;
    counted_ref refObject;
};

template<typename T, typename U>
inline bool operator==(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) 
{
    return lhs.get() == rhs.get();
}

template<typename T>
inline bool operator==(const SharedPtr<T>& lhs, std::nullptr_t) 
{
    if(lhs.get())
        return true;
    return false;
}

template<typename T>
inline bool operator==(std::nullptr_t,const SharedPtr<T>& rhs) 
{
    if(rhs.get())
        return true;
    return false;
}



template<typename T, typename U>
inline bool operator!=(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) 
{
    return lhs.get() != rhs.get();
}

template<typename T>
inline bool operator!=(const SharedPtr<T>& lhs, std::nullptr_t) 
{
    if(lhs.get())
        return false;
    return true;
}

template<typename T>
inline bool operator!=(std::nullptr_t,const SharedPtr<T>& rhs) 
{
    if(rhs.get())
        return false;
    return true;
}


template<typename T, typename U>
SharedPtr<T> static_pointer_cast(const SharedPtr<U>& ptr)
{
    return SharedPtr<T>(ptr, static_cast<typename SharedPtr<T>::element_type*>(ptr.get()));
}

// dynamic cast of SharedPtr
template<typename T, typename U>
SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U>& ptr)
{
    T* p = dynamic_cast<typename SharedPtr<T>::element_type*>(ptr.get());
    if (NULL != p)
    {
        return SharedPtr<T>(ptr, p);
    }
    else
    {
        return SharedPtr<T>();
    }
}

}

#endif
