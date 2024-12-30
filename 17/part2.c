#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_PROGRAM 20
#define MAX_OUTPUT 20
typedef struct {
    uint64_t PC;
    uint64_t A, B, C; // Registers
    uint64_t program[ MAX_PROGRAM ];
    int P; // Program length
    uint64_t output[ MAX_OUTPUT ];
    int U; // Output length
} Computer;

uint64_t combo( Computer *c, uint64_t arg ) {
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

void reset( Computer *c, uint64_t A, uint64_t B, uint64_t C ) {
    c->PC = 0;
    c->A = A;
    c->B = B;
    c->C = C;
    c->U = 0;
}

void step( Computer *c ) {

    uint64_t op  = c->program[ c->PC++ ];
    uint64_t arg = c->program[ c->PC++ ];

    switch ( op )
    {
    case 0: // adv
        arg = combo( c, arg );
        c->A /= ( 1ull << arg );
        break;
    case 1: // bxl
        c->B ^= arg;
        break;
    case 2: // bst
        arg = combo( c, arg );
        c->B = arg % 8;
        break;
    case 3: // jnz
        if ( c->A != 0ull ) c->PC = arg;
        break;
    case 4: // bxc
        c->B ^= c->C;
        break;
    case 5: // out
        assert( c->U < MAX_OUTPUT );
        arg = combo( c, arg );
        c->output[ c->U++ ] = arg % 8;
        break;
    case 6: // bdv
        arg = combo( c, arg );
        c->B = c->A / ( 1ull << arg );
        break;
    case 7: // cdv
        arg = combo( c, arg );
        c->C = c->A / ( 1ull << arg );
        break;
    default:
        assert( false );
        break;
    }
}

void run( Computer *c ) {
    while ( c->PC < c->P ) {
        step( c );
    }
}

void print( Computer *c ) {
    printf( "PC: %llu, A: %9llu, B: %9llu, C: %9llu OUT: ", 
            c->PC,  c->A,   c->B,   c->C 
    );
    for ( int i = 0 ; i < c->U ; i++ ) printf( "%llu,", c->output[ i ] );
    printf( "\n" );
}

bool doesOutputNotMatch( Computer *c ) {
    if ( c->U > c->P ) return true;
    for ( int i = 0 ; i < c->U ; i++ ) {
        if ( c->output[ i ] != c->program[ i ] ) return true;
    }
    return false;
}

bool outputEqualsProgram( Computer *c ) {
    if ( c->U != c->P ) return false;
    for ( int i = 0 ; i < c->U ; i++ ) {
        if ( c->output[ i ] != c->program[ i ] ) return false;
    }
    return true;
}

void check( bool success, char *format, ... );
bool tryGetDataFromFile( char *inFileName, Computer *outComputer );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Computer c = {};
    check( tryGetDataFromFile( argv[ 1 ], &c ), "Error: Could not read data from %s.", argv[ 1 ] );
    uint64_t iB = c.B;
    uint64_t iC = c.C;

    uint64_t A = 7ull << 3*(c.P-1 );
    uint64_t digitMask = 7ull;
    for ( int d = c.P-1 ; d >= 0 ; d-- ) {
        uint64_t base = ( A >> 3*d ) & digitMask;
        for ( int v = 0 ; v < 8 ; v++ ) {
            uint64_t place = ( base + v ) % 8;
            A = A & ~(digitMask << 3*d) | place << 3*d;
            reset( &c, A, iB, iC );
            run( &c );
            if ( c.output[ d ] == c.program[ d ] ) {
                printf( "A0: %llu, ", A );
                print( &c );
                goto DIGIT_FOUND;
            }
        }
        d += 2; // backtrack because a match was not found.
        uint64_t place  = ( A >> 3*d ) & digitMask;
        place++;
        A = A & ~(digitMask << 3*d) | place << 3*d;
        DIGIT_FOUND:
        ;
    }
    printf( "%llu\n", A );
    return 0;
}

bool tryGetDataFromFile( char *inFileName, Computer *outComputer ) {
    Computer *c = outComputer;
    FILE *f = fopen( inFileName, "r" );
    if ( f == NULL ) return false;

    // Get register initial values.
    check( 1 == fscanf( f, "Register A: %llu\n", &c->A ), "Failed to read register A" );
    check( 1 == fscanf( f, "Register B: %llu\n", &c->B ), "Failed to read register B" );
    check( 1 == fscanf( f, "Register C: %llu\n", &c->C ), "Failed to read register C" );

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