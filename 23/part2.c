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

void bkPrint( BkSet *bk ) {
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
    putchar( '\n' );
}

void bkNewCopy( BkSet *dst, BkSet *src ) {
    *dst = *src;
    dst->nodes = malloc( src->N * sizeof( int ) );
    memcpy( dst->nodes, src->nodes, src->count * sizeof( int ) );
}

void bkCopy( BkSet *dst, BkSet *src ) {
    assert( dst->N >= src->N );
    memcpy( dst->nodes, src->nodes, src->count * sizeof( int ) );
    dst->count = src->count;
}

void bronKerbosch( BkSet R, BkSet P, BkSet X, BkSet *M ) {
    if ( P.count == 0 && X.count == 0 ) {
        if ( R.count > M->count ) bkCopy( M, &R );
    }

    BkSet newR;
    BkSet newP;
    BkSet newX;

    bkNewCopy( &newR, &R );
    bkInit( &newP, D );
    bkInit( &newX, D );

    for ( int i = P.count-1; i >= 0; i-- )
    {
        int v = P.nodes[ i ];
        bkPush( &newR, v );

        // newP <-- P ∩ N( v )
        for (size_t j = 0; j < P.count; j++) {
            int p = P.nodes[ j ];
            if ( graph[ v ][ p ] ) bkPush( &newP, p );
        }
        
        // newX <-- X ∩ INT N( v )
        for (size_t j = 0; j < X.count; j++) {
            int x = X.nodes[ j ];
            if ( graph[ v ][ x ] ) bkPush( &newX, x );
        }

        // Recurse
        bronKerbosch( newR, newP, newX, M );

        // P <-- P \ v
        // X <-- X U v
        bkPop( &P );
        bkPush( &X, v );

        // Reset working sets for next loop
        bkPop( &newR );
        bkClear( &newP );
        bkClear( &newX );
    }
    
    // Clean up temp space.
    bkFree( &newR );
    bkFree( &newP );
    bkFree( &newX );
}

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
        graph[ c1 ][ c2 ] = 1;
        graph[ c2 ][ c1 ] = 1;
    }
    
    BkSet bkR;
    BkSet bkP;
    BkSet bkX;
    BkSet bkMax;
    bkInit( &bkR, D );
    bkInit( &bkP, D );
    bkInit( &bkX, D );
    bkInit( &bkMax, D );

    for ( int i = 0; i < D; i++ )
    for ( int j = 0; j < D; j++ )
    {
        if ( graph[ j ][ i ] ) {
            bkPush( &bkP, i );
            break;
        }
    }

    bronKerbosch( bkR, bkP, bkX, &bkMax );

    // Bubble sort
    for (size_t i = 0; i < bkMax.count; i++)
    for (size_t j = i; j < bkMax.count; j++)
    {
        if ( bkMax.nodes[ i ] > bkMax.nodes[ j ] ) {
            int temp = bkMax.nodes[ i ];
            bkMax.nodes[ i ] = bkMax.nodes[ j ];
            bkMax.nodes[ j ] = temp;
        }
    }
    
    bkPrint( &bkMax );
    
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