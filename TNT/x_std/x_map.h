////////////////////////////////////////////////////////////////////////////
//
//	x_std::map
//
////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
// *map<>::x_reinsert(iterator, key)  - this method is an efficient way to
//    modify the key of a given iterator.  This is faster than erasing the
//    iterator then reinserting the element again with the new key.
//
// *multimap<> is NOT DEFINED
//
// *Key - must support operator ==.
//
// *T - if class or struct, must provide a functional copy constructor.
//
////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE PARAMETERS
//
//    Key       - This is a unique identifier.  This can be a user defined
//                type, but it must support the == operator.  The < operator
//                must be supported before using the map comparison operators.
//    T         - This is the object type that is associated with each key.
//                The < operator must be provided to use the map comparison 
//                operators.
//                be supported before using the map comparison operators.
//    Predicate - This is class that defines the comparison operator for comparing
//                two Keys
//    Allocator - This is an allocator object used for allocation (if compiler
//                doesn't support the rebind mechanism then x_std::allocator<>
//                is used instead.)
//
////////////////////////////////////////////////////////////////////////////
//  
//  FUNCTION OVERVIEW
//
//    map
//        size          - Return the number of elements in the set
//        begin         - Returns an iterator pointing to the begining of the set
//        end           - Returns an iterator pointing off the end of the set
//        insert        - Inserts an element into the set (internal allocation)
//        erase         - Erases an element from the set (internal deallocation)
//        x_reinsert    - Changes the key associated with the specified iterator (no internal allocation/deallocation).
//        find          - Find the specified element
//        upper_bound   - Find the first element that is greater than the key specified
//        lower_bound   - Find the first element that is greater or equal than the key specified
//        equal_range   - Return a pair equal to <lower_bound(key), upper_bound(key)>
//        count         - Return number of elements in range [lower_bound(key), upper_bound(key))
//        clear         - Erases every element from the set
//        get_allocator - Returns copy of the allocator object
//        operator[]    - Returns element associated with a given key.  If key doesn't exist,
//                        an entry for the key is added to the map and the new element is
//                        returned.
//        operator=     - Copies contents of another map into this map.
//        operator==    - Determines if the contents of two maps are the same.
//                      
//    map::iterator     
//        operator*     - Returns T by reference (const in const_iterator)
//        operator->    - Returns T* (const in const_iterator)
//        operator==    - Compares two iterators and returns true if they point to the same element
//        operater!=    - Compares two iterators and returns true if they don't point to the same element
//        operator++    - Moves the iterator to the next highest element
//        operator--    - Moves the iterator to the immediate lower element
//
////////////////////////////////////////////////////////////////////////////
//
//  PERFORMANCE OVERVIEW
//
//    N is number of elements inside of the map.
//  
//    map
//        size          O(1)
//        begin         O(1)
//        end           O(1)
//        insert        O(log N)
//        erase         O(log N)
//        x_reinsert    O(2 log N)
//        find          O(log N)
//        upper_bound   O(log N)
//        lower_bound   O(log N)
//        equal_range   O(2 log N);
//        count         O(2 log N);
//        clear         O(N)
//        get_allocator O(1)
//        swap          O(1)
//        operator[]    O(log N)
//        operator=     O(N log N), inneficient - use sparingly
//        operator==    O(N)
//
//    map::iterator
//        operator*     O(1)
//        operator->    O(1)
//        operator==    O(1)
//        operater!=    O(1)
//        operator++    O(h), where h is less than 2 log N
//        operator--    O(h), where h is less than 2 log N
//
////////////////////////////////////////////////////////////////////////////
// 
// EXAMPLE
//
// class Foobar { ... };
//
// typedef x_std::map< s32, Foobar* > FoobarMap;
// 
// FoobarMap  myFoobarmap;
//
// foobar1 = new Foobar;
// foobar2 = new Foobar;
//
// myFoobarMap.insert( FoobarMap::value_type(1, foobar1) );
// myFoobarMap.insert( FoobarMap::value_type(2, foobar2) );
//
// FoobarMap::iterator it;
// 
// it = myFoobarMap.find( 1 );
//
// X_STD_ASSERT( it != myFoobarMap.end() );
// X_STD_ASSERT( (*it).second == foobar1 );
//
// it = myFoobarMap.begin();
// 
// X_STD_ASSERT( (*it).second == foobar1 );
//
// ++it;
//
// X_STD_ASSERT( it != myFoobarMap.end() );
// X_STD_ASSERT( (*it).second == foobar2 );
// 
// myFoobarMap.clear();
//
//////////////////////////////////////////////////////////////////////////////
#ifndef X_STD_MAP_H
#define X_STD_MAP_H

#include "x_private_tree.h"

namespace x_std
{

    template <  class Key,
                class T,
                class Predicate = less<Key>, 
                class Allocator = allocator<T> 
             >
    class map
    {
        public:
            //////////////////////
            // Types
            typedef pair<const Key, T>                                  value_type;
        
        private:         
            struct x_Kfn : public unary_function<value_type, Key>
            {
                inline const Key& operator()(const value_type& X) const
                {
                    return X.first;
                }
            };

            typedef map<Key, T, Predicate, Allocator>                   map_type;
            typedef x_std_private::x_tree<Key, value_type, x_Kfn, Predicate, Allocator>  tree_type;

        public:

            typedef tree_type::size_type                                size_type;
            typedef tree_type::iterator                                 iterator;
            typedef tree_type::const_iterator                           const_iterator;
            typedef tree_type::reverse_iterator                         reverse_iterator;
            typedef tree_type::const_reverse_iterator                   const_reverse_iterator;
            typedef tree_type::allocator_type                           allocator_type;
            typedef tree_type::key_type                                 key_type;
            typedef T                                                   referent_type;
            typedef tree_type::key_compare                              key_compare;
            typedef typename allocator_type::difference_type            difference_type;
            typedef typename allocator_type::const_reference            reference;
            typedef typename allocator_type::const_reference            const_reference;

            class value_compare : public binary_function<value_type, value_type, x_bool> 
            {    
            public:
                x_bool operator()(const value_type& x, const value_type& y)      { return (comp(x.first, y.first));  }
            protected:
                friend class map<Key, T, Predicate, Allocator>;
                value_compare(key_compare pr)
                    : comp(pr) {}
                key_compare comp;
            private:
                value_compare(); //Not defined
            };

            /////////////////////
            // Construction
            explicit map(const Predicate& comp = Predicate(), const Allocator& al = Allocator());
            map(const map<Key, T, Predicate, Allocator>&);
            map(const value_type *first, const value_type *last);

            /////////////////////
            // Accessors
            inline size_type                size() const                        { return m_tree.size();             }
            inline size_type                max_size() const                    { return m_tree.max_size();         }
            inline x_bool                   empty() const                       { return m_tree.empty();            }
            inline iterator                 begin()                             { return m_tree.begin();            }
            inline iterator                 end()                               { return m_tree.end();              }
            inline reverse_iterator         rbegin()                            { return m_tree.rbegin();           }
            inline reverse_iterator         rend()                              { return m_tree.rend();             }
            inline iterator                 find(const Key& k_)                 { return m_tree.find(k_);           }
            inline iterator                 upper_bound(const Key& k_)          { return m_tree.upper_bound(k_);    }
            inline iterator                 lower_bound(const Key& k_)          { return m_tree.lower_bound(k_);    }
            inline pair<iterator,iterator>  equal_range(const Key& k_)          { return m_tree.equal_range(k_);    }
                                                                                                                    
            inline const_iterator           begin() const                       { return m_tree.begin();            }
            inline const_iterator           end() const                         { return m_tree.end();              }
            inline const_reverse_iterator   rbegin() const                      { return m_tree.rbegin();           }
            inline const_reverse_iterator   rend() const                        { return m_tree.rend();             }
            inline const_iterator           find(const Key& k_) const           { return m_tree.find(k_);           }
            inline const_iterator           upper_bound(const Key& k_) const    { return m_tree.upper_bound(k_);    }
            inline const_iterator           lower_bound(const Key& k_) const    { return m_tree.lower_bound(k_);    }
            inline pair<const_iterator,                                                                             
                const_iterator>             equal_range(const Key& k_) const    { return m_tree.equal_range(k_);    }
            inline size_type                count(const Key& k_) const          { return m_tree.count(k_);          }
            inline allocator_type           get_allocator() const               { return m_tree.get_allocator();    }
            inline key_compare              key_comp() const                    { return m_tree.key_comp();         }
            inline value_compare            value_comp() const                  { return value_compare(key_comp()); }

            inline x_bool                    operator==(const map& rhs)const     { return m_tree == rhs.m_tree;      }
            inline x_bool                    operator< (const map& rhs)const     { return m_tree < rhs.m_tree;       }

            ////////////////////
            // Mutators
            inline pair<iterator, x_bool>   insert(const value_type& t_)             { return m_tree.insert(t_);            }
            inline iterator                 insert(iterator it, const value_type& t_){ return m_tree.insert(it, t_);        }
            inline void                     insert(const value_type* first_,
                                                    const value_type* last_)         {        m_tree.insert(first_, last_); }
            inline void                     x_reinsert(iterator& it_, const Key& k_) {        m_tree.reinsert(it_, k_);     }
            inline iterator                 erase(iterator it_)                      { return m_tree.erase(it_);            }
            inline iterator                 erase(iterator first_, iterator last_)   { return m_tree.erase(first_,last_);   }
            inline size_type                erase(const Key& k_)                     { return m_tree.erase(k_);             }
            inline void                     clear()                                  {        m_tree.clear();               }
            inline void                     swap(map& m_)                            {        m_tree.swap(m_.m_tree);       }

            T&                              operator[](const Key&);
            map&                            operator=(const map&);

        private:
            tree_type   m_tree;
    };

    template <class x_K, class x_T, class x_P, class x_A>
    x_T& map<x_K, x_T, x_P, x_A>::operator [](const x_K &key)
    {
        pair<iterator, x_bool> result( insert( value_type( key, x_T() ) ) );

        return (*(result.first)).second;
    }

    template <class x_K, class x_T, class x_P, class x_A>
    map<x_K, x_T, x_P, x_A>& map<x_K, x_T, x_P, x_A>::operator=(const map<x_K, x_T, x_P, x_A>& rt)
    {
        m_tree = rt.m_tree;

        return *this;
    }

    template <class x_K, class x_T, class x_P, class x_A>
    map<x_K, x_T, x_P, x_A>::map(const x_P &p, const x_A &a)
    : m_tree(p, a)
    {}
    
    template <class x_K, class x_T, class x_P, class x_A>
    map<x_K, x_T, x_P, x_A>::map(const map<x_K, x_T, x_P, x_A>& copy)
    : m_tree()
    {
        //Not optimal I know... fix later if really necessary.
        *this = copy;
    }

    template <class x_K, class x_T, class x_P, class x_A>
    map<x_K, x_T, x_P, x_A>::map(const value_type *first, const value_type *last)
    : m_tree()
    {
        insert(first, last);
    }
    
    template <class x_K, class x_T, class x_P, class x_A> 
    x_bool operator != ( const map<x_K, x_T, x_P, x_A> & lhs , const map<x_K, x_T, x_P, x_A> & rhs )
    {
        return ! ( lhs == rhs ) ; 
    }

    template <class x_K, class x_T, class x_P, class x_A> 
    x_bool operator >  ( const map<x_K, x_T, x_P, x_A> & lhs , const map<x_K, x_T, x_P, x_A> & rhs )
    {
        return rhs < lhs ;
    }

    template <class x_K, class x_T, class x_P, class x_A>
    x_bool operator <= ( const map<x_K, x_T, x_P, x_A> & lhs , const map<x_K, x_T, x_P, x_A> & rhs )
    {
        return ! ( rhs < lhs );
    }

    template <class x_K, class x_T, class x_P, class x_A>
    x_bool operator >= ( const map<x_K, x_T, x_P, x_A> & lhs , const map<x_K, x_T, x_P, x_A> & rhs )
    {
        return ! ( lhs < rhs );
    }


} //End of namespace

#endif
