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
    char *moves;
    size_t M;      // Number of moves
    int B;      // Number of bytes
    int stride;
    char *bytes;
} Warehouse;

void check( bool success, char *format, ... );
bool tryGetDataFromFile( char *inFileName, Warehouse *outWarehouse );

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
            wh.x = nx;
            wh.y = ny;
        } else
        if ( nc == 'O' ) {
            // There is a box in front of us, push the box if possible.
            int x = nx;
            int y = ny;
            char c;
            while ( ( c = map[ y ][ x ] ) != '#' ) {
                if ( c == '.' ) {
                    map[  y ][  x ] = 'O';
                    map[ ny ][ nx ] = '.';
                    wh.x = nx; 
                    wh.y = ny;
                    break;
                }
                x += dx;
                y += dy;
            }
        }
    }

    // Score the final positions of the boxes
    int sum = 0;
    for ( int y = 0 ; y < wh.Y ; y++ )
    for ( int x = 0 ; x < wh.X ; x++ )
    {
        if ( map[ y ][ x ] == 'O' ) sum += 100*y + x;
    }

    printf( "%d\n", sum );
    return 0;
}

bool tryGetDataFromFile( char *inFileName, Warehouse *outWarehouse ) {
    Warehouse *wh = outWarehouse;
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    fseek( f, 0, SEEK_END );
    wh->B = ftell( f ) + 1;
    fseek( f, 0, SEEK_SET );

    check( wh->bytes = malloc( wh->B ), "Error: Malloc failed." );

    check( fread( wh->bytes, 1, wh->B, f ), "Error: Failed to read file contents." );
    wh->bytes[ wh->B - 1 ] = '\0';

    char *eol = strchr( wh->bytes, '\r' );
    wh->X = (int)( eol - wh->bytes );
    wh->stride = wh->X + 2;
    
    char *eog = strstr( wh->bytes, "\r\n\r\n" );
    wh->Y = (int)( eog - wh->bytes + 2 ) / wh->stride;
    wh->moves = eog + 4;
    wh->M = strlen( wh->moves );

    check( wh->map = malloc( wh->Y * sizeof( char* ) ), "Error: Malloc failed." );
    for ( int i = 0 ; i < wh->Y ; i++ ) wh->map[ i ] = wh->bytes + i * wh->stride;

    for ( int y = 0 ; y < wh->Y ; y++ )
    for ( int x = 0 ; x < wh->X ; x++ )
    {
        if ( wh->map[ y ][ x ] == '@' ) {
            wh->map[ y ][ x ] = '.';
            wh->x = x;
            wh->y = y;
            goto DONE;
        }
    }
    DONE:

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