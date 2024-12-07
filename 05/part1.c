#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef struct {
    int *pages;
    int N;
} Update;

void check( bool success, char *format, ... );
bool tryGetRulesAndUpdates( char *inFileName, bool **outRules,  Update **outUpdates, int *outUpdateCount );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    bool *rules;
    Update *updates;
    int nUpdates;
    check( tryGetRulesAndUpdates( argv[ 1 ], &rules, &updates, &nUpdates ), "Error: Could not read rules and pages from %s.", argv[ 1 ] );

    int sum = 0;
    for ( int u = 0 ; u < nUpdates ; u++ ) {
        int N      = updates[ u ].N;
        int *pages = updates[ u ].pages;

        // Iterate through all pairs of pages with the one indexed by i always before j.
        for ( int i = 0   ; i < N ; i++ )
        for ( int j = i+1 ; j < N ; j++ )
        {
            // look for rule in reverse order to detect a rule which is violated by the pair.
            int page1 = pages[ i ];
            int page2 = pages[ j ];
            int idx = 100*page2 + page1;
            if ( rules[ idx ] ) goto RULE_VIOLATION;
        }
        // All pages satisfy the rules
        assert( N % 2 == 1 );
        sum += pages[ N / 2 ];
        continue;

    RULE_VIOLATION:
        continue;
    }

    printf( "%d\n", sum );
    return 0;
}

bool tryGetRulesAndUpdates( char *inFileName, bool **outRules,  Update **outUpdates, int *outUpdateCount ) {
    FILE *f = fopen( inFileName, "r" );
    if ( f == NULL ) return false;

    *outRules = malloc( 10000 * sizeof( bool ) );
    if ( *outRules == NULL ) return false;
    for ( int i = 0 ; i < 10000 ; i++ ) (*outRules)[ i ] = false;

    // Read the rules
    char rule[ 8 ];
    while ( fgets( rule, 8, f ) && rule[ 0 ] != '\n' ) {
        int p1, p2;
        check( 2 == sscanf( rule, "%d|%d", &p1, &p2 ), "Error: Could not scan rule %s", rule );
        check( p1 < 100 && p2 < 100, "Error: rule values must be only two digits." );
        int idx = 100 * p1 + p2;
        (*outRules)[ idx ] = true;
    }

    // Read the pages
    #define MAX_LINE 128
    char line[ MAX_LINE ];
    #define MAX_UPDATES 256
    *outUpdates = malloc( MAX_UPDATES * sizeof( Update ) );
    if ( *outUpdates == NULL ) return false;
    int i = 0;
    while ( fgets( line, MAX_LINE, f ) ) {
        check( i < MAX_UPDATES, "Error: Number of 'updates' exceeded the fixed max of %d", MAX_UPDATES );
        int len = (int)strlen( line );
        int n = ( len + 1 ) / 3;
        int *updatePtr = malloc( n * sizeof( int ) );
        if ( updatePtr == NULL ) return false;
        for ( int j = 0 ; j < n ; j++ ) {
            sscanf( line + 3*j, "%d", updatePtr + j );
        }
        (*outUpdates)[ i ].pages = updatePtr;
        (*outUpdates)[ i ].N     = n;
        i++;
    }
    fclose( f );
    *outUpdateCount = i;

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