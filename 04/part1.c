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

    // Count matches in rows
    for ( int r = 0 ; r < grid.R     ; r++ )
    for ( int c = 0 ; c < grid.C - 3 ; c++ )
    {
        count += g[r][c + 0] == 'X'
              && g[r][c + 1] == 'M'
              && g[r][c + 2] == 'A'
              && g[r][c + 3] == 'S'
        ;
        count += g[r][c + 0] == 'S'
              && g[r][c + 1] == 'A'
              && g[r][c + 2] == 'M'
              && g[r][c + 3] == 'X'
        ;
    }

    // Count matches in columns
    for ( int r = 0 ; r < grid.R - 3 ; r++ )
    for ( int c = 0 ; c < grid.C     ; c++ )
    {
        count += g[r + 0][c] == 'X'
              && g[r + 1][c] == 'M'
              && g[r + 2][c] == 'A'
              && g[r + 3][c] == 'S'
        ;
        count += g[r + 0][c] == 'S'
              && g[r + 1][c] == 'A'
              && g[r + 2][c] == 'M'
              && g[r + 3][c] == 'X'
        ;
    }

    // Count matches in positive diagonals
    for ( int r = 0 ; r < grid.R - 3 ; r++ )
    for ( int c = 0 ; c < grid.C - 3 ; c++ )
    {
        count += g[r + 0][c + 0] == 'X'
              && g[r + 1][c + 1] == 'M'
              && g[r + 2][c + 2] == 'A'
              && g[r + 3][c + 3] == 'S'
        ;
        count += g[r + 0][c + 0] == 'S'
              && g[r + 1][c + 1] == 'A'
              && g[r + 2][c + 2] == 'M'
              && g[r + 3][c + 3] == 'X'
        ;
    }

    // Count matches in negative diagonals
    for ( int r = 0 ; r < grid.R - 3 ; r++ )
    for ( int c = 0 ; c < grid.C - 3 ; c++ )
    {
        count += g[r + 3][c + 0] == 'X'
              && g[r + 2][c + 1] == 'M'
              && g[r + 1][c + 2] == 'A'
              && g[r + 0][c + 3] == 'S'
        ;
        count += g[r + 3][c + 0] == 'S'
              && g[r + 2][c + 1] == 'A'
              && g[r + 1][c + 2] == 'M'
              && g[r + 0][c + 3] == 'X'
        ;
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