//------------------------------------------------------------------------
// cstring: A simple string class
//------------------------------------------------------------------------

// Do NOT include any project-specific header files
#include "x_files.hpp"  //todo:Remove dependency on x-files
#include "string.h"
#include "cstring.h"

#if defined(_MSC_VER)
    // Suppress certain Microsoft Visual C++ warnings
    #pragma warning( disable: 4127 ) // Warning: conditional expression is constant
#endif

//------------------------------------------------------------------------
// Compilation control
//------------------------------------------------------------------------
#define ENABLE_SELF_TEST 0 // 0 to enable self test in debug build. 1 to disable it.

//------------------------------------------------------------------------
// The data reserved for the empty string.
//------------------------------------------------------------------------
cstring::CEmptyString cstring::m_EmptyString = 
    //  Length   Capacity     Data
    {  {  0    ,    0     } , ""    }
;
char * cstring::m_EmptyData = m_EmptyString.m_Data ;


//------------------------------------------------------------------------
// Allocate a new cstring with the given capacity. 
//------------------------------------------------------------------------
char * cstring::New( int Capacity ) 
{ 
    Capacity += 10 ; // Always add a little more ... this might prevent some unnecessary reallocations (depending on how the string is used).
    const int   NBytes  = sizeof(CPrefix) + Capacity + 1;  // +1 for trailing null.
    CPrefix *   Prefix  = reinterpret_cast<CPrefix *>( new char[NBytes] );

    assert( Capacity <= MaxSize );
    Prefix->Length      =   0           ;
    Prefix->Capacity    =   Capacity    ;

    char * Data = reinterpret_cast<char *>( &Prefix[1] );
    Data[0] = 0; // Add null termination.
    return Data;
}

//------------------------------------------------------------------------
// Delete the data allocated with New().
//------------------------------------------------------------------------
inline void cstring::Delete( char * Data )
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
void cstring::Append( const char * Source , int Length )
{
    if( Length > 0 )    
    {
        const int OldLength = this->length()        ;
        const int NewLength = OldLength + Length    ;

        reserve(NewLength);
        memcpy( & m_Data[OldLength] , Source , Length ); // Add in characters.
        m_Data[NewLength]   =   0           ; // Add null terminator.
        this->Prefix().Length   =   NewLength   ; // Set new length.
    }
}

//------------------------------------------------------------------------
// Insert Length characters from Source into *this at position pos.
//------------------------------------------------------------------------
void cstring::Insert( int pos , const char * Source , int Length )
{
    const int OldLength = this->length()        ;
    const int MoveLength = OldLength - pos      ; // How many characters need to be moved over to make room?
    if( pos < 0 || MoveLength < 0 )
    {
        assert(false);
    }
    else if( Length > 0 )    
    {
        const int NewLength = OldLength + Length    ;
        reserve(NewLength);
        memmove( &m_Data[ pos + Length ] , &m_Data[pos] , MoveLength );
        assert( m_Data != m_EmptyData );
        if( Source != 0 )
        {
            memcpy( &m_Data[pos] , Source , Length ); // Insert characters.
        }
        m_Data[NewLength]   =   0           ; // Add null terminator.
        this->Prefix().Length   =   NewLength   ; // Set new length.
    }
}

//------------------------------------------------------------------------
// Erase up to n characters from *this at position pos.
//------------------------------------------------------------------------
void cstring::Erase( int pos , int n )
{
    const int OldLength         = this->length()    ;
    const int MaxDeleteLength   = OldLength - pos   ;
    if( pos < 0 )
    {
        assert(false);
    }
    else if( n > 0 )
    {
        const int DeleteLength = Min( MaxDeleteLength , n );
        if( DeleteLength > 0 )
        {
            assert( m_Data != m_EmptyData );
            const int NewLength  = OldLength        - DeleteLength  ;
            const int MoveLength = MaxDeleteLength  - DeleteLength  ;
            memmove( &m_Data[pos] , &m_Data[pos+DeleteLength] , MoveLength );
            m_Data[NewLength]   =   0           ; // Add null terminator.
            this->Prefix().Length   =   NewLength   ; // Set new length.
        }
    }
}

//------------------------------------------------------------------------
// Replace up to n0 characters in *this at position pos with n 
// characters from source.
//------------------------------------------------------------------------
char * cstring::Replace( int pos , int n0 , const char * Source , int n )
{
    const int OldLength         = this->length()                ;
    const int MaxReplaceLength  = OldLength - pos               ;
    const int ReplaceLength     = Max( 0 , Min( n0 , MaxReplaceLength ) ); // Number of characters replaced
    const int Expansion         = n - ReplaceLength             ; // How many characters we need to expand the length by. If < 0, is how much we need to shrink by.
    const int NewLength         = OldLength + Expansion         ;
    char *    ReplacedAt        = 0;
    if( pos < 0 )
    {
        assert(false);
    }
    else if( Expansion > 0  ) // Replacing with a longer string from Source?
    {
        Insert( pos , 0 , Expansion ); // Insert room for Expansion additional characters.
        assert( m_Data != m_EmptyData );
        ReplacedAt = &m_Data[pos];
        if( n > 0 && Source != 0 )
        {
            memmove( ReplacedAt , Source , n ); // Copy in the replacement from Source.
        }
        m_Data[NewLength]   =   0           ; // Add null terminator.
        this->Prefix().Length   =   NewLength   ; // Set new length.
    }
    else if( Expansion < 0 ) // Replacing with a shorter string from Source?
    {
        Erase( pos , -Expansion );
        assert( m_Data != m_EmptyData );
        ReplacedAt = &m_Data[pos];
        if( n > 0 && Source != 0 )
        {
            memmove( ReplacedAt , Source , n );
        }
    }
    else if( ReplaceLength != 0 ) // Replacing with the same sized string from Source
    {
        assert( m_Data != m_EmptyData );
        ReplacedAt = &m_Data[pos];
        if( Source != 0 )
        {
            memmove( ReplacedAt , Source , ReplaceLength );
        }
    }
    return ReplacedAt ;
}


//------------------------------------------------------------------------
// Make sure the cstring has a capacity of at least MinCapacity.
// Note: Destroys the contents and sets length()=0
//------------------------------------------------------------------------
inline void cstring::Reallocate( int MinCapacity )
{
    assert( MinCapacity >= 0 );
    if( capacity() == 0 || capacity() < MinCapacity )
    {
        this->Delete();
        m_Data = New(MinCapacity);
    }
    this->Prefix().Length = 0;
    assert( m_Data != 0 );
    assert( m_Data != m_EmptyData );
}


//------------------------------------------------------------------------
// Deallocate *this and set m_Data == m_EmptyData.
//------------------------------------------------------------------------
void cstring::Delete()
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
cstring::cstring( const cstring & rhs )
{
    assert( rhs.m_Data != 0 );
    const int Length = rhs.length();
    m_Data = New( Length );
    memcpy( m_Data , rhs.data() , Length+1 ); // +1 to also copy trailing null. 
    this->Prefix().Length = Length;
}

//------------------------------------------------------------------------
// Construct a string with a copy of s. 
//------------------------------------------------------------------------
cstring::cstring( const char * s )
{
    const int Length = s==0 ? 0 : strlen(s);
    m_Data = m_EmptyData ;
    if( Length > 0 )
    {
        const int Length = strlen(s);
        m_Data = New( Length );
        memcpy( m_Data , s , Length+1 ); // +1 to also copy trailing null. 
        this->Prefix().Length = Length;
    }
}

//------------------------------------------------------------------------
// Construct a string out of n characters c.
//------------------------------------------------------------------------
cstring::cstring( int n , char c )
{
    m_Data = m_EmptyData ;
    if( n > 0 )
    {
        m_Data = New(n);
        memset( m_Data , c , n );
        m_Data[n] = 0 ; // Add null terminator.
        this->Prefix().Length = n;
    }
}

//------------------------------------------------------------------------
// Construct a string from n characters at s, or until s is exhausted.
//------------------------------------------------------------------------
cstring::cstring( const char * s , int n )
{
    m_Data = m_EmptyData ;
    if( s != 0 && n > 0 )
    {
        int Length = 0;
        while( s[Length] != 0 && Length < n ) // Determine length (until we reach end of string s or n characters).
        {
            Length++;
        }
        m_Data = New(Length);
        memcpy( m_Data , s , Length );
        m_Data[Length] = 0 ; // Add null terminator.
        this->Prefix().Length = Length;
    }
}

//------------------------------------------------------------------------
// Construct a string from n characters of rhs at position pos.
//------------------------------------------------------------------------
cstring::cstring( const cstring & rhs , int pos , int n  )
{
    assert( rhs.m_Data != 0 );
    m_Data = m_EmptyData ;
    const int Length = Min( n , rhs.length() - pos ); 
    if( Length > 0 )
    {
        m_Data = New(Length);
        memcpy( m_Data , &rhs.data()[pos] , Length );
        m_Data[Length] = 0 ; // Add null terminator.
        this->Prefix().Length = Length;
    }
}

//------------------------------------------------------------------------
// Construct a string from the sequence in [first,last).
//------------------------------------------------------------------------
cstring::cstring( const_iterator first , const_iterator last )
{
    const int Length = Max( last - first , 0 );
    m_Data = m_EmptyData ;
    if( Length > 0 )
    {
        m_Data = New(Length);
        memcpy( m_Data , & *first  , Length );
        this->Prefix().Length = Length;
    }
}

//------------------------------------------------------------------------
// Copy rhs into *this.
//------------------------------------------------------------------------
cstring & cstring::operator = ( const cstring & rhs )
{
    assert( rhs.data() != 0 );

    const int Length = rhs.length();
    this->Reallocate( Length );
    memcpy( m_Data , rhs.data() , Length+1 ); // +1 to copy trailing null.
    this->Prefix().Length = Length;

    return *this;
}

//------------------------------------------------------------------------
// Copy null-terminated s into *this.
//------------------------------------------------------------------------
cstring & cstring::operator = ( const char * s )
{
    if( s == 0 )
    {
        this->Delete();
    }
    else 
    {
        const int NewLength = strlen(s);
        this->Reallocate(NewLength);
        memcpy( m_Data , s , NewLength+1 ); // +1 to copy trailing null.
        this->Prefix().Length = NewLength;
    }
    return *this;
}

//------------------------------------------------------------------------
// Make *this a string with a single character c.
//------------------------------------------------------------------------
cstring & cstring::operator = ( char c )
{
    this->Reallocate(1); // Make sure there is enough room for a single character.
    assert( m_Data != m_EmptyData );
    m_Data[0] = c ; // Add the character.
    m_Data[1] = 0 ; // Add null termination.
    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
cstring & cstring::assign( const cstring & str , int pos , int n )
{
    assert( str.m_Data != 0 );

    const int MaxLength = str.length() - pos ; 
    const int Length    = Min( MaxLength , n );
    if( Length <= 0 )
    {
        resize(0);
    }
    else
    {
        Reallocate(Length);
        memcpy( m_Data , &str.m_Data[pos] , Length );
        m_Data[Length] = 0 ; // Add null terminator.
    }
    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
cstring & cstring::assign( const char * s , int n )
{
    if( s == 0 )
    {
        this->Delete();
    }
    else
    {
        int Length = 0;
        while( s[Length] != 0 && Length < n ) // Determine length (until we reach end of string s or n characters).
        { 
            Length++;
        }
        Reallocate(Length);
        memcpy( m_Data , s , Length );
        m_Data[Length] = 0 ; // Add null terminator.
    }
    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
cstring & cstring::assign( int n , char c )
{
    if( n <= 0 )
    {   
        resize(0);
    }
    else
    {
        Reallocate(n);
        memset( m_Data , c , n );
        m_Data[n] = 0 ; // Add null terminator.
    }
    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
cstring & cstring::assign( const_iterator first , const_iterator last )
{
    const int Length = Max( last - first , 0 );
    Reallocate(Length);
    memcpy( m_Data , & *first , Length );
    this->Prefix().Length = Length;
    return *this;
}

//------------------------------------------------------------------------
// Return a pointer to the underlying data.
//------------------------------------------------------------------------
char * cstring::private_data () 
{ 
    reserve( 0 );
    return m_Data; 
}  

//------------------------------------------------------------------------
// Private, unsafe version of resize().
//------------------------------------------------------------------------
void cstring::private_resize( int n )
{
    n = Max(n,0);
    reserve( n );
    m_Data[n]           = 0 ; // Add null termination.
    this->Prefix().Length   = n ; // Set new length.
}

//------------------------------------------------------------------------
// Change the size of a string
//------------------------------------------------------------------------
void cstring::resize( int n , char c )
{
    n = Max(n,0);
    const int OldLength = length();
    if( OldLength < n ) // Need to lengthen the string?
    {
        const int NChars = n - OldLength; // Number of characters to add.
        reserve(n);
        memset( &m_Data[OldLength] , c , NChars ); // Fill in new characters.
        m_Data[n]           = 0 ; // Add null termination at new length.
        this->Prefix().Length   = n ; // Set new length.
    }
    else if( OldLength > n ) // Need to shorten the string?
    {
        assert( m_Data != m_EmptyData );
        m_Data[n]           = 0 ; // Add null termination at new length.
        this->Prefix().Length   = n ; // Set new length.
    }
    assert( capacity() >= n );
}

//------------------------------------------------------------------------
// Reserve space in the string
//------------------------------------------------------------------------
// After this function, m_Data != m_EmptyData.
void cstring::reserve( int n )
{
    n = Max(n,0);
    if( m_Data == 0 )
    {
        Reallocate(n);
    }
    else if( capacity() < n )
    {
        const int Length = length();
        assert( Length < n );
        char * OldData = m_Data   ;
        char * NewData = New(n)   ;

        memcpy( NewData , OldData , Length+1 ); // Copy old contents into new data. (+1 to copy trailing null)
        m_Data = NewData ;
        this->Prefix().Length = Length;
        Delete(OldData);
    }
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
cstring & cstring::operator += ( const cstring & rhs )
{
    Append( rhs.m_Data , rhs.length() );
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
cstring & cstring::operator += ( const char * s )
{
    const int LengthToAdd = s == 0 ? 0 : strlen(s);
    Append( s , LengthToAdd );
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
cstring & cstring::operator += ( char c )
{
    Append( &c , 1 );
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
cstring & cstring::append( const cstring & str , int pos , int n )
{
    n = Max(n,0);
    const int LengthToAdd = Min( n , str.length() - pos );
    if( LengthToAdd > 0 )
    {
        Append( &str.m_Data[pos] , Min( LengthToAdd , n ) );
    }
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
cstring & cstring::append( const char * s , int n )
{
    if( s != 0 && n > 0 )
    {
        int Length = 0;
        while( s[Length] != 0 && Length < n ) // Determine length (until we reach end of string s or n characters).
        { 
            Length++;
        }
        if( Length > 0 )
        {
            Append( s , Length );
        };
    }
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
cstring & cstring::append( int n , char c )
{
    if( n > 0 )
    {
        const int OldLength = this->length()    ;
        const int NewLength = OldLength + n     ;

        reserve(NewLength);
        memset( &m_Data[OldLength] , c , n ); // Add in new characters.
        m_Data[NewLength]       =   0           ; // Add null terminator.
        this->Prefix().Length   =   NewLength   ; // Set new length.
    }
    return *this;
}

//------------------------------------------------------------------------
// Catenation
//------------------------------------------------------------------------
cstring & cstring::append( const_iterator first , const_iterator last )
{
    const int LengthToAdd = last - first ;
    if( LengthToAdd > 0 )
    {
        Append( & *first , LengthToAdd );
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
cstring & cstring::insert( int p0 , const cstring & str )
{
    assert( str.m_Data != 0 );
    Insert( p0 , str.m_Data , str.length() );
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
cstring & cstring::insert( int p0 , const cstring & str , int pos , int n )
{
    assert( str.m_Data != 0 );
    const int LengthToInsert = Min( str.length()-pos , n );
    if( LengthToInsert > 0 )
    {
        Insert( p0 , &str.m_Data[pos] , LengthToInsert );
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
cstring & cstring::insert( int p0 , const char * s , int n )
{
    if( n > 0 && s != 0 )
    {
        int LengthToInsert = 0;
        while( s[LengthToInsert] != 0 && LengthToInsert < n ) // Determine length (until we reach end of string s or n characters).
        { 
            LengthToInsert++;
        }
        if( LengthToInsert > 0 )
        {
            Insert( p0 , s , LengthToInsert );
        }
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
cstring & cstring::insert( int p0 , const char * s )
{
    const int LengthToInsert    = s == 0 ? 0 : strlen(s);
    if( LengthToInsert >= 0 )
    {
        Insert( p0 , s , LengthToInsert );
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
cstring & cstring::insert( int p0 , int n , char c )
{
    if( n > 0 )
    {
        Insert( p0 , 0 , n ); // Insert an uninitialized gap.
        memset( &m_Data[p0] , c , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
cstring::iterator cstring::insert( iterator it , char c )
{
    const int pos = it.m_Position ;
    Insert( pos , &c , 1 );
    return iterator( this->m_Data , this->length() , pos ); // Return new iterator at the newly inserted character.
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
void cstring::insert( iterator it , const_iterator first , const_iterator last )
{
    const int InsertLength = last - first ;
    assert( it.m_Data == this->m_Data );
    const int pos = it.m_Position ;
    if( InsertLength > 0 )
    {
        insert( pos , & *first , InsertLength );
    }
}

//------------------------------------------------------------------------
// Insertion
//------------------------------------------------------------------------
void cstring::insert( iterator it , int n , char c)
{
    assert( it.m_Data == this->m_Data );
    insert( it.m_Position , n , c );
}

//------------------------------------------------------------------------
// Erasing
//------------------------------------------------------------------------
cstring & cstring::erase( int p0 , int n)
{
    Erase( p0 , ( n == npos ? MaxSize : n ) );
    return *this;
}

//------------------------------------------------------------------------
// Erasing
//------------------------------------------------------------------------
cstring::iterator cstring::erase( iterator it )
{
    assert( it.m_Data == this->m_Data );
    const int pos = it.m_Position ;
    Erase( pos , 1 );
    return iterator( m_Data , length() , pos );
}

//------------------------------------------------------------------------
// Erasing
//------------------------------------------------------------------------
cstring::iterator cstring::erase( iterator first , iterator last )
{
    assert( first.m_Data == this->m_Data );
    const int pos = first.m_Position    ;
    const int n   = last  - first       ;
    Erase( pos , n );
    return iterator( m_Data , length() , pos );
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace( int p0 , int n0 , const cstring & str )
{
    Replace( p0 , n0 , str.m_Data , str.length() );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace( int p0 , int n0 , const cstring & str , int pos , int n )
{
    const int MaxSourceLength = str.length() - pos ;
    const int SourceLength    = Min( MaxSourceLength , n );
    Replace( p0 , n0 , &str.m_Data[pos] , SourceLength );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace( int p0 , int n0 , const char * s , int n )
{
    int ReplacementLength = 0;
    if( s != 0 && n > 0 )
    {
        while( s[ReplacementLength] != 0 && ReplacementLength < n ) // Determine length (until we reach end of string s or n characters).
        { 
            ReplacementLength++;
        }
    }
    Replace( p0 , n0 , s , ReplacementLength );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace( int p0 , int n0 , const char * s )
{
    const int SourceLength = s == 0 ? 0 : strlen(s);
    Replace( p0 , n0 , s , SourceLength );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace( int p0 , int n0 , int n , char c )
{
    n = Max(n,0);
    char * ReplaceHere = Replace( p0 , n0 , 0 , n );
    if( ReplaceHere != 0 )
    {
        memset( ReplaceHere , c , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace( iterator first0 , iterator last0 , const cstring & str )
{
    assert( first0.m_Data == m_Data );
    const int p0 = first0.m_Position    ;
    const int n0 = last0 - first0       ;
    Replace( p0 , n0 , str.m_Data , str.length() );
    return *this;
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace( iterator first0 , iterator last0 , const char * s , int n)
{
    assert( first0.m_Data == m_Data );
    const int p0 = first0.m_Position    ;
    const int n0 = last0 - first0       ;
    return replace( p0 , n0 , s , n );
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace(iterator first0 , iterator last0 , const char *s )
{
    assert( first0.m_Data == m_Data );
    const int p0 = first0.m_Position    ;
    const int n0 = last0 - first0       ;
    return replace( p0 , n0 , s );
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace(iterator first0 , iterator last0 , int n , char c )
{
    assert( first0.m_Data == m_Data );
    const int p0 = first0.m_Position    ;
    const int n0 = last0 - first0       ;
    return replace( p0 , n0 , n , c );
}

//------------------------------------------------------------------------
// Replacing
//------------------------------------------------------------------------
cstring & cstring::replace(iterator first0 , iterator last0 , const_iterator first , const_iterator last)
{
    assert( first0.m_Data == m_Data );
    const int p0  = first0.m_Position       ;
    const int n0  = last0      - first0     ;
    const int pos = first.m_Position        ;
    const int n   = last       - first      ;
    if( first.m_Data == m_Data && n > 0 ) // second iterators refer to *this?
    {
        // We have to make a copy of *this before we do the replacement
        cstring Copy = *this ;
        replace( p0 , n0 , &Copy.m_Data[pos] , n );
    }
    else
    {
        replace( p0 , n0 , &first.m_Data[first.m_Position] , n );
    }
    return *this;
}

//------------------------------------------------------------------------
// Copy to a string
//------------------------------------------------------------------------
int cstring::copy( char * s , int n , int pos ) const
{
    const int CopyLength = Max( Min( this->length() - pos , n ) , 0 );
    if( CopyLength > 0 )
    {
        memcpy( s , m_Data , CopyLength );
    }
    return CopyLength;
}

//------------------------------------------------------------------------
// Swap
//------------------------------------------------------------------------
void cstring::swap( cstring & str )
{
    char * ThisOldData = m_Data ;
    m_Data  = str.m_Data      ;
    str.m_Data    = ThisOldData   ;
}

//------------------------------------------------------------------------
// Substring
//------------------------------------------------------------------------
cstring cstring::substr( int pos , int n ) const
{
    const int Length            = Min( this->length() - pos , ( n==npos ? MaxSize : n ) );
    char      LocalBuffer[100]  ; // If possible, use this for better performance (fewer string allocations).

    if( Length <= 0 )
    {
        return "";
    }
    else if( Length < ARRAY_LENGTH(LocalBuffer) ) // Do we have room to use the local buffer (and add a trailing null)?
    {
        memcpy( LocalBuffer , &m_Data[pos] , Length );
        LocalBuffer[Length] = 0 ; // Add null termination.
        return LocalBuffer; // This will convert the local buffer into a cstring
    }
    else
    {
        return cstring( *this , pos , Length );
    }
}

//------------------------------------------------------------------------
// Compare two character sequences of known length. 
//------------------------------------------------------------------------
int cstring::Compare( const char * lhs , int lhsLength , const char * rhs , int rhsLength )
{
    int compare ;
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
        compare = memcmp( lhs , rhs , rhsLength );
        if( compare == 0 ) // Complete rhs found at start of lhs?
        {
            compare = 1; // (Since lhs is longer)
        }
    }
    else if( rhsLength > lhsLength ) // rhs is longer than lhs?
    {
        compare = memcmp( lhs , rhs , lhsLength );
        if( compare == 0 ) // Complete lhs found at start of rhs?
        {
            compare = -1; // (Since rhs is longer)
        }
    }
    else // Strings are the same non-zero length.
    {
        compare = memcmp( lhs , rhs , lhsLength );
    }

    return compare;
}

//------------------------------------------------------------------------
// Compare a character sequence of known length with a null-terminated string. 
//------------------------------------------------------------------------
int cstring::Compare( const char * lhs , int lhsLength , const char * rhs )
{
    int compare ;
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
                return int( pLhs[0] ) - int( pRhs[0] ); // <=========== Unstructured return.
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
int cstring::compare( const cstring & str ) const
{
    return Compare( m_Data , this->length() , str.m_Data , str.length() );
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int cstring::compare( int p0 , int n0 , const cstring & str ) const
{
    assert( p0 >= 0 && p0 <= length()   );
    assert( n0 >= 0                     );
    const int MaxLhsLength = length() - p0 ;
    const int LhsLength    = Min( MaxLhsLength , n0 );
    return Compare( &m_Data[p0] , LhsLength , str.m_Data , str.length() );
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int cstring::compare( int p0 , int n0 , const cstring & str , int pos , int n ) const
{
    assert( p0  >= 0 && p0 <= length()      );
    assert( n0  >= 0                        );
    assert( pos >= 0 && pos <= str.length() );
    assert( n   >= 0                        );
    const int MaxLhsLength = length() - p0              ;
    const int LhsLength    = Min( MaxLhsLength , n0 )   ;
    const int MaxRhsLength = str.length() - pos         ;
    const int RhsLength    = Min( MaxRhsLength , n  )   ;
    return Compare( &m_Data[p0] , LhsLength , &str.m_Data[pos] , RhsLength );
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int cstring::compare( const char * s ) const
{
    int compare;
    if( m_Data == 0 || m_Data[0] == 0 ) // *this is empty?
    {
        compare = ( s == 0 || s[0]==0 ) ? 0 : -1 ; // If s is not empty, *this < s.
    }
    else if( s == 0 ) // s is empty?
    {
        compare = m_Data[0] == 0 ? 0 : +1 ; // If *this is not empty, *this > s.
    }
    else
    {
        compare = strcmp( m_Data , s );
    }
    return compare;
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int cstring::compare( int p0 , int n0 , const char * s ) const
{
    int compare;
    assert( p0  >= 0 && p0 <= length()      );
    assert( n0  >= 0                        );
    const int LhsLength    = Max( 0 , Min( length() - p0 , n0 ) ) ;
    if( LhsLength <= 0 ) // Empty lhs? 
    {
        compare = ( s == 0 || s[0]==0 ) ? 0 : -1 ; // If s is not empty, lhs < s.
    }
    else if( s == 0 ) // s is empty?
    {
        compare = LhsLength == 0 ? 0 : +1 ; // If lhs is not empty, lhs > s.
    }
    else
    {
        compare = Compare( &m_Data[p0] , LhsLength , s );
    }
    return compare;
}

//------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------
int cstring::compare( int p0 , int n0 , const char * s , int pos ) const
{
    return compare( p0 , n0 , &s[pos] );
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
int cstring::find( const cstring & str , int pos ) const
{
    const int CompareLength = str.length();
    if( this->empty() || pos >= this->length() || CompareLength == 0 )  // Nothing to search in or nothing to search for?
    {
        // No need to search
    }
    else 
    {
        pos = Max(pos,0);
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
int cstring::find( const char * s , int pos , int n ) const
{
    if( this->empty() || pos >= this->length() || s == 0 || s[0]==0 || n <= 0 ) // Nothing to search in or search for
    {
        // No need to search
    }
    else // Do a search
    {
        pos = Max(pos,0);
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
                    if( Compare[0] == 0 || Compare >= FindLimit ) // Comparison reached end of find string?
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
int cstring::find( const char * s , int pos ) const
{
    if( this->empty() || pos >= length() || s == 0 || s[0]==0  )  // Nothing to search in or search for
    {
        // No need to search
    }
    else // Do a search
    {
        pos = Max(pos,0);
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
int cstring::find( char c , int pos ) const
{
    if( empty() || pos >= length() ) // Nothing to search in?
    {
    }
    else // Do a search:
    {
        pos = Max(pos,0);
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
int cstring::rfind( char c , int pos ) const
{
    const int   rightpos    = this->length() - 1    ; // Rightmost position in this.

    pos = pos == npos ? rightpos : Min( pos , rightpos );
    if( pos < 0 ) // Nothing to search in?
    {
    }
    else // Do a search:
    {

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
int cstring::rfind( const cstring & str , int pos ) const
{
    const int   CompareLength   = str.length()          ;
    const int   rightpos        = this->length() - 1    ; // Rightmost position in this.

    pos = pos == npos ? rightpos : Min( pos , rightpos );
    if( pos < 0 || CompareLength == 0 )  // Nothing to search in or nothing to search for?
    {
        // No need to search
    }
    else 
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
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
int cstring::rfind( const char * s , int pos , int n ) const
{
    const int   rightpos    = this->length() - 1    ; // Rightmost position in this.

    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
                            Compare[0] == 0         // Reached end of Find[]?
                        ||  Compare >= FindLimit    // Reached max number of characters to compare?
                    )
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
int cstring::rfind( const char * s , int pos ) const
{
    const int   rightpos    = this->length() - 1    ; // Rightmost position in this.

    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
int cstring::find_first_of( const cstring & str , int pos ) const
{
    const int   CompareLength = str.length() ;
    
    pos = Max(pos,0);
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
int cstring::find_first_of( const char * s , int pos , int n ) const
{
    pos = Max(pos,0);
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
            for( Compare = Find ; Compare[0] != 0 && Compare < FindLimit ; Compare++ )
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
int cstring::find_first_of( const char * s , int pos ) const
{
    pos = Max(pos,0);
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
int cstring::find_last_of( const cstring & str , int pos ) const
{
    const int rightpos      = this->length() - 1    ; // Rightmost position in this.
    const int CompareLength = str.length()          ;
    
    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
    return npos; // If we get here, no match was found.
}


//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
int cstring::find_last_of( const char * s , int pos ,  int n ) const
{
    const int rightpos = this->length() - 1    ; // Rightmost position in this.
    
    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
            for( F = Find ; F[0] != 0 && F < FindLimit ; F++ )
            {
                if( S == F[0] ) // Found a matching character?
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
int cstring::find_last_of( const char * s , int pos ) const
{
    const int rightpos = this->length() - 1    ; // Rightmost position in this.
    
    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
int cstring::find_first_not_of( const cstring & str , int pos ) const
{
    const int   CompareLength = str.length() ;
   
    pos = Max(pos,0);
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
int cstring::find_first_not_of( const char * s , int pos , int n ) const
{
    pos = Max(pos,0);
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
                if( Compare >= FindLimit || Compare[0] == 0 ) // Exhausted Find[]?
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
int cstring::find_first_not_of( const char * s , int pos ) const
{
    pos = Max(pos,0);
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
int cstring::find_first_not_of( char c , int pos ) const
{
    pos = Max(pos,0);
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
int cstring::find_last_not_of( const cstring & str , int pos ) const
{
    const int rightpos      = this->length() - 1    ; // Rightmost position in this.
    const int CompareLength = str.length()          ;
    
    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
    return npos; // If we get here, no match was found.
}


//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
int cstring::find_last_not_of( const char * s , int pos , int n ) const
{
    const int rightpos = this->length() - 1    ; // Rightmost position in this.
    
    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
                if( F[1] == 0 || F >= LastFind ) // Ran out of things to find?
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
int cstring::find_last_not_of( const char * s , int pos ) const
{
    const int rightpos = this->length() - 1    ; // Rightmost position in this.
    
    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// Searching
//------------------------------------------------------------------------
int cstring::find_last_not_of( char c , int pos ) const
{
    const int rightpos = this->length() - 1    ; // Rightmost position in this.
    
    pos = pos == npos ? rightpos : Min( pos , rightpos );
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
    return npos; // If we get here, no match was found.
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
void cstring::trim       () { trim      (" \t\r\n"); }
void cstring::trim_left  () { trim_left (" \t\r\n"); }
void cstring::trim_right () { trim_right(" \t\r\n"); }

//------------------------------------------------------------------------
// Remove from the front and end of *this any characters in *targets.
//------------------------------------------------------------------------
void cstring::trim( const char * targets )
{
    trim_left  ( targets );
    trim_right ( targets );
}

//------------------------------------------------------------------------
// Remove from the front of *this any characters in *targets.
//------------------------------------------------------------------------
void cstring::trim_left( const char * targets )
{
    const int pos = find_first_not_of( targets );
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
void cstring::trim_right( const char * targets )
{
    const int pos = find_last_not_of( targets );
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
void cstring::trim_left( int n )
{
    if( n > 0 )
    {
        erase( 0 , n );
    }
}

//------------------------------------------------------------------------
// Remove up to n characters from the right of this.
//------------------------------------------------------------------------
void cstring::trim_right ( int n ) 
{
    const int pos = length() - n;
    if( n <= 0 )
    {
    }
    else if( pos <= 0 ) // Trim string into nothing?
    {
        resize(0);
    }
    else
    {
        erase( pos , n );
    }
}

//------------------------------------------------------------------------
// The first n characters of this, or all of this if n >= length().
//------------------------------------------------------------------------
cstring cstring::left( int n ) const
{
    const int Length            = Min( n , this->length() );
    char      LocalBuffer[100]  ; // If possible, use this for better performance (fewer string allocations).
    if( Length <= 0 )
    {
        return "";
    }
    else if( Length < ARRAY_LENGTH(LocalBuffer) ) // Do we have room to use the local buffer (and add a trailing null)?
    {
        memcpy( LocalBuffer , &m_Data[0] , Length );
        LocalBuffer[Length] = 0 ; // Add null termination.
        return LocalBuffer; // This will convert the local buffer into a cstring
    }
    else
    {
        return cstring( *this , 0 , Length );
    }
}

//------------------------------------------------------------------------
// The last n characters of this, or all of this if n >= length().
//------------------------------------------------------------------------
cstring cstring::right( int n ) const
{
    const int Length            = Min( n , this->length() );
    char      LocalBuffer[100]  ; // If possible, use this for better performance (fewer string allocations).
    if( Length <= 0 )
    {
        return "";
    }
    else if( Length < ARRAY_LENGTH(LocalBuffer) ) // Do we have room to use the local buffer (and add a trailing null)?
    {
        memcpy( LocalBuffer , &m_Data[this->length()-Length] , Length );
        LocalBuffer[Length] = 0 ; // Add null termination.
        return LocalBuffer; // This will convert the local buffer into a cstring
    }
    else
    {
        return cstring( *this , this->length()-Length , Length );
    }
}

//------------------------------------------------------------------------
// Convert entire string to uppercase.
//------------------------------------------------------------------------
void cstring::to_upper()
{
    char * Data = m_Data ;
    if( Data != 0 )
    {
        for( ; Data[0] != 0 ; Data++ )
        {
            if( Data[0] >= 'a' && Data[0] <= 'z' )
            {
                Data[0] += (int)'A' - (int)'a' ;
            }
        }
    }
}

//------------------------------------------------------------------------
// Convert entire string to lowercase.
//------------------------------------------------------------------------
void cstring::to_lower() 
{
    char * Data = m_Data ;
    if( Data != 0 )
    {
        for( ; Data[0] != 0 ; Data++ )
        {
            if( Data[0] >= 'A' && Data[0] <= 'Z' )
            {
                Data[0] += (int)'a' - (int)'A' ;
            }
        }
    }
}

//------------------------------------------------------------------------
// Reverse the contents of the string
//------------------------------------------------------------------------
void cstring::reverse() 
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
// Debug: Check a condition and increment TestCount.
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static int TestCount = 0 ; // To count the tests done.
static inline void Test( bool Condition )
{
    assert(Condition);
    Condition = Condition ;
    TestCount++;
}
#endif

//------------------------------------------------------------------------
// Debug: Make sure the data at two memory locations is the same.
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static inline void Test( const void * Data1 , const void * Data2 , int N )
{
    Test( N <= 0 || memcmp(Data1,Data2,N) == 0 );
}
#endif

//------------------------------------------------------------------------
// Debug: Check for cstrings with the same contents
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static void TestContents( const cstring & S1 , const cstring & S2 )
{
    Test( S1.length() == S2.length() );
    Test( S1.data() , S2.data() , S1.length() );
}
#endif

//------------------------------------------------------------------------
// Debug: Check for a cstring with the same content as a string
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static void TestContents( const cstring & S1 , const char * S2 )
{
    const int S2Length = S2==0 ? 0 : strlen(S2);
    Test( S1.length() == S2Length );
    Test( S1.data() , S2 , S1.length() );
}
#endif

//------------------------------------------------------------------------
// Debug: A set of test integers
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST

class CIntegerSet
{
  public:
    CIntegerSet( int N , int Min , int Max ); // Make N integers from Min..Max.
    ~CIntegerSet() { delete[] m_Values; }
    int Count() const { return m_Count; }
    int operator [] ( int I ) const { return m_Values[I]; }
  private:
    int * m_Values  ;
    int   m_Count   ;
};

CIntegerSet::CIntegerSet( int N , int Min , int Max )
{
    const int Range     = Max - Min + 1;
    const int HalfRange = Range / 2 ;
    const int Middle    = ( Min + Max ) / 2 ;
    m_Values = new int[N];
    m_Count = N;
    int NAdded = 0; // Number of int's added.
    if( HalfRange == 0 || Range <= N )
    {
        while( NAdded < N )
        {
            m_Values[NAdded] = Min + ( NAdded % Range );
            NAdded++;
        }
        m_Count = ::Min( Range , N );
    }
    else
    {
        // Distribute the values around the end-points and the middle,
        // with increasing distance from these points.
        int Delta = 0 ; // How far away from endpoints and middle.
        int CountdownToDeltaJump = 5;
        while( NAdded < N )
        {
            // Add a value at or near Min
            if( NAdded < N ) 
            {
                m_Values[NAdded] = Min + ( Delta % Range ) ;
                NAdded++;
            }
            // Add a value at or near Max
            if( NAdded < N ) 
            {
                m_Values[NAdded] = Max - ( Delta % Range ) ;
                NAdded++;
            }
            // Add a value at or above Middle
            if( NAdded < N ) 
            {
                m_Values[NAdded] = Middle + ( Delta % HalfRange ) ;
                NAdded++;
            }
            // Add a value below Middle
            if( NAdded < N && Delta != 0 ) 
            {
                m_Values[NAdded] = Middle - ( Delta % HalfRange ) ;
                NAdded++;
            }
            CountdownToDeltaJump--;
            Delta++;
            if( CountdownToDeltaJump <= 0 ) //Time for extra increase in Delta?
            {
                CountdownToDeltaJump = 10 ;
                Delta += 1;
            }
        }
    }
}   

#endif

//------------------------------------------------------------------------
// Debug: Do a number of axiomatic tests on This of basic cstring functionality.
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static void Test( const cstring & This )
{
        
    //------------------------------------------------------------------------
    // Tests for all cstring's, null or otherwise.
    //------------------------------------------------------------------------
    Test( This.size()         == This.length()        );
    Test( This.capacity()     >= This.length()        );
    Test( This.data()         == This.c_str()         );
    Test( This.c_str()        == static_cast<const char *>(This) );
    Test( This.empty()        == (This.length() == 0) );

    //------------------------------------------------------------------------
    // Tests for all null cstring's
    //------------------------------------------------------------------------
    if( This.data() == 0 ) 
    {
        Test( This                == 0                );
        Test( This                == (const char *)0  );
        Test( This.c_str()        == 0                );
        Test( This.c_str()        == (const char *)0  );
        Test( This.data()         == 0                );
        Test( This.data()         == (const char *)0  );
        Test( This.capacity()     == 0                );
        Test( This.length()       == 0                );
    }
    //------------------------------------------------------------------------
    // Tests for all non-null cstring's
    //------------------------------------------------------------------------
    else 
    {
        Test( This                != 0                 );
        Test( This                != (const char *)0   );
        Test( This.c_str()        != 0                 );
        Test( This.c_str()        != (const char *)0   );
        Test( This.data()         != 0                 );
        Test( This.data()         != (const char *)0   );
        Test( This.length()       == int(strlen(This)) );

        // Check forward iteration
        {
            int Index = 0 ;
            int NChecked = 0;
            for( cstring::const_iterator iterator = This.begin() ; iterator != This.end() ; ++iterator, Index++ )
            {
                Test( *iterator == This.data()[Index] );
                NChecked++;
            }
            Test( NChecked == This.length() );
        }
        // Check reverse iteration
        {
            int Index = This.length()-1;
            int NChecked = 0;
            for( cstring::const_reverse_iterator iterator = This.rbegin() ; iterator != This.rend() ; iterator++, Index-- )
            {
                Test( *iterator == This.c_str()[Index] );
                NChecked++;
            }
            Test( NChecked == This.length() );
        }

        // Check indexing

        // Check indexing operations
        for( int pos = 0 ; pos < This.length() ; pos++ )
        {
            const char c = This[pos];
            Test( This.at(pos)    == c );
            Test( This.c_str()[pos]  == c );
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Test sizes
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static const int TestSizes[] = // A list of test sizes. Can include a size of 0.
{
    10      ,
    9       ,
    8       ,
    7       ,
    6       ,
    5       ,
    4       ,
    3       ,
    2       ,
    1       ,
    0       ,
    100     , 
    1000    , 
    10000
};
const int NTestSizes = sizeof(TestSizes) / sizeof(TestSizes[0]);
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring constructors
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestConstructors( const char * String )
{
    //------------------------------------------------------------
    // cstring()
    //------------------------------------------------------------
    {
        cstring S ; 

        Test(S);
        Test( S.empty()       );
        Test( S.length() == 0 );
    }

    //------------------------------------------------------------
    // cstring( int n , char c )
    //------------------------------------------------------------
    {
        char c = 0x01;
        for( int WhichSize = 0 ; WhichSize < NTestSizes ; ++WhichSize )
        {
            const int n = TestSizes[WhichSize];
            cstring S( n , c );
            Test(S);
            Test( S.length() == n );

            // Should have character c at each index.
            for( int pos = 0 ; pos < S.length() ; pos++ )
            {
                Test( S[pos] == c );
            }
            c++;
            if( c == 0 )
            {
                c++;
            }
        }
    }

    //------------------------------------------------------------
    // cstring( const char * s )
    //------------------------------------------------------------
    {
        cstring B0  (0)          ; 
        cstring B1  = 0          ; 
        cstring B   (String) ; 

        Test(B0)    ; 
        Test(B1)    ;
        Test(B)     ;

        Test( B0.empty() );
        Test( B1.empty() );
        Test( B.data() != 0 );
        TestContents( B , String );
        Test( ( B.data() != String) ); // B cannot use String directly
    }

    //------------------------------------------------------------
    // cstring( const cstring & rhs )
    //------------------------------------------------------------
    {
        cstring B0 (0)      ; 
        cstring B  (String) ; 
        cstring C0 (B0)     ; // Test using null rhs.
        cstring C  (B)      ; 

        Test(C0) ;
        Test(C)  ;

        Test( C0.empty() );
        TestContents( B , C );
    }

    //------------------------------------------------------------
    // cstring( const char * s , int n )
    //------------------------------------------------------------
    {
        const int MaxLength = String == 0 ? 0 : strlen(String);
        const int MaxN = MaxLength + 10;
        for( int n = 0 ; n < MaxN ; n++ )
        {
            const int ExpectedLength = Min( MaxLength , n );
            cstring S( String , n );
            Test(S);

            Test( S.length() == ExpectedLength );
            for( int pos = 0 ; pos < S.length() ; pos++ ) // Check each character
            {
                Test( S[pos] == String[pos] );
            }
        }
    }

    //------------------------------------------------------------
    // cstring( const cstring & rhs , int pos , int n  )
    //------------------------------------------------------------
    {
        cstring A(String);
        for( int posA = 0  ; posA < A.length() ; ++posA ) // Check all possible starting positions of rhs
        {
            const int RemainingLengthInA = A.length() - posA ;
            for( int n = 0 ; n <= RemainingLengthInA+5 ; n++ ) // Check all possible lengths, even going a little beyond the string.
            {
                const int ExpectedLength = Min( RemainingLengthInA , n );
                cstring S(A,posA,n);
                Test(S);

                Test( S.length() == ExpectedLength );
                for( int posS = 0 ; posS < S.length() ; posS++ ) // Check each character
                {
                    Test( S[posS] == A[posA+posS] );
                }
            }
        }
    }
    //------------------------------------------------------------
    // cstring( const_iterator first , const_iterator last )
    //------------------------------------------------------------
    {
        cstring A(String);
        for( cstring::const_iterator it1 = A.begin() ; it1 != A.end() ; it1++ )
        {
            for( cstring::const_iterator it2 = A.begin() ; it2 != A.end() ; it2++ )
            {
                const int ExpectedLength = Max( it2 - it1 , 0 );
                cstring S( it1 , it2 );
                Test( S.length() == ExpectedLength );

                cstring::const_iterator it3 = it1 ;
                for( int pos = 0 ; pos < S.length() && it3 != A.end() ; pos++ , it3++ )
                {
                    Test( *it3 == S[pos] );
                }
            }
        }
    }

}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring assignment
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestAssignment( const char * /*String1*/ , const char * /*String2*/ )
{

//?     cstring & operator = ( const cstring & rhs )    ; // Copy rhs into *this.
//?     cstring & operator = ( const char * s )         ; // Copy null-terminated s into *this.
//?     cstring & operator = ( char c )                 ; // Make *this a string with a single character c.
//?     cstring & assign( const cstring & str )         { return *this = str ; }
//?     cstring & assign( const cstring & str , int pos , int n ); // pos < str.length()
//?     cstring & assign( const char * s , int n );
//?     cstring & assign( const char * s )              { return *this = s ; }
//?     cstring & assign( int n , char c );
//?     cstring & assign( const_iterator first , const_iterator last );
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring iterators
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestIterators( const char * String )
{
    const int       Length  = String==0 ? 0 : strlen(String);
    cstring         S       = String ; // non-const cstring
    const cstring   C       = String ; // const cstring

    cstring::iterator               it      ;
    cstring::const_iterator         cit     ;
    cstring::reverse_iterator       rit     ;
    cstring::const_reverse_iterator crit    ;

    TestContents( S , C );

    // iterator                begin  ()       
    // const_iterator          begin  () const 
    // iterator                end    ()       
    // const_iterator          end    () const 
    // reverse_iterator        rbegin ()       
    // const_reverse_iterator  rbegin () const 
    // reverse_iterator        rend   ()       
    // const_reverse_iterator  rend   () const 
    if( Length == 0 ) // Empty string?
    {
        it   = S.begin  ()  ; Test( it   == S.end ()  );
        cit  = C.begin  ()  ; Test( cit  == C.end ()  );
        rit  = S.rbegin ()  ; Test( rit  == S.rend()  );
        crit = C.rbegin ()  ; Test( crit == C.rend()  );

    }
    else // Non-empty string
    {
        it   = S.begin  ()  ; Test( it   != S.end ()  );
        cit  = C.begin  ()  ; Test( cit  != C.end ()  );
        rit  = S.rbegin ()  ; Test( rit  != S.rend()  );
        crit = C.rbegin ()  ; Test( crit != C.rend()  );

        for( int pos = 0 ; pos < Length ; pos++ , it++ , cit++ , rit++ , crit++ )
        {
            Test( String[pos]           == *it   );
            Test( String[pos]           == *cit  );
            Test( String[Length-pos-1]  == *rit  );
            Test( String[Length-pos-1]  == *crit );
            Test( it   != S.end ()  );
            Test( cit  != C.end ()  );
            Test( rit  != S.rend()  );
            Test( crit != C.rend()  );
        }        
        Test( it   == S.end ()  );
        Test( cit  == C.end ()  );
        Test( rit  == S.rend()  );
        Test( crit == C.rend()  );
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring indexing
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestIndexing( const char * String )
{
    const int       Length  = String==0 ? 0 : strlen(String);
    cstring         S       = String    ; // non-const cstring
    const cstring   C       = String    ; // const cstring

    for( int pos = 0 ; pos < Length ; ++pos )
    {
        // char    at         ( int pos ) const
        // char &  at         ( int pos )      
        // char    operator[] ( int pos ) const
        // char &  operator[] ( int pos )      
        Test( S.at(pos) == String[pos] );
        Test( C.at(pos) == String[pos] );
        Test( S[pos]    == String[pos] );
        Test( C[pos]    == String[pos] );
    }

    // Also make sure there is a terminating null:
    Test( S == 0 || S.c_str()[Length] == 0 );
    Test( C == 0 || C.c_str()[Length] == 0 );
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring data access
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestDataAccess( const char * String )
{
    const cstring S = String ;
    const char *    Conversion = static_cast<const char *>(S)   ; // operator const char * () const
    const char *    c_str      = S.c_str()                      ; // const char * c_str () const
    const char *    data       = S.data()                       ; // const char * data  () const
    Test( Conversion != 0       ) ;
    Test( Conversion != String  ) ; // S cannot use String directly
    Test( Conversion == c_str );
    Test( data       == c_str );
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring private functions
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestPrivate( const char * String )
{
    cstring     S       = String ;
    const int   Length  = String==0 ? 0 : strlen(String);
    Test( S.private_data() == S.data() ); // char * private_data ()

    // void private_resize( int n );
    S.private_resize( Length*50 );
    Test( S.length() == (Length*50) );
    Test( S.private_data() , String , Length );
    S.private_resize( Length/2 );
    Test( S.length() == (Length/2) );
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring capacity- and length-related functions
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestCapacityAndLength( const char * String )
{
    // int     length   () const                 
    // int     size     () const                 
    // int     max_size () const                 
    // int     capacity () const                 
    // void    reserve  ( int n = 0 );
    // bool    empty    () const                 

    cstring     S       = String ;
    const int   Length  = String==0 ? 0 : strlen(String);
    TestContents( S , String );
    Test( S.length()    == Length       );
    Test( S.size()      == Length       );
    Test( S.max_size()  >= Length       );
    Test( S.capacity()  >= Length       );
    Test( S.empty()     == (Length==0)  );
    S.reserve( 0 );
    TestContents( S , String );
    S.reserve( Length/2 );
    TestContents( S , String );
    for( int n = Max(Length-2,0); n <= Length+5 ; n++ )
    {
        S.reserve( n );
        Test( S.length() == Length );
        Test( S.capacity() >= n );
        TestContents( S , String );
    }
    // void    resize   ( int n , char c = ' ' );
    {
        cstring     A           = S             ;
        char        c           = 'a'           ;
        const int   OldLength   = A.length()    ;
        for( int NewSize = OldLength ; NewSize <= OldLength + 30 ; NewSize++ )
        {
            A = S ;
            A.resize( NewSize , c );
            Test( A.length() == NewSize );
            Test( A.c_str()[NewSize] == 0 ) ; //Null-terminated?
            Test( A.data() , String , OldLength ); // First part the same?
            for( int pos = OldLength ; pos < NewSize ; pos++ )
            {
                Test( A[pos] == c ); // Rest filled in with c?
            }
            c = c == '\xff' ? char(0x01) : char(c+1); // Next character, skipping 0.
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring addition and catenation
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestCatenation( const char * String1 , const char * String2 )
{
    const int Length1 = String1==0 ? 0 : strlen(String1);
    const int Length2 = String2==0 ? 0 : strlen(String2);

    // cstring & operator += ( const cstring & rhs );
    {
        cstring         S  = String1 ;
        const cstring   S2 = String2 ;
        S += S2 ; 
        Test( S.length() == Length1+Length2 );
        Test( S.data() , String1 , Length1 ); // First part correct?
        Test( &S.data()[Length1] , String2 , Length2 );
        Test( S.data()[Length1+Length2] == 0 ); // Check for null termination
    }    

    // cstring & operator += ( const char * s );
    {
        cstring S  = String1 ;
        S += String2 ; 
        Test( S.length() == Length1+Length2 );
        Test( S.data() , String1 , Length1 ); // First part correct?
        Test( &S.data()[Length1] , String2 , Length2 ); // Is second part correct?
        Test( S.data()[Length1+Length2] == 0 ); // Check for null termination
    }    

    // cstring & operator += ( char c );
    {
        for( char c = '\x01' ; c <= '\x23' ; ++c )
        {
            cstring S = String1 ;
            S += c ;
            Test( S.length() == Length1+1 );
            Test( S.data() , String1 , Length1 ); // First part is correct?
            Test( S[Length1] == c ); // Added correct character?
            Test( S.data()[Length1+1] == 0 ); // Added null terminator?
        }
    }

    // cstring & append( const cstring & str )   
    {
        cstring         S  = String1 ;
        const cstring   S2 = String2 ;
        S.append( S2 );
        Test( S.length() == Length1+Length2 );
        Test( S.data() , String1 , Length1 );
        Test( &S.data()[Length1] , String2 , Length2);
        Test( S.data()[Length1+Length2] == 0 ); // Check for null termination
    }

    // cstring & append( const cstring & str , int pos , int n )
    {
        const cstring str = String2 ;
        for( int pos = 0 ; pos < Length2 ; pos++ )
        {
            const int MaxLength = Length2 - pos ;
            for( int n = 0 ; n <= MaxLength + 2 ; ++n )
            {
                const int AddedLength = Min( MaxLength , n );
                cstring S = String1;
                S.append( str , pos , n );
                Test( S.length() == Length1 + AddedLength );
                Test( S.data() , String1 , Length1 ); // First part is correct?
                Test( &S.data()[Length1] , &String2[pos] , AddedLength ); // Second part is correct?
                Test( S.data()[ Length1 + AddedLength ] == 0 ) ; // Check for null termination.
            }
        }
    }

    // cstring & append( const char * s , int n )
    {
        const int MaxLength = Length2;
        for( int n = -3 ; n <= MaxLength + 2 ; ++n )
        {
            const int AddedLength = FitToRange( n , 0 , MaxLength );
            cstring S = String1;
            S.append( String2 , n );
            Test( S.length() == Length1 + AddedLength );
            Test( S.data(),String1,Length1); // First part is correct?
            Test( &S.data()[Length1],String2,AddedLength); // Second part is correct?
            Test( S.data()[ Length1 + AddedLength ] == 0 ) ; // Check for null termination.
        }
    }

    // cstring & append( const char * s )
    {
        cstring S  = String1 ;
        S.append( String2 ); 
        Test( S.length() == Length1+Length2 );
        Test( S.data(),String1,Length1); // Is first part correct?
        Test( &S.data()[Length1],String2,Length2 ); // Is second part correct?
        Test( S.data()[Length1+Length2] == 0 ); // Check for null termination
    }    

    // cstring & append( int n , char c )
    {
        char c = '\x53' ;
        for( int n = -2 ; n <= 20 ; n++ )
        {
            const int LengthAdded = Max(n,0);
            cstring S = String1 ;
            S.append( n , c );
            Test( S.length() == Length1 + LengthAdded );
            Test( S.data(),String1,Length1 ); // Is first part correct?
            for( int pos = Length1 ; pos < Length1 + LengthAdded ; pos++ )
            {
                Test( S[pos] == c );
            }
            Test( S.data()[S.length()] == 0 ); // Check for null termination.

            c = c == '\xff' ? char(0x01) : char(c+1); // Next character, skipping 0.
        }
    }
    // cstring & append( const_iterator first , const_iterator last )
    {
        const cstring rhs = String2 ;
        cstring::const_iterator it1 ;
        cstring::const_iterator it2 ;
        for( it1 = rhs.begin() ; it1 != rhs.end() ; it1++ )
        {
            for( it2 = rhs.begin() ; it2 != rhs.end() ; it2++ )
            {
                const int AddedLength = Max( it2-it1 , 0 );
                cstring S = String1 ;
                S.append( it1 , it2 );
                Test( S.length() == Length1 + AddedLength );
                Test( &S.data()[0] , String1 , Length1 ); // First part as expected?
                Test( &S.data()[Length1] , &*it1 , AddedLength ); // Second part as expected?
                Test( S.data()[S.length()] == 0 ); // Check null termination
            } 
        } 
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring insertion
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestInsertion( const char * String1 , const char * String2 )
{
    const int Length1 = String1==0 ? 0 : strlen(String1);
    const int Length2 = String2==0 ? 0 : strlen(String2);

    // Insert       is the point of insertion and the length of the original string left at the beginning.
    // InsertLength is the length of the inserted string.
    // After        is the point after the last character of the string inserted.
    // AfterLength  is the length of the original string after insertion.

    {
        const cstring str = String2 ;
        CIntegerSet p0Values( 20 , 0 , Length1 );
        for( int Whichp0 = 0 ; Whichp0 < p0Values.Count() ; ++Whichp0 )
        {
            const int p0            = p0Values[Whichp0];
            const int Insert        = p0 ;
            const int AfterLength   = Length1 - Insert ;
            // cstring & insert( int p0 , const cstring & str );
            {
                const int InsertLength = str.length(); 
                const int After         = Insert + InsertLength ;
                cstring S = String1 ;
                S.insert( p0 , str );
                Test( S.length()   == Length1 + InsertLength );
                Test( &S.data()[0]      , String1           , Insert        ); // First part okay?
                Test( &S.data()[Insert] , String2           , InsertLength  ); // Inserted part okay?
                Test( &S.data()[After]  , &String1[Insert]  , AfterLength   ); // Final part okay?
                Test(  S.data()[ S.length() ] == 0 ); // Check null termination.
            }
            // cstring & insert( int p0 , const cstring & str , int pos , int n ); // pos < str.length()
            {
                CIntegerSet posValues( 20 , 0 , str.length() );
                for( int Whichpos = 0 ; Whichpos < posValues.Count() ; ++Whichpos )
                {
                    const int pos            = posValues[Whichpos];
                    CIntegerSet nValues( 11 , -1 , str.length()-pos+1 );
                    for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                    {
                        const int n            = nValues[Whichn];
                        const int InsertLength = Max( 0 , Min( str.length() - pos , n ) ); 
                        const int After         = Insert + InsertLength ;
                        cstring S = String1 ;
                        S.insert( p0 , str , pos , n );
                        Test( S.length()   == Length1 + InsertLength );
                        Test( &S.data()[0]      , String1           , Insert        ); // First part okay?
                        Test( &S.data()[Insert] , &String2[pos]     , InsertLength  ); // Inserted part okay?
                        Test( &S.data()[After]  , &String1[Insert]  , AfterLength   ); // Final part okay?
                        Test(  S.data()[ S.length() ] == 0 ); // Check null termination.
                    }
                }
            }
            // cstring & insert( int p0 , const char * s );
            {
                const int InsertLength = Length2 ;
                const int After        = Insert + InsertLength ;
                cstring S = String1 ;
                S.insert( p0 , String2 );
                Test( S.length()   == Length1 + InsertLength );
                Test( &S.data()[0]      , String1           , Insert        ); // First part okay?
                Test( &S.data()[Insert] , &String2[0]       , InsertLength  ); // Inserted part okay?
                Test( &S.data()[After]  , &String1[Insert]  , AfterLength   ); // Final part okay?
                Test(  S.data()[ S.length() ] == 0 ); // Check null termination.
            }
            // cstring & insert( int p0 , const char * s , int n );
            {
                CIntegerSet nValues( 20 , -1 , Length2+3 );
                for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                {
                    const int n            = nValues[Whichn];
                    const int InsertLength = Max( 0 , Min( Length2 , n ) ); 
                    const int After        = Insert + InsertLength ;
                    cstring S = String1 ;
                    S.insert( p0 , String2 , n );
                    Test( S.length()   == Length1 + InsertLength );
                    Test( &S.data()[0]      , String1           , Insert        ); // First part okay?
                    Test( &S.data()[Insert] , &String2[0]     , InsertLength  ); // Inserted part okay?
                    Test( &S.data()[After]  , &String1[Insert]  , AfterLength   ); // Final part okay?
                    Test(  S.data()[ S.length() ] == 0 ); // Check null termination.
                }
            }
            // cstring & insert( int p0 , int n , char c );
            {
                char c = '\x76';
                CIntegerSet nValues( 20 , -1 , 40 );
                for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                {
                    const int n            = nValues[Whichn];
                    const int InsertLength = Max( 0 , n ); 
                    const int After        = Insert + InsertLength ;
                    cstring S = String1 ;
                    S.insert( p0 , n , c );
                    Test( S.length()   == Length1 + InsertLength );
                    Test( &S.data()[0]      , String1           , Insert        ); // First part okay?
                    {
                        for( int Check = Insert ; Check < After ; Check++ )
                        {
                            Test( S[Check] == c ); // Inserted part okay?
                        }
                    }
                    Test( &S.data()[After]  , &String1[Insert]  , AfterLength   ); // Final part okay?
                    Test(  S.data()[ S.length() ] == 0 ); // Check null termination.
                    c = c == '\xff' ? '\x01' : char(c+1) ; // Use next character, but skip 0.
                }
            }
        }
    }

    {
        char c = '\x99';
        CIntegerSet posValues( 20 , 0 , Length1 );
        for( int Whichpos = 0 ; Whichpos < posValues.Count() ; ++Whichpos )
        {
            const int pos            = posValues[Whichpos];
            const int Insert        = pos ;
            const int AfterLength   = Length1 - Insert ;

            // iterator  insert( iterator it , char c );   // Insert c before it, and return an iterator to the newly inserted element.
            {
                cstring S = String1; 
                cstring::iterator it = S.begin() + pos;

                const int InsertLength  = 1 ;
                const int After         = Insert + InsertLength ;
                it = S.insert( it , c );
                assert( *it == S.data()[pos] );
                Test( S.length()   == Length1 + InsertLength );
                Test( &S.data()[0] , String1 , Insert ); // First part okay?
                {
                    Test( S[Insert] == c ); // Inserted part okay?
                }
                Test( &S.data()[After]  , &String1[Insert]  , AfterLength   ); // Final part okay?
                Test(  S.data()[ S.length() ] == 0 ); // Check null termination.
            }
            // void insert( iterator it , int n , char c);
            {
                CIntegerSet nValues( 7 , -1 , 10 );
                for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                {
                    cstring S = String1; 
                    cstring::iterator it = S.begin() + pos;

                    const int n            = nValues[Whichn];
                    const int InsertLength = Max( 0 , n ); 
                    const int After        = Insert + InsertLength ;
                    S.insert( it , n , c );
                    Test( S.length()   == Length1 + InsertLength );
                    Test( &S.data()[0]      , String1           , Insert        ); // First part okay?
                    {
                        for( int Check = Insert ; Check < After ; Check++ )
                        {
                            Test( S[Check] == c ); // Inserted part okay?
                        }
                    }
                    Test( &S.data()[After]  , &String1[Insert]  , AfterLength   ); // Final part okay?
                    Test(  S.data()[ S.length() ] == 0 ); // Check null termination.
                }
            }
            // void insert( iterator it , const_iterator first , const_iterator last );
            {
                const cstring rhs = String2 ;
                CIntegerSet pos1Values( 11 , 0 , rhs.length() );
                CIntegerSet pos2Values( 7  , 0 , rhs.length() );
                for( int Whichpos1 = 0 ; Whichpos1 < pos1Values.Count() ; ++Whichpos1 )
                {
                    const int pos1 = pos1Values[Whichpos1];
                    cstring::const_iterator it1 = rhs.begin() + pos1;

                    for( int Whichpos2 = 0 ; Whichpos2 < pos2Values.Count() ; ++Whichpos2 )
                    {
                        cstring S = String1; 
                        cstring::iterator it = S.begin() + pos;

                        const int pos2 = pos2Values[Whichpos2];
                        cstring::const_iterator it2 = rhs.begin() + pos2;

                        const int InsertLength = pos1 >= rhs.length() ? 0 : pos2 <= pos1 ? 0 : pos2-pos1 ;
                        const int After        = Insert + InsertLength ;
                        S.insert( it , it1 , it2 );
                        Test( S.length()   == Length1 + InsertLength );
                        Test( &S.data()[0]      , String1           , Insert        ); // First part okay?
                        Test( &S.data()[Insert] , &String2[pos1]    , InsertLength  ); // Inserted part okay?
                        Test( &S.data()[After]  , &String1[Insert]  , AfterLength   ); // Final part okay?
                        Test(  S.data()[ S.length() ] == 0 ); // Check null termination.
                    }
                }
            }


            c = c=='\xff' ? '\x01' : char(c+1); // Next character, skipping 0
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring erase
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestErase( const char * String )
{
    const int Length = String==0 ? 0 : strlen(String);
    // EraseAt        is the point of erasure and the length of the original string left at the beginning.
    // EraseLength    is the length of the erased string.
    // Leftover       is the point in the original string of anything after the point of erasure.
    // LeftoverLength is how much of the original string still follows the point of erasure.

    // cstring & erase( int p0 = 0 , int n = npos );
    {
        for( int p0 = 0 ; p0 <= Length ; p0++ ) 
        {
            const int EraseAt       = p0 ;
            for( int n = 0 ; n <= Length-p0+2; ++n )
            {
                const int EraseLength       = Max( 0 , Min( (n==cstring::npos?1000000:n) , Length-p0 ) )   ;
                const int Leftover          = EraseAt + EraseLength ;
                const int NewLength         = Length - EraseLength  ;
                const int LeftoverLength    = NewLength - EraseAt   ;
                cstring   S  = String ;
                cstring & S2 = S.erase(p0,n);
                Test( &S2 == &S );
                Test( S.length() == NewLength );
                Test( &S.data()[       0 ] , &String[        0 ] , EraseAt          ); // Test first part.
                Test( &S.data()[ EraseAt ] , &String[ Leftover ] , LeftoverLength   ); // Test second part.
                Test( S.data()[S.length()] == 0 ); // Test for null terminator. 
            }
        }
    }
    // iterator  erase( iterator it );
    {
        for( int pos = 0 ; pos <= Length ; pos++ )
        {
            cstring S = String ;
            cstring::iterator it = S.begin();
            it += pos ;
            if( it != S.end() )
            {   
                Test( *it == String[pos] ); // Make sure we are at the right spot inside S.
            }

            const int EraseAt           = pos ;
            const int EraseLength       = it == S.end() ? 0 : 1   ;
            const int Leftover          = EraseAt + EraseLength ;
            const int NewLength         = Length - EraseLength  ;
            const int LeftoverLength    = NewLength - EraseAt   ;

            cstring::iterator it2 = S.erase(it);

            Test( S.length() == NewLength );
            Test( &S.data()[       0 ] , &String[        0 ] , EraseAt          ); // Test first part.
            Test( &S.data()[ EraseAt ] , &String[ Leftover ] , LeftoverLength   ); // Test second part.
            Test( S.data()[S.length()] == 0 ); // Test for null terminator. 
            
            if( EraseAt == NewLength )
            {
                Test( it2 == S.end() ); // Iterator should be at end
            }
            else
            {
                Test( it2 - S.begin() == EraseAt );
            }
        }
    }
    // iterator erase( iterator first , iterator last );
    {
        for( int pos1 = 0 ; pos1 <= Length ; pos1++ )        
        {
            for( int pos2 = 0 ; pos2 <= Length ; pos2++ )        
            {
                cstring S = String ;
                cstring::iterator       first = S.begin() + pos1 ;
                cstring::iterator       last  = S.begin() + pos2 ;

                const int EraseAt           = pos1 ;
                const int EraseLength       = Max( last - first , 0 );
                const int Leftover          = EraseAt + EraseLength ;
                const int NewLength         = Length - EraseLength  ;
                const int LeftoverLength    = NewLength - EraseAt   ;

                const cstring::iterator it    = S.erase( first , last );

                Test( S.length() == NewLength );
                Test( &S.data()[       0 ] , &String[        0 ] , EraseAt          ); // Test first part.
                Test( &S.data()[ EraseAt ] , &String[ Leftover ] , LeftoverLength   ); // Test second part.
                Test( S.data()[S.length()] == 0 ); // Test for null terminator. 
            
                if( EraseAt == NewLength )
                {
                    Test( it == S.end() ); // Iterator should be at end
                }
                else
                {
                    Test( it - S.begin() == EraseAt );
                }

            }
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring replace
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestReplace( const char * String1 , const char * String2 )
{
    const int Length1 = String1==0 ? 0 : strlen(String1);
    const int Length2 = String2==0 ? 0 : strlen(String2);

    // ReplaceAt        is the point of replacement and the length of the original string left at the beginning.
    // EraseLength      is the amount of the original string erased.
    // ReplaceLength    is the length of the replacement string.
    // LeftoverWas      is the point in the original string of anything after the point of replacement.
    // LeftoverLength   is how much of the original string still follows the point of replacement.
    // LeftoverIsNow    is the point in the new string where the leftover has been moved to.

    // cstring & replace( int p0 , int n0 , const cstring & str );
    // cstring & replace( int p0 , int n0 , const char * s );
    if( true ) 
    {
        const cstring str = String2 ;
        CIntegerSet p0Values( 10 , 0 , Length1 );
        for( int Whichp0 = 0 ; Whichp0 < p0Values.Count() ; ++Whichp0 )
        {
            const int p0 = p0Values[Whichp0];
            CIntegerSet n0Values( 10 , 0 , Length1-p0+2 );
            for( int Whichn0 = 0 ; Whichn0 < n0Values.Count() ; ++Whichn0 )
            {
                const int n0                = n0Values[Whichn0];
                const int ReplaceAt         = p0                                     ;
                const int EraseLength       = Max( 0 , Min(Length1-p0,n0) )          ;
                const int ReplaceLength     = str.length()                           ;
                const int LeftoverWas       = ReplaceAt + EraseLength                ;
                const int LeftoverLength    = Length1 - ReplaceAt - EraseLength      ;
                const int LeftoverIsNow     = ReplaceAt + ReplaceLength              ;
                const int NewLength         = Length1 - EraseLength + ReplaceLength  ;

                for( int WhichKind = 1 ; WhichKind <= 2 ; WhichKind++ )
                {
                    cstring   S = String1 ;
                    if( WhichKind == 1 )
                    {
                        cstring & T = S.replace(p0,n0,str);
                        Test( &S == &T );
                    }
                    else
                    {
                        cstring & T = S.replace(p0,n0,String2);
                        Test( &S == &T );
                    }
                    Test( S.length() == NewLength );
                    Test( &S.data()[       0        ] , &String1[           0 ] , ReplaceAt      ); // Test first part.
                    Test( &S.data()[ ReplaceAt      ] , &String2[           0 ] , ReplaceLength  ); // Test second part.
                    Test( &S.data()[ LeftoverIsNow  ] , &String1[ LeftoverWas ] , LeftoverLength ); // Test third part.
                    Test( S.data()[NewLength] == 0 ); // Test for null terminator. 
                }
            }
        }
    }

    // cstring & replace( int p0 , int n0 , int n , char c );
    if( true ) 
    {
        char c = '\x98' ;
        CIntegerSet p0Values( 8 , 0 , Length1 );
        // ----- p0
        for( int Whichp0 = 0 ; Whichp0 < p0Values.Count() ; ++Whichp0 )
        {
            const int p0 = p0Values[Whichp0];
            CIntegerSet n0Values( 8 , 0 , Length1-p0+1 );
            // ----- n0
            for( int Whichn0 = 0 ; Whichn0 < n0Values.Count() ; ++Whichn0 )
            {
                const int n0 = n0Values[Whichn0];

                CIntegerSet nValues( 8 , 0 , 100 );
                // ----- n
                for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                {
                    const int n = nValues[Whichn];

                    const int ReplaceAt         = p0                                    ;
                    const int EraseLength       = Max( 0 , Min(Length1-p0,n0) )         ;
                    const int ReplaceLength     = Max( 0 , n )                          ;
                    const int LeftoverWas       = ReplaceAt + EraseLength               ;
                    const int LeftoverLength    = Length1 - ReplaceAt - EraseLength     ;
                    const int LeftoverIsNow     = ReplaceAt + ReplaceLength             ;
                    const int NewLength         = Length1 - EraseLength + ReplaceLength ;

                    cstring   S = String1 ;
                    cstring & T = S.replace(p0,n0,n,c);

                    Test( &S == &T );
                    Test( S.length() == NewLength );
                    Test( &S.data()[       0       ] , &String1[           0 ] , ReplaceAt      ); // Test first part.
                    for( int Check = 0 ; Check < ReplaceLength ; ++Check )
                    {
                        Test( S[ReplaceAt+Check] == c ); // Test middle part
                    }
                    Test( &S.data()[ LeftoverIsNow ] , &String1[ LeftoverWas ] , LeftoverLength ); // Test third part.
                    Test( S.data()[NewLength] == 0 ); // Test for null terminator. 

                    c = c == '\xff' ? '\x01' : char(c+1); // Next character, skipping null.
                }
            }
        }
    }

    // cstring & replace( int p0 , int n0 , const char * s , int n );
    if( true ) 
    {
        CIntegerSet p0Values( 8 , 0 , Length1 );
        // ----- p0
        for( int Whichp0 = 0 ; Whichp0 < p0Values.Count() ; ++Whichp0 )
        {
            const int p0 = p0Values[Whichp0];
            CIntegerSet n0Values( 8 , 0 , Length1-p0+1 );
            // ----- n0
            for( int Whichn0 = 0 ; Whichn0 < n0Values.Count() ; ++Whichn0 )
            {
                const int n0 = n0Values[Whichn0];

                CIntegerSet nValues( 8 , 0 , Length2+1 );
                // ----- n
                for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                {
                    const int n = nValues[Whichn];

                    const int ReplaceAt         = p0                                    ;
                    const int EraseLength       = Max( 0 , Min(Length1-p0,n0) )         ;
                    const int ReplaceLength     = Max( 0 , Min(Length2,n) )             ;
                    const int LeftoverWas       = ReplaceAt + EraseLength               ;
                    const int LeftoverLength    = Length1 - ReplaceAt - EraseLength     ;
                    const int LeftoverIsNow     = ReplaceAt + ReplaceLength             ;
                    const int NewLength         = Length1 - EraseLength + ReplaceLength ;

                    cstring   S = String1 ;
                    cstring & T = S.replace(p0,n0,String2,n);

                    Test( &S == &T );
                    Test( S.length() == NewLength );
                    Test( &S.data()[       0       ] , &String1[           0 ] , ReplaceAt      ); // Test first part.
                    Test( &S.data()[     ReplaceAt ] , &String2[           0 ] , ReplaceLength  ); // Test second part.
                    Test( &S.data()[ LeftoverIsNow ] , &String1[ LeftoverWas ] , LeftoverLength ); // Test third part.
                    Test( S.data()[NewLength] == 0 ); // Test for null terminator. 
                }
            }
        }
    }

    // cstring & replace( int p0 , int n0 , const cstring & str , int pos , int n ); // pos < str.length()
    if( true ) 
    {
        const cstring str = String2 ;
        CIntegerSet p0Values( 8 , 0 , Length1 );
        // ----- p0
        for( int Whichp0 = 0 ; Whichp0 < p0Values.Count() ; ++Whichp0 )
        {
            const int p0 = p0Values[Whichp0];
            CIntegerSet n0Values( 8 , 0 , Length1-p0+1 );
            // ----- n0
            for( int Whichn0 = 0 ; Whichn0 < n0Values.Count() ; ++Whichn0 )
            {
                const int n0 = n0Values[Whichn0];

                CIntegerSet posValues( 8 , 0 , Length2 );
                // ----- pos
                for( int Whichpos = 0 ; Whichpos < posValues.Count() ; ++Whichpos )
                {
                    const int pos = posValues[Whichpos];
                    CIntegerSet nValues( 8 , 0 , Length2-pos+1 );
                    // ----- n
                    for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                    {
                        const int n = nValues[Whichn];

                        const int ReplaceAt         = p0                                    ;
                        const int EraseLength       = Max( 0 , Min(Length1-p0,n0) )         ;
                        const int ReplaceLength     = Max( 0 , Min(Length2-pos,n) )         ;
                        const int LeftoverWas       = ReplaceAt + EraseLength               ;
                        const int LeftoverLength    = Length1 - ReplaceAt - EraseLength     ;
                        const int LeftoverIsNow     = ReplaceAt + ReplaceLength             ;
                        const int NewLength         = Length1 - EraseLength + ReplaceLength ;

                        cstring   S = String1 ;
                        cstring & T = S.replace(p0,n0,str,pos,n);

                        Test( &S == &T );
                        Test( S.length() == NewLength );
                        Test( &S.data()[       0       ] , &String1[           0 ] , ReplaceAt      ); // Test first part.
                        Test( &S.data()[     ReplaceAt ] , &String2[         pos ] , ReplaceLength  ); // Test second part.
                        Test( &S.data()[ LeftoverIsNow ] , &String1[ LeftoverWas ] , LeftoverLength ); // Test third part.
                        Test( S.data()[NewLength] == 0 ); // Test for null terminator. 
                    }
                }
            }
        }
    }

    // cstring & replace( iterator first0 , iterator last0 , const cstring & str );
    // cstring & replace(iterator first0 , iterator last0 , const char *s );
    if( true ) 
    {
        const cstring str = String2 ;
        CIntegerSet pos1Values( 8 , 0 , Length1 );
        // ----- pos1
        for( int Whichpos1 = 0 ; Whichpos1 < pos1Values.Count() ; ++Whichpos1 )
        {
            const int pos1 = pos1Values[Whichpos1];
            CIntegerSet pos2Values( 8 , 0 , Length1 );
            // ----- pos2
            for( int Whichpos2 = 0 ; Whichpos2 < pos2Values.Count() ; ++Whichpos2 )
            {
                const int pos2 = pos2Values[Whichpos2];

                const int ReplaceAt         = pos1                                  ;
                const int EraseLength       = Max( 0 , pos2-pos1 )                  ;
                const int ReplaceLength     = Length2                               ;
                const int LeftoverWas       = ReplaceAt + EraseLength               ;
                const int LeftoverLength    = Length1 - ReplaceAt - EraseLength     ;
                const int LeftoverIsNow     = ReplaceAt + ReplaceLength             ;
                const int NewLength         = Length1 - EraseLength + ReplaceLength ;

                for( int WhichKind = 1 ; WhichKind <= 2 ; WhichKind++ )
                {
                    cstring S = String1 ;
                    cstring::iterator first0 = S.begin() + pos1 ;
                    cstring::iterator last0  = S.begin() + pos2 ;
                    cstring & T = WhichKind==1 ? S.replace( first0 , last0 , str ) : S.replace( first0 , last0 , String2 );
                    Test( &T == &S );

                    Test( S.length() == NewLength );
                    Test( &S.data()[             0 ] , &String1[           0 ] , ReplaceAt      ); // Test first part.
                    Test( &S.data()[     ReplaceAt ] , &String2[           0 ] , ReplaceLength  ); // Test second part.
                    Test( &S.data()[ LeftoverIsNow ] , &String1[ LeftoverWas ] , LeftoverLength ); // Test third part.
                    Test( S.data()[NewLength] == 0 ); // Test for null terminator. 
                }
            }
        }
    }

    // cstring & replace( iterator first0 , iterator last0 , const char * s , int n);
    if( true ) 
    {
        CIntegerSet pos1Values( 8 , 0 , Length1 );
        // ----- pos1
        for( int Whichpos1 = 0 ; Whichpos1 < pos1Values.Count() ; ++Whichpos1 )
        {
            const int pos1 = pos1Values[Whichpos1];
            CIntegerSet pos2Values( 8 , 0 , Length1 );
            // ----- pos2
            for( int Whichpos2 = 0 ; Whichpos2 < pos2Values.Count() ; ++Whichpos2 )
            {
                const int pos2 = pos2Values[Whichpos2];
                CIntegerSet nValues( 8 , 0 , Length2+1 );
                // ----- n
                for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                {
                    const int n = nValues[Whichn];

                    const int ReplaceAt         = pos1                                  ;
                    const int EraseLength       = Max( 0 , pos2-pos1 )                  ;
                    const int ReplaceLength     = Max( 0 , Min(n,Length2) )             ;
                    const int LeftoverWas       = ReplaceAt + EraseLength               ;
                    const int LeftoverLength    = Length1 - ReplaceAt - EraseLength     ;
                    const int LeftoverIsNow     = ReplaceAt + ReplaceLength             ;
                    const int NewLength         = Length1 - EraseLength + ReplaceLength ;

                    cstring S = String1 ;
                    cstring::iterator first0 = S.begin() + pos1 ;
                    cstring::iterator last0  = S.begin() + pos2 ;
                    cstring & T = S.replace( first0 , last0 , String2 , n );
                    Test( &T == &S );

                    Test( S.length() == NewLength );
                    Test( &S.data()[             0 ] , &String1[           0 ] , ReplaceAt      ); // Test first part.
                    Test( &S.data()[     ReplaceAt ] , &String2[           0 ] , ReplaceLength  ); // Test second part.
                    Test( &S.data()[ LeftoverIsNow ] , &String1[ LeftoverWas ] , LeftoverLength ); // Test third part.
                    Test( S.data()[NewLength] == 0 ); // Test for null terminator. 
                }
            }
        }
    }

    // cstring & replace(iterator first0 , iterator last0 , int n , char c );
    if( true ) 
    {
        char c = '\xfe';
        CIntegerSet pos1Values( 8 , 0 , Length1 );
        // ----- pos1
        for( int Whichpos1 = 0 ; Whichpos1 < pos1Values.Count() ; ++Whichpos1 )
        {
            const int pos1 = pos1Values[Whichpos1];
            CIntegerSet pos2Values( 8 , 0 , Length1 );
            // ----- pos2
            for( int Whichpos2 = 0 ; Whichpos2 < pos2Values.Count() ; ++Whichpos2 )
            {
                const int pos2 = pos2Values[Whichpos2];
                CIntegerSet nValues( 8 , 0 , 71 );
                // ----- n
                for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                {
                    const int n = nValues[Whichn];

                    const int ReplaceAt         = pos1                                  ;
                    const int EraseLength       = Max( 0 , pos2-pos1 )                  ;
                    const int ReplaceLength     = Max( 0 , n )                          ;
                    const int LeftoverWas       = ReplaceAt + EraseLength               ;
                    const int LeftoverLength    = Length1 - ReplaceAt - EraseLength     ;
                    const int LeftoverIsNow     = ReplaceAt + ReplaceLength             ;
                    const int NewLength         = Length1 - EraseLength + ReplaceLength ;

                    cstring S = String1 ;
                    cstring::iterator first0 = S.begin() + pos1 ;
                    cstring::iterator last0  = S.begin() + pos2 ;
                    cstring & T = S.replace( first0 , last0 , n , c );
                    Test( &T == &S );

                    Test( S.length() == NewLength );
                    Test( &S.data()[             0 ] , &String1[           0 ] , ReplaceAt      ); // Test first part.
                    for( int Check = 0 ; Check < ReplaceLength ; ++Check )
                    {
                        Test( S[Check+ReplaceAt] == c ); // Test second part.
                    }
                    Test( &S.data()[ LeftoverIsNow ] , &String1[ LeftoverWas ] , LeftoverLength ); // Test third part.
                    Test( S.data()[NewLength] == 0 ); // Test for null terminator. 

                    c = c == '\xff' ? '\x01' : char(c+1) ; // Use next character, but skip 0.
                }
            }
        }
    }

    // cstring & replace(iterator first0 , iterator last0 , const_iterator first , const_iterator last);
    if( true ) 
    {
        const cstring str = String2;
        CIntegerSet pos1Values( 8 , 0 , Length1 );
        // ----- pos1
        for( int Whichpos1 = 0 ; Whichpos1 < pos1Values.Count() ; ++Whichpos1 )
        {
            const int pos1 = pos1Values[Whichpos1];
            CIntegerSet pos2Values( 8 , 0 , Length1 );
            // ----- pos2
            for( int Whichpos2 = 0 ; Whichpos2 < pos2Values.Count() ; ++Whichpos2 )
            {
                const int pos2 = pos2Values[Whichpos2];
                CIntegerSet pos3Values( 8 , 0 , Length2 );
                // ----- pos3
                for( int Whichpos3 = 0 ; Whichpos3 < pos3Values.Count() ; ++Whichpos3 )
                {
                    const int pos3 = pos3Values[Whichpos3];
                    CIntegerSet pos4Values( 8 , 0 , Length2 );
                    // ----- pos4
                    for( int Whichpos4 = 0 ; Whichpos4 < pos4Values.Count() ; ++Whichpos4 )
                    {
                        const int pos4 = pos4Values[Whichpos4];

                        const int ReplaceAt         = pos1                                  ;
                        const int EraseLength       = Max( 0 , pos2-pos1 )                  ;
                        const int ReplaceLength     = Max( 0 , pos4-pos3 )                  ;
                        const int LeftoverWas       = ReplaceAt + EraseLength               ;
                        const int LeftoverLength    = Length1 - ReplaceAt - EraseLength     ;
                        const int LeftoverIsNow     = ReplaceAt + ReplaceLength             ;
                        const int NewLength         = Length1 - EraseLength + ReplaceLength ;

                        cstring S = String1 ;
                        cstring::iterator       first0 = S.begin()   + pos1 ;
                        cstring::iterator       last0  = S.begin()   + pos2 ;
                        cstring::const_iterator first  = str.begin() + pos3 ;
                        cstring::const_iterator last   = str.begin() + pos4 ;
                        cstring & T = S.replace( first0 , last0 , first , last );
                        Test( &T == &S );

                        Test( S.length() == NewLength );
                        Test( &S.data()[             0 ] , &String1[           0 ] , ReplaceAt      ); // Test first part.
                        Test( &S.data()[     ReplaceAt ] , &String2[        pos3 ] , ReplaceLength  ); // Test second part.
                        Test( &S.data()[ LeftoverIsNow ] , &String1[ LeftoverWas ] , LeftoverLength ); // Test third part.
                        Test( S.data()[NewLength] == 0 ); // Test for null terminator. 
                    }
                }
            }
        }
    }

}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring copy()
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestCopy( const char * String )
{
    const int Length = String==0 ? 0 : strlen(String);
    char * Target = new char[Length+1];

    // int copy( char * s , int n , int pos = 0 ) const; // Note: Does not add a null terminator.
    {
        cstring S = String ;
        for( int pos = 0 ; pos <= Length ; pos++ )
        {
            for( int n = -1 ; n <= Length-pos+2 ; n++ )
            {
                const int CopyLength = Max( 0 , Min( n , Length-pos ) );
                memset( Target , '\xff' , Length );
                Target[CopyLength] = '\xa5' ; // This should not be changed by the copy.

                const int Copied = S.copy( Target , n , pos );
                Test( Copied == CopyLength );
                Test( Target[CopyLength] == '\xa5' ); // Test sentinel value.
                Test( Target , String , CopyLength );
            }
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring swap()
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestSwap( const char * String1 , const char * String2 )
{
    // void swap( cstring & str );

    const int Length1 = String1==0 ? 0 : strlen(String1);
    const int Length2 = String2==0 ? 0 : strlen(String2);
    cstring S1 = String1 ;
    cstring S2 = String2 ;
    const char * OldS1Data = S1.data();
    const char * OldS2Data = S2.data();

    S1.swap(S2);
    Test( S1.data() , String2 , Length2 ); // Check swap
    Test( S2.data() , String1 , Length1 ); // Check swap
    Test( OldS1Data == S2.data() );
    Test( OldS2Data == S1.data() );
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring find and rfind
//------------------------------------------------------------------------
// Use the given test strings wherever applicable.
#if ENABLE_SELF_TEST
static void TestFind( const char * String1 , const char * String2 )
{
    const int Length1 = String1==0 ? 0 : strlen(String1);
    const int Length2 = String2==0 ? 0 : strlen(String2);
    char * Copy2 = new char[Length2+1]; // Copy of String2, in case we need it.

    const cstring S   = String1 ;

    // int find( const cstring & str , int pos = 0 ) const; 
    // int find( const char * s , int pos = 0 ) const;
    {
        const cstring str = String2 ;
        for( int pos = 0 ; pos <= Length1 ; pos++ )
        {
            for( int WhichKind = 1 ; WhichKind <= 2 ; ++WhichKind )
            {
                const char * FoundAt  = 
                    pos >= Length1 || Length1 <= 0 || Length2 <= 0 
                ?   0 
                :   strstr( &S.data()[pos] , String2 )
                ;
                const int    Expected = FoundAt == 0 ? cstring::npos : FoundAt - S.data() ;
                const int    Actual   = WhichKind==1 ? S.find(str,pos) : S.find(String2,pos);
                Test( Actual == Expected );
            }
        }
    }

    // int rfind( const cstring & str , int pos = npos ) const;
    {
        const cstring str = String2 ;
        for( int pos = -1 ; pos <= Length1 ; pos++ )
        {
            const int    StartIndex = pos == cstring::npos ? Length1-1 : Min( pos , Length1-1 ); // Index of where the search should start.
            const char * StartAt    = &S.data()[StartIndex]; 
            const int    Actual      = S.rfind( str , pos );
            Test( Actual == cstring::npos || Actual <= StartIndex );
            if( Actual == cstring::npos )
            {
                const char * FoundAt = Length2 == 0 ? 0 : strstr( S.data() , str.data() );
                Test( FoundAt == 0 || FoundAt > StartAt );
            }
            else
            {   
                const char * FoundAgainAt = strstr( &S.data()[Actual+1] , str.data() );
                Test( S.compare(  Actual , Length2 , String2 ) == 0 );
                Test( FoundAgainAt == 0 || FoundAgainAt > StartAt );

            }
                    
        }
    }


    // int find( const char * s , int pos , int n ) const;
    {
        for( int pos = 0 ; pos <= Length1 ; pos++ )
        {
            for( int n = -1 ; n <= Length2+1 ; n++ )
            {
                const char * FoundAt  ;
                if( n <= 0 || Length2 <= 0 )
                {
                    FoundAt = 0;
                }
                else
                {
                    // Create a copy of String2, and move the termination to [n] so we can use strstr()
                    strcpy( Copy2 , String2 );
                    Copy2[ Min( Length2 , n ) ] = 0;
                    FoundAt = strstr( &S.data()[pos] , Copy2 );
                }
                const int    Expected = FoundAt == 0 ? cstring::npos : FoundAt - S.data() ;
                const int    Actual   = S.find( String2 , pos , n );
                Test( Actual == Expected );
            }
        }
    }

    // int find( char c , int pos = 0 ) const;
    {
        char c = '\x09';
        for( int pos = 0 ; pos <= Length1 ; pos++ )
        {
            const char * FoundAt  = pos >= Length1 
            ?   0 
            :  strchr( &S.data()[pos] , c )
            ;
            const int    Expected = FoundAt == 0 ? cstring::npos : FoundAt - S.data() ;
            const int    Actual   = S.find( c , pos );
            Test( Actual == Expected );
                    
            c = c == '\xff' ? '\x01' : char(c+1); // Next character, skipping null.
        }
    }

    // int rfind( const char * s , int pos , int n ) const;
    {
        for( int pos = 0 ; pos <= Length1 ; pos++ )
        {
            for( int n = -1 ; n <= Length2+1 ; n++ )
            {
                const int Actual = S.rfind( String2 , pos , n );
                if( Length2 <= 0 || n <= 0 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    int FoundAt = S.find( String2 , pos , n );
                    Test( FoundAt == cstring::npos || FoundAt > pos );
                }
                else
                {
                    Test( &S.c_str()[Actual] , String2 , Max( 0 , Min(n,Length2) ) );
                    const int FoundNextAt = S.find(String2,pos+1,n);
                    Test(  FoundNextAt == cstring::npos || FoundNextAt > pos );
                }
            }
        }
    }

    // int rfind( const char * s , int pos = npos ) const;
    {
        for( int pos = 0 ; pos <= Length1 ; pos++ )
        {
            const int Actual = S.rfind( String2 , pos );
            if( Length2 <= 0 )
            {
                Test( Actual == cstring::npos );
            }
            else if( Actual == cstring::npos )
            {
                int FoundAt = S.find( String2 , pos );
                Test( FoundAt == cstring::npos || FoundAt > pos );
            }
            else
            {
                Test( &S.c_str()[Actual] , String2 , Length2 );
                const int FoundNextAt = S.find(String2,pos+1);
                Test(  FoundNextAt == cstring::npos || FoundNextAt > pos );
            }
        }
    }

    // int rfind( char c , int pos = npos ) const;
    {
        char c = '\x09';
        for( int pos = 0 ; pos <= Length1 ; pos++ )
        {
            const int Actual = S.rfind( c , pos );
            if( Actual == cstring::npos )
            {
                const char * FoundFirstAt = strchr( &S.data()[0] , c );
                Test( FoundFirstAt == 0 || FoundFirstAt > &S.data()[pos] );
            }
            else
            {
                Test( S[Actual] == c );
                const char * FoundNextAt = strchr( &S.data()[Actual+1] , c );
                Test( FoundNextAt == 0 || FoundNextAt > &S.data()[pos] );
            }
                    
            c = c == '\xff' ? '\x01' : char(c+1); // Next character, skipping null.
        }
    }

    delete[] Copy2;
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring find_first_of and find_last_of
//------------------------------------------------------------------------
// Use the given test strings wherever applicable.
#if ENABLE_SELF_TEST
static void TestFindFirstLastOf( const char * String1 , const char * String2 )
{
    const int Length1 = String1==0 ? 0 : strlen(String1);
    const int Length2 = String2==0 ? 0 : strlen(String2);

    const cstring S   = String1 ;

    // int find_first_of( const char * s , int pos = 0 ) const;
    // int find_first_of( const cstring & str , int pos = 0 ) const;
    // int find_first_of( const char * s , int pos , int n ) const;
    // int find_first_of( char c , int pos = 0 ) const 
    {
        char c = '\x09';
        const cstring str = String2 ;
        for( int pos = 0 ; pos <= Length1+1 ; pos++ )
        {
            for( int WhichKind = 1 ; WhichKind <= 2 ; ++WhichKind )
            {
                const int Actual = WhichKind==1 ? S.find_first_of(str,pos) : S.find_first_of(String2,pos);
                if( Length1 <= 0 || Length2 <= 0 || pos >= Length1 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is not in String2.
                    for( int WhichChar = pos ; WhichChar < Length1 ; WhichChar++ )
                    {
                        Test( strchr(String2,S[WhichChar]) == 0 );
                    }
                }
                else
                {
                    Test( strchr( String2 , S[Actual] ) != 0 );
                    // Check each previous character in S and make sure it is not in String2.
                    for( int WhichPriorChar = pos ; WhichPriorChar < Actual ; WhichPriorChar++ )
                    {
                        Test( strchr(String2,S[WhichPriorChar]) == 0 );
                    }
                }
            }
            // int find_first_of( const char * s , int pos , int n ) const;
            for( int n = -1 ; n <= Length2+1 ; n++ )
            {
                const int Actual = S.find_first_of( String2 , pos , n );
                const int LastCheck2 = Min( n-1 , Length2-1 );
                if( Length1 <= 0 || Length2 <= 0 || pos >= Length1 || n <= 0 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is not in String2.
                    for( int WhichChar = pos ; WhichChar < Length1 ; WhichChar++ )
                    {
                        const char * FoundAt = strchr(String2,S[WhichChar]);
                        Test( FoundAt == 0 || FoundAt > &String2[LastCheck2] );
                    }
                }
                else
                {
                    const char * FoundAt = strchr( String2 , S[Actual] );
                    Test( FoundAt != 0 && FoundAt <= &String2[LastCheck2] );
                    // Check each previous character in S and make sure it is not in String2.
                    for( int WhichPriorChar = pos ; WhichPriorChar < Actual ; WhichPriorChar++ )
                    {
                        const char * FoundAt = strchr(String2,S[WhichPriorChar]);
                        Test( FoundAt == 0 || FoundAt > &String2[LastCheck2] );
                    }
                }
            }
            // int find_first_of( char c , int pos = 0 ) const 
            {
                const int Actual = S.find_first_of(c,pos);
                if( Length1 <= 0 || pos >= Length1 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is not c.
                    for( int WhichChar = pos ; WhichChar < Actual ; WhichChar++ )
                    {
                        Test( S[WhichChar] != c );
                    }
                }
                else
                {
                    Test( S[Actual] == c );
                    // Check each previous character in S and make sure it is not c.
                    for( int WhichPriorChar = pos ; WhichPriorChar < Actual ; WhichPriorChar++ )
                    {
                        Test( S[WhichPriorChar] != c );
                    }
                }
            }
            c = c == '\xff' ? '\x01' : char(c+1); // Next character, skipping null.
        }
    }

    // int find_last_of( const cstring & str , int pos = npos ) const;
    // int find_last_of( const char * s , int pos = npos ) const;
    // int find_last_of( const char * s , int pos ,  int n ) const;
    // int find_last_of( char c , int pos = npos ) const { return rfind(c,pos); }
    {
        char c = '\x09';
        const cstring str = String2 ;
        for( int pos = 0 ; pos <= Length1+1 ; pos++ )
        {
            const int LastPos = Min( pos , Length1-1 );
            for( int WhichKind = 1 ; WhichKind <= 2 ; ++WhichKind )
            {
                const int Actual = WhichKind==1 ? S.find_last_of(str,pos) : S.find_last_of(String2,pos);
                if( Length1 <= 0 || Length2 <= 0  )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is not in String2.
                    for( int WhichChar = 0 ; WhichChar <= LastPos ; WhichChar++ )
                    {
                        Test( strchr(String2,S[WhichChar]) == 0 );
                    }
                }
                else
                {
                    Test( strchr( String2 , S[Actual] ) != 0 );
                    // Check each later character in S and make sure it is not in String2.
                    for( int WhichLaterChar = Actual+1 ; WhichLaterChar <= LastPos ; WhichLaterChar++ )
                    {
                        Test( strchr(String2,S[WhichLaterChar]) == 0 );
                    }
                }
            }
            // int find_last_of( const char * s , int pos , int n ) const;
            for( int n = -1 ; n <= Length2+1 ; n++ )
            {
                const int Actual = S.find_last_of( String2 , pos , n );
                const int LastCheck2 = Min( n-1 , Length2-1 );
                if( Length1 <= 0 || Length2 <= 0 || n <= 0 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is not in String2.
                    for( int WhichChar = 0 ; WhichChar <= LastPos ; WhichChar++ )
                    {
                        const char * FoundAt = strchr(String2,S[WhichChar]);
                        Test( FoundAt == 0 || FoundAt > &String2[LastCheck2] );
                    }
                }
                else
                {
                    const char * FoundAt = strchr( String2 , S[Actual] );
                    Test( FoundAt != 0 && FoundAt <= &String2[LastCheck2] );
                    // Check each later character in S and make sure it is not in String2.
                    for( int WhichLaterChar = Actual+1; WhichLaterChar < LastPos ; WhichLaterChar++ )
                    {
                        const char * FoundAt = strchr(String2,S[WhichLaterChar]);
                        Test( FoundAt == 0 || FoundAt > &String2[LastCheck2] );
                    }
                }
            }
            // int find_last_of( char c , int pos = 0 ) const 
            {
                const int Actual = S.find_last_of(c,pos);
                if( Length1 <= 0 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is not c.
                    for( int WhichChar = 0 ; WhichChar <= LastPos ; WhichChar++ )
                    {
                        Test( S[WhichChar] != c );
                    }
                }
                else
                {
                    Test( S[Actual] == c );
                    // Check each later character in S and make sure it is not c.
                    for( int WhichLaterChar = Actual+1 ; WhichLaterChar <= LastPos ; WhichLaterChar++ )
                    {
                        Test( S[WhichLaterChar] != c );
                    }
                }
            }
            c = c == '\xff' ? '\x01' : char(c+1); // Next character, skipping null.
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring find_first_not_of and find_last_not_of
//------------------------------------------------------------------------
// Use the given test strings wherever applicable.
#if ENABLE_SELF_TEST
static void TestFindFirstLastNotOf( const char * String1 , const char * String2 )
{
    const int Length1 = String1==0 ? 0 : strlen(String1);
    const int Length2 = String2==0 ? 0 : strlen(String2);

    const cstring S   = String1 ;

    // int find_first_not_of( const cstring & str , int pos = 0 ) const;
    // int find_first_not_of( const char * s , int pos = 0 ) const;
    // int find_first_not_of( const char * s , int pos , int n ) const;
    // int find_first_not_of( char c , int pos = 0 ) const;
    {
        char c = '\x09';
        const cstring str = String2 ;
        for( int pos = 0 ; pos <= Length1+1 ; pos++ )
        {
            for( int WhichKind = 1 ; WhichKind <= 2 ; ++WhichKind )
            {
                const int Actual = WhichKind==1 ? S.find_first_not_of(str,pos) : S.find_first_not_of(String2,pos);
                if( Length1 <= 0 || pos >= Length1 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Length2 <= 0 )
                {
                    Test( Actual == pos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is also in String2.
                    for( int WhichChar = pos ; WhichChar < Length1 ; WhichChar++ )
                    {
                        Test( strchr(String2,S[WhichChar]) != 0 );
                    }
                }
                else
                {
                    Test( strchr( String2 , S[Actual] ) == 0 );
                    // Check each previous character in S and make sure it is in String2.
                    for( int WhichPriorChar = pos ; WhichPriorChar < Actual ; WhichPriorChar++ )
                    {
                        Test( strchr(String2,S[WhichPriorChar]) != 0 );
                    }
                }
            }
            // int find_first_not_of( const char * s , int pos , int n ) const;
            for( int n = -1 ; n <= Length2+1 ; n++ )
            {
                const int Actual = S.find_first_not_of( String2 , pos , n );
                const int LastCheck2 = Min( n-1 , Length2-1 );
                if( Length1 <= 0 || pos >= Length1 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( n <= 0 || Length2 <= 0 )
                {
                    Test( Actual == pos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is also in String2.
                    for( int WhichChar = pos ; WhichChar < Length1 ; WhichChar++ )
                    {
                        const char * FoundAt = strchr(String2,S[WhichChar]);
                        Test( FoundAt != 0 && FoundAt <= &String2[LastCheck2] );
                    }
                }
                else
                {
                    const char * FoundAt = strchr( String2 , S[Actual] );
                    Test( FoundAt == 0 || FoundAt > &String2[LastCheck2] );
                    // Check each previous character in S and make sure it is in String2.
                    for( int WhichPriorChar = pos ; WhichPriorChar < Actual ; WhichPriorChar++ )
                    {
                        const char * FoundAt = strchr(String2,S[WhichPriorChar]);
                        Test( FoundAt != 0 && FoundAt <= &String2[LastCheck2] );
                    }
                }
            }
            // int find_first_not_of( char c , int pos = 0 ) const;
            {
                const int Actual = S.find_first_not_of(c,pos);
                if( Length1 <= 0 || pos >= Length1 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is c.
                    for( int WhichChar = pos ; WhichChar < Actual ; WhichChar++ )
                    {
                        Test( S[WhichChar] == c );
                    }
                }
                else
                {
                    Test( S[Actual] != c );
                    // Check each previous character in S and make sure it is c.
                    for( int WhichPriorChar = pos ; WhichPriorChar < Actual ; WhichPriorChar++ )
                    {
                        Test( S[WhichPriorChar] == c );
                    }
                }
            }
            c = c == '\xff' ? '\x01' : char(c+1); // Next character, skipping null.
        }
    }


    // int find_last_not_of( const cstring & str , int pos = npos ) const;
    // int find_last_not_of( const char * s , int pos , int n ) const;
    // int find_last_not_of( const char * s , int pos = npos ) const;
    // int find_last_not_of( char c , int pos = npos ) const;
    {
        char c = '\x09';
        const cstring str = String2 ;
        for( int pos = 0 ; pos <= Length1+1 ; pos++ )
        {
            const int LastPos = Min( pos , Length1-1 );
            for( int WhichKind = 1 ; WhichKind <= 2 ; ++WhichKind )
            {
                const int Actual = WhichKind==1 ? S.find_last_not_of(str,pos) : S.find_last_not_of(String2,pos);
                if( Length1 <= 0 || Length2 <= 0  )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is in String2.
                    for( int WhichChar = 0 ; WhichChar <= LastPos ; WhichChar++ )
                    {
                        Test( strchr(String2,S[WhichChar]) != 0 );
                    }
                }
                else
                {
                    Test( strchr( String2 , S[Actual] ) == 0 );
                    // Check each later character in S and make sure it is in String2.
                    for( int WhichLaterChar = Actual+1 ; WhichLaterChar <= LastPos ; WhichLaterChar++ )
                    {
                        Test( strchr(String2,S[WhichLaterChar]) != 0 );
                    }
                }
            }
            // int find_last_not_of( const char * s , int pos , int n ) const;
            for( int n = -1 ; n <= Length2+1 ; n++ )
            {
                const int Actual = S.find_last_not_of( String2 , pos , n );
                const int LastCheck2 = Min( n-1 , Length2-1 );
                if( Length1 <= 0 || Length2 <= 0 || n <= 0 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is in String2.
                    for( int WhichChar = 0 ; WhichChar <= LastPos ; WhichChar++ )
                    {
                        const char * FoundAt = strchr(String2,S[WhichChar]);
                        Test( FoundAt != 0 && FoundAt <= &String2[LastCheck2] );
                    }
                }
                else
                {
                    const char * FoundAt = strchr( String2 , S[Actual] );
                    Test( FoundAt == 0 || FoundAt > &String2[LastCheck2] );
                    // Check each later character in S and make sure it is in String2.
                    for( int WhichLaterChar = Actual+1; WhichLaterChar < LastPos ; WhichLaterChar++ )
                    {
                        const char * FoundAt = strchr(String2,S[WhichLaterChar]);
                        Test( FoundAt != 0 && FoundAt <= &String2[LastCheck2] );
                    }
                }
            }
            // int find_last_not_of( char c , int pos = 0 ) const 
            {
                const int Actual = S.find_last_not_of(c,pos);
                if( Length1 <= 0 )
                {
                    Test( Actual == cstring::npos );
                }
                else if( Actual == cstring::npos )
                {
                    // Check each character in S and make sure it is c.
                    for( int WhichChar = 0 ; WhichChar <= LastPos ; WhichChar++ )
                    {
                        Test( S[WhichChar] == c );
                    }
                }
                else
                {
                    Test( S[Actual] != c );
                    // Check each later character in S and make sure it is c.
                    for( int WhichLaterChar = Actual+1 ; WhichLaterChar <= LastPos ; WhichLaterChar++ )
                    {
                        Test( S[WhichLaterChar] == c );
                    }
                }
            }
            c = c == '\xff' ? '\x01' : char(c+1); // Next character, skipping null.
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring substr()
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestSubstr( const char * String )
{
    const int Length = String == 0 ? 0 : strlen(String);
    cstring S = String;
    // cstring substr( int pos = 0 , int n = npos ) const;
    for( int pos = 0 ; pos < Length ; pos++ )
    {
        // Test: substr(pos)
        const cstring A = S.substr( pos );
        TestContents( A , ( String==0 ? 0 : &String[pos] ) );

        for( int n = 0 ; n <= Length ; n++ )
        {
            // Test: substr(pos,n)
            const int ExpectedLength = FitToRange( Length - pos , 0 , n );
            const cstring B = S.substr(pos,n);
            Test( B.length() == ExpectedLength );
            Test( B.c_str() , &S[pos] , ExpectedLength );
        }
    }
    // Test: substr()
    const cstring C = S.substr();
    TestContents( C , S );
}
#endif

//------------------------------------------------------------------------
// Debug: Find the sign of an integer
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static inline int Sign(int x) { return x < 0 ? -1 : x > 0 ? +1 : 0 ; }
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring comparison
//------------------------------------------------------------------------
// Use the given test strings wherever applicable.
#if ENABLE_SELF_TEST
static void TestComparison( const char * String1 , const char * String2 )
{
    const int       Length1 = String1==0 ? 0 : strlen(String1);
    const int       Length2 = String2==0 ? 0 : strlen(String2);
    const char *    Text1   = String1==0 ? "" : String1 ;
    const char *    Text2   = String2==0 ? "" : String2 ;

    const cstring S   = String1  ;
    const cstring str = String2 ;

    // int compare( const cstring & str ) const;
    {
        const int Expected = strcmp(Text1,Text2) ;
        const int Actual   = Sign(S.compare(str));
        Test( Sign(Expected) == Sign(Actual) );
    }

    // int compare( const char * s ) const;
    {
        const int Expected = strcmp(Text1,Text2) ;
        const int Actual   = Sign(S.compare(String2));
        Test( Sign(Expected) == Sign(Actual) );
    }

    // int compare( int p0 , int n0 , const cstring & str , int pos , int n );
    if( true )
    {
        // p0
        CIntegerSet p0Values( 11 , 0 , Length1 );
        for( int Whichp0 = 0 ; Whichp0 < p0Values.Count() ; ++Whichp0 )
        {
            const int p0            = p0Values[Whichp0];
            // n0
            CIntegerSet n0Values( 11 , 0 , Length1-p0+1 );
            for( int Whichn0 = 0 ; Whichn0 < n0Values.Count() ; ++Whichn0 )
            {
                const int n0            = n0Values[Whichn0];
                // pos
                CIntegerSet posValues( 11 , 0 , Length2 );
                for( int Whichpos = 0 ; Whichpos < posValues.Count() ; ++Whichpos )
                {
                    const int pos            = posValues[Whichpos];
                    // n
                    CIntegerSet nValues( 11 , 0 , Length2-pos+1 );
                    for( int Whichn = 0 ; Whichn < nValues.Count() ; ++Whichn )
                    {
                        const int n            = nValues[Whichn];
                        const int CompareLength1 = Max( 0 , Min( n0 , Length1-p0  ) );
                        const int CompareLength2 = Max( 0 , Min( n  , Length2-pos ) );
                        const int PreCompare = 
                            CompareLength1 == 0             ?   ( CompareLength2 > 0 ? -1 : 0 )
                        :   CompareLength2 == 0             ?   ( CompareLength1 > 0 ? +1 : 0 )
                        :                                       memcmp( &String1[p0] , &String2[pos] , Min(CompareLength1,CompareLength2) )
                        ;
                        const int Expected = 
                            PreCompare != 0                 ?   PreCompare
                        :   CompareLength1 > CompareLength2 ?   +1      // Longer lhs?
                        :   CompareLength1 < CompareLength2 ?   -1      // Longer rhs?
                        :                                        0
                        ;
                        const int Actual = S.compare(p0,n0,str,pos,n);
                        Test( Sign(Actual) == Sign(Expected) );
                    }
                }
            }
        }
    }

    // int compare( int p0 , int n0 , const cstring & str );
    // int compare( int p0 , int n0 , const char * s ) const;
    if( true )
    {
        // p0
        CIntegerSet p0Values( 11 , 0 , Length1 );
        for( int Whichp0 = 0 ; Whichp0 < p0Values.Count() ; ++Whichp0 )
        {
            const int p0            = p0Values[Whichp0];
            // n0
            CIntegerSet n0Values( 11 , 0 , Length1-p0+1 );
            for( int Whichn0 = 0 ; Whichn0 < n0Values.Count() ; ++Whichn0 )
            {
                const int n0            = n0Values[Whichn0];
                const int CompareLength1 = Max( 0 , Min( n0 , Length1-p0  ) );
                const int CompareLength2 = Length2                           ;
                const int PreCompare = 
                    CompareLength1 == 0             ?   ( CompareLength2 > 0 ? -1 : 0 )
                :   CompareLength2 == 0             ?   ( CompareLength1 > 0 ? +1 : 0 )
                :                                       memcmp( &String1[p0] , &String2[0] , Min(CompareLength1,CompareLength2) )
                ;
                const int Expected = 
                    PreCompare != 0                 ?   PreCompare
                :   CompareLength1 > CompareLength2 ?   +1      // Longer lhs?
                :   CompareLength1 < CompareLength2 ?   -1      // Longer rhs?
                :                                        0
                ;
                for( int WhichKind = 1 ; WhichKind <= 2 ; WhichKind++ )
                {
                    const int Actual = WhichKind==1 ? S.compare(p0,n0,str) : S.compare(p0,n0,String2);
                    Test( Sign(Actual) == Sign(Expected) );
                }
            }
        }
    }
    // int compare( int p0 , int n0 , const char * s , int pos ) const;
    if( true )
    {
        // p0
        CIntegerSet p0Values( 11 , 0 , Length1 );
        for( int Whichp0 = 0 ; Whichp0 < p0Values.Count() ; ++Whichp0 )
        {
            const int p0            = p0Values[Whichp0];
            // n0
            CIntegerSet n0Values( 11 , 0 , Length1-p0+1 );
            for( int Whichn0 = 0 ; Whichn0 < n0Values.Count() ; ++Whichn0 )
            {
                const int n0            = n0Values[Whichn0];
                // pos
                CIntegerSet posValues( 11 , 0 , Length2 );
                for( int Whichpos = 0 ; Whichpos < posValues.Count() ; ++Whichpos )
                {
                    const int pos            = posValues[Whichpos];
                    const int CompareLength1 = Max( 0 , Min( n0 , Length1-p0  ) );
                    const int CompareLength2 = Max( 0 , Length2-pos             );
                    const int PreCompare = 
                        CompareLength1 == 0             ?   ( CompareLength2 > 0 ? -1 : 0 )
                    :   CompareLength2 == 0             ?   ( CompareLength1 > 0 ? +1 : 0 )
                    :                                       memcmp( &String1[p0] , &String2[pos] , Min(CompareLength1,CompareLength2) )
                    ;
                    const int Expected = 
                        PreCompare != 0                 ?   PreCompare
                    :   CompareLength1 > CompareLength2 ?   +1      // Longer lhs?
                    :   CompareLength1 < CompareLength2 ?   -1      // Longer rhs?
                    :                                        0
                    ;
                    const int Actual = S.compare(p0,n0,String2,pos);
                    Test( Sign(Actual) == Sign(Expected) );
                }
            }
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring to_upper() and to_lower()
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestUpperLower( const char * String )
{
    const int Length = String == 0 ? 0 : strlen(String);
    cstring upper = String ;
    cstring lower = String ;

    upper.to_upper();
    lower.to_lower();

    Test( upper.length() == Length );
    Test( lower.length() == Length );
    for( int pos = 0 ; pos < Length ; pos++ )
    {
        const char C        = String[pos];
        const char UpperC   = char( IsInRange(C,'a','z') ? (int(C)-'a'+'A') : C );
        const char LowerC   = char( IsInRange(C,'A','Z') ? (int(C)-'A'+'a') : C );
        Test( upper[pos] == UpperC );
        Test( lower[pos] == LowerC );
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring trim functions
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestTrim( const char * String1 , const char * String2 )
{
    const int Length1 = String1==0 ? 0 : strlen(String1);
    const int Length2 = String2==0 ? 0 : strlen(String2);

    // void trim()
    {
        cstring S = String1 ;
        S.trim();
        cstring Expected = String1 ;
        Expected.trim( "\t \r\n " );
        TestContents( S , Expected );
    }

    // void trim_left()
    {
        cstring S = String1 ;
        S.trim_left();
        cstring Expected = String1 ;
        Expected.trim_left( "\r  \n\n\t \r\n " );
        TestContents( S , Expected );
    }

    // void trim_right()
    {
        cstring S = String2 ;
        S.trim_right();
        cstring Expected = String2 ;
        Expected.trim_right( " \r \r \t \r \n " );
        TestContents( S , Expected );
    }

    // void trim_left( int n )
    {
        for( int n = -1 ; n <= Length1+2 ; n++ )
        {
            const int ExpectedLength = Max( 0 , Min( Length1 , Length1 - n ) );
            cstring S = String1 ;
            S.trim_left(n);
            Test( S.length() == ExpectedLength );
            Test( S.c_str() , &String1[ Max(n,0) ] , ExpectedLength );
        }
    }
    // void trim_right ( int n )
    {
        for( int n = -1 ; n <= Length2+1 ; n++ )
        {
            const int ExpectedLength = Max( 0 , Min( Length2 , Length2 - n ) );
            cstring S = String2 ;
            S.trim_right(n);
            Test( S.length() == ExpectedLength );
            Test( S.c_str() , &String2[0] , ExpectedLength );
        }
    }
    // void trim ( const char * targets )
    {
        cstring S        = String1 ;
        cstring Expected = String1 ;
        S.trim(String2);
        Expected.trim_left(String2); 
        Expected.trim_right(String2); 
        TestContents( S , Expected );
    }

    // void trim_left( const char * targets )
    {
        cstring     S               = String1 ;
        const int   pos             = S.find_first_not_of(String2); // Position of first part left behind.
        const int   ExpectedLength  = pos == cstring::npos ? 0 : Length1 - pos ;
        S.trim_left(String2);
        Test( S.length() == ExpectedLength );
        Test( S.c_str() , &String1[pos] , ExpectedLength );
    }
    // void trim_right( const char * targets ) 
    {
        cstring     S               = String1 ;
        const int   pos             = S.find_last_not_of(String2); // Position of end of part left behind
        const int   ExpectedLength  = pos == cstring::npos ? 0 : pos+1 ;
        S.trim_right(String2);
        Test( S.length() == ExpectedLength );
        Test( S.c_str() , &String1[0] , ExpectedLength );
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do an axiomatic test on cstring left() and right()
//------------------------------------------------------------------------
// Use the given test string wherever applicable.
#if ENABLE_SELF_TEST
static void TestLeftRight( const char * String )
{
    const int Length = String == 0 ? 0 : strlen(String);
    cstring S = String ;
    int n ;
    for( n = 0 ; n <= Length+20 ; n++ )
    {
        const int Sublength = Min( n , Length );
        const cstring left  = S.left(n); 
        const cstring right = S.right(n);
        int   pos ;

        Test( left.length() == Sublength  );
        for( pos = 0 ; pos < left.length() ; ++ pos )
        {
            Test( left[pos] == String[pos] );
        }

        Test( right.length() == Sublength );
        for( pos = 1 ; pos <= right.length() ; ++ pos )
        {
            Test( right[Sublength-pos] == String[Length-pos] );
        }
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do a (nearly) complete axiomatic test using a string value
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static void DoFullTest( const char * String )
{
    if( true )
    {
        TestCopy                ( String );
        TestConstructors        ( String );
        TestErase               ( String );
        TestIterators           ( String );
        TestIndexing            ( String );
        TestDataAccess          ( String );
        TestPrivate             ( String );
        TestCapacityAndLength   ( String );
        TestSubstr              ( String );
        TestUpperLower          ( String );
        TestLeftRight           ( String );
    };
}
#endif

//------------------------------------------------------------------------
// Debug: Do a (nearly) complete axiomatic test using two string values
//------------------------------------------------------------------------
#if ENABLE_SELF_TEST
static void DoFullTest( const char * String1 , const char * String2 )
{
    if( true ) 
    {
        TestFind                ( String1 , String2 );
        TestComparison          ( String1 , String2 );
        TestSwap                ( String1 , String2 );
        TestReplace             ( String1 , String2 );
        TestInsertion           ( String1 , String2 );
        TestCatenation          ( String1 , String2 );
        TestAssignment          ( String1 , String2 );
        TestFindFirstLastOf     ( String1 , String2 );
        TestFindFirstLastNotOf  ( String1 , String2 );
        TestTrim                ( String1 , String2 );
    }
}
#endif

//------------------------------------------------------------------------
// Debug: Do a self-test of this class.
//------------------------------------------------------------------------
void cstring::test_all() 
{
    #if ENABLE_SELF_TEST
    {
        TestCount = 0;

        static const char * TestStrings[] = // A list of non-null test strings.
        {
            ""                                                                  ,
            " "                                                                 ,
            "                                                                 " ,
            ".                                                             ."   ,
            "Now is the time for all good men to come to the aid of the party." ,
            "The quick red fox jumped over the lazy brown dog."                 ,
            "To be or not"                                                      ,
            "to"                                                                ,
            "White \n space \t only \r in between"                              , 
            "\r White space before"                                             ,
            "White space after\r"                                               ,
            "\n  White space before and after\n \t \r"                          ,
            "b"                                                                 ,
            "\t"                                                                ,
            "\r"                                                                ,
            "\n"                                                                ,
            "xxx   yyyy    xxx"                                                 ,
            // Some simple substrings of the above strings:
            "Now"                                                               ,
            "Now is the time"                                                   ,
            "The quick red fox"                                                 ,
            "To be or not"                                                      ,
            "T"                                                                 ,
            "t"                                                                 ,
            // Some strings useful for trimming, find_first_of, find_last_of    
            "o"                                                                 ,
            "a"                                                                 ,
            " "                                                                 ,
            "\r ab"                                                             ,
            "->abcdefghijklmnopqrstuvwxyz"                                      ,
            "x"                                                                 ,

        };
        const int NTestStrings = sizeof(TestStrings) / sizeof(TestStrings[0]);

        //------------------------------------------------------------------------
        // Basic, intuitive testing 
        //------------------------------------------------------------------------
        // Makes sure the basic behaviour is as expected
        {
            {
                cstring S( "Hi" );
                Test( S.length() == 2 && strcmp(S,"Hi") == 0 );
            }
            {
                cstring S( 5 , 'x' );
                Test( S.length() == 5 && strcmp(S,"xxxxx") == 0 );
            }
            {
                cstring S( "Now is the" , 4 );
                Test( S.length() == 4 && strcmp(S,"Now ") == 0 );
            }
            {
                cstring S( "winter" , 6 );
                Test( S.length() == 6 && strcmp(S,"winter") == 0 );
                cstring S2( "winter" , 7 );
                Test( S2.length() == 6 && strcmp(S2,"winter") == 0 );
                cstring S3( S , 3 , 4 );
                Test( S3.length() == 3 && strcmp(S3,"ter") == 0 );
                cstring S4( S , 5 , 1 );
                Test( S4.length() == 1 && strcmp(S4,"r") == 0 );
                cstring S5( S , 1 , 3 );
                Test( S5.length() == 3 && strcmp(S5,"int") == 0 );
            }

            {
                cstring S = "Hi";
                S.resize( 7 , '-' );
                Test( S.length() == 7 && strcmp(S,"Hi-----") == 0 );
                S.resize(5);
                Test( S.length() == 5 && strcmp(S,"Hi---") == 0 );
                S.resize(1);
                Test( S.length() == 1 && strcmp(S,"H") == 0 );
                S.resize(0);
                Test( S.length() == 0 && strcmp(S,"") == 0 );
                S.reserve(100);
                Test( S.capacity() >= 100 );
            }

            {
                cstring S1 = "abcd";
                cstring S2 = "EFG";
                S1.insert( 3 , S2 );
                Test( S1.length() == 7 && strcmp(S1,"abcEFGd") == 0 );
                S1.insert( 4 , S2 , 1 , 1 );
                Test( S1.length() == 8 && strcmp(S1,"abcEFFGd") == 0 );
            }

            {
                cstring S = "abcd";
                S.insert( 1 , "now is the time" , 5 );
                Test( S.length() == 9 && strcmp(S,"anow ibcd") == 0 );
                S.insert( 3 , "later" , 6 );
                Test( S.length() == 14 && strcmp(S,"anolaterw ibcd") == 0 );
                S.insert( 0 , "--->" );
                Test( S.length() == 18 && strcmp(S,"--->anolaterw ibcd") == 0 );
            }

            {
                cstring S = "cdef" ;
                S.insert( 3 , 7 , '.' );
                Test( S.length() == 11 && strcmp(S,"cde.......f") == 0 );
            }

            {
                cstring S = "To be or not" ;
                cstring::iterator it = S.begin();
                it++;
                it++;
                it++;
                it = S.insert( it , 'X' );
                Test( S.length() == 13 && strcmp(S,"To Xbe or not") == 0 );
                S.insert( it , 3 , '*' );
                Test( S.length() == 16 && strcmp(S,"To ***Xbe or not") == 0 );
            }

            {
                cstring S = "abcdefghi" ;
                S.erase( 0 , 3 );
                Test( S.length() == 6 && strcmp(S,"defghi") == 0 );
                S.erase( 3 , 3 );
                Test( S.length() == 3 && strcmp(S,"def") == 0 );
                S.erase( 1 , 3 );
                Test( S.length() == 1 && strcmp(S,"d") == 0 );
            }

            {
                cstring S = "0123456789--" ;
                cstring::iterator it = S.begin();
                it = S.erase(it);
                Test( S.length() == 11 && strcmp(S,"123456789--") == 0 );
                it++; it++;
                it = S.erase( S.erase(it) );
                Test( S.length() == 9 && strcmp(S,"1256789--") == 0 );
                Test( *it == '5' );
            }

            {
                cstring S = "abcdefg" ;
                cstring::iterator it1 = S.begin();
                cstring::iterator it2 = S.end();
                it1++; it1++;
                it2--; it2--;
                S.erase( it1 , it2 );
                Test( S.length() == 4 && strcmp(S,"abfg") == 0 );
            }

            {
                cstring S = "000001111122222333334444455555";
                cstring A = "Hello" ;
                S.replace( 10 , 5 , A ); 
                Test( S.length() == 30 && strcmp(S,"0000011111Hello333334444455555") == 0 );
                S.replace( 5 , 4 , A ); 
                Test( S.length() == 31 && strcmp(S,"00000Hello1Hello333334444455555") == 0 );
                S.replace( 21 , 100 , A ); 
                Test( S.length() == 26 && strcmp(S,"00000Hello1Hello33333Hello") == 0 );
            }
            {
                cstring S = "hi";
                S.resize( 5 , 'x' );
                Test( S.length() == 5 && strcmp(S,"hixxx") == 0 );
            }
        }

        //------------------------------------------------------------------------
        // Artistic testing of: ==, !=
        //------------------------------------------------------------------------
        {
            cstring A = ""    ;
            cstring B = 0     ;
            cstring C = "Hi"  ;
            cstring D = "Hi " ;
            cstring E = "Hi " ;
            Test( A == B  );
            Test( A == 0  );
            Test( B == 0  );
            Test( A == "" );
            Test( B == "" );
            Test( C != 0  );
            Test( C != "" );
            Test( C != A  );
            Test( C != B  );
            Test( C != D  );
            Test( D != 0  );
            Test( D != C  );
            Test( D == E  );
            Test( E == "Hi " );
            Test( E != "HI " );

            cstring F = E;
            F.to_lower();
            Test( F == "hi " );
        }

        //------------------------------------------------------------------------
        // Artistic testing of: compare()
        //------------------------------------------------------------------------
        {
            {
                cstring A = "a"    ;
                cstring B = "a"    ;
                cstring C = "ab"   ;
                cstring D = ""     ;
                cstring E = "abc"  ;
                cstring F = "abcc" ;
                Test( A.compare(A)  ==  0 );
                Test( A.compare(B)  ==  0 );
                Test( A.compare(C)  <   0 );
                Test( A.compare(D)  >   0 );
                Test( A.compare(E)  <   0 );
                Test( B.compare(A)  ==  0 );
                Test( B.compare(B)  ==  0 );
                Test( B.compare(C)  <   0 );
                Test( C.compare(A)  >   0 );
                Test( C.compare(C)  ==  0 );
                Test( C.compare(D)  >   0 );
                Test( C.compare(E)  <   0 );
                Test( C.compare(F)  <   0 );
                Test( D.compare(A)  <   0 );
                Test( D.compare(B)  <   0 );
                Test( D.compare(C)  <   0 );
                Test( D.compare(D)  ==  0 );
                Test( D.compare(E)  <   0 );
                Test( D.compare(F)  <   0 );
                Test( E.compare(C)  >   0 );
                Test( E.compare(F)  <   0 );
                Test( F.compare(C)  >   0 );
                Test( F.compare(E)  >   0 );
            }

            {
                cstring A( "abcABCdefDEF" );
                cstring B( "ABCabcDEFdef" );
                Test( A.compare( 3 ,  3 , B )  <  0 );
                Test( A.compare( 3 ,  4 , B )  >   0 );
                Test( A.compare( 3 ,  2 , B )  <   0 );
                Test( A.compare( 3 , 20 , B )  >   0 );
                Test( A.compare( 3 , 3 , B , 0, 3 ) == 0 );
                Test( A.compare( 3 , 4 , B , 0, 3 ) >  0 );
                Test( A.compare( 9 , 2 , B , 6, 2 ) == 0 );
                Test( A.compare( 9 , 2 , B , 6, 1 ) >  0 );
                Test( A.compare( 9 , 2 , B , 7, 1 ) <  0 );
                Test( A.compare( "abc" )  >   0 );
                Test( A.compare( "b"   )  <   0 );
                Test( A.compare( "abcABCdefDEF" )  ==  0 );
                Test( A.compare( 4 , 2 , "BC"  )   ==  0 );
                Test( A.compare( 4 , 3 , "BCe" )   <   0 );
            }
        }

        //------------------------------------------------------------------------
        // Artistic testing of: find()
        //------------------------------------------------------------------------
        {
            // int find( const cstring & str , int pos = 0 ) const;
            {
                cstring A = "If I were a carpenter," ;
                cstring B = "er" ;
                cstring C = "z"  ;
                cstring D = 0    ;
                Test( A.find( B      )      ==    6 );
                Test( A.find( B ,  4 )      ==    6 );
                Test( A.find( B ,  6 )      ==    6 );
                Test( A.find( B ,  7 )      ==   19 );
                Test( A.find( B , 19 )      ==   19 );
                Test( A.find( B , 20 )      == npos );
                Test( A.find( C      )      == npos );
                Test( A.find( C , 10 )      == npos );
                Test( A.find( D      )      == npos );
                Test( A.find( D ,  5 )      == npos );
            }
            // int find( const char * s , int pos , int n ) const
            {
                cstring A = "Blast off at last!" ;
                Test( A.find( "last"  , 1 , 5 )  ==  1    );
                Test( A.find( "last"  , 2 , 5 )  ==  13   );
                Test( A.find( "last!" , 0 , 5 )  ==  13   );
                Test( A.find( "last!" , 0 , 4 )  ==   1   );
                Test( A.find( "last!" , 0 , 0 )  ==  npos );
            }
            // int find( const char * s , int pos ) const
            {
                cstring A = "abcabfg \t " ;
                Test( A.find( "g \x08"  )         == npos );
                Test( A.find( "g \x09"  )         == 6    );
                Test( A.find( "g \x09 " )         == 6    );
                Test( A.find( "g \x09  " )        == npos );
                Test( A.find( "ab"  , 0  )        == 0    );
                Test( A.find( "ab"  , 1  )        == 3    );
                Test( A.find( "ab"  , 2  )        == 3    );
                Test( A.find( "ab"  , 3  )        == 3    );
                Test( A.find( "ab"  , 4  )        == npos );
            }   
            // int find( char c , int pos ) const
            // int rfind( char c , int pos = npos ) const;
            {
                cstring S = "\r\n\tabAB" ;
                Test( S.find( '\x00'    )      == npos );
                Test( S.find( '\x0a'    )      == 1    );
                Test( S.find( '\x0a' , 2 )      == npos );
                Test( S.find( 'a'    , 2 )      == 3    );
                Test( S.find( 'a'    , 4 )      == npos );
                Test( S.find( 'A'    , 4 )      == 5    );
                Test( S.find( '\x0d' , 0 )      == 0    );
                Test( S.find( 'B'    , 0 )      == 6    );
            }

            // to_upper(), to_lower(), reverse()
            {
                cstring S = "Now;Is;The;Time " ;
                S.to_lower(); Test( strcmp(S,"now;is;the;time ") == 0 );
                S.to_upper(); Test( strcmp(S,"NOW;IS;THE;TIME ") == 0 );
                S.reverse() ; Test( strcmp(S," EMIT;EHT;SI;WON") == 0 );
            }

            
            {
                cstring S = "abCDefCDab" ;
                cstring A = "abCD" ;
                cstring B = "CD"   ;
                cstring C = "abc"  ;
                // int rfind( char c , int pos = npos ) const;
                Test( S.rfind( 'a'      )      == 8    );
                Test( S.rfind( 'a',  9  )      == 8    );
                Test( S.rfind( 'a',  8  )      == 8    );
                Test( S.rfind( 'a',  7  )      == 0    );
                Test( S.rfind( 'D',  7  )      == 7    );
                Test( S.rfind( 'D',  6  )      == 3    );
                Test( S.rfind( 'D',  2  )      == npos );
                // int rfind( const cstring & str , int pos ) const
                Test( S.rfind( A        )      == 0    );
                Test( S.rfind( A  , 1   )      == 0    );
                Test( S.rfind( B        )      == 6    );
                Test( S.rfind( B  , 6   )      == 6    );
                Test( S.rfind( B  , 5   )      == 2    );
                Test( S.rfind( B  , 2   )      == 2    );
                Test( S.rfind( B  , 1   )      == npos );
                Test( S.rfind( C        )      == npos );
                // int rfind( const char * s , int pos , int n ) const
                Test( S.rfind( "CD"    , npos ,  3 ) ==  6    );
                Test( S.rfind( "CDeg"  , npos ,  4 ) ==  npos );
                Test( S.rfind( "CDeg"  , npos ,  3 ) ==  2    );
                Test( S.rfind( "ab"    , 7    , 12 ) ==  0    );
                Test( S.rfind( "ab"    , 100  ,  1 ) ==  8    );
                // int rfind( const char * s , int pos = npos ) const
                Test( S.rfind( "ab"    , 100  ) ==  8    );
                Test( S.rfind( "abC"      ) ==  0    );

            }
        }
        //------------------------------------------------------------------------
        // Artistic testing of: find_first... find_last...
        //------------------------------------------------------------------------
        {
            {
                //          "0123456789012345" (index reference)
                cstring S = " -=(abcdef^%$ --" ;
                cstring A = "( de" ;
                // int find_first_of( const cstring & str , int pos = 0 ) const;
                Test( S.find_first_of( A         )  ==    0 );  
                Test( S.find_first_of( A    ,  3 )  ==    3 );  
                Test( S.find_first_of( A    ,  4 )  ==    7 );  
                Test( S.find_first_of( A    ,  7 )  ==    7 );  
                Test( S.find_first_of( A    ,  8 )  ==    8 );  
                Test( S.find_first_of( A    ,  9 )  ==   13 );  
                Test( S.find_first_of( A    , 14 )  == npos );  
                // int find_first_of( const char * s , int pos , int n ) const;
                Test( S.find_first_of( "xy"              )  == npos );  
                Test( S.find_first_of( "xyf"   ,  0 ,  2 )  == npos );  
                Test( S.find_first_of( "xyz"   ,  0 ,  5 )  == npos );  
                Test( S.find_first_of( "xyf"   ,  0 ,  5 )  ==    9 );  
                // int find_first_of( const char * s , int pos = 0 ) const;
                Test( S.find_first_of( "cba"   ,  5      )  ==    5 );  
                Test( S.find_first_of( "$"               )  ==   12 );  
                Test( S.find_first_of( '^'               )  ==   10 );
                // int find_first_of( char c , int pos = 0 ) const ;
                Test( S.find_first_of( '\x00'            )  == npos );
                Test( S.find_first_of( 'e'     , 7       )  ==    8 );
                Test( S.find_first_of( 'e'     , 8       )  ==    8 );
                Test( S.find_first_of( 'e'     , 9       )  == npos );
                // int find_last_of( const cstring & str , int pos = npos ) const;
                Test( S.find_last_of( A                 )  ==    13 );
                Test( S.find_last_of( A        , 12     )  ==     8 );
                Test( S.find_last_of( A        ,  8     )  ==     8 );
                Test( S.find_last_of( A        ,  7     )  ==     7 );
                Test( S.find_last_of( A        ,  6     )  ==     3 );
                Test( S.find_last_of( A        ,  3     )  ==     3 );
                Test( S.find_last_of( A        ,  2     )  ==     0 );
                Test( S.find_last_of( A        ,  0     )  ==     0 );
                // int find_last_of( char c , int pos = npos ) const;
                Test( S.find_last_of( '-'               )  ==    15 );
                Test( S.find_last_of( '-'      , 15     )  ==    15 );
                Test( S.find_last_of( '-'      , 14     )  ==    14 );
                Test( S.find_last_of( '-'      , 13     )  ==     1 );
                Test( S.find_last_of( '-'      ,  0     )  ==  npos );
                Test( S.find_last_of( '%'      ,  9     )  ==  npos );
                // int find_last_of( const char * s , int pos ,  int n = npos ) const;
                Test( S.find_last_of( "abc"  ,   16 , 1 )  ==     4 );
                Test( S.find_last_of( "xy^"  , npos , 5 )  ==    10 );
                Test( S.find_last_of( "xy^"  , npos , 2 )  ==  npos );
                Test( S.find_last_of( "def"  , npos , 5 )  ==     9 );
                // int find_last_of( const char * s , int pos = npos ) const;
                Test( S.find_last_of( " f"              )  ==    13 );
                Test( S.find_last_of( "g\t+"            )  ==  npos );
                Test( S.find_last_of( "$%"              )  ==    12 );
                Test( S.find_last_of( "$%"      ,  11   )  ==    11 );
                Test( S.find_last_of( "$%"      ,  10   )  ==  npos );
                Test( S.find_last_of( "(abcdef^%$" , 2  )  ==  npos );
            }
            {
                //           "0123456789 012345" (index reference)
                cstring S = "\t ABxx@#$%\"'" ;
                cstring A = "( de" ;
                // int find_first_not_of( const cstring & str , int pos = 0 ) const;
                Test( S.find_first_not_of( "AB# \t"   )  == 4 );
                // int find_first_not_of( const char * s , int pos , int n ) const;
                // int find_first_not_of( const char * s , int pos = 0 ) const;
                Test( S.find_first_not_of( S.c_str() )  == npos );
                // int find_first_not_of( char c , int pos = 0 ) const;
                // int find_last_not_of( const cstring & str , int pos = npos ) const;
                // int find_last_not_of( const char * s , int pos , int n ) const;
                // int find_last_not_of( const char * s , int pos = npos ) const;
                // int find_last_not_of( char c , int pos = npos ) const;
            }
        }

        //------------------------------------------------------------------------
        // Artistic testing of: trim...
        //------------------------------------------------------------------------
        {
            {
                cstring S ;
                S = "now"          ; S.trim()        ; Test( strcmp( S , "now"         ) == 0 );
                S = "i s "         ; S.trim_left()   ; Test( strcmp( S , "i s "        ) == 0 );
                S = "\nt he"       ; S.trim_right()  ; Test( strcmp( S , "\nt he"      ) == 0 );
                S = " to be or \r" ; S.trim()        ; Test( strcmp( S , "to be or"    ) == 0 );
                S = " to be or \r" ; S.trim_left()   ; Test( strcmp( S , "to be or \r" ) == 0 );
                S = " to be or \r" ; S.trim_right()  ; Test( strcmp( S , " to be or"   ) == 0 );
            }
        }

        //------------------------------------------------------------------------
        // Artistic testing of: left() right()
        //------------------------------------------------------------------------
        {

            {
                cstring S = "In the beginning" ;
                cstring X ;
                X = S.left ( 0); Test( X.length() == 0           && strcmp(X,"") == 0 );
                X = S.left ( 1); Test( X.length() == 1           && strcmp(X,"I") == 0 );
                X = S.left ( 5); Test( X.length() == 5           && strcmp(X,"In th") == 0 );
                X = S.left (16); Test( X.length() == S.length()  && strcmp(X,S) == 0 );
                X = S.left (20); Test( X.length() == S.length()  && strcmp(X,S) == 0 );
                X = S.right( 0); Test( X.length() == 0           && strcmp(X,"") == 0 );
                X = S.right( 3); Test( X.length() == 3           && strcmp(X,"ing") == 0 );
                X = S.right(10); Test( X.length() == 10          && strcmp(X," beginning") == 0 );
                X = S.right(16); Test( X.length() == S.length()  && strcmp(X,S) == 0 );
                X = S.right(17); Test( X.length() == S.length()  && strcmp(X,S) == 0 );
            }
        }


        //------------------------------------------------------------------------
        // Do a full test on all test strings
        //------------------------------------------------------------------------
        if( true )
        {
            DoFullTest(0); // Test null string.
            DoFullTest(""); // Test empty string.
            for( int WhichString = 0 ; WhichString < NTestStrings ; ++WhichString )
            {
                const char * TestString = TestStrings[WhichString];
                DoFullTest( TestString );
                DoFullTest( 0 , TestString );
                DoFullTest( TestString , 0 );
                for( int WhichOtherString = 0 ; WhichOtherString < NTestStrings ; ++WhichOtherString )
                {
                    const char * OtherTestString= TestStrings[WhichOtherString];
                    DoFullTest( TestString , OtherTestString );
                }
            }
        }
        TestCount = TestCount ; // Set a breakpoint here if you want to see how many tests were run.
    }
    #endif
}

