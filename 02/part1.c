#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

void check( bool success, int exitval, char *format, ... );
bool tryNextInt( char **inoutStr, int *outVal );

int main( int argc, char **argv ) {
    check( argc >= 2, 1, "Usage: %s filename", argv[0] );

    FILE *f = fopen( argv[1], "r" );
    check( f != NULL, 1, "Error: Could not open %s.", argv[1] );

    #define LEN 32
    char line[ LEN ];
    int count = 0;
    while ( fgets( line, LEN, f ) ) {
        char *cursor = line;
        int v1, v2;
        check( tryNextInt( &cursor, &v1 ), 1, "Expected at least two values." );
        check( tryNextInt( &cursor, &v2 ), 1, "Expected at least two values." );
        int diff = v2 - v1;
        
        int min = 0;
        int max = 0;
        if ( diff == 0 ) continue;
        if ( diff < -3 ) continue;
        if ( diff > +3 ) continue;
        if ( diff > 0) {
            min = 1;
            max = 3;
        }
        if ( diff < 0 ) {
            min = -3;
            max = -1;
        }

        v1 = v2;
        bool isSafe = true;
        while( tryNextInt( &cursor, &v2 ) ) {
            diff = v2 - v1;
            if ( diff < min || diff > max ) {
                isSafe = false;
                break;
            }
            v1 = v2;
        }
        if ( isSafe ) count++;
    }
    fclose( f );

    printf ("%d\n", count );

    return 0;
}

bool tryNextInt( char **inoutStr, int *outVal ) {
    while ( isspace( **inoutStr ) ) (*inoutStr)++;
    if ( **inoutStr == '\0' ) return false;

    if ( sscanf( *inoutStr, "%d", outVal ) == 1 ) {
        while ( isdigit( **inoutStr ) ) (*inoutStr)++;
        return true;
    }
    return false;
}

void check( bool success, int exitval, char *format, ... ) {
    if ( success ) return;
    va_list args;
    va_start( args, format );
    vprintf( format, args );
    va_end( args ); 
    printf("\n");
    exit( exitval );
}