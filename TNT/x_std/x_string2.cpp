///////////////////////////////////////////////////////////////////////////////
//
// String-related extensions to x_string.h
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_string2.h"

//------------------------------------------------------------------------
// Construction 
//------------------------------------------------------------------------
x_std::strings::strings( const strings & rhs ) // Construct a string with a copy of rhs.
{
    *this = rhs ;
}

//------------------------------------------------------------------------
// Copy rhs into *this, copying all of the strings as well.
//------------------------------------------------------------------------
x_std::strings & x_std::strings::operator = ( const strings & rhs ) 
{
    clear();
    m_Strings.reserve( rhs.size() );
    for( s32 WhichString =  0 ; WhichString < rhs.size() ; ++WhichString )
    {
        append( rhs[WhichString] );
    }
    return *this ;
}

//------------------------------------------------------------------------
// Destruction
//------------------------------------------------------------------------
x_std::strings::~strings()
{
    clear();
}

//------------------------------------------------------------------------
// Clear
//------------------------------------------------------------------------
void x_std::strings::clear()
{
    while( !empty() )
    {
        pop_back();
    }
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
void x_std::strings::pop_back()
{
    X_STD_ASSERT( !empty() );

    x_std::string * pString = m_Strings.back();
    m_Strings.pop_back();
    delete pString ;
    pString = 0 ;

}

//------------------------------------------------------------------------
// Insert a string at position p0, moving over entries at and after that position.
//------------------------------------------------------------------------
x_std::strings & x_std::strings::insert( s32 p0 , x_std::string * x )
{
    m_Strings.insert( m_Strings.begin()+p0 , x );
    return *this ;
}

//------------------------------------------------------------------------
// Erase the pos'th element. Return pos.
//------------------------------------------------------------------------
s32 x_std::strings::erase( s32 pos )
{
    X_STD_ASSERT( pos >= 0 && pos <= size() );
    if( pos < size() )
    {
        x_std::string * pString = m_Strings[pos];
        m_Strings.erase( m_Strings.begin()+pos );
        delete pString ;
        pString = 0 ;
    }
    return pos ;
}

//------------------------------------------------------------------------
// Erase up to n elements starting with the pos'th element. Return pos.
//------------------------------------------------------------------------
s32 x_std::strings::erase( s32 pos , s32 n )
{
    X_STD_ASSERT( pos >= 0 && pos <= size() );
    while( pos < size() && n > 0 )
    {
        erase(pos);
        n--;
    }
    return pos ;
}

//------------------------------------------------------------------------
// What is the first index starting at pos where string s can be found? < 0 if not found.
//------------------------------------------------------------------------
s32 x_std::strings::find( const char * s , s32 pos )
{
    s32 Index = -1 ; // The index where s is found, or <0 if not found.
    for( ; Index < 0 &&  pos < size() ; pos++ )
    {
        if( at(pos) == s )
        {
            Index = pos ;
        }
    }
    return Index ;
}

//------------------------------------------------------------------------
// Resize
//------------------------------------------------------------------------
void x_std::strings::resize( s32 n , const char * s )
{
    X_STD_ASSERT( n >= 0 );
    while( size() > n )
    {
        pop_back() ;        
    }
    while( size() < n )
    {
        push_back( s );
    }
    X_STD_ASSERT( size() == n );
}

//------------------------------------------------------------------------
// Add a new string at the end
//------------------------------------------------------------------------
void x_std::strings::push_back( x_std::string * x ) 
{
    m_Strings.push_back( x );
}
