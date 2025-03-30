#ifndef X_STD_STRING_H
#define X_STD_STRING_H

///////////////////////////////////////////////////////////////////////////////
//
// String-related declarations based on STL <string>
//
// Defines the class string and various supporting templates. A string is a 
// variable-length sequence of characters, with some accomodation for treating 
// the sequence like the usual null-terminated C/C++ string.
// 
// Warning: Unlike conventional null-terminated strings, an x_std::string 
// object can contain have null characters (0x00). The length of the string is 
// not affected by this. For example, changing X[3] = 0 does not affect 
// X.length(). An arbitrary sequence of characters can be appended to a string 
// - that sequence can include null characters. Thus, a string object can be 
// used as a general dynamically-sized data buffer.
//
///////////////////////////////////////////////////////////////////////////////
//  
// Overview of basic functions for string:
//   
//      x_std::string X ;    // Declare a string object.
//      x_std::string Y ;    // Declare a string object.
//      X.length()           // The length of the string.
//      X.reserve(100)       // Does not change the length of X, but reserves enough capcity for at least 100 characters.
//                           // (A string must be reallocated if it grows beyond its capacity).
//      X[0]                 // The first character in the string.
//      X[10]                // The 11'th character in the string.
//      X.c_str()            // A null-terminated C-string suitable for use as "const char *".
//                           // The result of c_str() is only valid as long as X exists and is not changed.
//      X.resize(12,' ')     // Change the length of X to 12, padding with blanks if needed.
//      X.empty()            // Is the string empty?
//
//   Catenation
//
//      X += Y ;             // Add a string to the end of X.
//      X += ".txt" ;        // Add a string to the end of X.
//      X += '.' ;           // Add a character to the end of X.
//      X.append(Y);         // Append a string to the end of X.
//      X.append(Y,3,4);     // Append a substring of Y (from index 3 for 4 characters) to the end of X.
//      X.append("abc");     // Append a null-terminated string to the end of X.
//      X.append(S,3);       // Append 3 characters from string S, including any null characters, to the end of X.
//      X.append(10,'.');    // Append 10 characters '.' to the end of X.
//      X.append(first,last);// Append a sequence of characters in an iterator range [first,last).
//
//   Insertion
//
//      X.insert(5,Y);       // Insert a copy of Y into X starting at X[5].     
//      X.insert(4,Y,3,8);   // Insert the 8-character substring of Y starting at Y[3] into X starting at X[4].     
//      X.insert(10,S);      // Insert a copy of the null-terminated string S into X starting at X[10].     
//      X.insert(2,S,14);    // Insert a copy of the 14 characters at string S, including any null characters, into X starting at X[2].     
//      X.insert(7,13,'x');  // Insert 13 characters 'x' into X starting at X[7].
//      X.insert(it,'!');    // Insert a single character '!' into X at iterator it.
//      X.insert(it,5,'!');  // Insert 5 characters '!' into X at iterator it.
//      X.insert(it,first,last); // Insert the characters in iterator range [first,last) into X at iterator it.
//
//   Deletion
//
//      X.erase(5,10);       // Erase the 10 characters starting at X[5].
//      X.erase(13);         // Erase the rest of the string starting at (and including) X[13].
//      X.erase(it);         // Erase the character at iterator it.
//      X.erase(first,last); // Erase the substring in X represented by the iterator range [first,last).
//
//   Replacement
//                              // Replace the 8 characters at X[5] with ...
//      X.replace(5,8,Y);       // ... a copy of Y.
//      X.replace(5,8,Y,4,2);   // ... the 2 characters starting at Y[4].
//      X.replace(5,8,S);       // ... the contents of a null-terminated string, not including the null.
//      X.replace(5,8,S,3);     // ... the 3 characters at S, including any null characters.
//      X.replace(5,8,4,' ');   // ... the 4 characters ' '.
//
//                              // Replace the contents of X in the iterator range [a,b) with ...
//      X.replace(a,b,Y);       // ... a copy of Y.
//      X.replace(a,b,S);       // ... a copy of the null-terminated string S, not including the null.
//      X.replace(a,b,S,100);   // ... 100 characters from S, including any null characters.
//      X.replace(a,b,2,'.');   // ... the 2 characters '.'.
//      X.replace(a,b,c,d);     // ... characters from iterator range [c,d).
//
//   Substrings (very inefficient)
//
//      X.substr(10)            // A new string with the contents from X[5] to the end of X.
//      X.substr(10,13)         // A new string with the contents of the 13 characters starting at X[10].
//      X.x_left(13)            // A new string with (up to) the first 13 characters of X. Same as X.substr(0,5).
//      X.x_right(13)           // A new string with (up to) the last 13 characters of X. Same as X.substr(X.size()-13), if X.size()>=13.
//
//   Comparison
//
//      Compare the first sequence (either the whole string X or a sequence from X) with
//      the second sequence defined by the function arguments. Return a negative value, zero,
//      or a positive value (respectively) if the first sequence is less than, equal, or
//      greater than the second sequence.
//
//      X.compare(Y)            // Compare X to Y.
//      X.compare(4,8,Y)        // Compare the 8 characters starting at X[4] to Y.
//      X.compare(4,8,Y,5,9)    // Compare the 8 characters starting at X[4] to the 9 characters starting at Y[9].
//      X.compare(S)            // Compare X to the null-terminated string S, not including S's trailing null.
//      X.compare(4,8,S)        // Compare the 8 characters starting at X[4] to the null-terminated string S, not including S's trailing null.
//      X.compare(4,8,S,5)      // Compare the 8 characters starting at X[4] to the 5 characters in string S (including nulls).
//
//   Searching 
//
//      // Return the position (or npos if not found) of the first occurrence in X of ...
//      X.find('?')             // ... '?'.
//      X.find('?',10)          // ... '?' at or after X[10].
//      X.find(S)               // ... null-terminated S, not including the null.
//      X.find(S,10)            // ... null-terminated S at or after X[10], not including the null. 
//      X.find(S,10,5)          // ... the 5 characters (including nulls) from S at or after X[10].
//      X.find(Y)               // ... Y.
//      X.find(Y,10)            // ... Y at or after X[10].
//
//      // Return the position (or npos if not found) of the last occurrence in X of ...
//      X.rfind('?')            // ... '?'.
//      X.rfind('?',10)         // ... '?' at or before X[10].
//      X.rfind(S)              // ... null-terminated S, not including the null.
//      X.rfind(S,10)           // ... null-terminated S at or before X[10], not including the null. 
//      X.rfind(S,10,5)         // ... the 5 characters (including nulls) from S at or before X[10].
//      X.rfind(Y)              // ... Y.
//      X.rfind(Y,10)           // ... Y at or before X[10].
//
//      // Return the position (or npos if not found) of the first character in X which is ...
//      X.find_first_of('?')    // ... '?'. Same as X.find('?').
//      X.find_first_of('?',10) // ... '?' and at or after X[10]. Same as X.find('?',10).
//      X.find_first_of(S)      // ... also in null-terminated S (excluding nulls).
//      X.find_first_of(S,10)   // ... also in null-terminated S (excluding nulls) and at or after X[10].
//      X.find_first_of(S,10,5) // ... also one of the 5 characters from S (including nulls) and at or after X[10].
//      X.find_first_of(Y)      // ... also one of the characters in Y (including nulls).
//      X.find_first_of(Y,10)   // ... also one of the characters in Y (including nulls) and at or after X[10].
//
//      // Return the position (or npos if not found) of the last character in X which is ...
//      X.find_last_of('?')     // ... '?'. Same as X.rfind('?').
//      X.find_last_of('?',10)  // ... '?' and at or after X[10]. Same as X.rfind('?',10).
//      X.find_last_of(S)       // ... also in null-terminated S (excluding nulls).
//      X.find_last_of(S,10)    // ... also in null-terminated S (excluding nulls) and at or before X[10].
//      X.find_last_of(S,10,5)  // ... also one of the 5 characters from S (including nulls) and at or before X[10].
//      X.find_last_of(Y)       // ... also one of the characters in Y (including nulls).
//      X.find_last_of(Y,10)    // ... also one of the characters in Y (including nulls) and at or before X[10].
//
//      // Return the position (or npos if not found) of the first character in X which is ...
//      X.find_first_not_of('?')    // ... not '?'.
//      X.find_first_not_of('?',10) // ... not '?' and at or after X[10].
//      X.find_first_not_of(S)      // ... null or not in null-terminated S.
//      X.find_first_not_of(S,10)   // ... null or not in null-terminated S and at or after X[10].
//      X.find_first_not_of(S,10,5) // ... not one of the 5 characters from S (including nulls) and at or after X[10].
//      X.find_first_not_of(Y)      // ... not one of the characters in Y (including nulls).
//      X.find_first_not_of(Y,10)   // ... not one of the characters in Y (including nulls) and at or after X[10].
//
//      // Return the position (or npos if not found) of the last character in X which is ...
//      X.find_last_not_of('?')    // ... not '?'.
//      X.find_last_not_of('?',10) // ... not '?' and at or before X[10].
//      X.find_last_not_of(S)      // ... null or not in null-terminated S.
//      X.find_last_not_of(S,10)   // ... null or not in null-terminated S and at or before X[10].
//      X.find_last_not_of(S,10,5) // ... not one of the 5 characters from S (including nulls) and at or before X[10].
//      X.find_last_not_of(Y)      // ... not one of the characters in Y (including nulls).
//      X.find_last_not_of(Y,10)   // ... not one of the characters in Y (including nulls) and at or before X[10].
//
//   Miscellaneous (non-standard extensions) 
//
//      X.x_to_upper();          // Convert X to uppercase.
//      X.x_to_lower();          // Convert X to lowercase.
//      X.x_reverse();           // Reverse the contents of the string.
//      X.x_trim();              // Remove all whitespace characters from the beginning and end of X.
//      X.x_trim_left();         // Remove all whitespace characters from the beginning of X.
//      X.x_trim_right();        // Remove all whitespace characters from the end of X.
//      X.x_trim("zab");         // Remove all characters in "zab" from the front and end of X.
//      X.x_trim_left("zab");    // Remove from the front of X any characters in "zab".
//      X.x_trim_right("zab");   // Remove from the end of X any characters in "zab".
//      X.x_trim_left(5);        // Remove up to 5 characters from the left of X.
//      X.x_trim_right(5);       // Remove up to 5 characters from the right of X.
//      X.x_format( const char * Format , ... ); // Change X using a printf-style format string Format.
//
///////////////////////////////////////////////////////////////////////////////
//
// Deviation from standard STL behavior
//
// The extremely general string support for arbitrary character types is not
// supported. Specifically, there is no implementation for the following:
//
//    template<class E,class T,class Allocator> class basic_string;
//    template<class E> struct char_traits;
//    struct char_traits<char>;
//    struct char_traits<wchar_t>;
//    typedef basic_string>wchar_t> wstring;
//    basic_ostream<E>& operator<<
//    basic_istream<E>& operator>>
//    basic_istream<E,T> & getline
//    basic_istream<E,T> & getline
//
// Note specifically that there is no basic_string<> template, and string does
// not provide the ability to specify an allocator. This affects the following
// string functions, which do not have the standard allocator parameter:
//
//    basic_string(const Allocator & a);
//    basic_string(const basic_string& rhs);
//    basic_string(const basic_string& rhs, size_type pos, size_type n, const Allocator& al);
//    basic_string(const E *s, size_type n, const Allocator& al);
//    basic_string(const E *s, const Allocator& al );
//    basic_string(size_type n, E c, const Allocator& al ;
//    basic_string(const_iterator first, const_iterator last, const Allocator& al);
//
///////////////////////////////////////////////////////////////////////////////
//
// Performance
//
// length() is fast :
//     No scanning for null is needed (strings can contain imbedded nulls).
//
// c_str() is fast :
//     In this implementation, a null terminator is always kept at the end
//     of the data, even though it is not counted in length(). Therefore, no
//     copying of data is needed when c_str() is called.
//
// There is additional storage allocated to hold the length and capacity of the 
// string. In additional, a small amount of additional capacity is reserved 
// during string operations (about 10 characters) so that small changes to a 
// string's length can be accomodated without reallocating the string.
//
// The time-complexity of each function is described near its declaration as 
// [O(f)], or [O(f1),O(f2)] where f, f1, and f2 are functions of the arguments 
// and *this. If two functions are given, f1 describes the case where the 
// string does not require reallocation and f2 describes the case where 
// reallocation is needed.
//
//    O(size())  The time depends on the size of the array.
//    O(n)       The time depends on the parameter n.
//    O(it2-it1) The time depends on the number of elements in the iterator range [it1,it2).
//
// If the complexity is omitted, it can be assumed to be O(1).
//
///////////////////////////////////////////////////////////////////////////////

#include    "x_std.h"
#include    "x_memory.h"

namespace x_std 
{

    ///////////////////////////////////////////////////////////////////////////////
    //  
    // class string
    //
    ///////////////////////////////////////////////////////////////////////////////
    class string 
    {
      public:
        //---------------------------------------------------------------------
        // Administrative types
        //---------------------------------------------------------------------
        // Note that this class is not based on basic_string<> and does not have
        // an allocator. That is why some of the types below are left undefined.

        typedef void                                            traits_type         ;
        typedef void                                            allocator_type      ;
        typedef char                                            char_type           ;
        typedef x_std::x_size_t                                 size_type           ;
        typedef x_std::allocator<char_type>::difference_type    difference_type     ;
        typedef char_type *                                     pointer             ;
        typedef const char_type *                               const_pointer       ;
        typedef char_type &                                     reference           ;
        typedef const char_type &                               const_reference     ;
        typedef char_type                                       value_type          ;

        // npos: The special value which can never represent a valid length of or 
        // position in a string. Uses include a return value from find functions 
        // to represent "not found".
        static const size_type npos ; 

        class iterator                ;
        class const_iterator          ;
        class reverse_iterator        ;
        class const_reverse_iterator  ;

        //------------------------------------------------------------------------
        // Construction
        //------------------------------------------------------------------------
        // When constructing from a string, the capacity of the source string is 
        // not used by the target string.

        string()                                             ; // Construct a string with "".
        string( const string & rhs                          ); // Construct a string with a copy of rhs. [O(rhs.length())]
        string( const char *   s                            ); // Construct a string with a copy of null-terminated s. [O(strlen(s))]
        string( size_type      n     , char           c     ); // Construct a string out of n characters c. [O(n)]
        string( const char *   s     , size_type      n     ); // Construct a string from n characters at s, including nulls. [O(n)]
        string( const_iterator first , const_iterator last  ); // Construct a string from the sequence [first,last) in another string. [O(last-first)]
        string( const string & rhs   , size_type pos , size_type n  ); // Construct a string from up to n characters of rhs at position pos. pos < rhs.length(). [O(n)]
        ~string();


        //------------------------------------------------------------------------
        // Assignment
        //------------------------------------------------------------------------
        // When copying from a string, the capacity of the source string is not
        // used in the target string. 

        string & operator = ( const string & rhs    ); // Copy rhs into *this. [O(rhs.length())]
        string & operator = ( const char *   s      ); // Copy null-terminated s into *this. [O(strlen(s))]
        string & operator = ( char c                ); // Make *this a string with a single character c.

        string & assign( const string & str                         ); // Copy str into *this. [O(str.length())]
        string & assign( const char *   s                           ); // Copy null-terminated s into *this. [O(strlen(s))]
        string & assign( const char *   s     , size_type n         ); // Copy the n characters at s, including nulls, into *this. [O(n)]
        string & assign( size_type      n     , char c              ); // Copy n characters c into *this. [O(n)]
        string & assign( const_iterator first , const_iterator last ); // Copy the sequence [first,last) from another string into *this. [O(last-first)]
        string & assign( const string & str   , size_type pos , size_type n ); // Copy up to n characters from str[pos] into this. pos < str.length() [O(n)]

        //------------------------------------------------------------------------
        // Iterators
        //------------------------------------------------------------------------
        // To iterate over string S:
        //    for( string::iterator X = S.begin()  ; X != S.end()  ; ++X ) { /* Use *X */ }
        //    for( string::iterator X = S.rbegin() ; X != S.rend() ; ++X ) { /* Use *X */ }
        // You can also use indexing:
        //    for( size_type pos = 0 ; pos < S.length() ; ++pos ) { /* Use S[pos] */ }

        iterator                begin  ()       ; // The start of forward iteration (the first character in the string).
        const_iterator          begin  () const ; // The start of forward iteration (the first character in the string).
        iterator                end    ()       ; // The end of forward iteration (beyond the last character in the string).
        const_iterator          end    () const ; // The end of forward iteration (beyond the last character in the string).
        reverse_iterator        rbegin ()       ; // The start of reverse iteration (last character in the string).
        const_reverse_iterator  rbegin () const ; // The start of reverse iteration (last character in the string).
        reverse_iterator        rend   ()       ; // The end of reverse iteration (beyond the first character in the string).
        const_reverse_iterator  rend   () const ; // The end of reverse iteration (beyond the first character in the string).

        //------------------------------------------------------------------------
        // Indexing operations
        //------------------------------------------------------------------------
        // WARNING: 
        //   1. If you set at(pos) = 0, you will NOT change the length of the 
        //      string. Use resize(pos) to resize.

        // These all return the pos'th character in the string. pos must be in [0,length()).
        const char &    at         ( size_type pos ) const   ; 
        char &          at         ( size_type pos )         ; 
        const char &    operator[] ( size_type pos ) const   ; 
        char &          operator[] ( size_type pos )         ; 

        //------------------------------------------------------------------------
        // Safe access to underlying data
        //------------------------------------------------------------------------

        //operator const char * () const { return m_Data ; } // Conversion to (const char *). [Non-standard extension not in template basic_string]
        const char *    c_str () const ;  // Return a pointer to a null-terminated string. Never returns 0.
        const char *    data  () const ; 

        // The non-standard function x_c_str0() is like c_str() except it returns 0 if the string is empty.
        const char *    x_c_str0() const ;

        //------------------------------------------------------------------------
        // Length, size, capacity, and so on
        //------------------------------------------------------------------------

        size_type   length   () const                       ; // How long is the string?
        size_type   size     () const                       ; // How long is the string? Same as length().
        size_type   max_size () const                       ; // The maximum length a string can have.
        void        resize   ( size_type n , char c = 0 )   ; // Change the current length, padding with c if necessary. [O(n),O(n+length())]
        size_type   capacity () const                       ; // What is the current capacity? If the length grows beyond this, reallocation is needed.
        void        reserve  ( size_type n = 0 )            ; // Make sure the current capacity is at least n. [O(1),O(length())]
        bool        empty    () const                       ; // Is the string empty?

        //------------------------------------------------------------------------
        // Catenation
        //------------------------------------------------------------------------
        // Let n be the length being added (rhs.length(), str.length(), 
        // parameter n, strlen(s), 1 [when adding a single character], or last-first). 
        // Then the time complexity is:
        //     O(n)             : If string reallocation is *not* needed.
        //     O(n+length())    : If string reallocation *is* needed.

        string & operator += ( const string & rhs ); // Append rhs to *this.
        string & operator += ( const char *   s   ); // Append null-terminated s to *this.
        string & operator += ( char           c   ); // Append character c to *this.
        string & append( const string & str                 ); // Append str to *this.
        string & append( const char *   s                   ); // Append null-terminated s to *this.
        string & append( const char *   s     , size_type n ); // Append n characters at s (including nulls) to *this.
        string & append( size_type      n     , char c      ); // Append n characters c to *this.
        string & append( const_iterator first , const_iterator last ); // Append [first,last) to *this.
        string & append( const string & str   , size_type pos , size_type n ); // Append up to n characters at str[pos], including nulls, to *this.

        //------------------------------------------------------------------------
        // Insertion
        //------------------------------------------------------------------------
        // Let n be the length being added (str.length(), parameter n, strlen(s), 
        // 1 [when adding a single character], or last-first). Let q be the 
        // length of the remaining string starting at position p0 or iterator 
        // it (that is, q = length()-p0 or end()-it).
        // Then the time complexity for any insert() function is:
        //     O(n+q)           : If string reallocation is *not* needed.
        //     O(n+length())    : If string reallocation *is* needed.

        string & insert( size_type p0 , const string & str );
        string & insert( size_type p0 , const string & str , size_type pos , size_type n ); 
        string & insert( size_type p0 , const char * s , size_type n );
        string & insert( size_type p0 , const char * s );
        string & insert( size_type p0 , size_type n , char c );
        iterator  insert( iterator it , char c );   // Insert c before it, and return an iterator to the newly inserted element.
        void      insert( iterator it , size_type n , char c);
        void      insert( iterator it , const_iterator first , const_iterator last ); // Insert the sequence [first,last) from ANOTHER string.
    
        //------------------------------------------------------------------------
        // Replacing and erasing parts of strings
        //------------------------------------------------------------------------
        // Let n be the length being added (str.length(), parameter n, strlen(s), 
        // last-first). Let q be the length of the remaining string starting at 
        // position p0+n0, iterator it, or iterator last0.
        // Then the time complexity of any replace(...) function below is:
        //     O(n+q)           : If string reallocation is *not* needed.
        //     O(n+length())    : If string reallocation *is* needed.

        string & erase  ( size_type p0 = 0 , size_type n = npos ); // [O(n)]
        iterator erase  ( iterator it ); // [O(end()-it)]
        iterator erase  ( iterator first , iterator last ); // [O(end()-last)]
        string & replace( size_type p0 , size_type n0 , const string & str );
        string & replace( size_type p0 , size_type n0 , const string & str , size_type pos , size_type n ); 
        string & replace( size_type p0 , size_type n0 , const char * s , size_type n );
        string & replace( size_type p0 , size_type n0 , const char * s );
        string & replace( size_type p0 , size_type n0 , size_type n , char c );
        string & replace( iterator first0 , iterator last0 , const string & str );
        string & replace( iterator first0 , iterator last0 , const char * s , size_type n);
        string & replace( iterator first0 , iterator last0 , const char *s );
        string & replace( iterator first0 , iterator last0 , size_type n , char c );
        string & replace( iterator first0 , iterator last0 , const_iterator first , const_iterator last); 
    
        //------------------------------------------------------------------------
        // Copy and swap
        //------------------------------------------------------------------------

        size_type copy( char * s , size_type n , size_type pos = 0 ) const; // Note: Does not add a null terminator. // [O(n)]
        void swap( string & str );

        //------------------------------------------------------------------------
        // Searching
        //------------------------------------------------------------------------
        // Let n be the number of characters you are looking for (parameter n, 
        // 1 [for a character search], str.length(), or strlen(s)). The L be 
        // the length of the string you are looking in (length(), pos 
        // [for limited reverse searches], or length()-pos [for limited forward 
        // searches]).
        // The time complexity for each of the following find functions is:
        //       O(n*L)

        size_type find( const string & str , size_type pos = 0 ) const; 
        size_type find( const char * s , size_type pos , size_type n ) const;
        size_type find( const char * s , size_type pos = 0 ) const;
        size_type find( char c , size_type pos = 0 ) const;

        size_type rfind( const string & str , size_type pos = npos ) const;
        size_type rfind( const char * s , size_type pos , size_type n ) const;
        size_type rfind( const char * s , size_type pos = npos ) const;
        size_type rfind( char c , size_type pos = npos ) const;

        size_type find_first_of( const string & str , size_type pos = 0 ) const;
        size_type find_first_of( const char * s , size_type pos , size_type n ) const;
        size_type find_first_of( const char * s , size_type pos = 0 ) const;
        size_type find_first_of( char c , size_type pos = 0 ) const ;

        size_type find_last_of( const string & str , size_type pos = npos ) const;
        size_type find_last_of( const char * s , size_type pos ,  size_type n ) const;
        size_type find_last_of( const char * s , size_type pos = npos ) const;
        size_type find_last_of( char c , size_type pos = npos ) const ;

        size_type find_first_not_of( const string & str , size_type pos = 0 ) const;
        size_type find_first_not_of( const char * s , size_type pos , size_type n ) const;
        size_type find_first_not_of( const char * s , size_type pos = 0 ) const;
        size_type find_first_not_of( char c , size_type pos = 0 ) const;

        size_type find_last_not_of( const string & str , size_type pos = npos ) const;
        size_type find_last_not_of( const char * s , size_type pos , size_type n ) const;
        size_type find_last_not_of( const char * s , size_type pos = npos ) const;
        size_type find_last_not_of( char c , size_type pos = npos ) const;

        //------------------------------------------------------------------------
        // Substring
        //------------------------------------------------------------------------
        string substr( size_type pos = 0 , size_type n = npos ) const; // [O(n)]

        //------------------------------------------------------------------------
        // Comparison
        //------------------------------------------------------------------------
        // Let n be the length being compare (str.length(), parameter n, or strlen(s)).
        // Let L be the length of *this being compared (length() or n0).
        // Then the time complexity for any compare() function is:
        //     O( ::min(n,L) )

        int compare( const string & str ) const;
        int compare( size_type p0 , size_type n0 , const string & str ) const;
        int compare( size_type p0 , size_type n0 , const string & str , size_type pos , size_type n ) const;
        int compare( const char * s ) const;
        int compare( size_type p0 , size_type n0 , const char * s ) const;
        int compare( size_type p0 , size_type n0 , const char * s , size_type n ) const;

        //------------------------------------------------------------------------
        // Miscellaneous
        //------------------------------------------------------------------------
        allocator<char> get_allocator() const ;

        //------------------------------------------------------------------------
        // Non-standard, unsafe access to the underlying data
        //------------------------------------------------------------------------
        // These functions are unsafe and are provided for cases where 
        // efficiency demands access to the underlying data, such as allocating 
        // a large string and reading the contents of a file directly into the 
        // string. 

        char *          x_data   ()              ; // Return a pointer to the underlying data. Make sure you first reserve() enough space for your needs.
        void            x_resize ( size_type n ) ; // Unsafe version of resize(). Does not extend-fill: the string contents are unchanged except for adding null termination ([n]=0).

        //------------------------------------------------------------------------
        // Other non-standard functions
        //------------------------------------------------------------------------
        // Note: Whitespace characters are: 
        //     0x09(tab), 0x0a(line feed), 0x0b(vertical tab), 
        //     0x0c(form-feed), 0x0d(carriage return), 
        //     and 0x20(space)
        void   x_to_upper   () ; // Convert entire string to uppercase. [O(length())]
        void   x_to_lower   () ; // Convert entire string to lowercase. [O(length())]
        void   x_reverse    () ; // Reverse the contents of the string. [O(length())]
        void   x_trim       () ; // Remove all whitespace characters from the beginning and end of this. [O(length())]
        void   x_trim_left  () ; // Remove all whitespace characters from the beginning of this. [O(length())]
        void   x_trim_right () ; // Remove all whitespace characters from the end of this. [O(length())]
        void   x_trim       ( const char * targets ) ; // Remove from the front and end of *this any characters in *targets. [O(length()*strlen(targets))]
        void   x_trim_left  ( const char * targets ) ; // Remove from the front of *this any characters in *targets. [O(length()*strlen(targets))]
        void   x_trim_right ( const char * targets ) ; // Remove from the end of *this any characters in *targets. [O(length()*strlen(targets))]
        void   x_trim_left  ( size_type n ) ; // Remove up to n characters from the left of this. [O(length()-n)]
        void   x_trim_right ( size_type n ) ; // Remove up to n characters from the right of this. [O(n)]

        // x_left() and x_right() are frighteningly inefficient because they
        // return string values, which requires the construction, copying,
        // and destruction of temporary objects.
        string x_left       ( size_type n ) const; // The first n characters of this, or all of this if n >= length(). Very inefficient. [O(n)]
        string x_right      ( size_type n ) const; // The last n characters of this, or all of this if n >= length(). Very inefficient. [O(n)]

        string & x_format( const char * Format , ... ); // [O(..unknown..)]
            // Use the printf-style Format string to fill in *this.
            // NOTE: The resulting string must be limited to <= 2000 characters,
            // excluding the trailing null, otherwise results are unpredictable.
            // Because a temporary buffer is used, the string itself can be used
            // in the argument list, such as in: S.format( "--%s--" , S.c_str() );


        //------------------------------------------------------------------------
        // const_iterator implementation
        //------------------------------------------------------------------------
        class const_iterator
        {
          public:
            const_iterator() { }
            const_iterator( iterator x )                     { m_Data = x.m_Data ; }
            const_iterator & operator ++ ()                  { m_Data += 1 ; return *this ; }
            const_iterator & operator -- ()                  { m_Data -= 1 ; return *this ; }
            const_iterator & operator += (size_type n)       { m_Data += n ; return *this ; }
            const_iterator & operator -= (size_type n)       { m_Data -= n ; return *this ; }
            const_iterator   operator +  (size_type n) const { return m_Data + n ; }
            const_iterator   operator -  (size_type n) const { return m_Data - n ; }
            const_iterator   operator ++ (int)               { return m_Data++ ; }
            const_iterator   operator -- (int)               { return m_Data--; }
            const char &     operator *  ()      const       { return m_Data[0]; }
            const char &     operator [] (x_size_t n) const  { return m_Data[n]; }
            bool             operator == (const const_iterator & rhs) const { return m_Data == rhs.m_Data ; }
            bool             operator != (const const_iterator & rhs) const { return m_Data != rhs.m_Data ; }

          protected :

            friend class string ;
            const_iterator( const char * Data ) : m_Data(Data) { }
            const char * m_Data; // The pointer to the character at the current iterator's position.
        
        };
        //------------------------------------------------------------------------
        // iterator implementation
        //------------------------------------------------------------------------
        class iterator 
        {
          public:
            iterator() { }
            iterator & operator ++ ()                   { m_Data += 1 ; return *this ; }
            iterator & operator -- ()                   { m_Data -= 1 ; return *this ; }
            iterator & operator += (size_type n)        { m_Data += n ; return *this ; }
            iterator & operator -= (size_type n)        { m_Data -= n ; return *this ; }
            iterator   operator +  (size_type n) const  { return m_Data + n ; }
            iterator   operator -  (size_type n) const  { return m_Data - n ; }
            iterator   operator ++ (int)                { return m_Data++; }
            iterator   operator -- (int)                { return m_Data--; }
            char &     operator *  ()            const  { return m_Data[0]; }
            char &     operator [] (size_type n) const  { return m_Data[n]; }
            bool       operator == (const iterator & rhs) const { return m_Data == rhs.m_Data ; }
            bool       operator != (const iterator & rhs) const { return m_Data != rhs.m_Data ; }

          private:

            friend class string ;
            friend class const_iterator ;
            iterator( char * Data ) : m_Data(Data) { }
            char * m_Data; // The pointer to the character at the current iterator's position.

        };

        //------------------------------------------------------------------------
        // const_reverse_iterator implementation
        //------------------------------------------------------------------------
        class const_reverse_iterator
        {
          public:
            const_reverse_iterator() { }
            const_reverse_iterator( reverse_iterator x )               { m_Data = x.m_Data ; }
            const_reverse_iterator & operator ++ ()                  { m_Data -= 1 ; return *this ; }
            const_reverse_iterator & operator -- ()                  { m_Data += 1 ; return *this ; }
            const_reverse_iterator & operator += (size_type n)       { m_Data -= n ; return *this ; }
            const_reverse_iterator & operator -= (size_type n)       { m_Data += n ; return *this ; }
            const_reverse_iterator   operator +  (size_type n) const { return m_Data - n ; }
            const_reverse_iterator   operator -  (size_type n) const { return m_Data + n ; } 
            const_reverse_iterator   operator ++ (int)               { return m_Data-- ; }
            const_reverse_iterator   operator -- (int)               { return m_Data++ ; }
            const char &             operator *  ()          const   { return m_Data[-1  ]; }
            const char &             operator [] (x_size_t n) const  { return m_Data[-x_int(n)-1]; }
            bool                     operator == (const const_reverse_iterator & rhs) const { return m_Data == rhs.m_Data ; }
            bool                     operator != (const const_reverse_iterator & rhs) const { return m_Data != rhs.m_Data ; }

          protected :

            friend class string ;
            const_reverse_iterator( const char * Data ) : m_Data(Data) { }
            const char * m_Data; // The pointer to ONE BEYOND the character at the current iterator's position.
        };

        //------------------------------------------------------------------------
        // reverse_iterator implementation
        //------------------------------------------------------------------------
        class reverse_iterator 
        {
          public:
            reverse_iterator() { }
            reverse_iterator & operator ++ ()                   { m_Data -= 1 ; return *this ; }
            reverse_iterator & operator -- ()                   { m_Data += 1 ; return *this ; }
            reverse_iterator & operator += (size_type n)        { m_Data -= n ; return *this ; }
            reverse_iterator & operator -= (size_type n)        { m_Data += n ; return *this ; }
            reverse_iterator   operator +  (size_type n) const  { return m_Data - n ; }
            reverse_iterator   operator -  (size_type n) const  { return m_Data + n ; }
            reverse_iterator   operator ++ (int)                { return m_Data-- ; }
            reverse_iterator   operator -- (int)                { return m_Data++ ; }
            char &             operator *  ()            const  { return m_Data[  -1 ]; }
            char &             operator [] (size_type n) const  { return m_Data[-x_int(n)-1 ]; }
            bool               operator == (const reverse_iterator & rhs) const { return m_Data == rhs.m_Data ; }
            bool               operator != (const reverse_iterator & rhs) const { return m_Data != rhs.m_Data ; }

          private:

            friend class string ;
            friend class const_reverse_iterator ;
            reverse_iterator( char * Data ) : m_Data(Data) { }
            char * m_Data; // The pointer to ONE BEYOND the character at the current iterator's position.
        };

    private :

        //------------------------------------------------------------------------
        // Beware: Herein you will find private things
        //------------------------------------------------------------------------

        enum { MaxSize = 1000000000 } ; // Arbitrary but large value.

        char * m_Data ; // A pointer to the string data, or 0 if not allocated. 

        // The prefix is allocated immediately before the data.
        struct CPrefix
        {
            size_type   Length   ; // The length of the string.
            size_type   Capacity ; // The capacity of the string. There is room to hold a string of this size plus the trailing null.
        };
        const CPrefix &     Prefix() const      { return ( reinterpret_cast<const CPrefix *>(m_Data) )[-1]; }
        CPrefix &           Prefix()            { return ( reinterpret_cast<      CPrefix *>(m_Data) )[-1]; }
    
        // Allocate new string data with the given capacity. 
        // Return the pointer to the data, not the prefix.
        // The prefix will have the specified Capacity and a length of 0.
        static char * New( size_type Capacity ) ; 

        static void Delete( char * Data ); // Delete the data allocated with New().

        // Add Length characters to the string from Source.
        void Append( const char * Source , size_type Length );

        // Insert Length characters from Source into *this at position pos.
        // Source can be 0, in which case there is an uninitialized gap.
        void Insert( size_type pos , const char * Source , size_type Length );

        // Replace up to n0 characters in *this at position pos with n 
        // characters from source. If Source==0, a gap is left to be
        // filled in. Returns a pointer to the start of the text that
        // was (or is to be) replaced.
        char * Replace( size_type pos , size_type n0 , const char * Source , size_type n );


        // Compare two character sequences of known length. Return
        // an integer with the usual "compare" meaning (<0, ==0, >0).
        static size_type Compare( const char * lhs , size_type lhsLength , const char * rhs , size_type rhsLength );

        // Compare a character sequence of known length with a null-terminated
        // string. Return an integer with the usual "compare" meaning (<0, ==0, >0).
        static size_type Compare( const char * lhs , size_type lhsLength , const char * rhs );

        // Erase up to n characters from *this at position pos.
        void Erase( size_type pos , size_type n );

        // Make sure the string has a capacity of at least MinCapacity.
        // Note: Destroys the contents and sets length()=0
        // Use reserve() if you need to preserve the contents.
        void Reallocate( size_type MinCapacity );

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

    ///////////////////////////////////////////////////////////////////////////
    //  
    // template functions for string operations
    //
    // WARNING:
    //   Be very careful about using the '+' operator. It is very inefficient, and
    //   simple statements like this:
    //
    //       string1 = string2 + string3 + string4
    //
    //   can result in a suprisingly large number of temporaries, copies, 
    //   allocations, and deallocations.
    //
    ///////////////////////////////////////////////////////////////////////////

           string operator +  ( const string & lhs , const string & rhs ) ;
           string operator +  ( const string & lhs , const char *   rhs ) ;
           string operator +  ( const string & lhs , char           rhs ) ;
           string operator +  ( const char *   lhs , const string & rhs ) ;
           string operator +  ( char           lhs , const string & rhs ) ;
    inline bool   operator == ( const string & lhs , const string & rhs ) { return lhs.compare(rhs) == 0 ; }
    inline bool   operator == ( const string & lhs , const char *   rhs ) { return lhs.compare(rhs) == 0 ; }
    inline bool   operator == ( const char *   lhs , const string & rhs ) { return rhs.compare(lhs) == 0 ; }
    inline bool   operator != ( const string & lhs , const string & rhs ) { return lhs.compare(rhs) != 0 ; }
    inline bool   operator != ( const string & lhs , const char *   rhs ) { return lhs.compare(rhs) != 0 ; }
    inline bool   operator != ( const char *   lhs , const string & rhs ) { return rhs.compare(lhs) != 0 ; }
    inline bool   operator <  ( const string & lhs , const string & rhs ) { return lhs.compare(rhs) <  0 ; }
    inline bool   operator <  ( const string & lhs , const char *   rhs ) { return lhs.compare(rhs) <  0 ; }
    inline bool   operator <  ( const char *   lhs , const string & rhs ) { return rhs.compare(lhs) >  0 ; }
    inline bool   operator >  ( const string & lhs , const string & rhs ) { return lhs.compare(rhs) >  0 ; }
    inline bool   operator >  ( const string & lhs , const char *   rhs ) { return lhs.compare(rhs) >  0 ; }
    inline bool   operator >  ( const char *   lhs , const string & rhs ) { return rhs.compare(lhs) <  0 ; }
    inline bool   operator <= ( const string & lhs , const string & rhs ) { return lhs.compare(rhs) <= 0 ; }
    inline bool   operator <= ( const string & lhs , const char *   rhs ) { return lhs.compare(rhs) <= 0 ; } 
    inline bool   operator <= ( const char *   lhs , const string & rhs ) { return rhs.compare(lhs) >= 0 ; } 
    inline bool   operator >= ( const string & lhs , const string & rhs ) { return lhs.compare(rhs) >= 0 ; }
    inline bool   operator >= ( const string & lhs , const char *   rhs ) { return lhs.compare(rhs) >= 0 ; } 
    inline bool   operator >= ( const char *   lhs , const string & rhs ) { return rhs.compare(lhs) <= 0 ; } 
    inline void   swap        ( string &       lhs , string &       rhs ) { lhs.swap(rhs) ; }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Implementation details
    //
    ///////////////////////////////////////////////////////////////////////////

    inline string::string() { m_Data = m_EmptyData; } 
    inline string::~string() { if(  m_Data != m_EmptyData ) { Delete(); } }
    inline string &                         string::assign( const string & str ) { return *this = str ; }
    inline string &                         string::assign( const char * s )     { return *this = s ; }
    inline string::iterator                 string::begin  ()       { return iterator               ( m_Data          );  }
    inline string::const_iterator           string::begin  () const { return const_iterator         ( m_Data          );  }
    inline string::iterator                 string::end    ()       { return iterator               ( m_Data + size() );  }
    inline string::const_iterator           string::end    () const { return const_iterator         ( m_Data + size() );  }
    inline string::reverse_iterator         string::rbegin ()       { return reverse_iterator       ( m_Data + size() );  }
    inline string::const_reverse_iterator   string::rbegin () const { return const_reverse_iterator ( m_Data + size() );  }
    inline string::reverse_iterator         string::rend   ()       { return reverse_iterator       ( m_Data          );  }
    inline string::const_reverse_iterator   string::rend   () const { return const_reverse_iterator ( m_Data          );  }
    inline const char &                     string::at         ( size_type pos ) const   { X_STD_ASSERT( pos >= 0 && pos < length() ); return m_Data[pos]; }
    inline char &                           string::at         ( size_type pos )         { X_STD_ASSERT( pos >= 0 && pos < length() ); return m_Data[pos]; }
    inline const char &                     string::operator[] ( size_type pos ) const   { X_STD_ASSERT( pos >= 0 && pos < length() ); return m_Data[pos]; }
    inline char &                           string::operator[] ( size_type pos )         { X_STD_ASSERT( pos >= 0 && pos < length() ); return m_Data[pos]; }
    inline const char *                     string::c_str () const { return m_Data ; } 
    inline const char *                     string::data  () const { return m_Data ; }
    inline const char *                     string::x_c_str0() const { return m_Data[0] == 0 ? 0 : m_Data; } 
    inline string::size_type                string::length   () const                   { return Prefix().Length ; }
    inline string::size_type                string::size     () const                   { return Prefix().Length ; }
    inline string::size_type                string::max_size () const                   { return MaxSize ; }
    inline string::size_type                string::capacity () const                   { return Prefix().Capacity; }
    inline bool                             string::empty    () const                   { return length() == 0; }
    inline string &                         string::append( const string & str )     { return *this += str; }
    inline string &                         string::append( const char * s )          { return *this += s  ; }
    inline string::size_type                string::find_first_of( char c , size_type pos ) const { return find(c,pos); }
    inline string::size_type                string::find_last_of( char c , size_type pos ) const { return rfind(c,pos); }
    inline allocator<char>                  string::get_allocator() const { return allocator<char>(); }
}

#endif
