#ifndef X_STD_STATISTICS_H
#define X_STD_STATISTICS_H

///////////////////////////////////////////////////////////////////////////////
//
// Statistics for x_std and related classes and templates
//
// This information helps you analyze certain performance characteristics of 
// your application. Although the information is not formally supported and is 
// subject to change at any time, it is also expected that users of x_std can 
// look at the information (to find how many slow allocations are being done, 
// for example) and to change the information (to add statistics for their own 
// allocators, for example).
//
// This information is only available when x_std has been compiled to gather
// statistics - see the explanation for X_STD_KEEP_STATISTICS in x_std.h.
// In other cases, the statistics will be meaningless.
//
///////////////////////////////////////////////////////////////////////////////


// This is pending approval by other reviewers and will be unceremoniously yanked 
// if such approval is not granted.

namespace x_std_statistics
{

    //-----------------------------------------------------------------------------
    // Print all the statistics
    //-----------------------------------------------------------------------------
    void print
    (
        void (*print_function)( const char * output_line) // Call this function for each line in the resulting output.
    );

    //-----------------------------------------------------------------------------
    // allocator<> statistics
    //-----------------------------------------------------------------------------

    // These are only to be updated by x_std::allocator<T>.
    extern long    calls_to_base_allocate         ; // Number of calls to x_std::allocator<T>::allocate()
    extern long    calls_to_base_deallocate       ; // Number of calls to x_std::allocator<T>::deallocate()
    extern long    calls_to_base_allocate_char    ; // Number of calls to x_std::allocator<T>::x_allocate_char()
    extern long    calls_to_base_deallocate_char  ; // Number of calls to x_std::allocator<T>::x_deallocate_char()
    extern long    size_of_base_allocate          ; // Total bytes allocated by x_std::allocator<T>::allocate()
    extern long    size_of_base_deallocate        ; // Total bytes deallocated by x_std::allocator<T>::deallocate()
    extern long    size_of_base_allocate_char     ; // Total bytes allocated by x_std::allocator<T>::x_allocate_char()
    extern long    size_of_base_deallocate_char   ; // Total bytes deallocated by x_std::allocator<T>::x_deallocate_char()

    // Computed values:
    long base_allocations               () ; // Total calls to x_std::allocator<T>::allocate() and x_std::allocator<T>::x_allocate_char().
    long base_deallocations             () ; // Total calls to x_std::allocator<T>::deallocate() and x_std::allocator<T>::x_deallocate_char().
    long size_of_base_allocations       () ; // Total bytes allocated by all calls to x_std::allocator<T>::allocate() and x_std::allocator<T>::x_allocate_char().
    long size_of_base_deallocations     () ; // Total bytes deallocated by all calls to x_std::allocator<T>::deallocate() and x_std::allocator<T>::x_deallocate_char().
    long base_net_allocations           () ; // Net number of calls to x_std::allocator<T>::allocate() and x_std::allocator<T>::x_allocate_char() without corresponding calls to deallocate() or x_deallocate_char()
    long size_of_base_net_allocations   () ; // Net undeallocated storage in allocations counted by base_net_allocations().

    // These are to be updated by all allocators.
    extern long    calls_to_all_allocate          ; // Number of calls to any allocator function: allocate()
    extern long    calls_to_all_deallocate        ; // Number of calls to any allocator function: deallocate()
    extern long    calls_to_all_allocate_char     ; // Number of calls to any allocator function: x_allocate_char()
    extern long    calls_to_all_deallocate_char   ; // Number of calls to any allocator function: x_deallocate_char()
    extern long    size_of_all_allocate           ; // Total bytes allocated by any allocator function: allocate()
    extern long    size_of_all_deallocate         ; // Total bytes deallocated by any allocator function: deallocate()
    extern long    size_of_all_allocate_char      ; // Total bytes allocated by any allocator function: x_allocate_char()
    extern long    size_of_all_deallocate_char    ; // Total bytes deallocated by any allocator function: x_deallocate_char()
    extern long    size_of_all_fast_allocations   ; // Total bytes returned by "fast" allocations done by any allocator (fast allocations are taken from reserved pools or lists).
    extern long    size_of_all_allocator_reserves ; // Total bytes kept by allocators in reserve pools or lists, memory which is not currently allocated but also not yet returned to the free store.

    // Computed values:
    long all_allocations                () ; // Total calls to x_std::allocator<T>::allocate() and x_std::allocator<T>::x_allocate_char().
    long all_deallocations              () ; // Total calls to x_std::allocator<T>::deallocate() and x_std::allocator<T>::x_deallocate_char().
    long size_of_all_allocations        () ; // Total bytes allocated by all calls to x_std::allocator<T>::allocate() and x_std::allocator<T>::x_allocate_char().
    long size_of_all_deallocations      () ; // Total bytes deallocated by all calls to x_std::allocator<T>::deallocate() and x_std::allocator<T>::x_deallocate_char().
    long all_net_allocations            () ; // Net number of calls to x_std::allocator<T>::allocate() and x_std::allocator<T>::x_allocate_char() without corresponding calls to deallocate() or x_deallocate_char()
    long size_of_all_net_allocations    () ; // Net undeallocated storage in allocations counted by all_net_allocations().


    //-----------------------------------------------------------------------------
    // list<> statistics   //todo: Not working yet.
    //-----------------------------------------------------------------------------
    
    extern long         list_insertions             ; // How many elements have been added to all lists. Includes construction, insertion, push_front(), push_back(), resize(), and so on.
    extern long         list_removals               ; // How many elements have been removed from all lists. Includes destruction, erase(), pop_front(), pop_back(), and so on.
    extern long         size_of_all_list_elements   ; // Total bytes currently used to store list elements, not including the list overhead (such as link pointers).
    extern long         size_of_all_list_storage    ; // Total bytes used in all lists, including any overhead.
    long list_elements       () ; // How many elements are currently allocated in lists.


    //-----------------------------------------------------------------------------
    // vector<> statistics  //todo: Not working yet.
    //-----------------------------------------------------------------------------
    extern long vector_insertions                   ; // How many elements have been added to all vectors. Includes construction, insertion, push_front(), push_back(), resize(), and so on.
    extern long vector_removals                     ; // How many elements have been removed from all vectors. Includes destruction, erasing, pop_front(), resize(), and so on.
    extern long vector_reallocations                ; // How many vector reallocations have been done.
    extern long vector_reallocations                ; // How many vector reallocations have been done.
    extern long size_of_all_vector_shifting         ; // How many bytes have been moved to manage insertion or removal into a vector without reallocation.

    //-----------------------------------------------------------------------------
    // string statistics  //todo: Not working yet.
    //-----------------------------------------------------------------------------

}

#endif
