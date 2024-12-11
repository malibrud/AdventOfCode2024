#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_NUMBERS 16
#define MAX_EQNS    900

typedef struct {
    uint64_t result;
    int numbers[ MAX_NUMBERS ];
    int count;
} Equation;

void check( bool success, char *format, ... );
bool tryGetEqnsFromFile( char *inFileName, Equation *outEqns, int *outEqnCount );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Equation eqns[ MAX_EQNS ];
    int E;
    check( tryGetEqnsFromFile( argv[ 1 ], eqns, &E ), "Error: Could not read equations from %s.", argv[ 1 ] );

    uint64_t sum = 0;
    for ( int e = 0 ; e < E ; e++ ) {
        Equation *eq = eqns + e;
        int *nums = eq->numbers;
        int nOp = eq->count - 1;
        int P = 1 << nOp;
        for ( int mask = 0 ; mask < P ; mask++ )
        {
            uint64_t result = nums[ 0 ];
            for ( int op = 0 ; op < nOp ; op++ ) {
                int num = nums[ op + 1 ];
                if ( mask & ( 1 << op ) ) result += num;
                else                      result *= num;
            }
            if ( result == eq->result ) {
                sum += result;
                break;
            }
        }
    }

    printf( "%lld\n", sum );
    return 0;
}

bool tryGetEqnsFromFile( char *inFileName, Equation *outEqns, int *outEqnCount ){
    FILE *f = fopen( inFileName, "r" );
    if ( f == NULL ) return false;

    #define MAX_LINE 64
    char line[ MAX_LINE ];
    int ec = 0; // Equation count
    while ( fgets( line, MAX_LINE, f ) ) {
        assert( strlen( line ) < MAX_LINE );
        assert( ec < MAX_EQNS );
        check( 1 == sscanf( line, "%lld", &outEqns[ ec ].result ), "Error: Expected a value on line %d", ec+1 );
        char *cur = strchr( line, ':' );
        cur++;
        int nc = 0; // Number count
        while( *cur != '\n' && *cur != '\0' ) {
            assert( nc < MAX_NUMBERS );
            int val;
            check( 1 == sscanf( cur, "%d", &val ), "Error: reading integer in %s", cur );
            outEqns[ ec ].numbers[ nc ] = val;

            // consume the last int read ' ddd'
            while ( *cur == ' ' ) cur++;
            while ( isdigit( *cur ) ) cur++;
            nc++;
        }
        outEqns[ ec++ ].count = nc;
    }
    *outEqnCount = ec;
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