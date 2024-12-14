#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define VISITED_BIT 0x40
#define VALUE_MASK  0x3f

typedef struct {
    int X, Y;   // x and y dimensions of the map
    char **map;
    char *rawMap;
    int rawSize;
    int stride;
} Map;

void check( bool success, char *format, ... );
bool tryGetMapFromFile( char *inFileName, Map *outMap );
int getTrailTo9Count( Map *map, int x, int y );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read disk map from %s.", argv[ 1 ] );
    int X = map.X;
    int Y = map.Y;

    int count = 0;
    for ( int y = 0 ; y < Y ; y++ )
    for ( int x = 0 ; x < X ; x++ )
    {
        if ( map.map[ y ][ x ] != '0' ) continue;
        count += getTrailTo9Count( &map, x, y );

        // clear the visited bit
        for ( int i = 0 ; i < map.rawSize ; i++ ) {
            map.rawMap[ i ] &= VALUE_MASK;
        }
    }

    printf( "%d\n", count );
    return 0;
}

int getTrailTo9Count( Map *map, int x, int y ) {
    char curVal = map->map[ y ][ x ];
    if ( curVal & VISITED_BIT ) return 0;
    map->map[ y ][ x ] = curVal | VISITED_BIT;

    if ( curVal == '9' ) {
        // printf( "(%d, %d) = %c, count = %d\n", x, y, curVal, 1 );
        return 1;
    }

    int count = 0;
    int dx = 1;
    int dy = 0;
    for ( int i = 0 ; i < 4 ; i++ ) {
        int nx = x + dx;
        int ny = y + dy;
        int ndx = -dy;
        int ndy = +dx;
        dy = ndy;
        dx = ndx;

        if ( nx < 0 || nx >= map->X ) continue;
        if ( ny < 0 || ny >= map->Y ) continue;
        char nxtVal = map->map[ ny ][ nx ];
        if ( nxtVal == curVal + 1 ) count += getTrailTo9Count( map, nx, ny );
    }
    // printf( "(%d, %d) = %c, count = %d\n", x, y, curVal, count );
    return count;
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

    int nRead = (int)fread( data, 1, N, f );
    fclose( f );
    if ( nRead != N ) return false;
    data[ N ] = '\0';

    char *nl = strstr( data, "\r\n" );
    outMap->X = (int)( nl - data );
    outMap->stride = outMap->X + 2;
    outMap->Y = ( N + 2 ) / outMap->stride;

    // Load the map into a 2D array
    outMap->map = malloc( outMap->Y * sizeof( char* ) );
    if ( outMap->map == NULL ) return false;
    for ( int i = 0 ; i < outMap->Y ; i++ ) {
        outMap->map[ i ]  = outMap->rawMap  + i*outMap->stride;
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