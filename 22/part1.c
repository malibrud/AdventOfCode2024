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
    uint64_t seeds[ MAX_SEEDS ];
    int S;  // Number of seeds.
} Numbers;

bool tryGetDataFromFile( char *inFileName, Numbers *outNumbers );

uint64_t prngNext( uint64_t seed ) {
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

    return val;
}

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Numbers n = {};
    check( tryGetDataFromFile( argv[ 1 ], &n ), "Error: Could not read data from %s.", argv[ 1 ] );

    uint64_t sum = 0;
    for ( int i = 0 ; i < n.S ; i++ ) {
        uint64_t val = n.seeds[ i ];
        for ( int j = 0 ; j < 2000 ; j++ ) val = prngNext( val );
        sum += val;
    }
    printf( "%llu\n", sum );
    return 0;
}

bool tryGetDataFromFile( char *inFileName, Numbers *outNumbers ) {
    Numbers *n = outNumbers;
    FILE *f = fopen( inFileName, "r" );
    check( f, "Error: Could not open %s", inFileName );

    uint64_t seed;
    n->S = 0;
    while( 1 == fscanf( f, "%llu\n", &seed ) ) {
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