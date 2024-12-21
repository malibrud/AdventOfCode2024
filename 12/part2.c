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

#define R_BIT ( 1u << 0 )
#define B_BIT ( 1u << 1 )
#define L_BIT ( 1u << 2 )
#define T_BIT ( 1u << 3 )

typedef struct {
    int X, Y;   // x and y dimensions of the map
    char **map;
    char *rawMap;
    uint8_t **perim;
    uint8_t *rawPerim;
    int rawSize;
    int stride;
} Map;

void initPerimeterMap( Map *m ) {
    m->rawPerim = malloc( m->rawSize );
    memcpy( m->rawPerim, m->rawMap, m->rawSize );
    m->perim = malloc( m->Y * sizeof( uint8_t* ) );
    for ( int i = 0 ; i < m->Y ; i++ ) {
        m->perim[ i ]  = m->rawPerim  + i*m->stride;
    }
}

void resetPerimeterMap( Map *m ) {
    int X = m->X;
    int Y = m->Y;
    for ( int y = 0 ; y < Y ; y++ )
    for ( int x = 0 ; x < X ; x++ )
    {
        m->perim[ y ][ x ]  = 0;
    }
}

void check( bool success, char *format, ... );
bool tryGetMapFromFile( char *inFileName, Map *outMap );
void getAreaAndMarkPerimeter( Map *map, int x, int y, int *outArea );
void getSideCount( Map *map, int *outSides );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read disk map from %s.", argv[ 1 ] );
    initPerimeterMap( &map );
    int X = map.X;
    int Y = map.Y;

    int total = 0;
    for ( int y = 0 ; y < Y ; y++ )
    for ( int x = 0 ; x < X ; x++ )
    {
        if ( map.map[ y ][ x ] & VISITED_BIT ) continue;
        int sides = 0;
        int area = 0;
        resetPerimeterMap( &map );
        getAreaAndMarkPerimeter( &map, x, y, &area );
        getSideCount( &map, &sides );
        total += sides * area;
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

void getAreaAndMarkPerimeter( Map *map, int x, int y, int *outArea ) {
    int X = map->X;
    int Y = map->Y;
    char **m = map->map;
    uint8_t **p = map->perim;

    assert( 0 <= x && x < X );
    assert( 0 <= y && y < Y );
    assert( ( m[ y ][ x ] & VISITED_BIT)  == 0 );

    char c = m[ y ][ x ] |= VISITED_BIT;
    (*outArea)++;
    
    int dx = 1;
    int dy = 0;

    for ( int i = 0 ; i < 4 ; i++ ) {
        rot90( &dx, &dy );
        int nx = x + dx;
        int ny = y + dy;

        char side = '\0';
        if ( dx == -1 ) side = L_BIT;
        if ( dx == +1 ) side = R_BIT;
        if ( dy == -1 ) side = T_BIT;
        if ( dy == +1 ) side = B_BIT;
        
        // Test for boundary edges
        if ( nx <  0 || nx >= X || ny <  0 || ny >= Y ) { 
            p[ y ][ x ] |= ' ' + side; 
            continue; 
        }
        uint8_t np = m[ ny ][ nx ];

        // Test to see if the new plot has been visited.
        if ( c == np ) continue;

        // Test to see if this is the boundary of a region;
        if ( c != ( np | VISITED_BIT ) ) {
            p[ y ][ x ] |= ' ' + side; 
            continue;
        }

        // This new point is in the region, recurse.
        getAreaAndMarkPerimeter( map, nx, ny, outArea );
    }
}

int searchDirs[4][2] = {
    {  0,  1 },  // Boundary on right, search down.
    {  1,  0 },  // Boundary on bottom, search right.
    {  0,  1 },  // Boundary on left, search down.
    {  1,  0 },  // Boundary on top, search right.
};

void getSideCount( Map *map, int *outSides ) {
    int X = map->X;
    int Y = map->Y;
    char **m = map->perim;

    for ( int x = 0 ; x < X ; x++ )
    for ( int y = 0 ; y < Y ; y++ )
    {
        char c = m[ y ][ x ];
        if ( c == ' ' ) continue;

        int bit = 1;
        for ( int i = 0 ; i < 4 ; i++, bit <<= 1 ) {
            if ( !( c & bit ) ) continue;
            (*outSides)++;
            int dx = searchDirs[ i ][ 0 ];
            int dy = searchDirs[ i ][ 1 ];

            int nx = x;
            int ny = y;
            while ( nx >= 0 && nx < X && ny >= 0 && ny < Y ) {
                if ( ( m[ ny ][ nx ] & bit ) == 0 ) break;
                m[ ny ][ nx ] &= ~bit;
                nx += dx;
                ny += dy;
            }
        }

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