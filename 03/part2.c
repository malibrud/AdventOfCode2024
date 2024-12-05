#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

void check( bool success, char *format, ... );
bool tryGetFileBytes( char *inFileName, char **outDataPtr, int *outByteCount );
bool tryParseMult( char **inoutCursor, int *outProduct );
bool tryParseInt( char **inoutCursor, int *outVal );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    char *cursor;
    int N;
    check( tryGetFileBytes( argv[ 1 ], &cursor, &N ), "Error: Could not read bytes from %s.", argv[ 1 ] );

    int sum = 0;
    bool enabled = true;
    while ( *cursor ) {
        int product;
        if ( strncmp( cursor, "do()", 4 ) == 0 ) { 
            enabled = true;
            cursor += 4; 
            continue; 
        } else
        if ( strncmp( cursor, "don't()", 7 )  == 0) { 
            enabled = false;
            cursor += 7; 
            continue; 
        } else
        if ( enabled && strncmp( cursor, "mul(", 4 ) == 0 ) { 
            if ( tryParseMult( &cursor, &product ) ) {
                sum += product;
            } 
        } else { 
            cursor++; 
        }
    }
    printf( "%d\n", sum );

    return 0;
}

bool tryParseInt( char **inoutCursor, int *outVal ) {
    char *ptr = *inoutCursor;
    while( isdigit( *ptr ) ) ptr++;
    if ( ptr == *inoutCursor ) return false;
    sscanf( *inoutCursor, "%d", outVal );
    *inoutCursor = ptr;
    return true;
} 

bool tryParseMult( char **inoutCursor, int *outProduct ) {
    int a, b;
    assert( strncmp( *inoutCursor, "mul(", 4 ) == 0 );
    *inoutCursor += 4;

    if ( !tryParseInt( inoutCursor, &a ) ) return false;
    if ( a > 999 )                         return false;
    if ( *(*inoutCursor)++ != ',' )        return false;
    if ( !tryParseInt( inoutCursor, &b ) ) return false;
    if ( a > 999 )                         return false;
    if ( *(*inoutCursor)++ != ')' )        return false;
    *outProduct = a * b;
    return true;
}

bool tryGetFileBytes( char *inFileName, char **outDataPtr, int *outByteCount ){
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    fseek( f, 0, SEEK_END);
    int N = ftell( f );
    fseek( f, 0, SEEK_SET );

    *outDataPtr = malloc( N + 1 );
    if ( *outDataPtr == NULL ) return false;

    size_t nRead = fread( *outDataPtr, 1, N, f );
    fclose( f );
    if ( nRead != N ) { 
        free( *outDataPtr );
        return false;
    }
    (*outDataPtr)[ N ] = '\0';
    *outByteCount = N;
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