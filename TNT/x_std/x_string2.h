#ifndef X_STD_STRING2_H
#define X_STD_STRING2_H

///////////////////////////////////////////////////////////////////////////////
//
// Non-standard extensions based on STL <string>
//
// This declares templates and classes for additional string
// classes and functions beyond those provided in x_string.h.
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_string.h"
#include    "x_vector.h"
#include	"x_types.hpp"

namespace x_std 
{

    ///////////////////////////////////////////////////////////////////////////
    //  
    // class strings
    //
    // A dynamically-sized vector of x_std::string.
    //
    ///////////////////////////////////////////////////////////////////////////

    class strings
    {
      public:

        //------------------------------------------------------------------------
        // Construction and assignment
        //------------------------------------------------------------------------
        strings() { } 
        strings( const strings & rhs )              ; // Construct a string with a copy of rhs.
        strings & operator = ( const strings & rhs ) ; // Copy rhs into *this, copying all of the strings as well.
        ~strings();

        //------------------------------------------------------------------------
        // Indexing operations, front and back
        //------------------------------------------------------------------------
        const x_std::string &     at         ( s32 pos ) const    { return *m_Strings[pos]; }
        x_std::string &           at         ( s32 pos )          { return *m_Strings[pos]; }
        const x_std::string &     operator[] ( s32 pos ) const    { return *m_Strings[pos]; }
        x_std::string &           operator[] ( s32 pos )          { return *m_Strings[pos]; }
              x_std::string &     front      ()                   { return *m_Strings.front(); }
        const x_std::string &     front      ()          const    { return *m_Strings.front(); }
              x_std::string &     back       ()                   { return *m_Strings.back(); }
        const x_std::string &     back       ()          const    { return *m_Strings.back(); }

        //------------------------------------------------------------------------
        // Length, size, capacity, and so on
        //------------------------------------------------------------------------
        s32     size     () const                   { return m_Strings.size(); }
        xbool   empty    () const                   { return m_Strings.empty(); }
        void    resize   ( s32 n , const char * s = "" );  // If the list is lengthened, new elements use the given string.
        void    clear    () ; // Clear all the strings, deallocating them.                       

        //------------------------------------------------------------------------
        // Add a new string at the end
        //------------------------------------------------------------------------
        void push_back( x_std::string * x ) ; // Insert x at the end of *this. Takes ownership of x.
        void push_back( const char * s )                            { push_back( new x_std::string(s)         ); }
        strings & operator += ( const x_std::string & rhs )              { push_back( new x_std::string(rhs)       ); return *this ; }
        strings & operator += ( const char * s )                   { push_back( new x_std::string(s)         ); return *this ; }
        strings & append( const x_std::string & str )                    { push_back( new x_std::string(str)       ); return *this ; }
        strings & append( const x_std::string & str , s32 pos , s32 n )  { push_back( new x_std::string(str,pos,n) ); return *this ; }
        strings & append( const char * s , s32 n )                 { push_back( new x_std::string(s,n)       ); return *this ; }
        strings & append( const char * s )                         { push_back( new x_std::string(s)         ); return *this ; }

        //------------------------------------------------------------------------
        // Insertion
        //------------------------------------------------------------------------
        // Insert a x_std::string at position p0, moving over entries at and after that position.
        strings & insert( s32 p0 , x_std::string * x );
        strings & insert( s32 p0 , const x_std::string & str )                       { return insert( p0, new x_std::string(str)       ); }
        strings & insert( s32 p0 , const x_std::string & str , s32 pos , s32 n )     { return insert( p0, new x_std::string(str,pos,n) ); }
        strings & insert( s32 p0 , const char * s , s32 n )                    { return insert( p0, new x_std::string(s,n)       ); }
        strings & insert( s32 p0 , const char * s )                            { return insert( p0, new x_std::string(s)         ); }

        //------------------------------------------------------------------------
        // Search
        //------------------------------------------------------------------------
        s32 find( const char * s , s32 pos = 0 ); // What is the first index starting at pos where string s can be found? < 0 if not found.

        //------------------------------------------------------------------------
        // Deletion
        //------------------------------------------------------------------------
        void        pop_back    ( )                 ; // Remove the last element from *this.
        s32         erase       ( s32 pos )         ; // Erase the pos'th element. Return pos.
        s32         erase       ( s32 pos , s32 n ) ; // Erase up to n elements starting with the pos'th element. Return pos.

      private :

        x_std::vector<x_std::string *>  m_Strings ;
    };

}

#endif
    
