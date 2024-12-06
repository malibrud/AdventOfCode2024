#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    int R, C;   // Row and column count
    int stride;
    char **grid;
    char *_rawData;
} Grid;

void check( bool success, char *format, ... );
bool tryGetGridFromFile( char *inFileName, Grid *outGrid );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Grid grid;
    check( tryGetGridFromFile( argv[ 1 ], &grid ), "Error: Could not read grid from %s.", argv[ 1 ] );

    int count = 0;
    char **g = grid.grid;

    // Count matches in positive diagonals
    for ( int r = 0 ; r < grid.R - 2 ; r++ )
    for ( int c = 0 ; c < grid.C - 2 ; c++ )
    {
        if ( g[r + 1][c + 1] != 'A' ) continue;
        char ul = g[r + 0][c + 0];
        char ur = g[r + 0][c + 2];
        char ll = g[r + 2][c + 0];
        char lr = g[r + 2][c + 2];

        bool posDiag = 
            ul == 'M' && lr == 'S' ||
            ul == 'S' && lr == 'M'
        ;
        bool negDiag = 
            ur == 'M' && ll == 'S' ||
            ur == 'S' && ll == 'M'
        ;
        count += posDiag && negDiag;
    }

    printf( "%d\n", count );
    return 0;
}

bool tryGetGridFromFile( char *inFileName, Grid *outGrid ){
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    fseek( f, 0, SEEK_END);
    int N = ftell( f );
    fseek( f, 0, SEEK_SET );

    outGrid->_rawData = malloc( N );
    if ( outGrid->_rawData == NULL ) return false;
    char *data = outGrid->_rawData;

    int nRead = fread( data, 1, N, f );
    fclose( f );
    if ( nRead != N ) return false;

    char *nl = strstr( data, "\r\n" );
    outGrid->C = (int)( nl - data );
    int stride = outGrid->C + 2;
    outGrid->R = ( N + 2 ) / stride;

    outGrid->grid = malloc( outGrid->R * sizeof( char* ) );
    if ( outGrid->grid == NULL ) return false;

    for ( int i = 0 ; i < outGrid->R ; i++ ) {
        outGrid->grid[ i ] = data + i*stride;
    }

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