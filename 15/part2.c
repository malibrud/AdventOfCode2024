#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_BOTS 500

typedef struct {
    int x, y;   // Position of the robot.
    int X, Y;   // Size of grid in x and y.
    char **map;
    char *mapStr;
    char *moves;
    size_t M;      // Number of moves
    int stride;
} Warehouse;

void check( bool success, char *format, ... );
bool tryGetDataFromFile( char *inFileName, Warehouse *outWarehouse );
void pushBoxHoriz( Warehouse *wh, int dx );
bool canPushBoxVert( Warehouse *wh, int x, int y, int dy );
void moveBoxVert( Warehouse *wh, int x, int y, int dy );
void pushBoxVert( Warehouse *wh, int dy );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Warehouse wh;
    check( tryGetDataFromFile( argv[ 1 ], &wh ), "Error: Could not read data from %s.", argv[ 1 ] );
     char **map = wh.map;

    for ( int m = 0 ; m < wh.M ; m++ ) {
        char move = wh.moves[ m ];
        if ( move <= ' ' ) continue;

        int dx = 0, dy = 0;
        if      ( move == '<' ) dx = -1;
        else if ( move == '>' ) dx = +1;
        else if ( move == '^' ) dy = -1;
        else if ( move == 'v' ) dy = +1;

        int nx = wh.x + dx;
        int ny = wh.y + dy;
        char nc = map[ ny ][ nx ];
        if ( nc == '#' ) {
            // There is a wall in front of us.  Don't move.
        } else
        if ( nc == '.' ) {
            // There is room, move to next position.
            map[ wh.y ][ wh.x ] = '.';
            map[ ny ][ nx ]     = '@';
            wh.x = nx;
            wh.y = ny;
        } else
        if ( nc == '[' || nc == ']' ) {
            // There is a box in front of us, push the box if possible.
            if ( dx != 0 ) pushBoxHoriz( &wh, dx );
            else           pushBoxVert(  &wh, dy );
        }

        // printf( "After %d: %c\n%s\n", m, move, wh.mapStr );
    }

    // Score the final positions of the boxes
    int sum = 0;
    for ( int y = 0 ; y < wh.Y ; y++ )
    for ( int x = 0 ; x < wh.X ; x++ )
    {
        if ( map[ y ][ x ] == '[' ) sum += 100*y + x;
    }

    printf( "%d\n", sum );
    return 0;
}

void pushBoxHoriz( Warehouse *wh, int dx ) {
    int X = wh->X;
    int xInit = wh->x;
    char *row = wh->map[ wh->y ];
    int nx = xInit + dx;
    int x = nx;
    char c;
    while ( x >= 0 && x < X ) {
        c = row[ x ];
        if ( c == '#' ) break;
        if ( c == '.' ) {
            if ( dx == +1 ) {
                int n = x - xInit;
                memmove( row + xInit + dx, row + xInit, n );
            } else
            if ( dx == -1 ) {
                int n = xInit - x;
                memmove( row + x, row + x + 1, n );
            }
            row[ xInit ]      = '.';
            row[ nx ] = '@';
            wh->x = nx;
            break;
        }
        x += dx;
    }
}

bool canPushBoxVert( Warehouse *wh, int x, int y, int dy ) {
    char **map = wh->map;

    int xl = x, xr = x;
    char c = map[ y ][ x ];
    if ( c == '[') {
        xl = x;
        xr = x + 1;
    } else
    if ( c == ']') {
        xl = x - 1;
        xr = x;
    } 
    else assert( false );
    
    char cnl = map[ y + dy ][ xl ];
    char cnr = map[ y + dy ][ xr ];
    if ( cnl == '#' || cnr == '#' ) return false;
    if ( cnl == '.' && cnr == '.' ) return true;

    if ( cnl == '[') return canPushBoxVert( wh, xl, y+dy, dy );

    bool canMove = true;
    if ( cnl == ']' ) canMove = canMove && canPushBoxVert( wh, xl, y+dy, dy );
    if ( cnr == '[' ) canMove = canMove && canPushBoxVert( wh, xr, y+dy, dy );
    return canMove;
}

void moveBoxVert( Warehouse *wh, int x, int y, int dy ) {
    char **map = wh->map;

    int xl = x, xr = x;
    char c = map[ y ][ x ];
    if ( c == '[') {
        xl = x;
        xr = x + 1;
    } else
    if ( c == ']') {
        xl = x - 1;
        xr = x;
    } 
    else assert( false );
    
    int ny = y + dy;
    char cnl = map[ ny ][ xl ];
    char cnr = map[ ny ][ xr ];
    assert( cnl != '#' && cnr != '#' );

    if ( cnl == '[' ) {
        moveBoxVert( wh, xl, ny, dy );
    }
    else { 
        if ( cnl == ']' ) moveBoxVert( wh, xl, ny, dy );
        if ( cnr == '[' ) moveBoxVert( wh, xr, ny, dy );
    }

    map[ ny ][ xl ] = '[';
    map[ ny ][ xr ] = ']';
    map[ y ][ xl ] = '.';
    map[ y ][ xr ] = '.';
}

void pushBoxVert( Warehouse *wh, int dy ) {
    char **map = wh->map;
    int x = wh->x;
    int y = wh->y;
    int ny = y + dy;
    if ( canPushBoxVert( wh, x, ny, dy ) ) {
        moveBoxVert( wh, x, ny, dy );
        map[ y ][ x ]  = '.';
        map[ ny ][ x ] = '@';
        wh->y = ny;
    }
}

bool tryGetDataFromFile( char *inFileName, Warehouse *outWarehouse ) {
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    // Get the size of the file
    fseek( f, 0, SEEK_END );
    int B = ftell( f );
    fseek( f, 0, SEEK_SET );

    // Read all of the file bytes into a buffer.
    char *allBytes;
    check( allBytes = malloc( B + 1 ), "Error: Malloc failed for 'allBytes'." );
    check( fread( allBytes, 1, B, f ), "Error: Failed to read file contents." );
    fclose( f );
    allBytes[ B ] = '\0';

    // Get the length of a line
    char *eol = strchr( allBytes, '\r' );
    int X = (int)( eol - allBytes );
    int stride = X + 2; // line plus the \r\n Windows line ending.
    int dstride = 2*X + 1;
    
    // Get the number of rows in a map
    char *eog = strstr( allBytes, "\r\n\r\n" );
    int Y = (int)( eog - allBytes + 2 ) / stride;

    // Get the location of moves.
    char *moves =  eog + 4;

    // Build the double wide map
    char *mapMem;
    char **map;
    int rx = 0, ry = 0; // Robot x and y initial location.
    check( mapMem = malloc( Y * dstride + 1 ), "Error: Could not malloc for 'mapMem'." );
    check( map    = malloc( Y * sizeof( char* ) ), "Error: Malloc failed for 'map'." );
    for ( int y = 0 ; y < Y ; y++ ) {
        map[ y ] = mapMem + y * dstride;
        map[ y ][ 2*X ] = '\n';
        for ( int x = 0 ; x < X ; x++ ) {
            int sidx = y*stride + x;
            int didx = y*dstride + 2*x;
            char c = allBytes[ sidx ];
            if ( c == '#' || c == '.' ) {
                mapMem[ didx + 0 ] = c;
                mapMem[ didx + 1 ] = c;
            } else
            if ( c == 'O' ) {
                mapMem[ didx + 0 ] = '[';
                mapMem[ didx + 1 ] = ']';
            } else
            if ( c == '@' ) {
                mapMem[ didx + 0 ] = '@';
                mapMem[ didx + 1 ] = '.';
                rx = 2*x;
                ry = y;
            }
            else assert(false);
        }
    }
    mapMem[ Y * dstride ] = 0;

    printf( "%s\n", mapMem );

    // Populate the output structure
    Warehouse *wh = outWarehouse;
    wh->map = map;
    wh->mapStr = mapMem;
    wh->moves = moves;
    wh->M = strlen( moves );
    wh->stride = dstride;
    wh->x = rx;
    wh->y = ry;
    wh->X = 2*X;
    wh->Y = Y;

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