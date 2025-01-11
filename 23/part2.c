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
int graph[ D ][ D ]; // connectivity graph

typedef struct {
    int *nodes;
    int count;
    int N;
} BkSet;

void bkInit( BkSet *bk, int N ) {
    check( bk->nodes = malloc( N * sizeof( int ) ), "Error: Out of Memory" );
    bk->N = N;
    bk->count = 0;
}

void bkFree( BkSet *bk ) {
    assert( bk );
    free( bk->nodes );
    bk->N = 0;
    bk->count = 0;
}

void bkPush( BkSet *bk, int val ) {
    assert( bk->count < bk->N );
    bk->nodes[ bk->count++ ] = val;
}

int bkPop( BkSet *bk ) {
    return bk->nodes[ --bk->count ];
}

int bkClear( BkSet *bk ) {
    return bk->count = 0;
}

int bkPrint( BkSet *bk ) {
    int n = bk->nodes[ 0 ];
    int c1 = n / 26 + 'a';
    int c2 = n % 26 + 'a';
    printf( "%c%c", c1, c2 );
    for (size_t i = 1; i < bk->count; i++) {
        n = bk->nodes[ i ];
        c1 = n / 26 + 'a';
        c2 = n % 26 + 'a';
        printf( ",%c%c", c1, c2 );
    }
}

void bkRemoveAt( BkSet *bk, size_t i ) {
    size_t bytes = ( bk->count - i - 1 ) * sizeof( int );
    memmove( bk->nodes + i, bk->nodes + i + 1, bytes );
    bk->count--;
}

void bkCopy( BkSet *dst, BkSet *src ) {
    *dst = *src;
    dst->nodes = malloc( src->N * sizeof( int ) );
    memcpy( dst->nodes, src->nodes, src->count * sizeof( int ) );
}

void bronKerbosch( BkSet R, BkSet P, BkSet X ) {
    if ( P.count == 0 && X.count == 0 ) {
        bkPrint( &R );
    }

    BkSet newR;
    BkSet newP;
    BkSet newX;

    bkCopy( &newR, &R );
    bkInit( &newP, D );
    bkInit( &newX, D );

    for (int i = P.count-1; i >= 0; i++)
    {
        int v = P.nodes[ i ];
        bkPush( &newR, v );

        for (size_t j = 0; j < P.count; j++) {
            int p = P.nodes[ j ];
            if ( graph[ v ][ p ] ) bkPush( &newP, p );
        }
        
        for (size_t j = 0; j < X.count; j++) {
            int x = X.nodes[ j ];
            if ( graph[ v ][ x ] ) bkPush( &newX, x );
        }

        // Recurse
        bronKerbosch( newR, newP, newX );

        // Move node from P to X
        bkPop( &P );
        bkPush( &X, v );

        // Reset working sets for next loop
        bkPop( &newR );
        bkClear( &newP );
        bkClear( &newX );
    }
    

}
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Computers c = {};
    check( tryGetDataFromFile( argv[ 1 ], &c ), "Error: Could not read data from %s.", argv[ 1 ] );
    int P = c.P;

    int neighborCount[ D ] = {};
    for (size_t i = 0; i < P; i++)
    {
        char *pair = c.pairs[ i ];
        int c1 = (pair[ 0 ] - 'a')*26 + ( pair[ 1 ] - 'a' );
        int c2 = (pair[ 3 ] - 'a')*26 + ( pair[ 4 ] - 'a' );
        graph[ c1 ][ c2 ] = 1;
        graph[ c2 ][ c1 ] = 1;
        neighborCount[ c1 ]++;
        neighborCount[ c2 ]++;
    }
    int maxNeighbor = 0;
    for (size_t i = 0; i < D; i++)
    {
        maxNeighbor = max( maxNeighbor, neighborCount[ i ] );
    }
    int maxCount = 0;
    for (size_t i = 0; i < D; i++)
    {
        if ( neighborCount[ i ] == maxNeighbor ) maxCount++;
    }
    printf( "max neighbor = %d, max neighbor count = %d\n", maxNeighbor, maxCount );
    

    int count = 0;
    for (int i = 0; i < D; i++)
    {
        for (int j = i+1; j < D; j++)
        {
            if ( graph[ i ][ j ] == 0 ) continue;
            for (int k = j+1; k < D; k++)
            {
                if ( graph[ i ][ k ] == 0 ) continue;
                if ( graph[ j ][ k ] == 0 ) continue;
                for (int m = k+1; m < D; m++)
                {
                    if ( graph[ i ][ m ] == 0 ) continue;
                    if ( graph[ j ][ m ] == 0 ) continue;
                    if ( graph[ k ][ m ] == 0 ) continue;
                    for (int n = m+1; n < D; n++)
                    {
                        if ( graph[ i ][ n ] == 0 ) continue;
                        if ( graph[ j ][ n ] == 0 ) continue;
                        if ( graph[ k ][ n ] == 0 ) continue;
                        if ( graph[ m ][ n ] == 0 ) continue;
                        for (int p = n+1; p < D; p++)
                        {
                            if ( graph[ i ][ p ] == 0 ) continue;
                            if ( graph[ j ][ p ] == 0 ) continue;
                            if ( graph[ k ][ p ] == 0 ) continue;
                            if ( graph[ m ][ p ] == 0 ) continue;
                            if ( graph[ n ][ p ] == 0 ) continue;
                            count++;
                        }
                    }
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