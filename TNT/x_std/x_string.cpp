///////////////////////////////////////////////////////////////////////////////
//
// String-related declarations based on STL <string>
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_string.h"
#include    "x_algorithm.h"
#include    <stdarg.h>
#include    <stdio.h>


//------------------------------------------------------------------------
// The data reserved for the empty string.
//------------------------------------------------------------------------
x_std::string::CEmptyString x_std::string::m_EmptyString = 
    //  Length   Capacity     Data
    {  {  0    ,    0     } , ""    }
;
char * x_std::string::m_EmptyData = m_EmptyString.m_Data ;

const x_std::string::size_type x_std::string::npos = size_type(-1) ; 

//------------------------------------------------------------------------
// Allocate a new x_std::string with the given capacity. 
//------------------------------------------------------------------------
char * x_std::string::New( size_type Capacity ) 
{ 
    Capacity += 10 ; // Always add a little more ... this might prevent some unnecessary reallocations (depending on how the string is used).
    const size_type   NBytes  = sizeof(CPrefix) + Capacity + 1;  // +1 for trailing null.
    CPrefix *   Prefix  = reinterpret_cast<CPrefix *>( new char[NBytes] );

    X_STD_ASSERT( Capacity <= MaxSize );
    Prefix->Length      =   0           ;
    Prefix->Capacity    =   Capacity    ;

    char * Data = reinterpret_cast<char *>( &Prefix[1] );
    Data[0] = 0; // Add null termination.
    return Data;
}

//------------------------------------------------------------------------
// Delete the data allocated with New().
//------------------------------------------------------------------------
inline void x_std::string::Delete( char * Data )
{
    if( Data != m_EmptyData )
    {
        CPrefix *   Prefix = &( reinterpret_cast<CPrefix *>(Data) )[-1];
        char *      Memory = reinterpret_cast<char *>( Prefix );
        delete[] Memory;
        Data  = m_EmptyData ;
    }
}


//------------------------------------------------------------------------
// Add Length characters to the string from Source.
//------------------------------------------------------------------------
void x_std::string::Append( const char * Source , size_type Length )
{
    if( Length > 0 )    
    {
        const size_type OldLength = this->length()        ;
        const size_type NewLength = OldLength + Length    ;

        reserve(NewLength);
        x_memcpy( & m_Data[OldLength] , Source , Length ); // Add in characters.
        m_Data[NewLength]   =   0           ; // Add null terminator.
        this->Prefix().Length   =   NewLength   ; // Set new length.
    }
}

//------------------------------------------------------------------------
// Insert Length characters from Source into *this at position pos.
//------------------------------------------------------------------------
void x_std::string::Insert( size_type pos , const char * Source , size_type Length )
{
    const size_type OldLength  = this->length()               ;
    const x_int    MoveLength = x_int(OldLength) - x_int(pos)  ; // How many characters need to be moved over to make room?
    if( pos < 0 || MoveLength < 0 )
    {
        X_STD_ASSERT(false);
    }
    else if( Length > 0 )    
    {
        const size_type NewLength = OldLength + Length    ;
        reserve(NewLength);
        x_memmove( &m_Data[ pos + Length ] , &m_Data[pos] , MoveLength );
        X_STD_ASSERT( m_Data != m_EmptyData );
        if( Source != 0 )
        {
            x_memcpy( &m_Data[pos] , Source , Length ); // Insert characters.
        }
        m_Data[NewLength]   =   0           ; // Add null terminator.
        this->Prefix().Length   =   NewLength   ; // Set new length.
    }
}

//------------------------------------------------------------------------
// Erase up to n characters from *this at position pos.
//------------------------------------------------------------------------
void x_std::string::Erase( size_type pos , size_type n )
{
    const size_type OldLength         = this->length()                  ;
    const x_int     MaxDeleteLength   = x_int(OldLength) - x_int(pos)   ;
    if( pos < 0 )
    {
        X_STD_ASSERT(false);
    }
    else if( n > 0 )
    {
        const x_int DeleteLength = x_std::min( MaxDeleteLength , x_int(n) );
        if( DeleteLength > 0 )
        {
            X_STD_ASSERT( m_Data != m_EmptyData );
            const x_int NewLength  = OldLength        - DeleteLength  ;
            const x_int MoveLength = MaxDeleteLength  - DeleteLength  ;
            x_memmove( &m_Data[pos] , &m_Data[pos+DeleteLength] , MoveLength );
            m_Data[NewLength]   =   0           ; // Add null terminator.
            this->Prefix().Length   =   NewLength   ; // Set new length.
        }
    }
}

//------------------------------------------------------------------------
// Replace up to n0 characters in *this at position pos with n 
// characters from source.
//------------------------------------------------------------------------
char * x_std::string::Replace( size_type pos , size_type n0 , const char * Source , size_type n )
{
    //tood:Fix this and other abuses of x_int(?)
    const size_type OldLength         = this->length()                  ;
    const x_int     MaxReplaceLength  = x_int(OldLength) - x_int(pos)   ;
    const x_int     ReplaceLength     = x_std::max( x_int(0) , x_std::min( x_int(n0) , MaxReplaceLength ) ); // Number of characters replaced
    const x_int     Expansion         = n - ReplaceLength             ; // How many characters we need to expand the length by. If < 0, is how much we need to shrink by.
    const size_type NewLength         = OldLength + Expansion         ;
    char *    ReplacedAt        = 0;
    if( pos < 0 )
    {
        X_STD_ASSERT(false);
    }
    else if( Expansion > 0  ) // Replacing with a longer string from Source?
    {
        Insert( pos , 0 , Expansion ); // Insert room for Expansion additional characters.
        X_STD_ASSERT( m_Data != m_EmptyData );
        ReplacedAt = &m_Data[pos];
        if( n > 0 && Source != 0 )
        {
            x_memmove( ReplacedAt , Source , n ); // Copy in the replacement from Source.
        }
        m_Data[NewLength]   =   0           ; // Add null terminator.
        this->Prefix().Length   =   NewLength   ; // Set new length.
    }
    else if( Expansion < 0 ) // Replacing with a shorter string from Source?
    {
        Erase( pos , -Expansion );
        X_STD_ASSERT( m_Data != m_EmptyData );
        ReplacedAt = &m_Data[pos];
        if( n > 0 && Source != 0 )
        {
            x_memmove( ReplacedAt , Source , n );
        }
    }
    else if( ReplaceLength != 0 ) // Replacing with the same sized string from Source
    {
        X_STD_ASSERT( m_Data != m_EmptyData );
        ReplacedAt = &m_Data[pos];
        if( Source != 0 )
        {
            x_memmove( ReplacedAt , Source , ReplaceLength );
        }
    }
    return ReplacedAt ;
}


//------------------------------------------------------------------------
// Make sure the x_std::string has a capacity of at least MinCapacity.
// Note: Destroys the contents and sets length()=0
//------------------------------------------------------------------------
inline void x_std::string::Reallocate( size_type MinCapacity )
{
    X_STD_ASSERT( MinCapacity >= 0 );
    if( capacity() == 0 || capacity() < MinCapacity )
    {
        this->Delete();
        m_Data = New(MinCapacity);
    }
    this->Prefix().Length = 0;
    X_STD_ASSERT( m_Data != 0 );
    X_STD_ASSERT( m_Data != m_EmptyData );
}


//------------------------------------------------------------------------
// Deallocate *this and set m_Data == m_EmptyData.
//------------------------------------------------------------------------
void x_std::string::Delete()
{
    if( m_Data != m_EmptyData )
    {
        Delete(m_Data);
        m_Data = m_EmptyData;
    }
}

//------------------------------------------------------------------------
// Construct a string with a copy of rhs.
//------------------------------------------------------------------------
x_std::string::string( const x_std::string & rhs )
{
    X_STD_ASSERT( rhs.m_Data != 0 );
    const x_int Length = rhs.length();
    if( Length > 0 )
    {
        m_Data = New( Length );
        x_memcpy( m_Data , rhs.data() , Length+1 ); // +1 to also copy trailing null. 
        this->Prefix().Length = Length;
    }
    else
    {
        m_Data = m_EmptyData ;
    }
}

//------------------------------------------------------------------------
// Construct a string with a copy of s. 
//------------------------------------------------------------------------
x_std::string::string( const char * s )
{
    const x_int Length = s==0 ? 0 : x_strlen(s);
    m_Data = m_EmptyData ;
    if( Length > 0 )
    {
        const x_int Length = x_strlen(s);
        m_Data = New( Length );
        x_memcpy( m_Data , s , Length+1 ); // +1 to also copy trailing null. 
        this->Prefix().Length = Length;
    }
}

//------------------------------------------------------------------------
// Construct a string out of n characters c.
//------------------------------------------------------------------------
x_std::string::string( size_type n , char c )
{
    m_Data = m_EmptyData ;
    if( n > 0 )
    {
        m_Data = New(n);
        x_memset( m_Data , c , n );
        m_Data[n] = 0 ; // Add null terminator.
        this->Prefix().Length = n;
    }
}

//------------------------------------------------------------------------
// Construct a string from n characters at s
//------------------------------------------------------------------------
x_std::string::string( const char * s , size_type n )
{
    m_Data = m_EmptyData ;
    X_STD_ASSERT( s != 0 );
    if( n > 0 )
    {
        m_Data = New(n);
        x_memcpy( m_Data , s , n );
        m_Data[n] = 0 ; // Add null terminator.
        this->Prefix().Length = n;
    }
}

//------------------------------------------------------------------------
// Construct a string from n characters of rhs at position pos.
//------------------------------------------------------------------------
x_std::string::string( const x_std::string & rhs , size_type pos , size_type n  )
{
    X_STD_ASSERT( rhs.m_Data != 0 );
    m_Data = m_EmptyData ;
    X_STD_ASSERT( pos >= 0 && pos <= rhs.length() );
    if( pos + n >= rhs.length() )
    {
        n  = rhs.length() - pos ;
    }
    if( n > 0 )
    {
        m_Data = New(n);
        x_memcpy( m_Data , &rhs.data()[pos] , n );
        m_Data[n] = 0 ; // Add null terminator.
        this->Prefix().Length = n;
    }
}

//------------------------------------------------------------------------
// Construct a string from the sequence in [first,last).
//------------------------------------------------------------------------
x_std::string::string( const_iterator first , const_iterator last )
{
    m_Data = m_EmptyData ;
    if( last.m_Data > first.m_Data )
    {
        const size_type n = last.m_Data - first.m_Data ;
        m_Data = New(n);
        x_memcpy( m_Data , & *first  , n );
        this->Prefix().Length = n;
    }
}

//------------------------------------------------------------------------
// Copy rhs into *this.
//------------------------------------------------------------------------
x_std::string & x_std::string::operator = ( const x_std::string & rhs )
{
    X_STD_ASSERT( rhs.data() != 0 );

    if( this != &rhs ) // If not assigning to self.
    {
        const x_int Length = rhs.length();
        Reallocate( Length );
        x_memcpy( m_Data , rhs.data() , Length+1 ); // +1 to copy trailing null.
        Prefix().Length = Length;
    }

    return *this;
}

//------------------------------------------------------------------------
// Copy null-terminated s into *this.
//------------------------------------------------------------------------
x_std::string & x_std::string::operator = ( const char * s )
{
    if( s == 0 ) //todo: Non-standard behaviour
    {
        this->Delete();
    }
    else 
    {
        const x_int NewLength = x_strlen(s);
        this->Reallocate(NewLength);
        x_memcpy( m_Data , s , NewLength+1 ); // +1 to copy trailing null.
        this->Prefix().Length = NewLength;
    }
    return *this;
}

//------------------------------------------------------------------------
// Make *this a string with a single character c.
//------------------------------------------------------------------------
x_std::string & x_std::string::operator = ( char c )
{
    this->Reallocate(1); // Make sure there is enough room for a single character.
    X_STD_ASSERT( m_Data != m_EmptyData );
    m_Data[0] = c ; // Add the character.
    m_Data[1] = 0 ; // Add null termination.
    this->Prefix().Length = 1;
    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
x_std::string & x_std::string::assign( const x_std::string & str , size_type pos , size_type n )
{
    X_STD_ASSERT( str.m_Data != 0 );
    if( pos >= str.length() )
    {
        n = 0 ;
    }
    else
    {
        const size_type max_length = str.length() - pos ;
        if( n > max_length ) { n = max_length ; }
    }

    if( n <= 0 )
    {
        resize(0);
    }
    else
    {
        Reallocate(n);
        x_memcpy( m_Data , &str.m_Data[pos] , n );
        m_Data[n] = 0 ; // Add null terminator.
        Prefix().Length = n ;
    }
    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
x_std::string & x_std::string::assign( const char * s , size_type n )
{
    X_STD_ASSERT( s != 0 );
    if( n > 0 )
    {
        Reallocate(n);
        x_memcpy( m_Data , s , n );
        m_Data[n] = 0 ; // Add null terminator.
        Prefix().Length = n ;
    }
    else
    {
        resize(0);
    }
    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
x_std::string & x_std::string::assign( size_type n , char c )
{
    if( n <= 0 )
    {   
        resize(0);
    }
    else
    {
        Reallocate(n);
        x_memset( m_Data , c , n );
        m_Data[n] = 0 ; // Add null terminator.
        Prefix().Length = n ;
    }
    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
x_std::string & x_std::string::assign( const_iterator first , const_iterator last )
{
    if( last.m_Data > first.m_Data )
    {
        const size_type n = last.m_Data - first.m_Data ;
        resize( n );
        x_memcpy( m_Data , & *first , n );
        Prefix().Length = n;
    }
    else
    {
        resize(0);
    }
    return *this;
}

//------------------------------------------------------------------------
// Return a pointer to the underlying data.
//------------------------------------------------------------------------
char * x_std::string::x_data () 
{ 
    reserve( 0 );
    return m_Data; 
}  

//------------------------------------------------------------------------
// Private, unsafe version of resize().
//------------------------------------------------------------------------
void x_std::string::x_resize( size_type n )
{
    n = x_std::max(n,size_type(0));
    reserve( n );
    m_Data[n]           = 0 ; // Add null termination.
    this->Prefix().Length   = n ; // Set new length.
}

//------------------------------------------------------------------------
// Change the size of a string
//------------------------------------------------------------------------
void x_std::string::resize( size_type n , char c )
{
    const size_type old_length = length();
    X_STD_ASSERT(n>=0);
    if( old_length < n ) // Need to lengthen the string?
    {
        const size_type new_chars = n - old_length; // Number of characters to add.
        reserve(n);
        x_memset( &m_Data[old_length] , c , new_chars ); // Fill in new characters.
        m_Data[n]           = 0 ; // Add null termination at new length.
        this->Prefix().Length   = n ; // Set new length.
    }
    else if( old_length > n ) // Need to shorten the string?
    {
        X_STD_ASSERT( m_Data != m_EmptyData );
        m_Data[n]           = 0 ; // Add null termination at new length.
        this->Prefix().Length   = n ; // Set new length.
    }
    X_STD_ASSERT( capacity() >= n );
}

//------------------------------------------------------------------------
// Reserve space in the string
//------------------------------------------------------------------------
// After this function, m_Data != m_EmptyData.
void x_std::string::reserve( size_type n )
{
    if( n <= 0 )
    {
    }
    else if( m_Data == 0 )
    {
        Reallocate(n);
    }
    else if( capacity() < n )
    {
        const size_type Length = length();
        X_STD_ASSERT( Length < n );
        char * OldData = m_Data   ;
        char * NewData = New(n)   ;

        x_memcpy( NewData , OldData , Length+1 ); // Copy old contents into new data. (+1 to copy trailing null)
        m_Data = NewData ;
        this->Prefix().Length = Length;
        Delete(OldData);
    }
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
x_std::string & x_std::string::operator += ( const x_std::string & rhs )
{
    Append( rhs.m_Data , rhs.length() );
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
x_std::string & x_std::string::operator += ( const char * s )
{
    const x_int LengthToAdd = s == 0 ? 0 : x_strlen(s);
    Append( s , LengthToAdd );
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
x_std::string & x_std::string::operator += ( char c )
{
    Append( &c , 1 );
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
x_std::string & x_std::string::append( const x_std::string & str , size_type pos , size_type n )
{
    if( n > 0 )
    {
        if( pos + n >= str.length() )
        {
            n = str.length() - pos ;
        }
        if( n > 0 )
        {
            Append( &str.m_Data[pos] , n );
        }
    }
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
x_std::string & x_std::string::append( const char * s , size_type n )
{
    X_STD_ASSERT( s != 0 );
    if( n > 0 )
    {
        Append( s , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
x_std::string & x_std::string::append( size_type n , char c )
{
    if( n > 0 )
    {
        const size_type OldLength = this->length()    ;
        const size_type NewLength = OldLength + n     ;

        reserve(NewLength);
        x_memset( &m_Data[OldLength] , c , n ); // Add in new characters.
        m_Data[NewLength]       =   0           ; // Add null terminator.
        this->Prefix().Length   =   NewLength   ; // Set new length.
    }
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
x_std::string & x_std::string::append( const_iterator first , const_iterator last )
{
    if( last.m_Data > first.m_Data )
    {
        const size_type n = last.m_Data - first.m_Data;
        Append( & *first , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
x_std::string & x_std::string::insert( size_type p0 , const x_std::string & str )
{
    X_STD_ASSERT( str.m_Data != 0 );
    Insert( p0 , str.m_Data , str.length() );
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
x_std::string & x_std::string::insert( size_type p0 , const x_std::string & str , size_type pos , size_type n )
{
    if( n > 0 )
    {
        X_STD_ASSERT( str.m_Data != 0 );
        if( pos + n >= str.length() )
        {
            n = str.length() - pos ;
        }
        Insert( p0 , &str.m_Data[pos] , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
x_std::string & x_std::string::insert( size_type p0 , const char * s , size_type n )
{
    X_STD_ASSERT( s != 0 );
    if( n > 0 )
    {
        Insert( p0 , s , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
x_std::string & x_std::string::insert( size_type p0 , const char * s )
{
    X_STD_ASSERT( s != 0 );
    Insert( p0 , s , x_strlen(s) );
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
x_std::string & x_std::string::insert( size_type p0 , size_type n , char c )
{
    if( n > 0 )
    {
        Insert( p0 , 0 , n ); // Insert an uninitialized gap.
        x_memset( &m_Data[p0] , c , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
x_std::string::iterator x_std::string::insert( iterator it , char c )
{
    const size_type pos = it.m_Data - m_Data ;
    Insert( pos , &c , 1 );
    return iterator( &m_Data[pos] ); // Return new iterator at the newly inserted character.
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
void x_std::string::insert( iterator it , const_iterator first , const_iterator last )
{
    if( last.m_Data > first.m_Data )
    {
        const size_type n   =   last.m_Data - first.m_Data  ;
        const size_type pos =   it.m_Data   - m_Data        ;
        X_STD_ASSERT( pos <= length() );
        insert( pos , & *first , n );
    }
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
void x_std::string::insert( iterator it , size_type n , char c)
{
    insert( it.m_Data-m_Data , n , c );
}

//------------------------------------------------------------------------
// Erasing
//------------------------------------------------------------------------
x_std::string & x_std::string::erase( size_type p0 , size_type n)
{
    Erase( p0 , ( n == npos ? max_size() : n ) );
    return *this;
}

//------------------------------------------------------------------------
// Erasing
//------------------------------------------------------------------------
x_std::string::iterator x_std::string::erase( iterator it )
{
    const size_type pos = it.m_Data - m_Data ;
    Erase( pos , 1 );
    return iterator( &m_Data[pos] );
}

//------------------------------------------------------------------------
// Erasing
//------------------------------------------------------------------------
x_std::string::iterator x_std::string::erase( iterator first , iterator last )
{
    const size_type pos =   first.m_Data - m_Data       ;
    X_STD_ASSERT( pos >= 0 && pos <= length() );
    if( last.m_Data > first.m_Data )
    {
        const size_type n   =   last.m_Data - first.m_Data  ;
        Erase( pos , n );
    }
    return iterator( &m_Data[pos] );
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace( size_type p0 , size_type n0 , const x_std::string & str )
{
    Replace( p0 , n0 , str.m_Data , str.length() );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace( size_type p0 , size_type n0 , const x_std::string & str , size_type pos , size_type n )
{
    X_STD_ASSERT( pos <= str.length() );
    if( n <= 0 )
    {
        n = 0 ;
    }
    if( pos + n >= str.length() )
    {
        n = str.length() - pos ;
    }
    Replace( p0 , n0 , &str.m_Data[pos] , n );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace( size_type p0 , size_type n0 , const char * s , size_type n )
{
    X_STD_ASSERT( s != 0 );
    Replace( p0 , n0 , s , n );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace( size_type p0 , size_type n0 , const char * s )
{
    X_STD_ASSERT( s != 0 );
    const size_type n = x_strlen(s);
    Replace( p0 , n0 , s , n );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace( size_type p0 , size_type n0 , size_type n , char c )
{
    if( n <= 0 )
    {
        n = 0 ;
    }
    char * ReplaceHere = Replace( p0 , n0 , 0 , n );
    if( ReplaceHere != 0 )
    {
        x_memset( ReplaceHere , c , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace( iterator first0 , iterator last0 , const x_std::string & str )
{
    const size_type p0  = first0.m_Data - m_Data          ;
    if( last0.m_Data > first0.m_Data )
    {
        const size_type n0  =   last0.m_Data  - first0.m_Data   ;
        Replace( p0 , n0 , str.m_Data , str.length() );
    }
    else
    {
        insert( p0 , str );
    }
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace( iterator first0 , iterator last0 , const char * s , size_type n)
{
    const size_type p0  = first0.m_Data - m_Data ;
    if( last0.m_Data > first0.m_Data )
    {
        const size_type n0  =   last0.m_Data  - first0.m_Data   ;
        replace( p0 , n0 , s , n );
    }
    else
    {
        insert( p0 , s , n );
    }
    return *this ;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace(iterator first0 , iterator last0 , const char *s )
{
    const size_type p0 = first0.m_Data - m_Data ;
    if( last0.m_Data > first0.m_Data )
    {
        const size_type n0  =   last0.m_Data  - first0.m_Data   ;
        replace( p0 , n0 , s );
    }
    else
    {
        insert( p0 , s );
    }
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace(iterator first0 , iterator last0 , size_type n , char c )
{
    if( last0.m_Data > first0.m_Data )
    {
        const size_type p0  =   first0.m_Data - m_Data          ;
        const size_type n0  =   last0.m_Data  - first0.m_Data   ;
        replace( p0 , n0 , n , c );
    }
    else
    {
        insert( first0 , n , c );
    }
    return *this ;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
x_std::string & x_std::string::replace(iterator first0 , iterator last0 , const_iterator first , const_iterator last)
{
    if( last0.m_Data > first0.m_Data )
    {
        const size_type p0  =   first0.m_Data - m_Data          ;
        const size_type n0  =   last0.m_Data  - first0.m_Data   ;
        const size_type n   =   last.m_Data   > first.m_Data ? last.m_Data  - first.m_Data : 0 ;
        if( first.m_Data >= m_Data && first.m_Data < m_Data+size() && n > 0 ) // second iterators refer to *this?
        {
            const size_type pos = first.m_Data - m_Data ;
            x_std::string Copy = *this ; // We have to make a copy of *this before we do the replacement
            replace( p0 , n0 , &Copy.m_Data[pos] , n );
        }
        else
        {
            replace( p0 , n0 , first.m_Data , n );
        }
    }
    else
    {
        insert( first0 , first , last );
    }
    return *this;
}

//------------------------------------------------------------------------
// Copy to a string
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::copy( char * s , size_type n , size_type pos ) const
{
    X_STD_ASSERT( s != 0 );
    if( n > 0 )
    {
        if( pos + n >= length() )
        {
            n = length() - pos ;
        }
        x_memcpy( s , m_Data , n );
    }
    return n ;
}

//------------------------------------------------------------------------
// Swap
//------------------------------------------------------------------------
void x_std::string::swap( x_std::string & str )
{
    char * old_this = m_Data ;
    m_Data      =   str.m_Data  ;
    str.m_Data  =   old_this    ;
}

//------------------------------------------------------------------------
// Substring
//------------------------------------------------------------------------
x_std::string x_std::string::substr( size_type pos , size_type n ) const
{
    char LocalBuffer[100]  ; // If possible, use this for better performance (fewer string allocations).
    X_STD_ASSERT( pos >= 0 && pos <= length() );
    if( n == npos )
    {
        n = max_size() ;
    }
    else if( n > 0 && pos + n >= length() )
    {
        n = length() - pos ;
    }
    if( n <= 0 )
    {
        return "";
    }
    else if( n < sizeof(LocalBuffer)/sizeof(LocalBuffer[0]) ) // Do we have room to use the local buffer (and add a trailing null)?
    {
        x_memcpy( LocalBuffer , &m_Data[pos] , n );
        LocalBuffer[n] = 0 ; // Add null termination.
        return LocalBuffer; // This will convert the local buffer into a x_std::string
    }
    else
    {
        return x_std::string( *this , pos , n );
    }
}

//------------------------------------------------------------------------
// Compare two character sequences of known length. 
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::Compare( const char * lhs , size_type lhsLength , const char * rhs , size_type rhsLength )
{
    x_int compare ;
    if( lhsLength <= 0 )
    {
        compare = rhsLength <= 0 ? 0 : -1 ; // If rhs is longer, compare is -1
    }
    else if( rhsLength <= 0 )
    {
        compare = +1 ; // Since lhs is longer (we checked for empty lhs above), compare is +1
    }
    else if( lhsLength > rhsLength ) // lhs is longer than rhs?
    {
        compare = x_memcmp( lhs , rhs , rhsLength );
        if( compare == 0 ) // Complete rhs found at start of lhs?
        {
            compare = 1; // (Since lhs is longer)
        }
    }
    else if( rhsLength > lhsLength ) // rhs is longer than lhs?
    {
        compare = x_memcmp( lhs , rhs , lhsLength );
        if( compare == 0 ) // Complete lhs found at start of rhs?
        {
            compare = -1; // (Since rhs is longer)
        }
    }
    else // Strings are the same non-zero length.
    {
        compare = x_memcmp( lhs , rhs , lhsLength );
    }

    return compare;
}

//------------------------------------------------------------------------
// Compare a character sequence of known length with a null-terminated string. 
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::Compare( const char * lhs , size_type lhsLength , const char * rhs )
{
    x_int compare ;
    if( lhsLength <= 0 ) // Empty lhs?
    {
        compare = ( rhs==0 || rhs[0] == 0 ) ? 0 : -1 ;
    }
    else if( rhs == 0 || rhs[0] == 0 ) // Empty rhs?
    {
        compare = +1 ; 
    }
    else
    {
        const char * pLhs       = lhs ; 
        const char * LhsLimit   = lhs + lhsLength ;
        const char * pRhs       = rhs ;

        for( ; pLhs < LhsLimit ; pLhs++, pRhs++ )
        {
            if( pRhs[0] == 0 ) // Did we exhaust rhs?
            {
                return +1; // We exhausted rhs while we still had some lhs, so lhs > rhs.  <=========== Unstructured return.
            }
            else if( pLhs[0] != pRhs[0] ) // Different characters?
            {
                return x_int( pLhs[0] ) - x_int( pRhs[0] ); // <=========== Unstructured return.
            }
        }
        // We exhausted lhs.
        compare = pRhs[0] == 0 ? 0 : -1 ;
    }
    return compare ;
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int x_std::string::compare( const x_std::string & str ) const
{
    return Compare( m_Data , this->length() , str.m_Data , str.length() );
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int x_std::string::compare( size_type p0 , size_type n0 , const x_std::string & str ) const
{
    X_STD_ASSERT( p0 >= 0 && p0 <= length()   );
    if( n0 > 0 && p0 + n0 >= length() )
    {
        n0 = length() - p0 ;
    }
    return Compare( &m_Data[p0] , n0 , str.m_Data , str.length() );
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int x_std::string::compare( size_type p0 , size_type n0 , const x_std::string & str , size_type pos , size_type n ) const
{
    X_STD_ASSERT( p0  >= 0 && p0 <= length()      );
    X_STD_ASSERT( pos >= 0 && pos <= str.length() );
    if( n0 > 0 && p0 + n0 >= length() )
    {
        n0 = length() - p0 ;
    }
    if( n > 0 && pos + n >= str.length() )
    {
        n = str.length() - pos ;
    }
    return Compare( &m_Data[p0] , n0 , &str.m_Data[pos] , n );
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int x_std::string::compare( const char * s ) const
{
    int compare;
    if( empty() ) // *this is empty?
    {
        compare = ( s == 0 || s[0]==0 ) ? 0 : -1 ; // If s is not empty, *this < s.
    }
    else if( s == 0 ) // s is empty?
    {
        compare = +1 ; // Since *this is not empty, *this > s.
    }
    else
    {
        compare = x_strcmp( m_Data , s );
    }
    return compare;
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int x_std::string::compare( size_type p0 , size_type n0 , const char * s ) const
{
    int compare;
    X_STD_ASSERT( p0 >= 0 && p0 <= length()   );
    if( n0 > 0 && p0 + n0 >= length() )
    {
        n0 = length() - p0 ;
    }
    if( n0 <= 0 ) // Empty lhs? 
    {
        compare = ( s == 0 || s[0]==0 ) ? 0 : -1 ; // If s is not empty, lhs < s.
    }
    else if( s == 0 ) // s is empty?
    {
        compare = n0 == 0 ? 0 : +1 ; // If lhs is not empty, lhs > s.
    }
    else
    {
        compare = Compare( &m_Data[p0] , n0 , s );
    }
    return compare;
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int x_std::string::compare( size_type p0 , size_type n0 , const char * s , size_type n ) const
{
    X_STD_ASSERT( p0 >= 0 && p0 <= length()   );
    if( n0 > 0 && p0 + n0 >= length() )
    {
        n0 = length() - p0 ;
    }
    return Compare( &m_Data[p0] , n0 , s , n );
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find( const x_std::string & str , size_type pos ) const
{
    const size_type CompareLength = str.length();
    if( this->empty() || pos >= this->length() || CompareLength == 0 )  // Nothing to search in or nothing to search for?
    {
        // No need to search
    }
    else 
    {
        X_STD_ASSERT( pos >= 0 && pos <= length() );
        pos = x_std::max(pos,size_type(0));
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = str.m_Data        ; // What we are trying to find.
        const char * const FindLimit    = &str.m_Data[CompareLength]; // Limit of comparing to the find string. 
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchEnd    = &m_Data[length()-str.length()];
        for( ; Search <= SearchEnd ; Search++ )
        {
            if( Search[0] == Find[0] ) // First characters match?
            {
                const char * Check   = Search ; // Where to continue comparing rest of characters in Search[]. 
                const char * Compare = Find   ; // Where to continue comparing rest of characters in Find[].
                do
                {
                    Check++;
                    Compare++;
                    if( Compare >= FindLimit ) // Comparison reached end of find string?
                    {
                        return Search - m_Data ; // <=========================== Unstructured return.
                    }
                }
                while( Check[0] == Compare[0] );
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find( const char * s , size_type pos , size_type n ) const
{
    if( this->empty() || pos >= this->length() || s == 0 || s[0]==0 || n <= 0 ) // Nothing to search in or search for
    {
        // No need to search
    }
    else // Do a search
    {
        pos = x_std::max(pos,size_type(0));
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = s                 ; // What we are trying to find.
        const char * const FindLimit    = &s[n]             ; // Limit of comparing to the find string. 
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchEnd    = &m_Data[length()];
        for( ; Search <= SearchEnd ; Search++ )
        {
            if( Search[0] == Find[0] ) // First characters match?
            {
                const char * Check   = Search ; // Where to continue comparing rest of characters in Search[]. 
                const char * Compare = Find   ; // Where to continue comparing rest of characters in Find[].
                do
                {
                    Check++;
                    Compare++;
                    if( Compare >= FindLimit ) 
                    {
                        return Search - m_Data ; // <=========================== Unstructured return.
                    }
                }
                while( Check[0] == Compare[0] );
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find( const char * s , size_type pos ) const
{
    if( this->empty() || pos >= length() || s == 0 || s[0]==0  )  // Nothing to search in or search for
    {
        // No need to search
    }
    else // Do a search
    {
        if( pos < 0 ) { pos = 0 ; }
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = s                 ; // What we are trying to find.
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchEnd    = &m_Data[length()];
        for( ; Search <= SearchEnd ; Search++ )
        {
            if( Search[0] == Find[0] ) // First characters match?
            {
                const char * Check   = Search ; // Where to continue comparing rest of characters in Search[]. 
                const char * Compare = Find   ; // Where to continue comparing rest of characters in Find[].
                do
                {
                    Check++;
                    Compare++;
                    if( Compare[0] == 0 ) // Comparison reached end of find string?
                    {
                        return Search - m_Data ; // <=========================== Unstructured return.
                    }
                }
                while( Check[0] == Compare[0] );
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find( char c , size_type pos ) const
{
    if( empty() || pos >= length() ) // Nothing to search in?
    {
    }
    else // Do a search:
    {
        pos = x_std::max(pos,size_type(0));
        const char * pD     = &m_Data[pos] ;
        const char * DLimit = &pD[length()-pos]; // One beyond last character to check.
        do
        {
            if( pD[0] == c ) // Found c?
            {
                return pD - m_Data ; // <========== Unstructured return
            }
            pD++;
        }
        while( pD < DLimit );
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::rfind( char c , size_type pos ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }

        const char * pD     = &m_Data[ pos ] ;
        const char * FirstD = m_Data ; 
        while( pD >= FirstD )
        {
            if( pD[0] == c ) // Found c?
            {
                return pD - m_Data ; // <========== Unstructured return
            }
            pD--;
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::rfind( const x_std::string & str , size_type pos ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
        const size_type   CompareLength   = str.length()          ;
        if( CompareLength > 0 )
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (str).
            const char * const Find         = str.m_Data        ; // What we are trying to find.
            const char * const FindLimit    = &str.m_Data[CompareLength]; // Limit of comparing to the find string. 
            const char * const SearchEnd    = &m_Data[0]   ; // The limit of Search: Search no lower than this.
            const char * const Safe         = &m_Data[this->length()-CompareLength]; // It is not safe to search above this (because there isn't enough room for a full comparison).
            const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
            if( Search > Safe )
            {
                Search = Safe;
            }
            // Search Search[], going backwards, looking for Find[].
            for( ; Search >= SearchEnd ; Search-- )
            {
                if( Search[0] == Find[0] ) // First characters match?
                {
                    const char * Check   = Search ; // Where to continue comparing rest of characters in Search[]. 
                    const char * Compare = Find   ; // Where to continue comparing rest of characters in Find[].
                    do
                    {
                        Check++;
                        Compare++;
                        if( Compare >= FindLimit ) // Comparison reached end of find string?
                        {
                            return Search - m_Data ; // <=========================== Unstructured return.
                        }
                    }
                    while( Check[0] == Compare[0] );
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::rfind( const char * s , size_type pos , size_type n ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
        if( pos < 0 || n <= 0 || s == 0 || s[0] == 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (s).
            const char * const Find         = s ; // What we are trying to find.
            const char * const FindLimit    = &s[ n ]; // Limit of comparing to the find string. 
            const char * const SearchEnd    = &m_Data[0]   ; // The limit of Search: Search no lower than this.
            const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
            // Search Search[], going backwards, looking for Find[].
            for( ; Search >= SearchEnd ; Search-- )
            {
                if( Search[0] == Find[0] ) // First characters match?
                {
                    const char * Check   = Search ; // Where to continue comparing rest of characters in Search[]. 
                    const char * Compare = Find   ; // Where to continue comparing rest of characters in Find[].
                    do
                    {
                        Check++;
                        Compare++;
                        if
                        ( 
                            Compare >= FindLimit    // Reached max number of characters to compare?
                        )
                        {
                            return Search - m_Data ; // <=========================== Unstructured return.
                        }

                    }
                    while( Check[0] == Compare[0] );
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::rfind( const char * s , size_type pos ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
        if( pos < 0 || s == 0 || s[0] == 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (s).
            const char * const Find         = s ; // What we are trying to find.
            const char * const SearchEnd    = &m_Data[0]   ; // The limit of Search: Search no lower than this.
            const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
            // Search Search[], going backwards, looking for Find[].
            for( ; Search >= SearchEnd ; Search-- )
            {
                if( Search[0] == Find[0] ) // First characters match?
                {
                    const char * Check   = Search ; // Where to continue comparing rest of characters in Search[]. 
                    const char * Compare = Find   ; // Where to continue comparing rest of characters in Find[].
                    do
                    {
                        Check++;
                        Compare++;
                        if( Compare[0] == 0 ) // Reached end of Find[]?
                        {
                            return Search - m_Data ; // <=========================== Unstructured return.
                        }

                    }
                    while( Check[0] == Compare[0] );
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_first_of( const x_std::string & str , size_type pos ) const
{
    const size_type CompareLength = str.length() ;
    if( pos < 0 )
    {
        pos = 0 ;
    }
    if( pos >= this->length() || CompareLength == 0 )  // Nothing to search in or nothing to search for?
    {
        // No need to search
    }
    else 
    {
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = str.m_Data        ; // What we are trying to find.
        const char * const FindLimit    = &str.m_Data[CompareLength]; // Limit of comparing to the find string. 
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchLimit  = &m_Data[length()];
        for( ; Search < SearchLimit ; Search++ )
        {
            const char   Check   = Search[0];
            const char * Compare ; // Where to continue comparing rest of characters in Find[].
            for( Compare = Find ; Compare < FindLimit ; Compare++ )
            {
                if( Check == Compare[0] )
                {
                    return Search - m_Data ;
                }
            }
        }

    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_first_of( const char * s , size_type pos , size_type n ) const
{
    if( pos < 0 )
    {
        pos = 0 ;
    }
    if( pos >= this->length() || s == 0 || s[0] == 0 )  // Nothing to search in or nothing to search for?
    {
        // No need to search
    }
    else 
    {
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = s ; // What we are trying to find.
        const char * const FindLimit    = &s[n]; // Limit of comparing to the find string. 
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchLimit  = &m_Data[length()];
        for( ; Search < SearchLimit ; Search++ )
        {
            const char   Check   = Search[0];
            const char * Compare ; // Where to continue comparing rest of characters in Find[].
            for( Compare = Find ; Compare < FindLimit ; Compare++ )
            {
                if( Check == Compare[0] )
                {
                    return Search - m_Data ;
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_first_of( const char * s , size_type pos ) const
{
    if( pos < 0 ) { pos = 0 ; }

    if( pos >= this->length() || s == 0 || s[0] == 0 )  // Nothing to search in or nothing to search for?
    {
        // No need to search
    }
    else 
    {
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = s ; // What we are trying to find.
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchLimit  = &m_Data[length()];
        for( ; Search < SearchLimit ; Search++ )
        {
            const char   Check   = Search[0];
            const char * Compare ; // Where to continue comparing rest of characters in Find[].
            for( Compare = Find ; Compare[0] != 0 ; Compare++ )
            {
                if( Check == Compare[0] )
                {
                    return Search - m_Data ; // <==================== unstructured return
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_last_of( const x_std::string & str , size_type pos ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
        const size_type CompareLength = str.length()          ;
    
        if( pos < 0 || CompareLength == 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (str).
            const char * const Find         = str.m_Data                  ; // What we are trying to find.
            const char * const FindLimit    = &str.m_Data[CompareLength]  ; // Limit of Find[].
            const char *       Search       = &m_Data[pos]          ; // Where we are currently searching.
            const char * const SearchEnd    = &m_Data[0]            ; // Where we end the search.
            for( ; Search >= SearchEnd ; Search-- )
            {
                const char   Check   = Search[0];
                const char * Compare ; // Where to continue comparing rest of characters in Find[].
                for( Compare = Find ; Compare < FindLimit ; Compare++ )
                {
                    if( Check == Compare[0] ) // Found a matching character?
                    {
                        return Search - m_Data ; // <==================== Unstructured return
                    }
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}


//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_last_of( const char * s , size_type pos ,  size_type n ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
    
        if( pos < 0 || s == 0 || s[0] == 0 || n <= 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (str).
            const char * const Find         = s                 ; // What we are trying to find.
            const char * const FindLimit    = &s[n]             ; // Limit of Find[].
            const char *       Search       = &m_Data[pos]  ; // Where we are currently searching.
            const char * const SearchEnd    = &m_Data[0]    ; // Where we end the search.
            for( ; Search >= SearchEnd ; Search-- )
            {
                const char   S   = Search[0];
                const char * F   ; // Where to continue comparing rest of characters in Find[].
                for( F = Find ; F < FindLimit ; F++ )
                {
                    if( S == F[0] ) // Found a matching character?
                    {
                        return Search - m_Data ; // <==================== Unstructured return
                    }
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_last_of( const char * s , size_type pos ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
        if( pos < 0 || s == 0 || s[0] == 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (str).
            const char * const Find         = s                 ; // What we are trying to find.
            const char *       Search       = &m_Data[pos]  ; // Where we are currently searching.
            const char * const SearchEnd    = &m_Data[0]    ; // Where we end the search.
            for( ; Search >= SearchEnd ; Search-- )
            {
                const char   S   = Search[0];
                const char * F   ; // Where to continue comparing rest of characters in Find[].
                for( F = Find ; F[0] != 0  ; F++ )
                {
                    if( S == F[0] ) // Found a matching character?
                    {
                        return Search - m_Data ; // <==================== Unstructured return
                    }
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_first_not_of( const x_std::string & str , size_type pos ) const
{
    if( pos < 0 ) { pos = 0 ; }
    const size_type CompareLength = str.length() ;
    if( pos >= this->length() )  // Nothing to search in?
    {
        return npos ; // <==================== Unstructured return
    }
    else if( CompareLength <= 0 )  // Nothing to search for?
    {
        return pos ; // <==================== Unstructured return
    }
    else 
    {
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = str.m_Data        ; // What we are trying to find.
        const char * const FindLimit    = &str.m_Data[CompareLength]; // Limit of comparing to the find string. 
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchLimit  = &m_Data[length()];
        for( ; Search < SearchLimit ; Search++ )
        {
            const char   Check   = Search[0];
            const char * Compare = Find  ; // Where to continue comparing rest of characters in Find[].
            while( Compare[0] != Check ) // While Check not found in Find[]
            {
                Compare++;
                if( Compare >= FindLimit ) // Exhausted Find[]?
                {
                    return Search - m_Data ; // <==================== Unstructured return
                }
            }
        }

    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_first_not_of( const char * s , size_type pos , size_type n ) const
{
    if( pos < 0 ) { pos = 0 ; }
    if( pos >= this->length() )  // Nothing to search in?
    {
        return npos ; // <==================== Unstructured return
    }
    else if( n <= 0 || s == 0  ||  s[0] == 0 )  // Nothing to search for?
    {
        return pos ; // <==================== Unstructured return
    }
    else 
    {
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = s ; // What we are trying to find.
        const char * const FindLimit    = &s[n]; // Limit of comparing to the find string. 
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchLimit  = &m_Data[length()];
        for( ; Search < SearchLimit ; Search++ )
        {
            const char   Check   = Search[0];
            const char * Compare = Find ; // Where to continue comparing rest of characters in Find[].
            while( Compare[0] != Check ) // While Check not found in Find[]
            {
                Compare++;
                if( Compare >= FindLimit ) // Exhausted Find[]?
                {
                    return Search - m_Data ; // <==================== Unstructured return
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_first_not_of( const char * s , size_type pos ) const
{
    if( pos < 0 ) { pos = 0 ; }
    if( pos >= this->length() )  // Nothing to search in?
    {
        return npos ; // <==================== Unstructured return
    }
    else if( s == 0  ||  s[0] == 0 )  // Nothing to search for?
    {
        return pos ; // <==================== Unstructured return
    }
    else 
    {
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * const Find         = s ; // What we are trying to find.
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchLimit  = &m_Data[length()];
        for( ; Search < SearchLimit ; Search++ )
        {
            const char   Check   = Search[0] ;
            const char * Compare = Find      ; // Where to continue comparing rest of characters in Find[].
            while( Compare[0] != Check ) // While Check not found in Find[]
            {
                Compare++;
                if( Compare[0] == 0 ) // Exhausted Find[]?
                {
                    return Search - m_Data ; // <==================== Unstructured return
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}


//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_first_not_of( char c , size_type pos ) const
{
    if( pos < 0 ) { pos = 0 ; }
    if( pos >= this->length() )  // Nothing to search in?
    {
        // No need to search
    }
    else 
    {
        // "search" refers to the string being searched (*this).
        // "find" refers to the string we try to find (str).
        const char * Search             = &m_Data[pos]  ; // Where we are currently searching.
        const char * const SearchLimit  = &m_Data[length()];
        for( ; Search < SearchLimit ; Search++ )
        {
            if( Search[0] != c )
            {
                return Search - m_Data ; // <==================== unstructured return
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_last_not_of( const x_std::string & str , size_type pos ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
        const x_int CompareLength = str.length()          ;
    
        if( pos < 0 || CompareLength == 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (str).
            const char * const Find         = str.m_Data                  ; // What we are trying to find.
            const char * const LastFind     = &str.m_Data[CompareLength-1]; // Limit of Find[].
            const char *       Search       = &m_Data[pos]          ; // Where we are currently searching.
            const char * const SearchEnd    = &m_Data[0]            ; // Where we end the search.
            for( ; Search >= SearchEnd ; Search-- )
            {
                const char   Check   = Search[0];
                const char * Compare ; // Where to continue comparing rest of characters in Find[].
                for( Compare = Find ; Check != Compare[0] ; Compare++ )
                {
                    if( Compare >= LastFind ) // Ran out of things to compare to?
                    {
                        return Search - m_Data ; // <==================== Unstructured return
                    }
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}


//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_last_not_of( const char * s , size_type pos , size_type n ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
    
        if( pos < 0 || s == 0 || s[0] == 0 || n <= 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (str).
            const char * const Find         = s                 ; // What we are trying to find.
            const char * const LastFind     = &s[n-1]           ; // Limit of Find[].
            const char *       Search       = &m_Data[pos]  ; // Where we are currently searching.
            const char * const SearchEnd    = &m_Data[0]    ; // Where we end the search.
            for( ; Search >= SearchEnd ; Search-- )
            {
                const char   S   = Search[0];
                const char * F   ; // Where to continue comparing rest of characters in Find[].
                for( F = Find ; S != F[0] ; F++ )
                {
                    if( F >= LastFind ) // Ran out of things to find?
                    {
                        return Search - m_Data ; // <==================== Unstructured return
                    }
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_last_not_of( const char * s , size_type pos ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
    
        if( pos < 0 || s == 0 || s[0] == 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (str).
            const char * const Find         = s                 ; // What we are trying to find.
            const char *       Search       = &m_Data[pos]  ; // Where we are currently searching.
            const char * const SearchEnd    = &m_Data[0]    ; // Where we end the search.
            for( ; Search >= SearchEnd ; Search-- )
            {
                const char   S   = Search[0];
                const char * F   ; // Where to continue comparing rest of characters in Find[].
                for( F = Find ; S != F[0] ; F++ )
                {
                    if( F[1] == 0 ) // Ran out of things to find?
                    {
                        return Search - m_Data ; // <==================== Unstructured return
                    }
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
x_std::string::size_type x_std::string::find_last_not_of( char c , size_type pos ) const
{
    if( length() > 0 )
    {
        if( pos == npos || pos >= length() )
        {
            pos = length() - 1 ;
        }
        if( pos < 0 )  // Nothing to search in or nothing to search for?
        {
            // No need to search
        }
        else 
        {
            // "search" refers to the string being searched (*this).
            // "find" refers to the string we try to find (str).
            const char *       Search       = &m_Data[pos]  ; // Where we are currently searching.
            const char * const SearchEnd    = &m_Data[0]    ; // Where we end the search.
            for( ; Search >= SearchEnd ; Search-- )
            {
                if( Search[0] != c ) // Found non-matching character?
                {
                    return Search - m_Data ; // <==================== Unstructured return
                }
            }
        }
    }
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
void x_std::string::x_trim       () { x_trim      (" \x09\x0a\x0b\x0c\x0d"); }
void x_std::string::x_trim_left  () { x_trim_left (" \x09\x0a\x0b\x0c\x0d"); }
void x_std::string::x_trim_right () { x_trim_right(" \x09\x0a\x0b\x0c\x0d"); }

//------------------------------------------------------------------------
// Remove from the front and end of *this any characters in *targets.
//------------------------------------------------------------------------
void x_std::string::x_trim( const char * targets )
{
    x_trim_left  ( targets );
    x_trim_right ( targets );
}

//------------------------------------------------------------------------
// Remove from the front of *this any characters in *targets.
//------------------------------------------------------------------------
void x_std::string::x_trim_left( const char * targets )
{
    const size_type pos = find_first_not_of( targets );
    if( pos == npos )
    {
        resize(0);
    }
    else if( pos > 0 )
    {
        erase( 0 , pos );
    }
}

//------------------------------------------------------------------------
// Remove from the end of *this any characters in *targets.
//------------------------------------------------------------------------
void x_std::string::x_trim_right( const char * targets )
{
    const size_type pos = find_last_not_of( targets );
    if( pos == npos )
    {
        resize(0);
    }
    else if( pos >= 0 )
    {
        erase( pos+1 );
    }
}

//------------------------------------------------------------------------
// Remove up to n characters from the left of this.
//------------------------------------------------------------------------
void x_std::string::x_trim_left( size_type n )
{
    if( n > 0 )
    {
        erase( 0 , n );
    }
}

//------------------------------------------------------------------------
// Remove up to n characters from the right of this.
//------------------------------------------------------------------------
void x_std::string::x_trim_right ( size_type n ) 
{
    if( n <= 0 )
    {
    }
    else if( n >= length() ) // Trim string into nothing?
    {
        resize(0);
    }
    else
    {
        erase( length() - n , n );
    }
}

//------------------------------------------------------------------------
// The first n characters of this, or all of this if n >= length().
//------------------------------------------------------------------------
x_std::string x_std::string::x_left( size_type n ) const
{
    n = x_std::min( n , length() );
    char      LocalBuffer[100]  ; // If possible, use this for better performance (fewer string allocations).
    if( n <= 0 )
    {
        return "";
    }
    else if( n < sizeof(LocalBuffer)/sizeof(LocalBuffer[0]) ) // Do we have room to use the local buffer (and add a trailing null)?
    {
        x_memcpy( LocalBuffer , &m_Data[0] , n );
        LocalBuffer[n] = 0 ; // Add null termination.
        return LocalBuffer; // This will convert the local buffer into a x_std::string
    }
    else
    {
        return x_std::string( *this , 0 , n );
    }
}

//------------------------------------------------------------------------
// The last n characters of this, or all of this if n >= length().
//------------------------------------------------------------------------
x_std::string x_std::string::x_right( size_type n ) const
{
    n = x_std::min( n , length() );
    char      LocalBuffer[100]  ; // If possible, use this for better performance (fewer string allocations).
    if( n <= 0 )
    {
        return "";
    }
    else if( size_type(n) < sizeof(LocalBuffer)/sizeof(LocalBuffer[0]) ) // Do we have room to use the local buffer (and add a trailing null)?
    {
        x_memcpy( LocalBuffer , &m_Data[this->length()-n] , n );
        LocalBuffer[n] = 0 ; // Add null termination.
        return LocalBuffer; // This will convert the local buffer into a x_std::string
    }
    else
    {
        return x_std::string( *this , this->length()-n , n );
    }
}

//------------------------------------------------------------------------
// Use the printf-style Format string to fill in *this.
//------------------------------------------------------------------------
x_std::string & x_std::string::x_format( const char * Format , ... )
{
    const x_int   MaxLength   =   5000  ;
    char          Buffer      [MaxLength+1] ; // +1 for trailing null.

    va_list Arguments ;
    va_start( Arguments , Format );
    x_int Length ;
    Length = vsprintf( Buffer , Format , Arguments );
    va_end( Arguments );
    X_STD_ASSERT( Length <= MaxLength );
    *this = Buffer ;
    return *this ;
}

//------------------------------------------------------------------------
// Convert entire string to uppercase.
//------------------------------------------------------------------------
void x_std::string::x_to_upper()
{
    char * Data = m_Data ;
    if( Data != 0 )
    {
        for( ; Data[0] != 0 ; Data++ )
        {
            if( Data[0] >= 'a' && Data[0] <= 'z' )
            {
                Data[0] += (x_int)'A' - (x_int)'a' ;
            }
        }
    }
}

//------------------------------------------------------------------------
// Convert entire string to lowercase.
//------------------------------------------------------------------------
void x_std::string::x_to_lower() 
{
    char * Data = m_Data ;
    if( Data != 0 )
    {
        for( ; Data[0] != 0 ; Data++ )
        {
            if( Data[0] >= 'A' && Data[0] <= 'Z' )
            {
                Data[0] += (x_int)'a' - (x_int)'A' ;
            }
        }
    }
}

//------------------------------------------------------------------------
// Reverse the contents of the string
//------------------------------------------------------------------------
void x_std::string::x_reverse() 
{
    char * Last  = &m_Data[length()-1]; 
    char * First = m_Data ;
    for( ; First < Last ; First++ , Last-- )
    {
        const char OldFirst = First[0];
        First[0] = Last[0]  ;
        Last[0]  = OldFirst ;
    }
}

//------------------------------------------------------------------------
// operator +
//------------------------------------------------------------------------
x_std::string x_std::operator + ( const string & lhs , const string & rhs ) 
{ 
    string s ;
    s.reserve( lhs.length() + rhs.length() );
    s =  lhs  ; 
    s += rhs  ;
    return s;
}

//------------------------------------------------------------------------
// operator +
//------------------------------------------------------------------------
x_std::string x_std::operator + ( const string & lhs , const char *   rhs ) 
{
    const string::size_type rhs_length = x_strlen(rhs);
    string s ;
    s.reserve( lhs.length() + rhs_length );
    s =  lhs  ; 
    s.append( rhs , rhs_length );
    return s;
}

//------------------------------------------------------------------------
// operator +
//------------------------------------------------------------------------
x_std::string x_std::operator + ( const string & lhs , char rhs ) 
{
    const string::size_type rhs_length = 1 ;
    string s ;
    s.reserve( lhs.length() + rhs_length );
    s =  lhs  ; 
    s += rhs  ;
    return s;
}

//------------------------------------------------------------------------
// operator +
//------------------------------------------------------------------------
x_std::string x_std::operator + ( const char * lhs , const string & rhs ) 
{
    const string::size_type lhs_length = x_strlen(lhs) ;
    string s ;
    s.reserve( lhs_length + rhs.length() );
    s.append( lhs , lhs_length );
    s += rhs ;
    return s;
}

