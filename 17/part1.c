#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_PROGRAM 32
#define MAX_OUTPUT 32
typedef struct {
    int PC;
    int A, B, C; // Registers
    int program[ MAX_PROGRAM ];
    int P; // Program length
    int output[ MAX_OUTPUT ];
    int U; // Output length
} Computer;

void print( Computer *c ) {
    printf( "PC: %d, A: %9d, B: %9d, C: %9d, OUT: ", 
            c->PC,  c->A,   c->B,   c->C 
    );
    for ( int i = 0 ; i < c->U ; i++ ) printf( "%d,", c->output[ i ] );
    printf( "\n" );
}

void check( bool success, char *format, ... );
bool tryGetDataFromFile( char *inFileName, Computer *outComputer );

int combo( Computer *c, int arg ) {
    switch ( arg ) 
    {
    case 0: return arg;
    case 1: return arg;
    case 2: return arg;
    case 3: return arg;
    case 4: return c->A;
    case 5: return c->B;
    case 6: return c->C;
    case 7: assert( false );
    default: assert( false );
    }
    return 0;
}

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Computer c = {};
    check( tryGetDataFromFile( argv[ 1 ], &c ), "Error: Could not read data from %s.", argv[ 1 ] );

    while ( c.PC < c.P ) {
        int op  = c.program[ c.PC++ ];
        int arg = c.program[ c.PC++ ];

        switch ( op )
        {
        case 0: // adv
            arg = combo( &c, arg );
            c.A /= ( 1 << arg );
            break;
        case 1: // bxl
            c.B ^= arg;
            break;
        case 2: // bst
            arg = combo( &c, arg );
            c.B = arg % 8;
            break;
        case 3: // jnz
            if ( c.A != 0 ) c.PC = arg;
            break;
        case 4: // bxc
            c.B ^= c.C;
            break;
        case 5: // out
            assert( c.U < MAX_OUTPUT );
            arg = combo( &c, arg );
            c.output[ c.U++ ] = arg % 8;
            break;
        case 6: // bdv
            arg = combo( &c, arg );
            c.B = c.A / ( 1 << arg );
            break;
        case 7: // cdv
            arg = combo( &c, arg );
            c.C = c.A / ( 1 << arg );
            break;
        default:
            assert( false );
            break;
        }
        print( &c );
    }

    return 0;
}

bool tryGetDataFromFile( char *inFileName, Computer *outComputer ) {
    Computer *c = outComputer;
    FILE *f = fopen( inFileName, "r" );
    if ( f == NULL ) return false;

    // Get register initial values.
    check( 1 == fscanf( f, "Register A: %d\n", &c->A ), "Failed to read register A" );
    check( 1 == fscanf( f, "Register B: %d\n", &c->B ), "Failed to read register B" );
    check( 1 == fscanf( f, "Register C: %d\n", &c->C ), "Failed to read register C" );

    char line[ 64 ];
    // Read the blank line.
    // check( fgets( line, 64, f ), "Could not read the blank line." );

    // Read the program line.
    check( fgets( line, 64, f ), "Could not read the program line." );
    char *cur = line + strlen( "Program:" );
    while ( *cur++ ) c->program[ c->P++ ] = *cur++ - '0';

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