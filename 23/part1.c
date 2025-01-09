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
    char **pairs;
    int P;  // Number of pairs.
} Computers;

bool tryGetDataFromFile( char *inFileName, Computers *outComputers );

#define D (26*26)
uint8_t graph[ D ][ D ] = {}; // connectivity graph

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Computers c = {};
    check( tryGetDataFromFile( argv[ 1 ], &c ), "Error: Could not read data from %s.", argv[ 1 ] );
    int P = c.P;

    for (size_t i = 0; i < P; i++)
    {
        char *pair = c.pairs[ i ];
        int c1 = (pair[ 0 ] - 'a')*26 + ( pair[ 1 ] - 'a' );
        int c2 = (pair[ 3 ] - 'a')*26 + ( pair[ 4 ] - 'a' );
        // only populate the upper triangle
        if ( c1 < c2) graph[ c1 ][ c2 ] = 1;
        else          graph[ c2 ][ c1 ] = 1;
    }

    int count = 0;
    for (int i = 0; i < D; i++)
    {
        char c11 = (char)( i / 26 + 'a' );
        char c12 = (char)( i % 26 + 'a' );
        for (int j = i+1; j < D; j++)
        {
            if ( graph[ i ][ j ] == 0 ) continue;
            char c21 = (char)( j / 26 + 'a' );
            char c22 = (char)( j % 26 + 'a' );
            for (int k = j+1; k < D; k++)
            {
                if ( graph[ j ][ k ] == 0 ) continue;
                if ( graph[ i ][ k ] == 0 ) continue;;
                char c31 = (char)( k / 26 + 'a' );
                char c32 = (char)( k % 26 + 'a' );
                bool containsT = c11 == 't' || c21 == 't' || c31 == 't';
                if ( containsT ) {
                    count++;
                    printf( "%c%c-%c%c-%c%c\n", c11, c12, c21, c22, c31, c32 );
                }
            }
        }
    }
    
    printf( "%d\n", count );
    return 0;
}

bool tryGetDataFromFile( char *inFileName, Computers *outComputers ) {
    FILE *f = fopen( inFileName, "rb" );
    check( f, "Error: Could not open %s", inFileName );

    // Get the number of rows
    fseek( f, 0, SEEK_END );
    size_t B = ftell( f );
    fseek( f, 0, SEEK_SET );
    int stride = (int)strlen( "aa-bb\r\n" );
    int P = (int)( B + 2 ) / stride;

    char *data = malloc( B + 1 );
    char **pairs = malloc( P * sizeof( char* ) );
    check( data && pairs, "Error: Malloc failed." );

    check( fread( data, 1, B, f ), "Error: fread() failed." );
    fclose( f );
    data[ B ] = '\0';

    for (size_t i = 0; i < P; i++)
    {
        pairs[ i ] = data + i*stride;
        pairs[ i ][ 5 ] = '\0';
    }

    outComputers->pairs = pairs;
    outComputers->P     = P;
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