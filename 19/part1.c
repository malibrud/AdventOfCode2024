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
    char *bytes;
    char **patterns;
    char **designs;
    int *patLens;

    int P; // Pattern count
    int D; // Design count
} Towels;

void check( bool success, char *format, ... );
bool tryGetDataFromFile( char *inFileName, Towels *outTowels );

bool matchFound( Towels *t, char* des ) {
    int dlen = (int)strlen( des );
    for ( int i = 0 ; i < t->P ; i++ ) {
        int plen = t->patLens[ i ];
        if ( 0 == strncmp( des, t->patterns[ i ], plen ) ) {
            if ( dlen == plen ) return true;
            if ( matchFound( t, des + plen ) ) return true;
        }
    }
    return false;
}

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Towels t = {};
    check( tryGetDataFromFile( argv[ 1 ], &t ), "Error: Could not read data from %s.", argv[ 1 ] );

    int count = 0;
    for ( int d = 0 ; d < t.D ; d++ ) {
        if ( matchFound( &t, t.designs[ d ] ) ) count++;
    }

    printf( "%d\n", count );

    return 0;
}

void *readFileIntoBuffer( FILE *f, int *outSize ) {
    fseek( f, 0, SEEK_END );
    int size = (int)ftell( f );
    fseek( f, 0, SEEK_SET );
    *outSize = size + 1;
    char *buff = malloc( *outSize );
    check( buff, "Failed to allocate buffer." );
    check( fread( buff, 1, size, f ), "fread failed." );
    buff[ size ] = '\0';
    return buff;
}

bool tryGetDataFromFile( char *inFileName, Towels *out ) {
    Towels *t = out;
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    int B;
    char *patStr = readFileIntoBuffer( f, &B );
    char *endp = patStr + B;
    
    // Find the end of the pattern string.
    char *nl = strstr( patStr, "\r\n\r\n" );
    *nl = '\0';

    // Read the patterns into an array
    char *c = patStr;
    int commaCount = 0;
    while ( *c++ ) if ( *c == ',' ) commaCount++;
    int P = commaCount + 1;
    char **patArr = malloc( P * sizeof( char* ) );
    check( patStr, "Allocation failure" );
    c = patStr;
    for ( int i = 0 ; i < P ; i++ ) {
        patArr[ i ] = c;
        while( *c >= 'a' ) c++;
        *c = '\0';
        while( *c < 'a' ) c++;
    }

    // Store the lengths of the patterns
    int *patlens = malloc( P * sizeof( int ) );
    check( patlens, "Failed to allocate memory." );
    for ( int i = 0 ; i < P ; i++ ) {
        patlens[ i ] = (int)strlen( patArr[ i ] );
    }


    // Read the designs into an array.
    char *desStr = nl + 4;
    int nlCount = 0;
    c = desStr;
    while ( *c++ ) if ( *c == '\r' ) nlCount++;
    int D = nlCount + 1;
    char **desArr = malloc( D * sizeof( char* ) );
    check( desStr, "Allocation failure" );
    c = desStr;
    for ( int i = 0 ; i < D ; i++ ) {
        desArr[ i ] = c;
        while( *c >= 'a' ) c++;
        *c = '\0';
        while( c < endp && *c < 'a' ) c++;
    }

    t->bytes = patStr;
    t->patterns = patArr;
    t->patLens = patlens;
    t->designs = desArr;
    t->P = P;
    t->D = D;
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