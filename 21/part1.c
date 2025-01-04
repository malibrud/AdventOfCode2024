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
#define N_CODES 5
#define CODE_SIZE 8
typedef struct {
    char codes[ N_CODES ][ CODE_SIZE ];
    int values[ N_CODES ];
} Codes;

bool tryGetDataFromFile( char *inFileName, Codes *outCodes );

typedef struct {
    int8_t x;
    int8_t y;
    char   k;
} Key;

// Encode key positions.
Key numKeyPad[ 11 ] = {
    { 1, 3, '0' },

    { 0, 2, '1' },
    { 1, 2, '2' },
    { 2, 2, '3' },

    { 0, 1, '4' },
    { 1, 1, '5' },
    { 2, 1, '6' },

    { 0, 0, '7' },
    { 1, 0, '8' },
    { 2, 0, '9' },

    { 2, 3, 'A' }
};
#define NUM_A 10

Key dirKeyPad[ 5 ] = {
    { 2, 1, '>' },
    { 1, 1, 'v' },
    { 0, 1, '<' },
    { 1, 0, '^' },
    { 2, 0, 'A' }
};

#define DIR_R 0
#define DIR_D 1
#define DIR_L 2
#define DIR_U 3
#define DIR_A 4
void printSequence( int8_t *seq, int S ) {
    for ( int i = 0; i < S; i++ ) {
        putchar( dirKeyPad[ seq[ i ] ].k );
    }
    printf( ", S = %d\n", S );
}

void encodeNumSequence( char *code, int8_t *outSeq, int *outS ) {
    int8_t x = numKeyPad[ NUM_A].x;
    int8_t y = numKeyPad[ NUM_A].y;
    int S = 0;
    while( *code ) {
        int idx = *code <= '9' ? *code - '0' : NUM_A;
        int8_t nx = numKeyPad[ idx ].x;
        int8_t ny = numKeyPad[ idx ].y;
        int8_t dx = nx - x;
        int8_t dy = ny - y;
        if ( nx == 0 && y == 3 ){
            for ( int j = 0 ; j < dy ; j++ ) outSeq[ S++ ] = DIR_D;
            for ( int j = 0 ; j > dy ; j-- ) outSeq[ S++ ] = DIR_U;
            for ( int j = 0 ; j < dx ; j++ ) outSeq[ S++ ] = DIR_R;
            for ( int j = 0 ; j > dx ; j-- ) outSeq[ S++ ] = DIR_L;
        } else
        if ( x == 0 && ny == 3 ){
            for ( int j = 0 ; j < dx ; j++ ) outSeq[ S++ ] = DIR_R;
            for ( int j = 0 ; j > dx ; j-- ) outSeq[ S++ ] = DIR_L;
            for ( int j = 0 ; j < dy ; j++ ) outSeq[ S++ ] = DIR_D;
            for ( int j = 0 ; j > dy ; j-- ) outSeq[ S++ ] = DIR_U;
        } else
        if ( dx < 0 ){
            for ( int j = 0 ; j < dx ; j++ ) outSeq[ S++ ] = DIR_R;
            for ( int j = 0 ; j > dx ; j-- ) outSeq[ S++ ] = DIR_L;
            for ( int j = 0 ; j < dy ; j++ ) outSeq[ S++ ] = DIR_D;
            for ( int j = 0 ; j > dy ; j-- ) outSeq[ S++ ] = DIR_U;
        }
        else {
            for ( int j = 0 ; j < dy ; j++ ) outSeq[ S++ ] = DIR_D;
            for ( int j = 0 ; j > dy ; j-- ) outSeq[ S++ ] = DIR_U;
            for ( int j = 0 ; j < dx ; j++ ) outSeq[ S++ ] = DIR_R;
            for ( int j = 0 ; j > dx ; j-- ) outSeq[ S++ ] = DIR_L;
        }
        outSeq[ S++ ] = DIR_A;
        x = nx;
        y = ny;
        code++;
    }
    *outS = S;
}

void encodeDirSequence( int8_t *inSeq, int inS, int8_t *outSeq, int *outS ) {
    int8_t x = dirKeyPad[ DIR_A ].x;
    int8_t y = dirKeyPad[ DIR_A ].y;
    int S = 0;
    for ( int i = 0; i < inS; i++ ) {
        int8_t nx = dirKeyPad[ inSeq[ i ] ].x;
        int8_t ny = dirKeyPad[ inSeq[ i ] ].y;
        int8_t dx = nx - x;
        int8_t dy = ny - y;
        if ( nx == 0 && y == 0 ) {
            for ( int j = 0 ; j < dy ; j++ ) outSeq[ S++ ] = DIR_D;
            for ( int j = 0 ; j > dy ; j-- ) outSeq[ S++ ] = DIR_U;
            for ( int j = 0 ; j < dx ; j++ ) outSeq[ S++ ] = DIR_R;
            for ( int j = 0 ; j > dx ; j-- ) outSeq[ S++ ] = DIR_L;
        } else
        if ( x == 0 && ny == 0 ) {
            for ( int j = 0 ; j < dx ; j++ ) outSeq[ S++ ] = DIR_R;
            for ( int j = 0 ; j > dx ; j-- ) outSeq[ S++ ] = DIR_L;
            for ( int j = 0 ; j < dy ; j++ ) outSeq[ S++ ] = DIR_D;
            for ( int j = 0 ; j > dy ; j-- ) outSeq[ S++ ] = DIR_U;
        } else
        if ( dx < 0 ){
            for ( int j = 0 ; j < dx ; j++ ) outSeq[ S++ ] = DIR_R;
            for ( int j = 0 ; j > dx ; j-- ) outSeq[ S++ ] = DIR_L;
            for ( int j = 0 ; j < dy ; j++ ) outSeq[ S++ ] = DIR_D;
            for ( int j = 0 ; j > dy ; j-- ) outSeq[ S++ ] = DIR_U;
        }
        else {
            for ( int j = 0 ; j < dy ; j++ ) outSeq[ S++ ] = DIR_D;
            for ( int j = 0 ; j > dy ; j-- ) outSeq[ S++ ] = DIR_U;
            for ( int j = 0 ; j < dx ; j++ ) outSeq[ S++ ] = DIR_R;
            for ( int j = 0 ; j > dx ; j-- ) outSeq[ S++ ] = DIR_L;
        }
        outSeq[ S++ ] = DIR_A;
        printSequence( outSeq, S );
        x = nx;
        y = ny;
    }
    *outS = S;
}

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Codes c = {};
    check( tryGetDataFromFile( argv[ 1 ], &c ), "Error: Could not read data from %s.", argv[ 1 ] );

    #define MAX_SEQUENCE 256
    int8_t sequence1[ MAX_SEQUENCE ];
    int8_t sequence2[ MAX_SEQUENCE ];
    int8_t sequence3[ MAX_SEQUENCE ];
    int S = 0;
    int complexity = 0;
    for ( int i = 0 ; i < N_CODES ; i++ ) {
        printf( "Trying %s\n", c.codes[ i ] );
        encodeNumSequence( c.codes[ i ], sequence1, &S );
        printSequence( sequence1, S );
        encodeDirSequence( sequence1, S, sequence2, &S );
        printSequence( sequence2, S );
        encodeDirSequence( sequence2, S, sequence3, &S );
        printSequence( sequence3, S );
        complexity += S * c.values[ i ];
    }

    printf( "%d\n", complexity );
    return 0;
}

bool tryGetDataFromFile( char *inFileName, Codes *outCodes ) {
    Codes *c = outCodes;
    FILE *f = fopen( inFileName, "r" );
    check( f, "Error: Could not open %s", inFileName );

    for ( int i = 0 ; i < N_CODES ; i++ ) {
        check( fgets( c->codes[ i ], CODE_SIZE, f ), "Error fgets() failed at line %d.", i+1 );
        c->codes[ i ][ 4 ] = 0;
        c->values[ i ] = atoi( c->codes[ i ] );
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