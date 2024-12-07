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
    int X, Y;   // x and y dimensions of the map
    int stride;
    char **map;
    char *_rawData;
    int x, y;   // Guard position
    int dx, dy; // Guard direction
} Map;

void check( bool success, char *format, ... );
bool tryGetMapFromFile( char *inFileName, Map *outMap );
void rotateBy90deg( int *inoutDx, int *inoutDy );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read map from %s.", argv[ 1 ] );
    int X = map.X;
    int Y = map.Y;

    int steps = 1;
    while ( true ) {
        map.map[ map.y ][ map.x ] = 'X';
        int nx = map.x + map.dx;
        int ny = map.y + map.dy;

        if ( nx < 0 || nx == X ) break;
        if ( ny < 0 || ny == Y ) break;
        
        if ( map.map[ ny ][ nx ] == '#' ) {
            rotateBy90deg( &map.dx, &map.dy );
            continue;
        }

        map.x = nx;
        map.y = ny;
        if ( map.map[ ny ][ nx ] != 'X' ) steps++;
    }
    printf( "%d\n", steps );
    return 0;
}

bool tryGetMapFromFile( char *inFileName, Map *outMap ){
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    fseek( f, 0, SEEK_END);
    int N = ftell( f );
    fseek( f, 0, SEEK_SET );

    outMap->_rawData = malloc( N );
    if ( outMap->_rawData == NULL ) return false;
    char *data = outMap->_rawData;

    int nRead = fread( data, 1, N, f );
    fclose( f );
    if ( nRead != N ) return false;

    char *nl = strstr( data, "\r\n" );
    outMap->X = (int)( nl - data );
    int stride = outMap->X + 2;
    outMap->Y = ( N + 2 ) / stride;

    // Load the map into a 2D array
    outMap->map = malloc( outMap->Y * sizeof( char* ) );
    if ( outMap->map == NULL ) return false;
    for ( int i = 0 ; i < outMap->Y ; i++ ) {
        outMap->map[ i ] = data + i*stride;
    }

    // Find the guard location
    for ( int y = 0 ; y < outMap->Y ; y++ )
    for ( int x = 0 ; x < outMap->X ; x++ )
    {
        if ( outMap->map[y][x] == '^' )
        {
            outMap->x = x;
            outMap->y = y;
            outMap->dx =  0;
            outMap->dy = -1;
            goto GUARD_FOUND;
        }
    } 
    GUARD_FOUND:

    return true;
}

void rotateBy90deg( int *inoutDx, int *inoutDy ) {
    int dx = *inoutDx;
    int dy = *inoutDy;

    *inoutDx = -dy;
    *inoutDy = +dx;
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