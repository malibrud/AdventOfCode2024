#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef struct {
    int X, Y;
    char **grid;
    int sx, sy;
    int ex, ey;
    int minSave;
} Racetrack;

void check( bool success, char *format, ... );
bool tryGetDataFromFile( char *inFileName, Racetrack *outComputer );

#define DIR_E ( (uint8_t)0x0 )
#define DIR_S ( (uint8_t)0x1 )
#define DIR_W ( (uint8_t)0x2 )
#define DIR_N ( (uint8_t)0x3 )

int dirs[4][2] = {
    {  1,  0 },
    {  0,  1 },
    { -1,  0 },
    {  0, -1 }
};

int **allocIntArray( int X, int Y ) {
    int size = X*Y*sizeof( int );
    int *data = malloc( size );
    int **arr = malloc( Y * sizeof( int* ) );
    for ( int y = 0 ; y < Y ; y++ ) {
        arr[ y ] = data + y*X;
    }
    memset( data, 0, size );
    return arr;
}

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Racetrack r = {};
    check( tryGetDataFromFile( argv[ 1 ], &r ), "Error: Could not read data from %s.", argv[ 1 ] );
    char **grid = r.grid;
    int X = r.X;
    int Y = r.Y;

    // allocate space to remember path and scores.
    int **pSec = allocIntArray( X, Y );
    int maxPsec = X*Y;
    int *xp = malloc( maxPsec * sizeof( int ) );
    int *yp = malloc( maxPsec * sizeof( int ) );

    // Walk the path.
    xp[0] = r.sx;
    yp[0] = r.sy;
    int P = 0; // Count of coordinates in the path.
    while( true ) {
        for ( int d = 0 ; d < 4 ; d++ ) {
            int nx = xp[P] + dirs[ d ][ 0 ];
            int ny = yp[P] + dirs[ d ][ 1 ];
            if ( grid[ ny ][ nx ] == '#' ) continue;
            if ( grid[ ny ][ nx ] == 'S' ) continue;
            if ( pSec[ ny ][ nx ] ) continue;
            assert( pSec[ ny ][ nx ] == 0 );
            pSec[ ny ][ nx ] = pSec[ yp[P] ][ xp[P] ] + 1;
            P++;
            xp[P] = nx;
            yp[P] = ny;
            break;
        }
        // Check to see if we are at the end;
        if ( xp[P] == r.ex && yp[P] == r.ey ) {
            P++;
            break;
        }
    }

    // Find cheats
    int count = 0;
    for ( int i = 0   ; i < P ; i++ )
    for ( int j = i+1 ; j < P ; j++ )
    {
        int x1   = xp[ i ];
        int y1   = yp[ i ];
        int x2   = xp[ j ];
        int y2   = yp[ j ];
        int cheatDist = abs( x2 - x1 ) + abs( y2 - y1 );
        if ( cheatDist > 20 ) continue;
        int p1 = pSec[ y1 ][ x1 ];
        int p2 = pSec[ y2 ][ x2 ];
        int diff = p2 - p1;
        int savedDist = diff - cheatDist;
        if ( savedDist >= r.minSave ) {
            count++;
        }
    }

    printf( "%d\n", count );

    return 0;
}


void *readFileIntoBuffer( char *fileName, int *outSize ) {
    FILE *f = fopen( fileName, "rb" );
    check( f, "Error: Could not open %s", fileName );

    fseek( f, 0, SEEK_END );
    int size = (int)ftell( f );
    fseek( f, 0, SEEK_SET );
    *outSize = size + 1;
    char *buff = malloc( *outSize );
    check( buff, "Failed to allocate buffer." );
    check( fread( buff, 1, size, f ), "fread failed." );
    buff[ size ] = '\0';
    return buff;
}

char **make2Darray( char *buff, int B, int size, int stride ) {
    int Y = ( B + (stride - 1 ) ) / stride;
    char **arr = malloc( Y * sizeof( char* ) );
    check( arr, "Allocation failed." );
    for ( int i = 0 ; i < Y ; i++ ) {
        arr[ i ] = buff + i*stride*size;
    }
    return arr;
}

bool tryGetDataFromFile( char *inFileName, Racetrack *outTrack ) {
    Racetrack *r = outTrack;
    int B;
    char* buff = readFileIntoBuffer( inFileName, &B );
    char *eol = strchr( buff, '\r' );
    int X = (int)( eol - buff );
    int stride = X + 2;
    int Y = ( B + 1 ) / stride;
    char **grid = make2Darray( buff, B, sizeof( char ), stride );

    for ( int y = 0 ; y < Y ; y++ )
    for ( int x = 0 ; x < X ; x++ )
    {
        if ( grid[ y ][ x ] == 'S' ) {
            r->sx = x;
            r->sy = y;
        }
        if ( grid[ y ][ x ] == 'E' ) {
            r->ex = x;
            r->ey = y;
        }
    }
    r->grid = grid;
    r->X = X;
    r->Y = Y;

    if ( strstr( inFileName, "test.txt" ) ) r->minSave = 50;
    if ( strstr( inFileName, "data.txt" ) ) r->minSave = 100;

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