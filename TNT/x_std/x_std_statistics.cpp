///////////////////////////////////////////////////////////////////////////////
//
// Statistics for x_std and related classes and templates
//
///////////////////////////////////////////////////////////////////////////////

#include "x_std.h"
#include <stdio.h>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
long x_std_statistics :: calls_to_base_allocate               =   0   ;
long x_std_statistics :: calls_to_base_deallocate             =   0   ;
long x_std_statistics :: calls_to_base_allocate_char          =   0   ;
long x_std_statistics :: calls_to_base_deallocate_char        =   0   ;
long x_std_statistics :: size_of_base_allocate                =   0   ;
long x_std_statistics :: size_of_base_deallocate              =   0   ;
long x_std_statistics :: size_of_base_allocate_char           =   0   ;
long x_std_statistics :: size_of_base_deallocate_char         =   0   ;

long x_std_statistics :: base_allocations               () { return calls_to_base_allocate     +   calls_to_base_allocate_char   ; }
long x_std_statistics :: base_deallocations             () { return calls_to_base_deallocate   +   calls_to_base_deallocate_char ; }
long x_std_statistics :: size_of_base_allocations       () { return size_of_base_allocate      +   size_of_base_allocate_char    ; }
long x_std_statistics :: size_of_base_deallocations     () { return size_of_base_deallocate    +   size_of_base_deallocate_char  ; }
long x_std_statistics :: base_net_allocations           () { return base_allocations()         -   base_deallocations()          ; } 
long x_std_statistics :: size_of_base_net_allocations   () { return size_of_base_allocations() -   size_of_base_deallocations()  ; }

long x_std_statistics :: calls_to_all_allocate                =   0   ;
long x_std_statistics :: calls_to_all_deallocate              =   0   ;
long x_std_statistics :: calls_to_all_allocate_char           =   0   ;
long x_std_statistics :: calls_to_all_deallocate_char         =   0   ;
long x_std_statistics :: size_of_all_allocate                 =   0   ;
long x_std_statistics :: size_of_all_deallocate               =   0   ;
long x_std_statistics :: size_of_all_allocate_char            =   0   ;
long x_std_statistics :: size_of_all_deallocate_char          =   0   ;
long x_std_statistics :: size_of_all_fast_allocations         =   0   ;
long x_std_statistics :: size_of_all_allocator_reserves       =   0   ;

long x_std_statistics :: all_allocations                () { return calls_to_all_allocate      +   calls_to_all_allocate_char    ; }
long x_std_statistics :: all_deallocations              () { return calls_to_all_deallocate    +   calls_to_all_deallocate_char  ; }
long x_std_statistics :: size_of_all_allocations        () { return size_of_all_allocate       +   size_of_all_allocate_char     ; }
long x_std_statistics :: size_of_all_deallocations      () { return size_of_all_deallocate     +   size_of_all_deallocate_char   ; }
long x_std_statistics :: all_net_allocations            () { return all_allocations()          -   all_deallocations()           ; } 
long x_std_statistics :: size_of_all_net_allocations    () { return size_of_all_allocations()  -   size_of_all_deallocations()   ; }


//-----------------------------------------------------------------------------
// Print all the statistics
//-----------------------------------------------------------------------------
void x_std_statistics::print
(
    void (*print_function)( const char * output_line) // Call this function for each line in the resulting output.
)
{
    const int max_line_length = 100 ;
    char output_line[ max_line_length + 1 ]; // +1 for trailing null.

    sprintf( output_line , "%s: %10li" , "calls_to_base_allocate        " , calls_to_base_allocate                 ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "calls_to_base_deallocate      " , calls_to_base_deallocate               ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "calls_to_base_allocate_char   " , calls_to_base_allocate_char            ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "calls_to_base_deallocate_char " , calls_to_base_deallocate_char          ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_base_allocate         " , size_of_base_allocate                  ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_base_deallocate       " , size_of_base_deallocate                ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_base_allocate_char    " , size_of_base_allocate_char             ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_base_deallocate_char  " , size_of_base_deallocate_char           ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "base_allocations              " , base_allocations                    () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "base_deallocations            " , base_deallocations                  () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_base_allocations      " , size_of_base_allocations            () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_base_deallocations    " , size_of_base_deallocations          () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "base_net_allocations          " , base_net_allocations                () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_base_net_allocations  " , size_of_base_net_allocations        () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "calls_to_all_allocate         " , calls_to_all_allocate                  ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "calls_to_all_deallocate       " , calls_to_all_deallocate                ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "calls_to_all_allocate_char    " , calls_to_all_allocate_char             ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "calls_to_all_deallocate_char  " , calls_to_all_deallocate_char           ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_allocate          " , size_of_all_allocate                   ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_deallocate        " , size_of_all_deallocate                 ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_allocate_char     " , size_of_all_allocate_char              ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_deallocate_char   " , size_of_all_deallocate_char            ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_fast_allocations  " , size_of_all_fast_allocations           ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_allocator_reserves" , size_of_all_allocator_reserves         ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "all_allocations               " , all_allocations                     () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "all_deallocations             " , all_deallocations                   () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_allocations       " , size_of_all_allocations             () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_deallocations     " , size_of_all_deallocations           () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "all_net_allocations           " , all_net_allocations                 () ); print_function(output_line); 
    sprintf( output_line , "%s: %10li" , "size_of_all_net_allocations   " , size_of_all_net_allocations         () ); print_function(output_line); 

}
