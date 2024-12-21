#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
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
        assert ( ma->ax * ma->by - ma->ay * ma->bx != 0 ); // If determinent is 0, this approach will not work.

        int ax = ma->ax;
        int ay = ma->ay;
        int bx = ma->bx;
        int by = ma->by;
        int64_t px = ma->px + 10000000000000LL;
        int64_t py = ma->py + 10000000000000LL;

        // Cast the problem as a system of equations.
        //   A * x = b
        //
        //   A = [ a11 a12 ] = [ ax ax ]
        //       [ a21 a22 ]   [ ay ay ]
        //
        //   b = [ b1 b2 ]^T = [ px py ]^T
        //
        double a11 = ax;
        double a12 = bx;
        double a21 = ay;
        double a22 = by;
        double b1 = (double)px;
        double b2 = (double)py;
        double x1, x2;

        // Solve by Gaussian elimination
        // Subtract portion of first row from second row to zero out a21.

        // Reduce to Row Echelon Form (REF)
        double f = a21 / a11;
        a21 -= f * a11;
        a22 -= f * a12;
        b2  -= f * b1;

        // Now solve: a22 * x2 = b2
        x2 = b2 / a22;

        // Now solve: a11 * x1 + a12 * x2 = b1
        x1 = ( b1 - a12 * x2 ) / a11;

        // Check our solution with constraints
        if ( x1 < 0 || x2 < 0 ) continue;

        // Convert to closest integer solution and check 
        int64_t a = (int64_t)round( x1 );
        int64_t b = (int64_t)round( x2 );
        if (
            a * ax + b * bx == px &&
            a * ay + b * by == py
        ) {
            sum += 3 * a + b;
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