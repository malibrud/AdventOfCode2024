#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_MACHINES 320

typedef struct {
    int ax, ay;  // Button A
    int bx, by;  // Button B
    int px, py;  // Prize location
} Machine;

void check( bool success, char *format, ... );
bool tryGetMachinesFromFile( char *inFileName, Machine *outMach, int *outMachCount );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Machine machines[ MAX_MACHINES ];
    int M;
    check( tryGetMachinesFromFile( argv[ 1 ], machines, &M ), "Error: Could not read equations from %s.", argv[ 1 ] );

    uint64_t sum = 0;
    for ( int m = 0 ; m < M ; m++ ) {
        Machine *ma = machines + m;
        for ( int a = 0 ; a * ma->ax <= ma->px ; a++ ) {
            int ax = ma->ax;
            int ay = ma->ay;
            int bx = ma->bx;
            int by = ma->by;
            int px = ma->px;
            int py = ma->py;

            if ( 
                ( px - a * ax ) % bx == 0 &&
                ( py - a * ay ) % by == 0
            ) {
                int b = ( px - a * ax ) / bx;
                if ( a * ay + b * by == py ) {
                    sum += 3 * a + b;
                    break;
                }
            }
        }
    }

    printf( "%lld\n", sum );
    return 0;
}

bool tryGetMachinesFromFile( char *inFileName, Machine *outMach, int *outMachCount ) {
    FILE *f = fopen( inFileName, "r" );
    if ( f == NULL ) return false;

    #define MAX_LINE 64
    char line[ MAX_LINE ];
    int mc = 0; // Machine count
    while (true) {
        Machine *m = outMach + mc++;
        check( fgets( line, MAX_LINE, f ), "Error: could not read file" );
        check( 2 == sscanf( line, "Button A: X+%d, Y+%d", &m->ax, &m->ay), "Error: Failed to parse line %s", line );

        check( fgets( line, MAX_LINE, f ), "Error: could not read file" );
        check( 2 == sscanf( line, "Button B: X+%d, Y+%d", &m->bx, &m->by), "Error: Failed to parse line %s", line );

        check( fgets( line, MAX_LINE, f ), "Error: could not read file" );
        check( 2 == sscanf( line, "Prize: X=%d, Y=%d", &m->px, &m->py), "Error: Failed to parse line %s", line );

        if ( fgets( line, MAX_LINE, f ) == NULL ) break;
    }
    *outMachCount = mc;
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