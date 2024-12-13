#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_NUMBERS 16
#define MAX_EQNS    900


typedef struct {
    int X, Y;   // x and y dimensions of the map
    char **map;
    char **amap;
    char *rawMap;
    char *rawAmap;
    int rawSize;
    int stride;
} Map;

int index( Map *m, int x, int y ) { return y * m->stride + x; }

void check( bool success, char *format, ... );
bool tryGetMapFromFile( char *inFileName, Map *outMap );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read equations from %s.", argv[ 1 ] );
    int Y = map.Y;
    int X = map.X;

    // Find the anti-nodes

    int N = map.rawSize;
    for ( int i = 0 ; i < N ; i++)
    {
        int x1 = i % map.stride;
        int y1 = i / map.stride;
        if ( map.map[y1][x1] == '.' ) continue;
        if ( x1 >= X ) continue;
        char c1 = map.map[y1][x1];
        for ( int j = i + 1 ; j < N ; j++)
        {
            int x2 = j % map.stride;
            int y2 = j / map.stride;
            if ( map.map[y2][x2] != c1 ) continue;
            if ( x2 >= X ) continue;
            int dx = x2 - x1;
            int dy = y2 - y1;
            int x, y;

            // Try antinode behind x1, y1
            x = x1 - dx;
            y = y1 - dy;
            if ( x >= 0 && x < X && y >= 0 && y < Y ) map.amap[y][x] = '#';

            // Try antinode beyond x2, y2
            x = x2 + dx;
            y = y2 + dy;
            if ( x >= 0 && x < X && y >= 0 && y < Y ) map.amap[y][x] = '#';
        }
    }

    // Count the anti-nodes.
    int count = 0;
    for ( int y = 0 ; y < Y ; y++)
    for ( int x = 0 ; x < Y ; x++)
    {
        if ( map.amap[y][x] == '#' ) count++;
    }

    printf( "%d\n", count );
    return 0;
}

bool tryGetMapFromFile( char *inFileName, Map *outMap ){
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    fseek( f, 0, SEEK_END);
    int N = ftell( f );
    fseek( f, 0, SEEK_SET );

    outMap->rawMap = malloc( N + 1 );
    if ( outMap->rawMap == NULL ) return false;
    char *data = outMap->rawMap;
    outMap->rawSize = N + 1;

    int nRead = fread( data, 1, N, f );
    fclose( f );
    if ( nRead != N ) return false;
    data[ N ] = '\0';

    // Create anti-node map
    outMap->rawAmap = malloc( outMap->rawSize );
    check( outMap->rawAmap, "Error: Failed to allocate amap." );
    memcpy( outMap->rawAmap, outMap->rawMap, outMap->rawSize );
    for ( int i = 0 ; i < outMap->rawSize ; i++ )
    {
        if ( outMap->rawMap[ i ] == '\r' ) continue;
        if ( outMap->rawMap[ i ] == '\n' ) continue;
        outMap->rawAmap[ i ] = '.';
    }

    char *nl = strstr( data, "\r\n" );
    outMap->X = (int)( nl - data );
    outMap->stride = outMap->X + 2;
    outMap->Y = ( N + 2 ) / outMap->stride;

    // Load the map into a 2D array
    outMap->map = malloc( outMap->Y * sizeof( char* ) );
    outMap->amap = malloc( outMap->Y * sizeof( char* ) );
    if ( outMap->map == NULL ) return false;
    for ( int i = 0 ; i < outMap->Y ; i++ ) {
        outMap->map[ i ]  = outMap->rawMap  + i*outMap->stride;
        outMap->amap[ i ] = outMap->rawAmap + i*outMap->stride;
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