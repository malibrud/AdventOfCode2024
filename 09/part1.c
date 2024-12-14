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
    char *map;
    int mapSize;
} DiskMap;

void check( bool success, char *format, ... );
bool tryGetDiskMapFromFile( char *inFileName, DiskMap *outMap );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    DiskMap map;
    check( tryGetDiskMapFromFile( argv[ 1 ], &map ), "Error: Could not read disk map from %s.", argv[ 1 ] );

    // Get the uncompressed size
    int uncSize = 0;
    char *ptr = map.map;
    while ( *ptr ) uncSize += *ptr++ - '0';
    int *uncDisk = malloc( uncSize * sizeof( int ) );

    // Uncompress the disk
    int ui = 0;
    for ( int i = 0 ; i < map.mapSize ; i++ ) {
        int blockSize = map.map[ i ] - '0';
        int fillVal = ( i % 2 == 0 ) ? i / 2 : -1;
        for ( int j = 0 ; j < blockSize ; j++ ) {
            uncDisk[ ui++ ] = fillVal;
        }
    }

    // Compress the blocks
    int *dst = uncDisk;
    int *src = uncDisk + uncSize - 1;
    while ( true ) {
        while ( *dst != -1 && dst <= src ) dst++;
        while ( *src == -1 && dst <= src ) src--;
        if ( dst >= src ) break;
        *dst = *src;
        *src = -1;
    }

    // Compute the checksum.
    uint64_t sum = 0;
    for ( int i = 0 ; uncDisk[ i ] != -1 ; i++ ) {
        sum += i * uncDisk[ i ];
    }

    printf( "%lld\n", sum );
    return 0;
}

bool tryGetDiskMapFromFile( char *inFileName, DiskMap *outMap ) {
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    fseek( f, 0, SEEK_END);
    int N = ftell( f );
    fseek( f, 0, SEEK_SET );

    outMap->map = malloc( N + 1 );
    if ( outMap->map == NULL ) return false;
    outMap->mapSize = N;

    int nRead = (int)fread( outMap->map, 1, N, f );
    fclose( f );
    if ( nRead != N ) return false;
    outMap->map[ N ] = '\0';

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