#ifndef X_STD_MEMORY2_H
#define X_STD_MEMORY2_H


// ********** THIS FILE IS UNDER CONSTRUCTION **********

///////////////////////////////////////////////////////////////////////////////
//
// Non-standard extensions based on STL <memory>
//
// This declares numerous templates for additional allocator and memory-related
// classes and functions beyond those provided in x_memory.h.
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_memory.h"

namespace x_std 
{
    //todo:Find a better name
    ///////////////////////////////////////////////////////////////////////////
    //
    // reference_allocator<T>
    //
    // This template class defines an allocator which can be used in the
    // container classes (set<>, list<>, and so on) but which indirectly
    // references another allocator.
    // 
    // This allows a single allocator object to be shared in multiple containers.
    //
    ///////////////////////////////////////////////////////////////////////////
    //template
    //< 
    //    class T             , 
    //    class BaseAllocator
    //>
    //class reference_allocator
    //{
    //  public :
    //    typedef typename BaseAllocator::size_type           size_type       ;
    //    typedef typename BaseAllocator::difference_type     difference_type ;
    //    typedef typename BaseAllocator::pointer             pointer         ;
    //    typedef typename BaseAllocator::const_pointer       const_pointer   ;
    //    typedef typename BaseAllocator::smart_ptr           smart_ptr       ;
    //    typedef typename BaseAllocator::const_reference     const_reference ;
    //    typedef typename BaseAllocator::value_type          value_type      ;
    //
    //    pointer         address( smart_ptr       x ) const { return BaseAllocator::address(x) ; }
    //    const_pointer   address( const_reference x ) const { return BaseAllocator::address(x) ; }
    //
    //    reference_allocator() {}
    //    reference_allocator<T> & operator=( const reference_allocator<T> ) { return *this; }
    //    x_bool         operator==( const reference_allocator<T>&){ return true; } //Used by container's swap() function
    //
    //    pointer allocate    ( size_type n , const void * /*hint*/= 0 ) { return reinterpret_cast<pointer>(new char[n*sizeof(T)]); }
    //    void    deallocate  ( pointer   p , size_type /* n */        ) { delete[] reinterpret_cast<char *>(p);  }
    //    void    construct   ( pointer   p                            ) { placement_new<T>( p );                 }
    //    void    construct   ( pointer   p , const T & value          ) { placement_new<T>( p , value );         }
    //    void    destroy     ( pointer   p                            ) { pointer q; q = p; q->T::~T();          }
    //
    //    size_type max_size() const { return 1000000000 ; }
    //
    //    // The rebind template class, by which container classes (such as set<> or list<>)
    //    // can use a supplied allocator to allocate types other than those passed 
    //    // as template arguments.
    //    template < class U >
    //    struct rebind 
    //    {
    //        typedef BaseAllocator<U> other;
    //    };
    //
    //  private :
    //};


    ///////////////////////////////////////////////////////////////////////////
    //
    // smart_ptr<T>
    // const_smart_ptr<T>
    //
    // The class describes an object that stores a pointer to an allocated 
    // object of type T. The stored pointer must either be null or designate an 
    // object allocated by a new expression. The object of type T must be
    // derived from the referenced class.
    //
    // Unlike auto_ptr<T>, smart_ptr<T> uses a true reference counting system, so
    // a smart_ptr<T> object may be passed around safely, ensuring that what it
    // points to is a valid instance.  The last smart_ptr<T> object holding a
    // pointer to the object deletes the object when it is destructed.
    //
    ///////////////////////////////////////////////////////////////////////////
    //
    // Deviation from standard STL behavior
    //
    //    N/A.  This class is not part of the STL.
    //
    ///////////////////////////////////////////////////////////////////////////
    //
    // Performance
    // 
    //   pointer        O(1)
    //   const_pointer  O(1)
    //   operator->     O(1)
    //   operator*      O(1)
    //   operator=      O(1)
    //   operator==     O(1)
    //   operator!=     O(1)
    //
    ///////////////////////////////////////////////////////////////////////////
    //
    // Usage
    //
    // class Foobar : public x_std::referenced_object
    // {
    //     public:
    //     typedef x_std::smart_ptr<Foobar>       Ptr;
    //     typedef x_std::const_smart_ptr<Foobar> ConstPtr;
    //
    //     void blah();
    // };
    //
    // void main()
    // {
    //     Foobar::Ptr spFoobar = new Foobar;
    //    
    //     spFoobar->blah();
    // }
    //
    ///////////////////////////////////////////////////////////////////////////

    class referenced_object
    {
    ///////////////////////////////////////////////////////////////////////////
    //  This interface shouldn't be used directly unless you are very sure of
    //  of what you are doing.  Usually only smart_ptr<T> will use this interface
    public:
        referenced_object() : m_referenceCount(0) {} 
        virtual ~referenced_object() {}

        void    increment_reference() const;
        void    decrement_reference() const;
        s32     get_reference_count() const;
    private:
        mutable s32 m_referenceCount;
    };

    inline void referenced_object::increment_reference() const
    {
        ++m_referenceCount;
    }

    inline void referenced_object::decrement_reference() const 
    {
        --m_referenceCount;
    }

    inline s32 referenced_object::get_reference_count() const
    {
        return m_referenceCount;
    }

    template <class T> 
    class smart_ptr
    {
        public:
            typedef T element_type;

            smart_ptr();
            smart_ptr(const smart_ptr<T>&);
            smart_ptr(const T*);
            ~smart_ptr();     

            T*              pointer();
            const T*        const_pointer() const;
                            
            T*              operator->();
            const T*        operator->() const;
            T&              operator*();
            const T&        operator*() const;

            smart_ptr<T>&   operator=(const T*);
            smart_ptr<T>&   operator=(const smart_ptr<T>&);
            x_bool          operator==(const smart_ptr<T>&) const;
            x_bool          operator!=(const smart_ptr<T>&) const;
        private:
            void            attach(const T*);
            void            detach();

            T*              m_referencedObject;
    };

    template <class T> 
    class const_smart_ptr
    {
        public:
            typedef T element_type;

            const_smart_ptr();
            const_smart_ptr(const smart_ptr<T>&);
            const_smart_ptr(const const_smart_ptr<T>&);
            const_smart_ptr(const T*);
            ~const_smart_ptr();     

            const T*            const_pointer() const;                            
            const T*            operator->() const;
            const T&            operator*() const;

            const_smart_ptr<T>& operator=(const T*);
            const_smart_ptr<T>& operator=(const const_smart_ptr<T>&);
            const_smart_ptr<T>& operator=(const smart_ptr<T>&);
            x_bool              operator==(const const_smart_ptr<T>&) const;
            x_bool              operator!=(const const_smart_ptr<T>&) const;
            x_bool              operator==(const smart_ptr<T>&) const;
            x_bool              operator!=(const smart_ptr<T>&) const;

        private:              
            void                attach(const T*);
            void                detach();

            T*              m_referencedObject;
    };

    /////////////////
    // Smart Pointer
    template <class T>
    smart_ptr<T>::smart_ptr()
    : m_referencedObject(NULL)
    {
    }

    template <class T>
    smart_ptr<T>::smart_ptr(const smart_ptr<T> &obj)
    : m_referencedObject(NULL)
    {
        attach(obj.m_referencedObject);
    }

    template <class T>
    smart_ptr<T>::smart_ptr(const T *obj)
    : m_referencedObject(NULL)
    {
        attach(obj); 
    }

    template <class T>
    smart_ptr<T>::~smart_ptr()
    {
        detach();
    }

    template <class T>
    inline T* smart_ptr<T>::pointer()
    {
        return m_referencedObject;
    }

    template <class T>
    inline const T* smart_ptr<T>::const_pointer() const
    {
        return const_cast<const T*>(m_referencedObject);
    }

    template <class T>
    inline T* smart_ptr<T>::operator->()
    {
        return m_referencedObject;
    }

    template <class T>
    inline const T* smart_ptr<T>::operator->() const
    {
        return m_referencedObject;
    }

    template <class T>
    inline T& smart_ptr<T>::operator*()
    {
        return (*m_referencedObject);
    }

    template <class T>
    inline const T& smart_ptr<T>::operator*() const
    {
        return (*m_referencedObject);
    }

    template <class T>
    smart_ptr<T>& smart_ptr<T>::operator=(const T *obj)
    {
        attach(obj);
        return (*this);
    }

    template <class T>
    smart_ptr<T>& smart_ptr<T>::operator=(const smart_ptr<T> &ref)
    {
        if (&ref != this)
        {
            attach(ref.m_referencedObject);
        }

        return (*this);
    }

    template <class T>
    x_bool smart_ptr<T>::operator==(const smart_ptr<T> &ref) const
    {
        return (ref.m_referencedObject == m_referencedObject);
    }

    template <class T>
    x_bool smart_ptr<T>::operator!=(const smart_ptr<T> &ref) const
    {
        return (ref.m_referencedObject != m_referencedObject);
    }

    template <class T>
    inline void smart_ptr<T>::attach(const T *obj)
    {
        detach();
        if (obj)
        {
            obj->increment_reference();
            //If the previous line fails compilation, it is because you are trying to use a smart_ptr
            //on an object NOT derived from referenced_object.  Only referenced objects may be used.
        }
        m_referencedObject = const_cast<T*>(obj);
        
    }

    template <class T>
    inline void smart_ptr<T>::detach()
    {
        if (m_referencedObject)
        {
            m_referencedObject->decrement_reference();

            if (m_referencedObject->get_reference_count() == 0)
            {
                delete m_referencedObject;
            }

            m_referencedObject = NULL;
        }
    }

    /////////////////
    // Const Smart Pointer
    template <class T>
    const_smart_ptr<T>::const_smart_ptr()
    : m_referencedObject(NULL)
    {
    }

    template <class T>
    const_smart_ptr<T>::const_smart_ptr(const smart_ptr<T> &obj)
    : m_referencedObject(NULL)
    {
        attach(obj.const_pointer());
    }

    template <class T>
    const_smart_ptr<T>::const_smart_ptr(const const_smart_ptr<T> &obj)
    : m_referencedObject(NULL)
    {
        attach(obj.m_referencedObject);
    }

    template <class T>
    const_smart_ptr<T>::const_smart_ptr(const T *obj)
    : m_referencedObject(NULL)
    {
        attach(obj); 
    }

    template <class T>
    const_smart_ptr<T>::~const_smart_ptr()
    {
        detach();
    }

    template <class T>
    inline const T* const_smart_ptr<T>::const_pointer() const
    {
        return const_cast<const T*>(m_referencedObject);
    }

    template <class T>
    inline const T* const_smart_ptr<T>::operator->() const
    {
        return m_referencedObject;
    }

    template <class T>
    inline const T& const_smart_ptr<T>::operator*() const
    {
        return (*m_referencedObject);
    }

    template <class T>
    const_smart_ptr<T>& const_smart_ptr<T>::operator=(const T *obj)
    {
        attach(obj);
        return (*this);
    }

    template <class T>
    const_smart_ptr<T>& const_smart_ptr<T>::operator=(const const_smart_ptr<T> &ref)
    {
        if (&ref != this)
        {
            attach(ref.m_referencedObject);
        }

        return (*this);
    }

    template <class T>
    const_smart_ptr<T>& const_smart_ptr<T>::operator=(const smart_ptr<T> &ref)
    {
        attach(ref.const_pointer());
        return (*this);
    }

    template <class T>
    x_bool const_smart_ptr<T>::operator==(const const_smart_ptr<T> &ref) const
    {
        return (ref.m_referencedObject == m_referencedObject);
    }

    template <class T>
    x_bool const_smart_ptr<T>::operator!=(const const_smart_ptr<T> &ref) const
    {
        return (ref.m_referencedObject != m_referencedObject);
    }

    template <class T>
    x_bool const_smart_ptr<T>::operator==(const smart_ptr<T> &ref) const
    {
        return (ref.const_pointer() == m_referencedObject);
    }

    template <class T>
    x_bool const_smart_ptr<T>::operator!=(const smart_ptr<T> &ref) const
    {
        return (ref.const_pointer() != m_referencedObject);
    }

    template <class T>
    inline void const_smart_ptr<T>::attach(const T *obj)
    {
        detach();
        if (obj)
        {
            obj->increment_reference();
            //If the previous line fails compilation, it is because you are trying to use a const_smart_ptr
            //on an object NOT derived from referenced_object.  Only referenced objects may be used.
        }
        m_referencedObject = const_cast<T*>(obj);
        
    }

    template <class T>
    inline void const_smart_ptr<T>::detach()
    {
        if (m_referencedObject)
        {
            m_referencedObject->decrement_reference();

            if (m_referencedObject->get_reference_count() == 0)
            {
                delete m_referencedObject;
            }

            m_referencedObject = NULL;
        }
    }
}; //End of namespace


#endif
