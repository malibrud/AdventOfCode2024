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
    int x, y;  // position
    int vx, vy; // velocity
} Robot;

void check( bool success, char *format, ... );
bool tryGetRobotsFromFile( char *inFileName, Robot *outBots, int *outBotCount, int *outMapWidth, int *outMapHeight );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Robot bots[ MAX_BOTS ];
    int R;  // Number of bots
    int X;  // Map width
    int Y;  // Map height
    check( tryGetRobotsFromFile( argv[ 1 ], bots, &R, &X, &Y ), "Error: Could not read bots from %s.", argv[ 1 ] );
    size_t G = X * Y * sizeof( int );
    int *grid = malloc( G );

    for ( int s = 1 ; ; s++ ) {
        memset( grid, 0, G );
        // Simulate one second for each robot and store count in the grid;
        for ( int r = 0 ; r < R ; r++ ) {
            Robot *b = &bots[ r ];
            b->x = ( b->x + b->vx + X ) % X;
            b->y = ( b->y + b->vy + Y ) % Y;
            int idx = b->y * X + b->x;
            grid[ idx ]++;
        }

        // Check for empty upper left and upper right quadrants
        int count = 0;
        int UB = X / 2 - 2;
        for ( int i = 0 ; i < UB     ; i++ )
        for ( int j = 0 ; j < UB - i ; j++ )
        {
            int idx = i * X + j;
            if ( grid[ idx ] ) count++;
        }
        if ( count < 11 ) {
            for ( int y = 0 ; y < Y ; y++ ) {
                for ( int x = 0 ; x < X ; x++ ) {
                    int idx = y * X + x;
                    if ( grid[ idx ] ) putchar( '.' );
                    else putchar( ' ' );
                }
                putchar( '\n' );
            }
            printf( "%d Seconds (count = %d):\n\n", s, count );
            if ( 'b' == getchar() ) break;
        }
    }

    return 0;
}

bool tryGetRobotsFromFile( char *inFileName, Robot *outBots, int *outBotCount, int *outMapWidth, int *outMapHeight ) {
    FILE *f = fopen( inFileName, "r" );
    if ( f == NULL ) return false;

    // Kind of hacky but get map dimensions from the name of the file.
    if ( 0 == strcmp( inFileName, "test.txt" ) ) {
        *outMapWidth  = 11;
        *outMapHeight = 7;
    } else
    if ( 0 == strcmp( inFileName, "data.txt" ) ) {
        *outMapWidth  = 101;
        *outMapHeight = 103;
    }
    else return false;

    #define MAX_LINE 64
    char line[ MAX_LINE ];
    int rc = 0; // Robot count
    while ( fgets( line, MAX_LINE, f ) ) {
        assert( rc < MAX_BOTS );
        assert( strlen( line ) < MAX_LINE );
        Robot *r = outBots + rc++;
        check( 4 == sscanf( line, "p=%d,%d v=%d,%d", &r->x, &r->y, &r->vx, &r->vy ), "Error: Failed to parse line %s", line );
    }
    *outBotCount = rc;
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