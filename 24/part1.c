#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

void check( bool success, char *format, ... );

typedef struct {
    char name[4];
    char in1[4];
    char in2[4];
    char op[4];
    int8_t value;
} Gate;
#define UNK ( (int8_t)-1 )

int gateComp( const void *_a, const void *_b ) {
    Gate *a = (Gate*)_a;
    Gate *b = (Gate*)_b;
    return strcmp( a->name, b->name );
}

#define MAX_GATES 512
typedef struct {
    Gate gates[ MAX_GATES ];
    int G;
} Device;

Gate *lookupGate( Device *d, char *name ) {
    int i1 = 0;
    int i2 = d->G - 1;
    while ( i1 <= i2 ) {
        int i = ( i1 + i2 ) / 2;
        int cmp = strcmp( name, d->gates[ i ].name );
        if ( cmp == 0 ) return &d->gates[ i ];
        else
        if ( cmp < 0 ) i2 = i - 1;
        else           i1 = i + 1;
    }
    return NULL;
}

int8_t getGateValue( Device *d, Gate *g ) {
    if ( g->value != UNK ) return g->value;
    Gate *gin1 = lookupGate( d, g->in1 );
    Gate *gin2 = lookupGate( d, g->in2 );
    int8_t v1 = getGateValue( d, gin1 );
    int8_t v2 = getGateValue( d, gin2 );

    int8_t value = 0;
    // Note: I am using bitwise operators here to be consistent with the xor operator which does not have a logic equivalent.
    switch( g->op[ 0 ] ) {
        case 'A':
            value = v1 & v2;
            break;
        case 'O':
            value = v1 | v2;
            break;
        case 'X':
            value = v1 ^ v2;
            break;
        default:
            assert( false );
    }
    g->value = value;
    return value;
}

bool tryGetDataFromFile( char *inFileName, Device *outDevice );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Device d = {};
    check( tryGetDataFromFile( argv[ 1 ], &d ), "Error: Could not read data from %s.", argv[ 1 ] );

    qsort( d.gates, d.G, sizeof( Gate ), gateComp );

    int z00idx = 0;
    for (int i = 0; i < d.G; i++) {
        if ( 0 == strcmp( d.gates[ i ].name, "z00" ) ) {
            z00idx = i;
            break;
        }
    }
    
    int64_t val = 0;
    int shift = 0;
    for (size_t i = z00idx; i < d.G; i++)
    {
        int64_t bit = (int64_t)getGateValue( &d, &d.gates[ i ] );
        val |= bit << shift++;
    }

    printf( "%lld\n", val );
    return 0;
}

bool tryGetDataFromFile( char *inFileName, Device *outDevice ) {
    FILE *f = fopen( inFileName, "r" );
    check( f, "Error: Could not open %s", inFileName );

    #define MAX_LINE 32
    char line[ MAX_LINE ];

    Gate *g = outDevice->gates;
    int G = 0;

    // Read the x and y gates.
    while ( true ) {
        fgets( line, MAX_LINE, f );
        if ( line[ 0 ] != 'x' && line[ 0 ] != 'y' ) break;
        check( 2 == sscanf( line, "%3s: %hhd", &g->name, &g->value ), "x/y sscanf failed." );
        g->in1[ 0 ] = '\0';
        g->in2[ 0 ] = '\0';
        g->op[ 0 ] = '\0';
        g++;
        G++;
    }

    // Read the gates
    while ( fgets( line, MAX_LINE, f ) ) {
        check( 4 == sscanf( line, "%s %s %s -> %s\n", g->in1, g->op, g->in2, g->name ), "gate sscanf failed." );
        g->value = UNK;
        g++;
        G++;
    }
    assert( G < MAX_GATES );
    outDevice->G = G;

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
    assert( false );
    exit( ERR_RET_VAL );
}