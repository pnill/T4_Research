#include "x_math.hpp"
#include "x_plus.hpp"
#include "x_types.hpp"
#include "x_color.hpp"
#include "x_debug.hpp"
#include "x_files.hpp"
#include "x_stats.hpp"
#include "x_stdio.hpp"
#include "x_memory.hpp"


struct thing
{
    byte    Data[10];
    thing( void  ) { Data[0] = 2; }
    thing( int i ) { Data[0] = i; }
};


int main( void )
{
    char*  p1;
    thing* p2;
    thing* p3;

    vector3 A( 1, 2, 3 );
    vector3 B( 4, 5, 6 );
    f32 C;

    x_Init();
    
    C = Dot( A, B );

    x_DumpHeapInfo( "Heap00.txt" );
    
    p1 = new char[10];
    p2 = new thing[10];
    p3 = new thing( 5 );

    x_DumpHeapInfo( "Heap01.txt" );

    delete( p1 );
    delete( p2 );
    delete( p3 );

    x_DumpHeapInfo( "Heap02.txt" );

    s32 j = 0;
    for( s32 i = 0; i < 100000; i++ )
    {
        if( i == 95000 )
        {
            x_printf( "%d\n", j++ );
            i = 0;
        }
    }

    x_printf( "%d", C );

    x_Kill();

    return( 0 );
}
