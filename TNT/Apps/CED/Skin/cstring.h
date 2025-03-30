#if !defined( cstring_H )
#define cstring_H

//------------------------------------------------------------------------
// cstring: A simple string class
//------------------------------------------------------------------------
// This is based closely on the C++ standard class "string" which is an 
// instantiation of the C++ standard template class basic_string. The extreme 
// generality of the C++ standard string has been sacrificed for simplicity, 
// safety, and possibly speed. Specifically:
//    - there are no iterators, allocators, or char_traits. 
//    - errors cause ASSERT() failures instead of throwing exceptions
//    - null (char *) pointers can be used for most functions
//
// Notes:
//   1. A cstring object is never null, it always contains a string,
//      even if just the empty string "".  Use empty() to see if a
//      string is empty.
//   2. A cstring object keeps track of its length and unused space, so (except 
//      for a null cstring) there is a small additional space overhead: 
//      2*sizeof(int) bytes.
//   3. The length() operation is always fast - it does not require scanning 
//      the string.
//   4. To reduce or eliminate reallocations during string copying or 
//      catenation, you can use the reserve() function.
//   5. The class is called cstring to prevent any conflict with the Windows 
//      CString or the C++ "string".
//------------------------------------------------------------------------

#include <assert.h>
#include "Basics.h"

#if defined(_MSC_VER)
    // Suppress certain Microsoft Visual C++ warnings
    #pragma warning( disable: 4514 ) // Warning:  unreferenced inline function removed
    #pragma warning( disable: 4710 ) // Warning: function 'x' not inlined
#endif

class cstring 
{
  public:

    // npos: The special value which can never represent a length of or 
    // position in a cstring. Uses include a return value from find functions 
    // to represent "not found". Guaranteed to be negative, so you can check 
    // explicitly for npos or just check for < 0.
    enum { npos = -1 }; // [Note: In basic_string this is a static const int]

    class                   iterator                ;
    class                   const_iterator          ;
    class                   reverse_iterator        ;
    class                   const_reverse_iterator  ;

    //------------------------------------------------------------------------
    // Construction
    //------------------------------------------------------------------------
    // When constructing from a cstring, the capacity of the source cstring is 
    // not used by the target cstring.

    cstring() { m_Data = m_EmptyData; }         ; // Construct a string with "".
    cstring( const cstring & rhs )              ; // Construct a string with a copy of rhs.
    cstring( const char *    s   )              ; // Construct a string with a copy of s. 
    cstring( int n , char c )                   ; // Construct a string out of n characters c.
    cstring( const char *    s   , int n )      ; // Construct a string from n characters at s, or until s is exhausted.
    cstring( const cstring & rhs , int pos , int n  ); // Construct a string from n characters of rhs at position pos. pos < rhs.length().
    cstring( const_iterator first , const_iterator last ); // Construct a string from the sequence [first,last) in another string.

    ~cstring() { if(  m_Data != m_EmptyData ) { Delete(); } }

    //------------------------------------------------------------------------
    // Assignment
    //------------------------------------------------------------------------
    // When copying from a cstring, the capacity of the source cstring is not
    // used in the target cstring. When copying a null string (0) into a cstring,
    // the target cstring is freed. You can therefore explicitly deallocate a string S with 
    //    S = 0;

    cstring & operator = ( const cstring & rhs )    ; // Copy rhs into *this.
    cstring & operator = ( const char * s )         ; // Copy null-terminated s into *this.
    cstring & operator = ( char c )                 ; // Make *this a string with a single character c.
    cstring & assign( const cstring & str )         { return *this = str ; }
    cstring & assign( const cstring & str , int pos , int n ); // pos < str.length()
    cstring & assign( const char * s , int n );
    cstring & assign( const char * s )              { return *this = s ; }
    cstring & assign( int n , char c );
    cstring & assign( const_iterator first , const_iterator last ); // Assign the sequence [first,last) from another string.

    //------------------------------------------------------------------------
    // Iterators
    //------------------------------------------------------------------------
    // To iterate over string S:
    //    for( cstring::iterator X = S.begin()  ; X != S.end()  ; X++ ) { /* Use *X */ }
    //    for( cstring::iterator X = S.rbegin() ; X != S.rend() ; X++ ) { /* Use *X */ }
    // You can also use indexing:
    //    for( int pos = 0 ; pos < S.length() ; pos++ ) { /* Use S[pos] */ }

    iterator                begin  ()       { return iterator               ( m_Data , length() , 0        ); }
    const_iterator          begin  () const { return const_iterator         ( m_Data , length() , 0        ); }
    iterator                end    ()       { return iterator               ( m_Data , length() , length() ); }
    const_iterator          end    () const { return const_iterator         ( m_Data , length() , length() ); }
    reverse_iterator        rbegin ()       { return reverse_iterator       ( m_Data , length() , length() ); }
    const_reverse_iterator  rbegin () const { return const_reverse_iterator ( m_Data , length() , length() ); }
    reverse_iterator        rend   ()       { return reverse_iterator       ( m_Data , length() ,        0 ); }
    const_reverse_iterator  rend   () const { return const_reverse_iterator ( m_Data , length() ,        0 ); }

    //------------------------------------------------------------------------
    // Indexing operations
    //------------------------------------------------------------------------
    // WARNING: 
    //   1. If you set at(pos) = 0, you will NOT change the length of the 
    //      string. Use resize(pos) to resize.

    char    at         ( int pos ) const   { assert( pos >= 0 && pos < length() ); return m_Data[pos]; }
    char &  at         ( int pos )         { assert( pos >= 0 && pos < length() ); return m_Data[pos]; }
    char    operator[] ( int pos ) const   { assert( pos >= 0 && pos < length() ); return m_Data[pos]; }
    char &  operator[] ( int pos )         { assert( pos >= 0 && pos < length() ); return m_Data[pos]; }

    //------------------------------------------------------------------------
    // Access to underlying data
    //------------------------------------------------------------------------

    operator const char * () const { return m_Data ; } // Conversion to (const char *). [Non-standard extension not in template basic_string]
    const char *    c_str () const { return m_Data ; } // Return a pointer to a null-terminated string. Never returns 0.
    const char *    c_str0() const { return m_Data[0] == 0 ? 0 : m_Data; } // Like c_str(), but returns 0 if the string is empty. [Non-standard extension not in template basic_string]
    const char *    data  () const { return m_Data ; }

    //------------------------------------------------------------------------
    // Exposure of private things 
    //------------------------------------------------------------------------
    // These functions are unsafe and are provided for cases where efficiency
    // demands access to the underlying data, such as allocating a large string
    // and reading the contents of a file directly into the string. If you 
    // use private_resize(n), you must ensure that there are no null characters
    // prior to position n. If you use private_data, you must ensure that 
    // private_resize() is called to be consistent with any null characters you
    // put in the string.
    char *          private_data ()         ; // Return a pointer to the underlying data. Make sure you reserve() enough space for your needs.
    void            private_resize( int n ) ; // Unsafe version of resize(). Does not extend-fill: the string contents are unchanged except for adding null termination ([n]=0).

    //------------------------------------------------------------------------
    // Length, size, capacity, and so on
    //------------------------------------------------------------------------

    int     length   () const                   { return Prefix().Length ; }
    int     size     () const                   { return Prefix().Length ; }
    int     max_size () const                   { return MaxSize ; }
    void    resize   ( int n , char c = ' ' );
    int     capacity () const                   { return Prefix().Capacity; }
    void    reserve  ( int n = 0 );
    bool    empty    () const                   { return m_Data[0] == 0; }

    //------------------------------------------------------------------------
    // Catenation
    //------------------------------------------------------------------------

    cstring & operator += ( const cstring & rhs );
    cstring & operator += ( const char * s );
    cstring & operator += ( char c );
    cstring & append( const cstring & str )     { return *this += str; }
    cstring & append( const cstring & str , int pos , int n ); // pos < str.length()
    cstring & append( const char * s , int n );
    cstring & append( const char * s )          { return *this += s  ; }
    cstring & append( int n , char c );
    cstring & append( const_iterator first , const_iterator last ); // Append the sequence [first,last) from ANOTHER string.

    //------------------------------------------------------------------------
    // Insertion
    //------------------------------------------------------------------------

    cstring & insert( int p0 , const cstring & str );
    cstring & insert( int p0 , const cstring & str , int pos , int n ); 
    cstring & insert( int p0 , const char * s , int n );
    cstring & insert( int p0 , const char * s );
    cstring & insert( int p0 , int n , char c );
    iterator  insert( iterator it , char c );   // Insert c before it, and return an iterator to the newly inserted element.
    void      insert( iterator it , int n , char c);
    void      insert( iterator it , const_iterator first , const_iterator last ); // Insert the sequence [first,last) from ANOTHER string.
    
    //------------------------------------------------------------------------
    // Replacing and erasing parts of strings
    //------------------------------------------------------------------------

    cstring & erase  ( int p0 = 0 , int n = npos );
    iterator  erase  ( iterator it );
    iterator  erase  ( iterator first , iterator last );
    cstring & replace( int p0 , int n0 , const cstring & str );
    cstring & replace( int p0 , int n0 , const cstring & str , int pos , int n ); 
    cstring & replace( int p0 , int n0 , const char * s , int n );
    cstring & replace( int p0 , int n0 , const char * s );
    cstring & replace( int p0 , int n0 , int n , char c );
    cstring & replace( iterator first0 , iterator last0 , const cstring & str );
    cstring & replace( iterator first0 , iterator last0 , const char * s , int n);
    cstring & replace( iterator first0 , iterator last0 , const char *s );
    cstring & replace( iterator first0 , iterator last0 , int n , char c );
    cstring & replace( iterator first0 , iterator last0 , const_iterator first , const_iterator last); 
    
    //------------------------------------------------------------------------
    // Copy and swap
    //------------------------------------------------------------------------

    int copy( char * s , int n , int pos = 0 ) const; // Note: Does not add a null terminator.
    void swap( cstring & str );

    //------------------------------------------------------------------------
    // Searching
    //------------------------------------------------------------------------
    // Searching for an empty or null string yields npos.
    // Searching in an empty string yields npos.

    int find( const cstring & str , int pos = 0 ) const; 
    int find( const char * s , int pos , int n ) const;
    int find( const char * s , int pos = 0 ) const;
    int find( char c , int pos = 0 ) const;
    int rfind( const cstring & str , int pos = npos ) const;
    int rfind( const char * s , int pos , int n ) const;
    int rfind( const char * s , int pos = npos ) const;
    int rfind( char c , int pos = npos ) const;
    int find_first_of( const cstring & str , int pos = 0 ) const;
    int find_first_of( const char * s , int pos , int n ) const;
    int find_first_of( const char * s , int pos = 0 ) const;
    int find_first_of( char c , int pos = 0 ) const { return find(c,pos); }
    int find_last_of( const cstring & str , int pos = npos ) const;
    int find_last_of( const char * s , int pos ,  int n ) const;
    int find_last_of( const char * s , int pos = npos ) const;
    int find_last_of( char c , int pos = npos ) const { return rfind(c,pos); }
    int find_first_not_of( const cstring & str , int pos = 0 ) const;
    int find_first_not_of( const char * s , int pos , int n ) const;
    int find_first_not_of( const char * s , int pos = 0 ) const;
    int find_first_not_of( char c , int pos = 0 ) const;
    int find_last_not_of( const cstring & str , int pos = npos ) const;
    int find_last_not_of( const char * s , int pos , int n ) const;
    int find_last_not_of( const char * s , int pos = npos ) const;
    int find_last_not_of( char c , int pos = npos ) const;

    //------------------------------------------------------------------------
    // Substring
    //------------------------------------------------------------------------
    cstring substr( int pos = 0 , int n = npos ) const;

    //------------------------------------------------------------------------
    // Comparison
    //------------------------------------------------------------------------
    int compare( const cstring & str ) const;
    int compare( int p0 , int n0 , const cstring & str ) const;
    int compare( int p0 , int n0 , const cstring & str , int pos , int n ) const;
    int compare( const char * s ) const;
    int compare( int p0 , int n0 , const char * s ) const;
    int compare( int p0 , int n0 , const char * s , int pos ) const;

    bool operator == ( const cstring & str ) const { return this->compare(str) == 0; }
    bool operator == ( const char *    s   ) const { return this->compare(s)   == 0; }

    //------------------------------------------------------------------------
    // Other non-standard extensions
    //------------------------------------------------------------------------
    // Note: Whitespace characters are: 0x09(tab), 0x0a(new line), 0x0d(carriage return), and 0x20(space)
    void to_upper   () ; // Convert entire string to uppercase.
    void to_lower   () ; // Convert entire string to lowercase.
    void reverse    () ; // Reverse the contents of the string.
    void trim       () ; // Remove all whitespace characters from the beginning and end of this.
    void trim_left  () ; // Remove all whitespace characters from the beginning of this.
    void trim_right () ; // Remove all whitespace characters from the end of this.
    void trim       ( const char * targets ) ; // Remove from the front and end of *this any characters in *targets.
    void trim_left  ( const char * targets ) ; // Remove from the front of *this any characters in *targets.
    void trim_right ( const char * targets ) ; // Remove from the end of *this any characters in *targets.
    void trim_left  ( int n ) ; // Remove up to n characters from the left of this.
    void trim_right ( int n ) ; // Remove up to n characters from the right of this.

    cstring left ( int n ) const; // The first n characters of this, or all of this if n >= length().
    cstring right( int n ) const; // The last n characters of this, or all of this if n >= length().


    //------------------------------------------------------------------------
    // const_iterator implementation
    //------------------------------------------------------------------------
    class const_iterator
    {
      public:
        const_iterator() { }
        const_iterator & operator ++ ()                           { m_Position = Min( m_Position+1 , m_Length ); return *this; }
        const_iterator & operator ++ (int)                        { m_Position = Min( m_Position+1 , m_Length ); return *this; }
        const_iterator & operator += (int n)                      { m_Position = Min( m_Position+n , m_Length ); return *this; }
        const_iterator & operator -- ()                           { m_Position = Max( m_Position-1 ,        0 ); return *this; }
        const_iterator & operator -- (int)                        { m_Position = Max( m_Position-1 ,        0 ); return *this; }
        const_iterator & operator -= (int n)                      { m_Position = Max( m_Position-n ,        0 ); return *this; }
        const_iterator   operator +  (int n) const                { return const_iterator( m_Data , m_Length , Min( m_Position+n , m_Length ) ); }
        const_iterator   operator -  (int n) const                { return const_iterator( m_Data , m_Length , Max( m_Position-n ,        0 ) ); }
        bool             operator == (const const_iterator & rhs) const { return m_Position == rhs.m_Position ; }
        bool             operator != (const const_iterator & rhs) const { return m_Position != rhs.m_Position ; }
        const char &     operator *  ()                           { assert( m_Position < m_Length ); return m_Data[m_Position]; }
        int operator - ( const const_iterator & rhs ) const { assert(m_Data==rhs.m_Data); return m_Position - rhs.m_Position ; }

      protected :

        friend class cstring ;
        const_iterator( const char * Data , int Length , int Position = 0 ) 
        {  
            m_Data      = Data      ;
            m_Length    = Length    ;
            m_Position  = Position  ; 
        }
        const char *    m_Data      ; // The beginning of the string
        int             m_Length    ; // The length of the string.
        int             m_Position  ; // The current position. From 0 to m_Length-1, or m_Length for end of iteration.
        
    };
    //------------------------------------------------------------------------
    // iterator implementation
    //------------------------------------------------------------------------
    class iterator : public const_iterator
    {
      public:
        iterator() { }
        iterator & operator ++ ()                           { m_Position = Min( m_Position+1 , m_Length ); return *this; }
        iterator & operator ++ (int)                        { m_Position = Min( m_Position+1 , m_Length ); return *this; }
        iterator & operator += (int n)                      { m_Position = Min( m_Position+n , m_Length ); return *this; }
        iterator & operator -- ()                           { m_Position = Max( m_Position-1 ,        0 ); return *this; }
        iterator & operator -- (int)                        { m_Position = Max( m_Position-1 ,        0 ); return *this; }
        iterator & operator -= (int n)                      { m_Position = Max( m_Position-n ,        0 ); return *this; }
        iterator   operator +  (int n) const                { return iterator( const_cast<char *>(m_Data) , m_Length , Min( m_Position+n , m_Length ) ); }
        iterator   operator -  (int n) const                { return iterator( const_cast<char *>(m_Data) , m_Length , Max( m_Position-n ,        0 ) ); }
        char & operator * () { assert( m_Position < m_Length ); return const_cast<char &>( m_Data[m_Position] ); }
        int operator - ( const iterator & rhs ) const { assert(m_Data==rhs.m_Data); return m_Position - rhs.m_Position ; }

      private:

        friend class cstring ;
        iterator( char * Data , int Length , int Position = 0 ) 
        {  
            m_Data      = Data      ;
            m_Length    = Length    ;
            m_Position  = Position  ; 
        }
        
    };

    //------------------------------------------------------------------------
    // const_reverse_iterator implementation
    //------------------------------------------------------------------------
    class const_reverse_iterator
    {
      public:
        const_reverse_iterator() { }
        const_reverse_iterator & operator ++ ()                           { m_Position = Max( m_Position-1 ,        0 ); return *this; }
        const_reverse_iterator & operator ++ (int)                        { m_Position = Max( m_Position-1 ,        0 ); return *this; }
        const_reverse_iterator & operator += (int n)                      { m_Position = Max( m_Position-n ,        0 ); return *this; }
        const_reverse_iterator & operator -- ()                           { m_Position = Min( m_Position+1 , m_Length ); return *this; }
        const_reverse_iterator & operator -- (int)                        { m_Position = Min( m_Position+1 , m_Length ); return *this; }
        const_reverse_iterator & operator -= (int n)                      { m_Position = Min( m_Position+n , m_Length ); return *this; }
        const_reverse_iterator   operator +  (int n) const                { return const_reverse_iterator( m_Data , m_Length , Max( m_Position-n ,        0 ) ); }
        const_reverse_iterator   operator -  (int n) const                { return const_reverse_iterator( m_Data , m_Length , Min( m_Position+n , m_Length ) ); }
        bool                     operator == (const const_reverse_iterator & rhs) const { return m_Position == rhs.m_Position ; }
        bool                     operator != (const const_reverse_iterator & rhs) const { return m_Position != rhs.m_Position ; }
        const char &             operator *  ()                           { assert( m_Position > 0 ); return m_Data[m_Position-1]; }

      protected :

        friend class cstring ;
        const_reverse_iterator( const char * Data , int Length , int Position ) 
        {  
            m_Data      = Data      ;
            m_Length    = Length    ;
            m_Position  = Position  ; 
        }
        const char *    m_Data      ; // The beginning of the string
        int             m_Length    ; // The length of the string.
        int             m_Position  ; // The position 1 beyond the current object.  From 0 to m_Length, or 0 for end of iteration.
    };

    //------------------------------------------------------------------------
    // reverse_iterator implementation
    //------------------------------------------------------------------------
    class reverse_iterator : public const_reverse_iterator
    {
      public:
        reverse_iterator() { }
        reverse_iterator & operator ++ ()                           { m_Position = Max( m_Position-1 ,        0 ); return *this; }
        reverse_iterator & operator ++ (int)                        { m_Position = Max( m_Position-1 ,        0 ); return *this; }
        reverse_iterator & operator += (int n)                      { m_Position = Max( m_Position-n ,        0 ); return *this; }
        reverse_iterator & operator -- ()                           { m_Position = Min( m_Position+1 , m_Length ); return *this; }
        reverse_iterator & operator -- (int)                        { m_Position = Min( m_Position+1 , m_Length ); return *this; }
        reverse_iterator & operator -= (int n)                      { m_Position = Min( m_Position+n , m_Length ); return *this; }
        reverse_iterator   operator +  (int n) const                { return reverse_iterator( const_cast<char *>(m_Data) , m_Length , Max( m_Position-n ,        0 ) ); }
        reverse_iterator   operator -  (int n) const                { return reverse_iterator( const_cast<char *>(m_Data) , m_Length , Min( m_Position+n , m_Length ) ); }
        char & operator * () { assert( m_Position > 0 ); return const_cast<char &>( m_Data[m_Position-1] ); }

      private:

        friend class cstring ;
        reverse_iterator( char * Data , int Length , int Position ) 
        {  
            m_Data      = Data      ;
            m_Length    = Length    ;
            m_Position  = Position  ; 
        }
    };

    //------------------------------------------------------------------------
    // Debug
    //------------------------------------------------------------------------
    static void test_all(); // Do a self-test of this class. [Non-standard function not defined in basic_string]

private :

    //------------------------------------------------------------------------
    // Beware: Herein you will find private things
    //------------------------------------------------------------------------

    enum { MaxSize = 2000000000 } ; // Arbitrary but large value.

    char * m_Data ; // A pointer to the string data, or 0 if not allocated. 

    // The prefix is allocated immediately before the data.
    struct CPrefix
    {
        int     Length   ; // The length of the string.
        int     Capacity ; // The capacity of the string. There is room to hold a string of this size plus the trailing null.
    };
    const CPrefix &     Prefix() const      { return ( reinterpret_cast<const CPrefix *>(m_Data) )[-1]; }
    CPrefix &           Prefix()            { return ( reinterpret_cast<      CPrefix *>(m_Data) )[-1]; }
    
    // Allocate new cstring data with the given capacity. 
    // Return the pointer to the data, not the prefix.
    // The prefix will have the specified Capacity and a length of 0.
    static char * New( int Capacity ) ; 

    static void Delete( char * Data ); // Delete the data allocated with New().

    // Add Length characters to the string from Source.
    void Append( const char * Source , int Length );

    // Insert Length characters from Source into *this at position pos.
    // Source can be 0, in which case there is an uninitialized gap.
    void Insert( int pos , const char * Source , int Length );

    // Replace up to n0 characters in *this at position pos with n 
    // characters from source. If Source==0, a gap is left to be
    // filled in. Returns a pointer to the start of the text that
    // was (or is to be) replaced.
    char * Replace( int pos , int n0 , const char * Source , int n );


    // Compare two character sequences of known length. Return
    // an integer with the usual "compare" meaning (<0, ==0, >0).
    static int Compare( const char * lhs , int lhsLength , const char * rhs , int rhsLength );

    // Compare a character sequence of known length with a null-terminated
    // string. Return an integer with the usual "compare" meaning (<0, ==0, >0).
    static int Compare( const char * lhs , int lhsLength , const char * rhs );

    // Erase up to n characters from *this at position pos.
    void Erase( int pos , int n );

    // Make sure the cstring has a capacity of at least MinCapacity.
    // Note: Destroys the contents and sets length()=0
    // Use reserve() if you need to preserve the contents.
    void Reallocate( int MinCapacity );

    // Deallocate *this and set  m_Data == m_EmptyData.
    void Delete(); 

    struct CEmptyString
    {
        CPrefix m_Prefix    ;
        char    m_Data[2]   ; // Room for "" with trailing null.
    };
    static CEmptyString     m_EmptyString   ;
    static char *           m_EmptyData     ; // The data reserved for the empty string. Points to m_EmptyString.m_Data

};

#endif
