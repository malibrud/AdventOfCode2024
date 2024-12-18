#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define VISITED_BIT ( 1u << 5 )

typedef struct {
    int X, Y;   // x and y dimensions of the map
    char **map;
    char *rawMap;
    int rawSize;
    int stride;
} Map;

void check( bool success, char *format, ... );
bool tryGetMapFromFile( char *inFileName, Map *outMap );
void getPerimeterAndArea( Map *map, int x, int y, int *outPerim, int *outArea );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read disk map from %s.", argv[ 1 ] );
    int X = map.X;
    int Y = map.Y;

    int total = 0;
    for ( int y = 0 ; y < Y ; y++ )
    for ( int x = 0 ; x < X ; x++ )
    {
        if ( map.map[ y ][ x ] & VISITED_BIT ) continue;
        int perimeter = 0;
        int area = 0;
        getPerimeterAndArea( &map, x, y, &perimeter, &area );
        total += perimeter * area;
    }

    printf( "%d\n", total );
    return 0;
}

void rot90( int *x, int *y ) { 
    assert( *x * *y == 0 );
    int nx = -*y;
    int ny = +*x;
    *x = nx;
    *y = ny;
}

void getPerimeterAndArea( Map *map, int x, int y, int *outPerim, int *outArea ) {
    int X = map->X;
    int Y = map->Y;
    char **m = map->map;

    assert( 0 <= x && x < X );
    assert( 0 <= y && y < Y );
    assert( ( m[ y ][ x ] & VISITED_BIT)  == 0 );

    char p = m[ y ][ x ] |= VISITED_BIT;
    (*outArea)++;
    
    int dx = 1;
    int dy = 0;

    for ( int i = 0 ; i < 4 ; i++ ) {
        rot90( &dx, &dy );
        int nx = x + dx;
        int ny = y + dy;
        
        // Test for edge of map both horizontal and vertical.
        if ( nx <  0 || nx >= X || ny <  0 || ny >= Y ) {
            (*outPerim)++;
            continue;
        }
        char np = m[ ny ][ nx ];

        // Test to see if the new plot has been visited.
        if ( p == np ) continue;

        // Test to see if this is the boundary of a region;
        if ( p != ( np | VISITED_BIT ) ) {
            (*outPerim)++;
            continue;
        }

        // This new point is in the region, recurse.
        getPerimeterAndArea( map, nx, ny, outPerim, outArea );
    }
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