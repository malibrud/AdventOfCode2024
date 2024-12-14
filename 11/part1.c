#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

bool tryGetStonesFromFile( char *inFileName, uint64_t **outStones, int *outCount );
uint64_t countStones( uint64_t stone, int blinks );
void check( bool success, char *format, ... );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    uint64_t *stones;
    int N;
    check( tryGetStonesFromFile( argv[ 1 ], &stones, &N ), "Error: Could not read %s.", argv[ 1 ] );

    uint64_t count = 0;
    for ( int i = 0 ; i < N ; i++ ) 
        count += countStones( stones[ i ], 25 );

    printf( "%lld\n", count );
    return 0;
}

int digits( uint64_t val );
uint64_t pow10( int pow );

uint64_t countStones( uint64_t stone, int blinks ) {
    if ( blinks == 0 ) return 1;
    if ( stone == 0 ) return countStones( 1, blinks - 1 );
    int d = digits( stone );
    if ( d % 2 == 0 ) {
        uint64_t mod = pow10( d / 2 );
        return 
            countStones( stone / mod, blinks - 1) + 
            countStones( stone % mod, blinks - 1)
        ;
    }
    return countStones( stone * 2024, blinks - 1 );
}

uint64_t pow10( int pow ) {
    uint64_t ans = 1;
    for ( int i = 0 ; i < pow ; i++ ) ans *= 10;
    return ans;
}

int digits( uint64_t val ) {
    const uint64_t kilo = 1000ull;
    const uint64_t mega = 1000ull * kilo;
    const uint64_t giga = 1000ull * mega;
    const uint64_t tera = 1000ull * giga;

    if ( val < 10   ) return 1;
    if ( val < 100  ) return 2;
    if ( val < 1000 ) return 3;

    if ( val < 10   * kilo ) return 4;
    if ( val < 100  * kilo ) return 5;
    if ( val < 1000 * kilo ) return 6;

    if ( val < 10   * mega ) return 7;
    if ( val < 100  * mega ) return 8;
    if ( val < 1000 * mega ) return 9;

    if ( val < 10   * giga ) return 10;
    if ( val < 100  * giga ) return 11;
    if ( val < 1000 * giga ) return 12;

    if ( val < 10   * tera ) return 13;
    if ( val < 100  * tera ) return 14;
    if ( val < 1000 * tera ) return 15;

    assert( false );
    return 0;
}

bool tryGetStonesFromFile( char *inFileName, uint64_t **outStones, int *outCount ) {
    FILE *f = fopen( inFileName, "r" );
    if ( f == NULL ) return false;

    char line[ 40 ];
    check( fgets( line, 40, f ), "Error: Could not read from %s", inFileName );

    int spaceCount = 0;
    char *cur = line;
    while ( *cur != '\0' ) if ( *cur++ == ' ' ) spaceCount++;
    *outCount = spaceCount + 1;
    *outStones = malloc( *outCount * sizeof( uint64_t ) );
    check ( *outStones, "Error: allocation failure." );

    cur = line;
    uint64_t *stones = *outStones;
    for ( int i = 0 ; i < *outCount ; i++ ) {
        stones[ i ] = atoi( cur );
        while( *cur != ' ' && *cur != '\0' ) cur++;
        cur++;
    }

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