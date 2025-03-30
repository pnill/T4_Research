////////////////////////////////////////////////////////////////////////////
//
//	x_std_private::x_tree
//
////////////////////////////////////////////////////////////////////////////
//
//  This is used internally by x_std.  Use set<> or map<> instead of x_tree<>
//
////////////////////////////////////////////////////////////////////////////
//
// BACKGROUND
//
//    This data structure is a red-black tree.  A red-black tree is a special
//  type of binary tree with the special property that it is guaranteed to
//  remain balanced.  In fact the depth (or height) of the tree will never
//  be greater 2 log N (base 2 log).  Basically the red-black tree autobalances
//  itself as you add elements to it.
//
//    This implementation requires that each node have a unique key to
//  identify it.  This key determines where in the tree an element is placed.
//
//    This implementation is based on the Red-Black tree psuedocode found
//  in INTRODUCTION TO ALGORITHMS by Cormen, Leiserson, and Rivest.
//
////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE PARAMETERS
//
//    Key       - The unique identifier per node.  Must support <, =, ==, !=
//    T         - This is what the "data" of each node in the tree is. Must support =.
//    KeyFn     - This function returns the key value associated with T
//    Pred      - This is class that defines the comparison operator for comparing
//                two keys
//    AllocNode - This is an allocator object that is used to allocate each 
//                node in the red-black tree.
//
////////////////////////////////////////////////////////////////////////////
//
// NOTES
//
//     Every element must have a unique key to identify it.  If you don't 
//   ensure unique keys then the behavior of the x_tree will not be what you
//   expect.  If anyone wants (or knows how) to change this implementation
//   to support non-unique keys tell me.
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_STD_PRIVATE_TREE_H
#define X_STD_PRIVATE_TREE_H

#if defined(_MSC_VER)
    // Suppress certain Microsoft Visual C++ warnings
    #pragma warning( disable: 4512 ) // Warning: <node_type> : assignment operator could not be generated
        //This warning appeared when node_type::first became const.
#endif

#include "x_std.h"
#include "x_memory.h"
#include "x_functional.h"
#include "x_utility.h"
#include "x_algorithm.h"

namespace x_std_private
{

template < class T > 
struct x_rb_node
{
    x_rb_node();
    x_rb_node(const T&);
    ~x_rb_node();
    enum Color
    {
        black,
        red
    };

    x_rb_node   *left;
    x_rb_node   *right;
    x_rb_node   *parent;
    T           data;
    Color       color;

    void set_data( const T& );
    x_rb_node& operator=(const x_rb_node&);
};

template < class T >
x_rb_node<T>::x_rb_node()
: left( NULL )
, right( NULL )
, parent( NULL )
, data( T() )
, color( x_rb_node<T>::black )
{}

template < class T >
x_rb_node<T>::x_rb_node(const T& x_t)
: left( NULL )
, right( NULL )
, parent( NULL )
, data( x_t )
, color( x_rb_node<T>::black )
{}

template < class T >
x_rb_node<T>::~x_rb_node()
{
    data.~T();
}


template < class T >
inline void x_rb_node<T>::set_data(const T& val)
{
    (&data) -> ~T();

    const void* ptr = reinterpret_cast<const void*>(&data);
    placement_new( const_cast<void*>(ptr) , val );
}

template < class T >
x_rb_node<T>& x_rb_node<T>::operator=(const x_rb_node<T>& rt)
{
    left    = rt.left;
    right   = rt.right;
    parent  = rt.parent;
    color   = rt.color;

    set_data( rt.data );
}



template <  class Key,
            class T,
            class KeyFn,
            class Pred,  
            class Alloc
         >
class x_tree
{
    public:
        typedef x_rb_node< T > node_type;
 
        #if X_STD_USES_TEMPLATE_REBIND
            typedef typename Alloc::rebind< node_type >::other          AllocNode;
        #else
            typedef x_std::allocator< node_type >                       AllocNode;
        #endif

        class iterator;
        class const_iterator;
        class reverse_iterator;
        class const_reverse_iterator;

        ///////////////////////////////////////
        //  Iterator 
        ///////////////////////////////////////
        //
        //  Note:  MSVC gets confused and I can't get it to link with
        //         iterator method implementations if I implement them outside
        //         of their declarations.  That is why the implementation of
        //         each method is here in the declaration instead of at the
        //         end of the file.  I know it looks ugly -- if you can get
        //         it to work, be my guest. ;-)  RK
        //  
        class iterator
        {
        public:
            iterator()
                : m_rbNode(NULL)
                , m_list(NULL)
                {}

            iterator(const iterator&rt)
                : m_rbNode(rt.m_rbNode)
                , m_list(rt.m_list)
                {}
            iterator(const const_iterator&rt) //Required due to way set<> had to be implemented for GC compiler
                : m_rbNode(rt.m_rbNode)
                , m_list(rt.m_list)
                {}

            ~iterator()  
                {}

            T&          operator*()
            {
                X_STD_ASSERT(m_rbNode);
                return m_rbNode->data;
            }
            
            T*          operator->()
            {
                X_STD_ASSERT(m_rbNode);
                return &m_rbNode->data;
            }

            x_bool      operator==(const iterator& rt) const
            {            
                return rt.m_rbNode == m_rbNode;
            }
            
            x_bool      operator!=(const iterator& rt) const
            {
                return rt.m_rbNode != m_rbNode;
            }
            
            
            // Mutators            
            iterator&    operator++()
            {
                if (m_rbNode == m_list->m_end.get_node())
                {
                    m_rbNode = m_list->m_begin.get_node();
                    return (*this);
                }

                X_STD_ASSERT(!m_list->isNil(m_rbNode));
                
                if (!m_list->isNil(m_rbNode->right))
                {
                    m_rbNode = m_rbNode->right;
                    while (!m_list->isNil(m_rbNode->left))
                    {
                        m_rbNode = m_rbNode->left;
                    }
                    return (*this);
                }
                
                node_type *node;
                
                node = m_rbNode->parent;
                
                while (!m_list->isNil(node) && (m_rbNode == node->right))
                {
                    m_rbNode = node;
                    node = node->parent;
                }
                
                m_rbNode = node;
                return (*this);
            }
            
            iterator&   operator--()
            {
                if (m_rbNode == m_list->m_end.get_node())
                {
                    m_rbNode = m_list->m_last.get_node();
                    return (*this);
                }

                X_STD_ASSERT(!m_list->isNil(m_rbNode));

                if (!m_list->isNil(m_rbNode->left))
                {
                    m_rbNode = m_rbNode->left;
                    while (!m_list->isNil(m_rbNode->right))
                    {
                        m_rbNode = m_rbNode->right;
                    }
                    return (*this);
                }
                
                node_type *node;
                
                node = m_rbNode->parent;
                
                while (!m_list->isNil(node) && (m_rbNode == node->left))
                {
                    m_rbNode = node;
                    node = node->parent;
                }
                
                m_rbNode = node;
                return (*this);
            }

            const iterator operator++( int ) //must be native platform "int" type
            {
                iterator it(*this);

                ++(*this);

                return it;
            }

            const iterator operator--( int ) //must be native platform "int" type
            {
                iterator it(*this);

                --(*this);

                return it;
            }
        protected:
            friend class x_tree<Key, T, KeyFn, Pred, Alloc>;
            friend class const_iterator;
            //friend class reverse_iterator;
             
            iterator(node_type*data, const x_tree<Key, T, KeyFn, Pred, Alloc> *list)
                    : m_rbNode(data)
                    , m_list( const_cast< x_tree<Key, T, KeyFn, Pred, Alloc>* >(list) )
                    {}

            inline node_type* get_node() const { return m_rbNode; }
    
            node_type *m_rbNode;
            x_tree<Key, T, KeyFn, Pred, Alloc> *m_list;
        };

        class const_iterator
        {
        public:
            const_iterator()
                : m_rbNode(NULL)
                , m_list(NULL)
                {}

            const_iterator(const const_iterator&rt)
                : m_rbNode(rt.m_rbNode)
                , m_list(rt.m_list)
                {}

            const_iterator(const iterator &it)
                : m_rbNode( it.m_rbNode )
                , m_list( it.m_list )
            {}

            ~const_iterator()  
                {}

            // Accessors
            const T&    operator*() const
            {
                X_STD_ASSERT(m_rbNode);
                return m_rbNode->data;
            }
            
            const T*    operator->() const
            {
                X_STD_ASSERT(m_rbNode);
                return &(m_rbNode->data);
            }
            
            x_bool      operator==(const const_iterator& rt) const
            {            
                return rt.m_rbNode == m_rbNode;
            }
            
            x_bool      operator!=(const const_iterator& rt) const
            {
                return rt.m_rbNode != m_rbNode;
            }
                        
            // Mutators            
            const_iterator& operator++()
            {
                if (m_rbNode == m_list->m_end.get_node())
                {
                    m_rbNode = m_list->m_begin.get_node();
                    return (*this);
                }

                X_STD_ASSERT(!m_list->isNil(m_rbNode));
                
                if (!m_list->isNil(m_rbNode->right))
                {
                    m_rbNode = m_rbNode->right;
                    while (!m_list->isNil(m_rbNode->left))
                    {
                        m_rbNode = m_rbNode->left;
                    }
                    return (*this);
                }
                
                node_type *node;
                
                node = m_rbNode->parent;
                
                while (!m_list->isNil(node) && (m_rbNode == node->right))
                {
                    m_rbNode = node;
                    node = node->parent;
                }
                
                m_rbNode = node;
                return (*this);
            }
            
            const_iterator& operator--()
            {
                if (m_rbNode == m_list->m_end.get_node())
                {
                    m_rbNode = m_list->m_last.get_node();
                    return (*this);
                }

                X_STD_ASSERT(!m_list->isNil(m_rbNode));

                if (!m_list->isNil(m_rbNode->left))
                {
                    m_rbNode = m_rbNode->left;
                    while (!m_list->isNil(m_rbNode->right))
                    {
                        m_rbNode = m_rbNode->right;
                    }
                    return (*this);
                }
                
                node_type *node;
                
                node = m_rbNode->parent;
                
                while (!m_list->isNil(node) && (m_rbNode == node->left))
                {
                    m_rbNode = node;
                    node = node->parent;
                }
                
                m_rbNode = node;
                return (*this);
            }

            const const_iterator operator++( int ) //must be native platform "int" type
            {
                const_iterator it(*this);

                ++(*this);

                return it;
            }

            const const_iterator operator--( int ) //must be native platform "int" type
            {
                const_iterator it(*this);

                --(*this);

                return it;
            }
        protected:
            friend class x_tree<Key, T, KeyFn, Pred, Alloc>;
            friend class iterator;
            //friend class const_reverse_iterator;
             
            const_iterator(node_type*data, const x_tree<Key, T, KeyFn, Pred, Alloc> *list)
                    : m_rbNode(data)
                    , m_list( const_cast< x_tree<Key, T, KeyFn, Pred, Alloc>* >(list) )
                    {}
             
            inline node_type* get_node() const { return m_rbNode; }

            node_type *m_rbNode;
            x_tree<Key, T, KeyFn, Pred, Alloc> *m_list;
        };

        class reverse_iterator : public iterator
        {
        public:
            reverse_iterator()
                {}

            reverse_iterator(const reverse_iterator&rt)
                : iterator(rt.m_rbNode, rt.m_list)
                {}
            reverse_iterator(const const_reverse_iterator&rt) //Required due to way set<> had to be implemented for GC compiler
                : iterator(rt.m_rbNode, rt.m_list)
                {}

            ~reverse_iterator()  
                {}
            
            reverse_iterator& operator++()
            {
                iterator::operator--();

                return (*this);
            }
            
            reverse_iterator& operator--()
            {
                iterator::operator++();

                return (*this);
            }

            const reverse_iterator operator++( int ) //must be native platform "int" type
            {
                reverse_iterator it(*this);

                ++(*this);

                return it;
            }

            const reverse_iterator operator--( int ) //must be native platform "int" type
            {
                reverse_iterator it(*this);

                --(*this);

                return it;
            }
        protected:
            friend class x_tree<Key, T, KeyFn, Pred, Alloc>;
            friend class const_reverse_iterator;

            explicit reverse_iterator(const iterator& rIt)
                : iterator(rIt) // iterator(rIt.m_rbNode, rIt.m_list)
                {}

        };

        class const_reverse_iterator : public const_iterator
        {
        public:
            const_reverse_iterator()
                {}

            const_reverse_iterator(const reverse_iterator&rt)
                : const_iterator(rt.m_rbNode, rt.m_list)
                {}

            ~const_reverse_iterator()  
                {}
            
            const_reverse_iterator& operator++()
            {
                const_iterator::operator--();

                return (*this);
            }
            
            const_reverse_iterator& operator--()
            {
                const_iterator::operator++();

                return (*this);
            }

            const const_reverse_iterator operator++( int ) //must be native platform "int" type
            {
                const_reverse_iterator it(*this);

                ++(*this);

                return it;
            }

            const const_reverse_iterator operator--( int ) //must be native platform "int" type
            {
                const_reverse_iterator it(*this);

                --(*this);

                return it;
            }

        protected:
            friend class reverse_iterator;
            /*
            explicit const_reverse_iterator( const reverse_iterator& it)
                : const_iterator(it.m_rbNode,  it.m_list )
                {}
                */
        };

        /////////////////////////////////////////////////////////////////////////////////
        //
        //  Tree Implementation
        x_tree();
        x_tree(const Pred&, const Alloc&);
        ~x_tree();

        typedef x_tree<Key, T, KeyFn, Pred, Alloc>           tree_type;
        typedef x_std::pair<const_iterator, const_iterator> const_iterator_pair;
        typedef x_std::pair<iterator, iterator>             iterator_pair;
        typedef typename AllocNode::size_type               size_type;
        typedef AllocNode                                   allocator_type;
        typedef Key                                         key_type;
        typedef T                                           referent_type;
        typedef Pred                                        key_compare;
        typedef KeyFn                                       key_function;

        //Accessors
        size_type               size() const;
        size_type               max_size() const;
        x_bool                  empty() const;
                                
        const_iterator          begin() const;
        const_iterator          end() const;
        const_reverse_iterator  rbegin() const;
        const_reverse_iterator  rend() const;
        const_iterator          find(const Key&) const;
        const_iterator          upper_bound(const Key&) const;
        const_iterator          lower_bound(const Key&) const;
        const_iterator_pair     equal_range(const Key&) const;
                                
        iterator                begin();
        iterator                end();
        reverse_iterator        rbegin();
        reverse_iterator        rend();
        iterator                find(const Key&);
        iterator                upper_bound(const Key&);
        iterator                lower_bound(const Key&);
        iterator_pair           equal_range(const Key&);
        key_compare             key_comp() const;
        size_type               count(const Key&) const;
                                
        const allocator_type&   get_allocator() const;

        x_bool                  operator==(const tree_type&) const;
        x_bool                  operator< (const tree_type&) const;
                                
        //Mutators              
        x_std::pair<iterator, x_bool> insert(const T&val)
		{
			node_type *x;
			x_bool already_exists;

			iterator it;
 
			//Find nearest node
			it = find_nearest(KeyFn()(val), already_exists);

			if (already_exists)
			{
				return x_std::pair<x_tree<Key, T, KeyFn, Pred, Alloc>::iterator, x_bool>(it, false);
			}

			//Update tree size
			++m_size;

			//Create the node
			x = m_allocNode.allocate(1);
			m_allocNode.construct(x, val);

			//Insert the node
			insertNode(x, it.m_rbNode);

			return x_std::make_pair(iterator(x, this), true);
		}

        
		iterator                insert(iterator it, const T&);
        void                    insert(const T *first, const T *last);
        void                    reinsert(iterator&, const Key&);
        iterator                erase(iterator);
        iterator                erase(iterator first, iterator last);
        size_type               erase(const Key&);
        void                    swap(tree_type&);
        void                    clear();
                                
        allocator_type&         get_allocator();

        tree_type&              operator=(const tree_type&);

    private:
        friend class x_tree<Key, T, KeyFn, Pred, Alloc>::iterator;
        friend class x_tree<Key, T, KeyFn, Pred, Alloc>::const_iterator;

        iterator                find_nearest(const Key&, x_bool &already_exists);
        x_bool                  isNil(const node_type*) const;
        void                    insertNode(node_type* node, node_type* parent = NULL);
        node_type*              remove(node_type*&, node_type*&);
        void                    deleteFixup(node_type*);
        void                    rotateLeft(node_type*);
        void                    rotateRight(node_type*);
        
        key_compare             m_comp;
        AllocNode               m_allocNode;
        node_type*              m_root;
        node_type*              m_nil;
        iterator                m_begin;
        iterator                m_end;
        iterator                m_last;
        size_type               m_size;
};

///////////////////////////////////////////////////////
// x_tree methods

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
x_tree<x_K, x_T, x_Kfn, x_P, x_A >::x_tree()
: m_size(0)
{
    m_nil = m_allocNode.allocate(1);
    //m_allocNode.x_construct(m_nil);

    m_nil->color = node_type::black;
    m_nil->left = m_nil;
    m_nil->right= m_nil;
    m_nil->parent=m_nil;

    m_root = m_nil;
     
    m_begin.m_rbNode = m_nil;
    m_begin.m_list = this;

    m_end   = m_begin;
    m_last  = m_end;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
x_tree<x_K, x_T, x_Kfn, x_P, x_A >::x_tree(const x_P& comp, const x_A& )
: m_comp( comp )
, m_size(0) 
//Can't initialize allocator... it is of different type than x_A
{
    m_nil = m_allocNode.allocate(1);
    //m_allocNode.x_construct(m_nil);

    m_nil->color = node_type::black;
    m_nil->left = NULL;
    m_nil->right= NULL;
    m_nil->parent=m_nil;

    m_root = m_nil;
     
    m_begin.m_rbNode = m_nil;
    m_begin.m_list = this;

    m_end   = m_begin;
    m_last  = m_end;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
x_tree<x_K, x_T, x_Kfn, x_P, x_A>::~x_tree()
{
    if (m_size != 0)
    {
        clear();
    }

    //m_allocNode.destroy(m_nil);
    m_allocNode.deallocate(m_nil, 0);
}

//Accessors
template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::size_type x_tree<x_K, x_T, x_Kfn, x_P, x_A>::size() const
{
    return m_size;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::size_type x_tree<x_K, x_T, x_Kfn, x_P, x_A>::max_size() const
{
    return 100000000;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
x_bool x_tree<x_K, x_T, x_Kfn, x_P, x_A>::empty() const
{
    return (m_size == 0);
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::begin()
{
    return m_begin;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::end()
{
    return m_end;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::reverse_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::rbegin()
{
    reverse_iterator it( m_end );

    ++it;

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::reverse_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::rend()
{
    reverse_iterator it( m_end );
    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
const typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::allocator_type& x_tree<x_K, x_T, x_Kfn, x_P, x_A>::get_allocator() const
{
    return m_allocNode;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::allocator_type& x_tree<x_K, x_T, x_Kfn, x_P, x_A>::get_allocator()
{
    return m_allocNode;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::key_compare x_tree<x_K, x_T, x_Kfn, x_P, x_A>::key_comp() const
{
    return m_comp;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::size_type x_tree<x_K, x_T, x_Kfn, x_P, x_A>::count(const x_K& key) const
{
    const_iterator_pair pr( equal_range(key) );

    size_type n = 0;

    while (pr.first != pr.second)
    {
        ++n;
        ++pr.first;
    }

    return n;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
void x_tree<x_K, x_T, x_Kfn, x_P, x_A>::swap(tree_type& tree)
{
    if (!(m_allocNode == tree.m_allocNode))
    {
        x_std::swap( m_allocNode, tree.m_allocNode );
    }
    x_std::swap( m_comp, tree.m_comp );
    x_std::swap( m_root, tree.m_root );
    x_std::swap( m_nil,  tree.m_nil  );
    x_std::swap( m_begin,tree.m_begin);
    x_std::swap( m_end,  tree.m_end  );
    x_std::swap( m_last, tree.m_last );
    x_std::swap( m_size, tree.m_size );
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::find(const x_K& val)
{
    node_type *node;

    node = m_root;

    while (node != m_nil)
    {
        if (x_Kfn()(node->data) == val)
        {
            break;
        }

        if (m_comp(val, x_Kfn()(node->data)))
        {
            node = node->left;
        }
        else
        {
            node = node->right;
        }
    }

    iterator it(node, this);

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::find_nearest(const x_K& val, x_bool &already_exists)
{
    node_type *y, *x;
    
    already_exists = false;

    if (m_root == m_nil)
    {
        return m_end;
    }

    y = m_nil;
    x = m_root;

    while (x != m_nil)
    {
        y = x;

        const x_K &key = x_Kfn()(x->data);

        if (key == val)
        {
            already_exists = true;
            break;
        }
            
        if (m_comp(val, key))
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    iterator it(y, this);

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
inline x_bool x_tree<x_K, x_T, x_Kfn, x_P, x_A>::isNil(const node_type* node) const
{
    return node == m_nil;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::upper_bound(const x_K& val)
{
    node_type *y; //last node greater than val
    node_type *x; //current node

    x = m_root;
    y = m_nil;

    x_Kfn keyFn;

    while (x != m_nil)
    {
        if (m_comp(val, keyFn(x->data)))
        {
            y = x;
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    return iterator(y, this);
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::lower_bound(const x_K& val)
{
    node_type *y; //last node which is not less than val
    node_type *x; //current node

    x = m_root;
    y = m_nil;

    x_Kfn keyFn;

    while (x != m_nil)
    {
        if (!m_comp(keyFn(x->data), val))
        {
            y = x;
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    return iterator(y, this);
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator_pair x_tree<x_K, x_T, x_Kfn, x_P, x_A>::equal_range(const x_K& val)
{
    return iterator_pair( lower_bound(val), upper_bound(val) );
}


//Mutators

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
void x_tree<x_K, x_T, x_Kfn, x_P, x_A>::rotateLeft(node_type* x)
{
    node_type* y;

    y = x->right;
    x->right = y->left;
     
    if (y->left != m_nil)
    {
        y->left->parent = x;
    }
     
    y->parent = x->parent;
     
    if (x->parent == m_nil)
    {
        m_root = y;
    }
    else
    {
        if (x == x->parent->left)
        {
            x->parent->left = y;
        }
        else
        {
            x->parent->right = y;
        }
    }
     
    y->left = x;
    x->parent = y;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
void x_tree<x_K, x_T, x_Kfn, x_P, x_A>::rotateRight(node_type* y)
{
    node_type* x;

    x = y->left;
    y->left = x->right;

    if (x->right != m_nil)
    {
        x->right->parent = y;
    }

    x->parent = y->parent;

    if (y->parent == m_nil)
    {
        m_root = x;
    }
    else
    {
        if (y == y->parent->right)
        {
            y->parent->right = x;
        }
        else
        {
            y->parent->left = x;
        }
    }

    x->right = y;
    y->parent = x;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
void x_tree<x_K, x_T, x_Kfn, x_P, x_A>::insertNode(node_type* x, node_type* parent)
{
    if (parent == NULL)
    {
        iterator it;
        x_bool already_exists;

        it = find_nearest(x_Kfn()(x->data), already_exists);

        parent = it.m_rbNode;
    }

    x->color    = node_type::red;
    x->left     = m_nil;
    x->right    = m_nil;
    x->parent   = parent;

    if (parent != m_nil)
    {
        if (m_comp(x_Kfn()(x->data), x_Kfn()(parent->data)))
        {
            parent->left = x;

            if (parent == m_begin.m_rbNode)
            {
                m_begin.m_rbNode = x;
            }
        }
        else
        {
            parent->right = x;

            if (m_comp(x_Kfn()(m_last.m_rbNode->data), x_Kfn()(x->data)))
            {
                m_last.m_rbNode = x;
            }
        }
    }
    else
    {
        m_root = x;
        m_begin.m_rbNode = x;
        m_last.m_rbNode = x;
    }

    //Re-balance tree
    node_type *y;

    while ((x != m_root) && (x->parent->color == node_type::red))
    {
        if (x->parent == x->parent->parent->left)
        {
            y = x->parent->parent->right;
            if (y->color == node_type::red) //if nil color is black, so don't test that
            {
                x->parent->color = node_type::black;
                y->color = node_type::black;
                x->parent->parent->color = node_type::red;
                x = x->parent->parent;
            }
            else  
            {
                if (x == x->parent->right)
                {
                    x = x->parent;
                    rotateLeft(x);
                }
                x->parent->color = node_type::black;
                x->parent->parent->color = node_type::red;
                rotateRight(x->parent->parent);
            }
        }
        else
        {
            y = x->parent->parent->left;
            if (y->color == node_type::red) //if nil color is black, so don't test that
            {
                x->parent->color = node_type::black;
                y->color = node_type::black;
                x->parent->parent->color = node_type::red;
                x = x->parent->parent;
            }
            else
            {
                if (x == x->parent->left)
                {
                    x = x->parent;
                    rotateRight(x);
                }
                x->parent->color = node_type::black;
                x->parent->parent->color = node_type::red;
                rotateLeft(x->parent->parent);
            }
        }
    }

    m_root->color = node_type::black;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::insert(iterator /*it*/, const x_T& val)
{
    //TODO: implement this the *right* way, for now just make sure it works
    return (insert(val)).first;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
void x_tree<x_K, x_T, x_Kfn, x_P, x_A>::insert(const x_T *first, const x_T *last)
{
    while (first != last)
    {
        insert( *first );
        ++first;
    }
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::node_type* x_tree<x_K, x_T, x_Kfn, x_P, x_A>::remove(node_type *&z, node_type* &successor)
{
    node_type *y, *x;

    if ((z->left == m_nil) || (z->right == m_nil))
    {
        //We're splicing out z
        y = z;
        iterator it(y, this);
        ++it;
        successor = it.m_rbNode;
    }
    else
    {
        //We're splicing out z's successor
        iterator it(z, this);
        ++it;
        y = it.m_rbNode;
        successor = z;
    }

    //y is now the node getting spliced out
    if (y == m_begin.m_rbNode)
    {
        m_begin = iterator( successor, this );
    }
    if (y == m_last.m_rbNode)
    {
        --m_last;
    }

    if (y->left != m_nil)
    {
        x = y->left;
    }
    else
    {
        x = y->right;
    }

    x->parent = y->parent;

    if (y->parent == m_nil)
    {
        m_root = x;
    }
    else
    {
        if (y == y->parent->left)
        {
            y->parent->left = x;
        }
        else
        {
            y->parent->right = x;
        }
    }

    if (y != z)
    {
        x_T temp(z->data);

        z->set_data(y->data);

        y->set_data(temp); //this address should be returned so that memory can be released
    }

    if (y->color == node_type::black)
    {
        deleteFixup(x);
    }

    return y;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
void x_tree<x_K, x_T, x_Kfn, x_P, x_A>::deleteFixup(node_type *x)
{
    node_type *w;

    while ((x != m_root) && (x->color == node_type::black))
    {
        if (x == x->parent->left)
        {
            w = x->parent->right;
            if (w->color == node_type::red)
            {
                w->color = node_type::black;
                x->parent->color = node_type::red;
                rotateLeft(x->parent);
                w = x->parent->right;
            }
            if ((w->left->color == node_type::black) && (w->right->color == node_type::black))
            {
                w->color = node_type::red;
                x = x->parent;
            }
            else
            {
                if (w->right->color == node_type::black)
                {
                    w->left->color = node_type::black;
                    w->color = node_type::red;
                    rotateRight(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = node_type::black;
                w->right->color = node_type::black;
                rotateLeft(x->parent);
                x = m_root;
            }
        }
        else
        {
            w = x->parent->left;
            if (w->color == node_type::red)
            {
                w->color = node_type::black;
                x->parent->color = node_type::red;
                rotateRight(x->parent);
                w = x->parent->left;
            }
            if ((w->right->color == node_type::black) && (w->left->color == node_type::black))
            {
                w->color = node_type::red;
                x = x->parent;
            }
            else
            {
                if (w->left->color == node_type::black)
                {
                    w->right->color = node_type::black;
                    w->color = node_type::red;
                    rotateLeft(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = node_type::black;
                w->left->color = node_type::black;
                rotateRight(x->parent);
                x = m_root;
            }
        }
    }

    x->color = node_type::black;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
void x_tree<x_K, x_T, x_Kfn, x_P, x_A>::reinsert(iterator& it, const x_K& newKey)
{
    node_type *y, *x, *z;

    X_STD_ASSERT(it.m_rbNode);

    //Remove it
    z = it.m_rbNode;

    y = remove(z, x);

    //Reinsert it
    const_cast<x_K&>(x_Kfn()(y->data)) = newKey;

    y->left   = m_nil;
    y->right  = m_nil;
    y->parent = m_nil;

    insertNode(y, NULL);
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::erase(iterator it)
{
    node_type *y, *x, *z;

    if (it == m_end)
    {
        return m_end;
    }

    z = it.m_rbNode;
    if (z == m_nil)
    {
        return m_end;
    }

    y = remove(z, x);

    m_allocNode.destroy(y);
    m_allocNode.deallocate(y, 0);

    --m_size;

    return iterator(x, this);
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::erase(iterator first, iterator last)
{
    while (first != last)
    {
        first = erase(first);
    }

    return last;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::size_type x_tree<x_K, x_T, x_Kfn, x_P, x_A>::erase(const x_K& key)
{
    iterator_pair iPair( equal_range(key) );
    size_type count( 0 );

    while (iPair.first != iPair.second)
    {
        iPair.first = erase( iPair.first );
        ++count;
    }
    
    return count;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
void x_tree<x_K, x_T, x_Kfn, x_P, x_A>::clear()
{
    iterator it;

    while (m_begin != m_end)
    {

        erase(m_begin);
        
        /*
        //Do this faster -- don't need to rebalance tree as I go.
        //Bug in this somewhere... don't have time to debug right now, so use the 
        //generic safe and slow way.

        it = m_begin;

        ++m_begin;

        m_allocNode.destroy(it.m_rbNode);
        m_allocNode.deallocate(it.m_rbNode, 0);

        m_begin.m_rbNode->left = m_nil;

        --m_size;
        */
        
    }

    m_last = m_begin;

    X_STD_ASSERT(m_size == 0);
    X_STD_ASSERT(m_begin.m_rbNode == m_nil);
}


template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::const_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::begin() const
{
    const_iterator it( const_cast<tree_type*>(this)->begin() );

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::const_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::end() const
{
    const_iterator it( const_cast<tree_type*>(this)->end() );

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::const_reverse_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::rbegin() const
{
    const_reverse_iterator it( const_cast<tree_type*>(this)->rbegin() );

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::const_reverse_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::rend() const
{
    const_reverse_iterator it( const_cast<tree_type*>(this)->rend() );

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::const_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::find(const x_K& key) const
{
    const_iterator it( const_cast<tree_type*>(this)->find( key ) );

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::const_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::upper_bound(const x_K& key) const
{
    const_iterator it( const_cast<tree_type*>(this)->upper_bound( key ) );

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::const_iterator x_tree<x_K, x_T, x_Kfn, x_P, x_A>::lower_bound(const x_K& key) const
{
    const_iterator it( const_cast<tree_type*>(this)->lower_bound( key ) );

    return it;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
typename x_tree<x_K, x_T, x_Kfn, x_P, x_A>::const_iterator_pair x_tree<x_K, x_T, x_Kfn, x_P, x_A>::equal_range(const x_K& val) const
{
    return const_iterator_pair( lower_bound(val), upper_bound(val) );
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
x_tree<x_K, x_T, x_Kfn, x_P, x_A>& x_tree<x_K, x_T, x_Kfn, x_P, x_A>::operator=(const tree_type& rt)
{
    ////////////
    // HIGHLY INNEFICIENT!
    //
    // Coded the *easiest* way... this *should* rarely get used.

    if (&rt != this)
    {
        clear();

        const_iterator it;
        for (it = rt.begin(); it!=rt.end(); ++it)
        {
            insert(*it);
        }
    }

    return *this;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
x_bool x_tree<x_K, x_T, x_Kfn, x_P, x_A>::operator==(const tree_type& rt) const
{
    if (size() != rt.size())
    {
        return false;
    }

    const_iterator me( begin() );
    const_iterator him( rt.begin() );

    while ((me != end()) && (him != rt.end()))
    {
        if ( !((*me) == (*him)) ) //Use operator == on T... You must have T::operator==() defined.
        {
            return false;
        }
        ++me;
        ++him;
    }

    return true;
}

template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
x_bool x_tree<x_K, x_T, x_Kfn, x_P, x_A>::operator<(const tree_type& rt) const
{
    return x_std::lexicographical_compare(begin(), end(), rt.begin(), rt.end());
}

//template <class x_K, class x_T, class x_Kfn, class x_P, class x_A>
//x_std::pair<x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator, x_std_private::x_bool> x_std_private::x_tree<x_K, x_T, x_Kfn, x_P, x_A>::insert(const T& val)
//{
//    node_type *x;
//    x_bool already_exists;
//
//    iterator it;
//
//    //Find nearest node
//    it = find_nearest(x_Kfn()(val), already_exists);
//
//    if (already_exists)
//    {
//        return x_std::pair<x_tree<x_K, x_T, x_Kfn, x_P, x_A>::iterator, x_bool>(it, false);
//    }
//
//    //Update tree size
//    ++m_size;
//
//    //Create the node
//    x = m_allocNode.allocate(1);
//    m_allocNode.construct(x, val);
//
//    //Insert the node
//    insertNode(x, it.m_rbNode);
//
//    return x_std::make_pair(iterator(x, this), true);
//}


} //End of namespace

#endif
