#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

void check( bool success, char *format, ... );
#define MAX_SEEDS 2080
typedef struct {
    int seeds[ MAX_SEEDS ];
    int S;  // Number of seeds.
} Numbers;

bool tryGetDataFromFile( char *inFileName, Numbers *outNumbers );

int prngNext( int seed ) {
    uint64_t val = seed;

    // Step 1
    val ^= val * 64;            // Multiply and Mix
    val %= 16777216;            // Prune

    // Step 2
    val ^= val / 32;            // Divide and Mix
    val %= 16777216;            // Prune

    // Step 3
    val ^= val * 2048;          // Multiply and Mix
    val %= 16777216;            // Prune

    return (int)val;
}

#define MAX_SIGNATURE ( 19 * 19 *19 * 19 )
int8_t diffs[ MAX_SEEDS ][ MAX_SIGNATURE ];

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Numbers n = {};
    check( tryGetDataFromFile( argv[ 1 ], &n ), "Error: Could not read data from %s.", argv[ 1 ] );

    for ( int i = 0 ; i < n.S           ; i++ )
    for ( int j = 0 ; j < MAX_SIGNATURE ; j++ )
    {
        diffs[ i ][ j ] = -1;
    }

    int d0 = 0;
    int d1 = 0;
    int d2 = 0;
    int d3 = 0;
    for ( int i = 0 ; i < n.S ; i++ ) {
        int prev = n.seeds[ i ];
        int val;
        for ( int j = 0 ; j < 2000 ; j++ ) {
            val = prngNext( prev );
            d3 = d2;
            d2 = d1;
            d1 = d0;
            d0 = val % 10 - prev % 10 + 9;
            int idx = (19*19*19)*d3 + (19*19)*d2 + (19)*d1 + d0;
            assert( idx >= 0 );
            assert( idx <  MAX_SIGNATURE );
            if ( diffs[ i ][ idx ] == -1 && j >= 3 ) diffs[ i ][ idx ] = val % 10; 
            prev = val;
        }
    }

    int maxBananas = 0;
    for ( int i = 0 ; i < MAX_SIGNATURE ; i++ ) {

        int bananas = 0;
        for ( int j = 0 ; j < n.S ; j++ ) {
            if ( diffs[ j ][ i ] != -1 ) 
            bananas += diffs[ j ][ i ];
        }
        maxBananas = max( maxBananas, bananas );
    }
    printf( "%d\n", maxBananas );
    return 0;
}

bool tryGetDataFromFile( char *inFileName, Numbers *outNumbers ) {
    Numbers *n = outNumbers;
    FILE *f = fopen( inFileName, "r" );
    check( f, "Error: Could not open %s", inFileName );

    int seed;
    n->S = 0;
    while( 1 == fscanf( f, "%d\n", &seed ) ) {
        assert( n->S < MAX_SEEDS );
        n->seeds[ n->S++ ] = seed;
    }

    fclose( f );
    return true;
}

#define ERR_RET_VAL 1
void check( bool success, char *format, ... ) {
    if ( success ) return;
    va_list args;
    va_start( args, format );
    vprintf( format, args );
    va_end( args ); 
    printf("\n");
    exit( ERR_RET_VAL );
}