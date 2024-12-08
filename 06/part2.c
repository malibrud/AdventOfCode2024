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
    int nRawBytes;
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
    char* origMap = malloc( map.nRawBytes );
    check ( origMap, "Could not allocate origMap." );

    // Remember original state;
    memcpy( origMap, map._rawData, map.nRawBytes );
    Map map0 = map;
    int X = map.X;
    int Y = map.Y;

    printf( "\n\nSearching for infinite loops...\n\n");
    int count = 0;
    for ( int yo = 0 ; yo < Y ; yo++ )
    for ( int xo = 0 ; xo < X ; xo++ )
    {
        // Restore original state.
        memcpy( map._rawData, origMap, map.nRawBytes );
        map = map0;

        char p = map.map[ yo ][ xo ];
        if ( p == '#'  ) continue;
        if ( p == '^'  ) continue;

        // Place the extra obstacle.
        map.map[ yo ][ xo ] = 'O';

        while ( true ) {
            if ( map.map[ map.y ][ map.x ] == '4' ) {
                count++;
                // printf( "\nInfinite Path Found for %d, %d:\n%s\n", xo, yo, map._rawData);
                break;
            }

            int nx = map.x + map.dx;
            int ny = map.y + map.dy;

            if ( nx < 0 || nx == X ) break;
            if ( ny < 0 || ny == Y ) break;
            
            char c = map.map[ ny ][ nx ];
            if ( c == '#' || c == 'O' ) {
                rotateBy90deg( &map.dx, &map.dy );
                continue;
            }

            c = map.map[ map.y ][ map.x ];
            if      ( c == '.' ) c = '1';
            else if ( c == '^' ) c = '1';
            else if ( c >= '1' ) c++;
            map.map[ map.y ][ map.x ] = c;

            map.x = nx;
            map.y = ny;
        }
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

    outMap->_rawData = malloc( N + 1 );
    if ( outMap->_rawData == NULL ) return false;
    char *data = outMap->_rawData;

    int nRead = (int)fread( data, 1, N, f );
    fclose( f );
    if ( nRead != N ) return false;
    outMap->nRawBytes = nRead;
    data[ N ] = '\0';

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