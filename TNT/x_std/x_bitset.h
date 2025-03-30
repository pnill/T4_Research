#if !defined( X_STD_BITSET_H )
#define X_STD_BITSET_H
///////////////////////////////////////////////////////////////////////////////
//
// A sequence of N bits, based on STL <bitset>
//
// Defines the template class bitset<> and various supporting templates.
//
///////////////////////////////////////////////////////////////////////////////
//  
// Overview of basic functions for list<T,Allocator>:
//
//     x_std::bitset<55> X ;    // Declare X as a sequence of 55 bits.
//     x_std::bitset<55> Y ;    // Declare Y as a sequence of 55 bits.
//  
//     X &= Y ;             // Replace X with the bit-wise logical "and" of X and Y.
//     X |= Y ;             // Replace X with the bit-wise logical "or" of X and Y.
//     X ^= Y ;             // Replace X with the bit-wise logical "xor" of X and Y.
//     X << 5 ;             // Shift X 5 bits to the "left", replacing each bit with 
//                          // the one 5 positions earlier (or 0 if there is no such bit).
//     X >> 7 ;             // Shift X 7 bits to the "right", replacing each bit with
//                          // the one 7 positions later (or 0 if there is no such bit).
//     ~X                   // A new bitset with each bit of X flipped.
//
//     X.set();             // Sets all bits in X to 1.
//     X.set(12);           // Set bit 12 to 1.
//     X.set(12,true);      // Set bit 12 to true.
//     X.set(12,false);     // Reset bit 12 to false.
//
//     X.reset();           // Resets all bits in X to 0.
//     X.reset(13);         // Reset bit 13.
//
//     X.flip();            // Flip all the bits in X.
//     X.flip(13);          // Flip bit 13.
//    
//     X[12]                // Bit 12 of X.
//     X[12] = true ;       // Change bit 12 of X.
//
//     X.to_ulong()         // Return the sum of the bit values in X.
//     X.count()            // How many bits are set in X?
//     X.any()              // Are any bits in X set?
//     X.none()             // Are none of the bits in X set?
//     X.size()             // How many bits are in X? This is the template parameter N.
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
// 1. Assertion failures are used instead of throwing exception out_of_range.
// 2. In class bitset<>, these two functions have been replaced with versions
//    which work only on x_std::string:
//      template<class E, class T, class A> bitset
//      (
//          const string<E, T, A>& str,
//          string<E, T, A>size_type pos = 0,
//          string<E, T, A>size_type n = string<E, T, A>::npos
//      );
//      template<class E, class T, class A> string to_string() const;
//
// 2. The following standard template functions are not provided:
//
//    template<class E, class T, x_size_t N> 
//      basic_istream<E, T>& operator>>(basic_istream<E, T>& is, bitset<N>& x);
//    template<class E, class T, x_size_t N>
//      basic_ostream<E, T>& operator<<(basic_ostream<E, T>& os,const bitset<N>& x);
//
///////////////////////////////////////////////////////////////////////////////
//
// Performance
//
// The time-complexity of each function is described near its declaration as 
// O(x), where x is some function of the arguments and *this. Examples:
//
//    O(x)       The time depends on the parameter x.
//    O(N)       The time depends on the template parameter N (the number of bits).
//
// If the complexity is omitted, it is O(1).
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_string.h"

namespace x_std 
{

    ///////////////////////////////////////////////////////////////////////////
    //  
    // class bitset<N>
    //
    // A sequence of N bits. A bit is set if its value is true or 1, reset if 
    // its value is false or 0. To flip a bit is to change its value from false 
    // to true or true to false. When converting between an object of class 
    // bitset<N> and an object of some integral type, bit position j 
    // corresponds to the bit value 1 << j. The integral value corresponding to 
    // two or more bits is the sum of their bit values.
    //
    ///////////////////////////////////////////////////////////////////////////

    template<x_size_t N>
    class bitset 
    {
      public:

        typedef x_bool          element_type    ;
        static const x_size_t   bitset_size     ; // The size of the bitset (template argument N).

        //---------------------------------------------------------------------
        // A bit reference
        //---------------------------------------------------------------------
        // This allows a single bit to easily be accessed, as in BitSet1[i] = 
        // BitSet2[j], but still allowing the bit-set to be packed to save 
        // memory.
        class reference
        {
          public :

            reference() {}
            reference( const reference & rhs ) : bits(rhs.bits) , pos(rhs.pos) {}

            // Assign b to the bit.
            reference & operator = ( x_bool b ) { bits->set(pos,b); return *this; }

            // Assign x to the bit.
            reference & operator = ( const reference & x ) { bits->set(pos,x); return *this; }

            // What is the flipped value of the bit?
            x_bool      operator ~ () const { return !bits->at(pos); }

            // What is the value of the bit?
            operator    x_bool     () const { return const_cast< const bitset * >(bits)->at(pos); }

            // Flip the bit.
            reference & flip       () { bits->flip(pos); return *this; }

          private :
            friend class bitset<N> ;
            reference( bitset * bits_ , x_size_t pos_ ) : bits(bits_) , pos(pos_) { }
            bitset *    bits    ; // The bitset in which the referenced bit resides.
            x_size_t    pos     ; // The position of the bit in the bitset.
        };
    
        //---------------------------------------------------------------------
        // Construction
        //---------------------------------------------------------------------

        // [O(N)] A bitset which is initially all reset (to false).
        bitset(); 

        // [O(N)] A bitset with those bits j set for which value&(1<<j) is non-zero.
        bitset(unsigned long value); 

        // [O(N)] A bitset from up to n characters at str[pos], using characters '\x00' for true and '\x01' for false.
        explicit bitset 
        ( 
            string &            str                 , 
            string::size_type   pos = 0             , 
            string::size_type   n   = string::npos
        );

        bitset( const bitset & rhs ); 

        bitset & operator = ( const bitset & rhs ); 

        //---------------------------------------------------------------------
        // Indexing and testing
        //---------------------------------------------------------------------

        reference   operator [] ( x_size_t pos )       ; // A reference to the bit at position pos.
        x_bool      operator [] ( x_size_t pos ) const ; // The bit at position pos.
        reference   at          ( x_size_t pos )       ; // A reference to the bit at position pos.
        x_bool      at          ( x_size_t pos ) const ; // The bit at position pos.
        x_size_t    count       ()               const ; // The number of bits set (to true). [O(N)] 
        x_size_t    size        ()               const ; // The number of bits (same as N, or bitset_size).
        x_bool      test        ( x_size_t pos ) const ; // Is the bit at position pos set (to true)?
        x_bool      any         ()               const ; // Is any bit set (to true)? [O(N), but faster than count()!=0] 
        x_bool      none        ()               const ; // Are all bits reset (to false)? [O(N), but faster than count()==0] 

        //---------------------------------------------------------------------
        // Set, reset, flip
        //---------------------------------------------------------------------

        bitset & set     ()                                       ; // Set all the bits (to true). [O(N)] 
        bitset & set     ( x_size_t pos , x_bool value = true )   ; // Change the bit at pos to value.
        bitset & reset   ()                                       ; // Reset all the bits (to false). [O(N)] 
        bitset & reset   ( x_size_t pos )                         ; // Reset the bit at pos (to false).
        bitset & flip    ()                                       ; // Flip all the bits. [O(N)] 
        bitset & flip    ( x_size_t pos )                         ; // Flip the bit at pos.

        //---------------------------------------------------------------------
        // Bit-wise operations
        //---------------------------------------------------------------------

        bitset & operator &=  ( const bitset & rhs )       ; // Change this to the bitwise "and" of this and rhs. [O(N)] 
        bitset & operator |=  ( const bitset & rhs )       ; // Change this to the bitwise "or" of this and rhs. [O(N)] 
        bitset & operator ^=  ( const bitset & rhs )       ; // Change this to the bitwise "xor" of this and rhs. [O(N)] 
        bitset & operator <<= ( x_size_t n )               ; // Shift this to the "left" by n positions. [O(N)] 
        bitset & operator >>= ( x_size_t n )               ; // Shift this to the "right" by n positions. [O(N)] 
        x_bool   operator ==  ( const bitset & rhs ) const ; // Are this and rhs the same bit sequence? [O(N)] 
        x_bool   operator !=  ( const bitset & rhs ) const ; // Are this and rhs different bit sequences? [O(N)] 
        bitset   operator &   ( const bitset & rhs ) const ; // Return a new bitset from the bitwise "and" of this and rhs. [O(N)] 
        bitset   operator |   ( const bitset & rhs ) const ; // Return a new bitset from the bitwise "or" of this and rhs. [O(N)] 
        bitset   operator ^   ( const bitset & rhs ) const ; // Return a new bitset from the bitwise "xor" of this and rhs. [O(N)] 
        bitset   operator <<  ( x_size_t n   ) const       ; // Return a new bitset from this shifted to the "left" by n. [O(N)] 
        bitset   operator >>  ( x_size_t n   ) const       ; // Return a new bitset from this shifted to the "right" by n. [O(N)] 
        bitset   operator ~   ()               const       ; // Return a new bitset from the flipped value of this. [O(N)] 

        //---------------------------------------------------------------------
        // Miscellaneous
        //---------------------------------------------------------------------

        // The sum of the bit values: (*this)[0] + 2*(*this)[1] + 4*(*this)[2] ...
        unsigned long   to_ulong () const ; // [O(N)] 

        // A string of values '\x00' (for true) and '\x00' (false) built from (*this)[0], (*this)[1], ...
        string          to_string() const ; // [O(N)] 

      private :

        //---------------------------------------------------------------------
        // Miscellaneous
        //---------------------------------------------------------------------

        // We implement the bitset as an array of chunks of this size. Must be unsigned
        typedef unsigned char chunk_type ; 

        enum
        {
            bits_per_chunk  =   8*sizeof(chunk_type)    ,   // Assumes sizeof(x) is in bytes.
            chunk_count     =   ( N + bits_per_chunk - 1 ) / bits_per_chunk , // Number of chunks need to store N bits.
            unused_count    =   bits_per_chunk * chunk_count - N , // The unused bits in the last chunk.
            all_bits        =   chunk_type(-1)          , // All the bits in each chunk, except the last chunk
            last_bits       =   chunk_type(all_bits>>unused_count) , // All the bits in the last chunk.
        };
        chunk_type chunks[chunk_count]; //todo:Explain organization of bits

        // The chunk index [0,chunk_count) for the bit at position pos [0,N).
        inline x_size_t find_chunk( x_size_t pos ) const { X_STD_ASSERT(pos>=0 && pos<N); return pos / bits_per_chunk ; }

        // The bit mask to identify the bit in the chunk for the bit in *this at position pos.
        inline chunk_type find_bit  ( x_size_t pos ) const { return chunk_type( 1 << ( pos % bits_per_chunk ) ); }

    };

    ///////////////////////////////////////////////////////////////////////////
    //  
    // Implementation details 
    //
    ///////////////////////////////////////////////////////////////////////////

    //-------------------------------------------------------------------------
    template<x_size_t N> const x_size_t bitset<N>::bitset_size = N ;
    template<x_size_t N> x_size_t       bitset<N>::size        () const { return N; }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N>::bitset()
    {
        x_memset( chunks , 0 , sizeof(chunks) );
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N>::bitset( const bitset & rhs )
    {
        x_memcpy( chunks , rhs.chunks , sizeof(chunks) );
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N>::bitset
    ( 
        string &            str                 , 
        string::size_type   pos                 , 
        string::size_type   n                  
    )
    {
        reset();
        X_STD_ASSERT( pos >= 0 && pos < str.size() );
        n = x_std::min( str.size()-pos , n );
        x_size_t i = 0 ;
        for( ; n > 0 ; -- n , ++ pos , ++ i )
        {
            if( str[pos] == 1 )
            {
                set(i);
            }
            else 
            {
                X_STD_ASSERT( str[pos]==0 ); // Each string character must be 0 or 1.
            }
        }
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::operator = ( const bitset & rhs )
    {
        x_memcpy( chunks , rhs.chunks , sizeof(chunks) );
        return *this ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    x_size_t bitset<N>::count() const
    {
        // Total them up chunk by chunk. 
        x_size_t            count   =   0       ;
        const chunk_type *  limit   =   &chunks[chunk_count]; // Stop before here
        for( const chunk_type * pChunk = chunks ; pChunk < limit ; ++ pChunk )
        {
            // For each chunk, count by shifting and testing the rightmost bit
            // until the chunk is empty.
            for( chunk_type chunk = *pChunk ; chunk != 0 ; chunk >>= 1 )
            {
                if( (chunk&1) != 0 ) { ++ count ; } 
            }
        }
        return count ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    x_bool bitset<N>::any() const
    {
        const chunk_type *  limit   =   &chunks[chunk_count]; // Stop before here
        for( const chunk_type * pChunk = chunks ; pChunk < limit ; ++ pChunk )
        {
            if( pChunk[0] != 0 ) // Is any bit in the chunk set?
            {
                return true ; // <===== unstructured return.
            }
        }
        return false ; // No set bits were found.
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    x_bool bitset<N>::none() const
    {
        const chunk_type *  limit   =   &chunks[chunk_count]; // Stop before here
        for( const chunk_type * pChunk = chunks ; pChunk < limit ; ++ pChunk )
        {
            if( pChunk[0] != 0 ) // Is any bit in the chunk set?
            {
                return false ; // <===== unstructured return.
            }
        }
        return true ; // No set bits were found.
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::set()
    {
        const chunk_type * limit = &chunks[chunk_count-1]; // Stop before here (-1 to handle the last chunk specially).
        for( chunk_type * pChunk = chunks ; pChunk < limit ; ++ pChunk )
        {
            pChunk[0] = all_bits ;
        }
        chunks[chunk_count-1] = last_bits ; // Handle the last chunk specially so that the unused bits remain 0.
        return *this ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::reset()
    {
        const chunk_type * limit = &chunks[chunk_count]; 
        for( chunk_type * pChunk = chunks ; pChunk < limit ; ++ pChunk )
        {
            pChunk[0] = 0 ;
        }
        return *this ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::flip()
    {
        const chunk_type * limit = &chunks[chunk_count]; 
        for( chunk_type * pChunk = chunks ; pChunk < limit ; ++ pChunk )
        {
            pChunk[0] = ~pChunk[0] ;
        }
        return *this ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::set( x_size_t pos , x_bool value )
    {
        const x_size_t      index       =   find_chunk(pos) ;
        const chunk_type    bit_mask    =   find_bit(pos)   ;
        if( value )
        {
            chunks[index] |= bit_mask ;
        }
        else
        {
            chunks[index] &= ~bit_mask ;
        }
        return *this ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::reset( x_size_t pos )
    {
        const x_size_t      index       =   find_chunk(pos) ;
        const chunk_type    bit_mask    =   find_bit(pos)   ;
        chunks[index] &= ~bit_mask ;
        return *this ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::flip( x_size_t pos )
    {
        const x_size_t      index       =   find_chunk(pos) ;
        const chunk_type    bit_mask    =   find_bit(pos)   ;
        chunks[index] ^= bit_mask ;
        return *this ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    x_bool bitset<N>::test( x_size_t pos ) const
    {
        const x_size_t      index       =   find_chunk(pos) ;
        const chunk_type    bit_mask    =   find_bit(pos)   ;
        return ( chunks[index] & bit_mask ) != 0 ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N>::bitset( unsigned long value )
    {
        X_STD_ASSERT( sizeof(value) <= N || value < ( 1UL << N ) );
        //todo:Inefficient
        reset();
        for( x_size_t n = 0 ; value != 0 ; value >>= 1 , ++ n )
        {
            if( (value&1) != 0 )
            {
                set(n);
            }
        }
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    x_bool bitset<N>::operator [] ( x_size_t pos ) const 
    {
        const x_size_t      index       =   find_chunk(pos) ;
        const chunk_type    bit_mask    =   find_bit(pos)   ;
        return ( chunks[index] & bit_mask ) != 0 ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    x_bool bitset<N>::at( x_size_t pos ) const 
    {
        const x_size_t      index       =   find_chunk(pos) ;
        const chunk_type    bit_mask    =   find_bit(pos)   ;
        return ( chunks[index] & bit_mask ) != 0 ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    typename bitset<N>::reference bitset<N>::operator [] ( x_size_t pos ) 
    {
        return reference( this , pos );
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    typename bitset<N>::reference bitset<N>::at( x_size_t pos ) 
    {
        return reference( this , pos );
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::operator &= ( const bitset & rhs )
    {
        const chunk_type * limit = &chunks[chunk_count]; 
        chunk_type *       lhs_chunk = this -> chunks ;
        const chunk_type * rhs_chunk = rhs  .  chunks ;
        for( ; rhs_chunk < limit ; ++ lhs_chunk , ++ rhs_chunk )
        {
            lhs_chunk[0] &= rhs_chunk[0] ;
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::operator |= ( const bitset & rhs )
    {
        const chunk_type * limit = &chunks[chunk_count]; 
        chunk_type *       lhs_chunk = this -> chunks ;
        const chunk_type * rhs_chunk = rhs  .  chunks ;
        for( ; rhs_chunk < limit ; ++ lhs_chunk , ++ rhs_chunk )
        {
            lhs_chunk[0] |= rhs_chunk[0] ;
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> & bitset<N>::operator ^= ( const bitset & rhs )
    {
        const chunk_type * limit = &chunks[chunk_count]; 
        chunk_type *       lhs_chunk = this -> chunks ;
        const chunk_type * rhs_chunk = rhs  .  chunks ;
        for( ; lhs_chunk < limit ; ++ lhs_chunk , ++ rhs_chunk )
        {
            lhs_chunk[0] ^= rhs_chunk[0] ;
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    x_bool bitset<N>::operator == ( const bitset & rhs) const
    {
        return x_memcmp( this->chunks , rhs.chunks , sizeof(this->chunks) ) == 0;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    x_bool bitset<N>::operator != ( const bitset & rhs) const
    {
        return x_memcmp( this->chunks , rhs.chunks , sizeof(this->chunks) ) != 0;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> bitset<N>::operator & ( const bitset & rhs ) const
    {
        bitset<N> result = *this ;
        result &= rhs ;
        return result ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> bitset<N>::operator | ( const bitset & rhs ) const
    {
        bitset<N> result = *this ;
        result |= rhs ;
        return result ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> bitset<N>::operator ^ ( const bitset & rhs ) const
    {
        bitset<N> result = *this ;
        result ^= rhs ;
        return result ;
    }

    //-------------------------------------------------------------------------
    template< x_size_t N >
    bitset<N> bitset<N>::operator ~ () const
    {
        bitset<N> result = *this ;
        result.flip();
        return result ;
    }

#if 0 //todo:

        bitset & operator <<= ( x_size_t  n )             ; // Shift this to the "left" by n positions. [O(N)] 
        bitset & operator >>= ( x_size_t  n )             ; // Shift this to the "right" by n positions. [O(N)] 
        bitset   operator <<  ( x_size_t n   ) const      ; // Return a new bitset from this shifted to the "left" by n. [O(N)] 
        bitset   operator >>  ( x_size_t n   ) const      ; // Return a new bitset from this shifted to the "right" by n. [O(N)] 
        unsigned long   to_ulong () const ; // [O(N)] 
        string          to_string() const ; // [O(N)] 
#endif

}

#endif
