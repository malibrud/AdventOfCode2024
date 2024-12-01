#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_ELEMENTS 1000

void check( bool success, int exitval, char *format, ... );

int compare( const void *a, const void *b ) { return *(int*)a - *(int*)b; }

int main( int argc, char **argv ) {
    check( argc >= 2, 1, "Usage: %s filename", argv[0] );

    FILE *f = fopen( argv[1], "r" );
    check( f != NULL, 1, "Error: Could not open %s.", argv[1] );

    int lList[ MAX_ELEMENTS ];
    int rList[ MAX_ELEMENTS ];

    #define LEN 32
    char line[ LEN ];
    int idx = 0;
    while ( fgets( line, LEN, f ) ) {
        check( idx < MAX_ELEMENTS, 1, "Too many lines in the file. Lines must be <= %d", MAX_ELEMENTS );
        sscanf( line, "%d %d", &lList[idx], &rList[idx] );
        idx++;
    }
    fclose( f );
    int count = idx;

    qsort( lList, count, sizeof(int), compare );
    qsort( rList, count, sizeof(int), compare );

    int sum = 0;
    for ( int i = 0 ; i < count ; i++ ) {
        sum += abs( lList[i] - rList[i] );
    }
    printf ("%d\n", sum );

    return 0;
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