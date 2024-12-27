#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_BOTS 500

typedef struct {
    char **grid;
    char *bytes;
    uint8_t X, Y;   // Size of grid in x and y.
    uint8_t sx, sy;  // Location of the start.
    uint8_t ex, ey;  // Location of the end.
} Maze;

#define DIR_E ( (uint8_t)0x0 )
#define DIR_S ( (uint8_t)0x1 )
#define DIR_W ( (uint8_t)0x2 )
#define DIR_N ( (uint8_t)0x3 )

int dirs[4][2] = {
    {  1,  0 },
    {  0,  1 },
    { -1,  0 },
    {  0, -1 }
};

typedef struct {
    uint32_t score;
    uint8_t x;
    uint8_t y;
    uint8_t dir;
    char type;
} Node;

// Priority queue for 'Node' elements ordered by 'score' with the min at the top.
// Inspired by: https://www.geeksforgeeks.org/c-program-to-implement-priority-queue/
typedef struct {
    Node **nodes;
    int   count;
    int   max;
} Queue;

void qInit( Queue *q, int X, int Y ) {
    q->max = X * Y * 4;
    q->nodes = malloc( q->max * sizeof( Node* ) );
    q->count = 0;
}

bool qIsEmpty( Queue *q ) { return q->count == 0; }

void qSwap( Queue *q, int ia, int ib ) {
    Node **nodes = q->nodes;
    Node *t = nodes[ ia ];
    nodes[ ia ] = nodes[ ib ];
    nodes[ ib ] = t;
}

bool qLt( Queue *q, int ia, int ib ) {
    return q->nodes[ ia ]->score < q->nodes[ ib ]->score;
}

bool qGt( Queue *q, int ia, int ib ) {
    return q->nodes[ ia ]->score > q->nodes[ ib ]->score;
}

void qAppend( Queue *q, Node *n ) {
    q->nodes[ q->count++ ] = n;
}

bool qIsValid( Queue *q ) {
    if ( q->count > q->max ) return false;
    for ( int i = 0 ; i < q->count ; i++ ) {
        int ip = ( i - 1 ) / 2;
        int il = 2*i + 1;
        int ir = 2*i + 2;

        bool valid = true;
        if ( i != 0 )        valid = valid && !qGt( q, ip, i );
        if ( il < q->count ) valid = valid && !qGt( q, i,  il );
        if ( ir < q->count ) valid = valid && !qGt( q, i,  ir );
        if ( !valid ) return false;
    }
    return true;
}

bool qFind( Queue *q, Node *n, int *outIdx ) {
    for ( int i = 0 ; i < q->count ; i++ ) {
        if ( n == q->nodes[ i ] ) {
            *outIdx = i;
            return true;
        }
    }
    return false;
}

void qHeapUp( Queue *q, int idx ) {
    if ( idx == 0 ) return;
    int pidx = (idx-1)/2;
    if ( qGt( q, pidx, idx ) ) {
        qSwap( q, pidx, idx );
        qHeapUp( q, pidx );
    }
}

void qEnqueue( Queue *q, Node *n ) {
    assert( q->count < q->max );
    q->nodes[ q->count++ ] = n;
    qHeapUp( q, q->count - 1);
}

void qHeapDown( Queue *q, int idx ) {
    int sm = idx;
    int il = 2*idx + 1;
    int ir = 2*idx + 2;
    if ( il < q->count && qLt( q, il, sm ) ) sm = il;
    if ( ir < q->count && qLt( q, ir, sm ) ) sm = ir;
    if ( sm != idx ) {
        qSwap( q, idx, sm );
        qHeapDown( q, sm );
    }
}

Node *qDequeue( Queue *q ) {
    assert( q->count > 0 );
    Node **nodes = q->nodes;
    Node *item = nodes[ 0 ];
    nodes[ 0 ] = nodes[ --q->count ];
    qHeapDown( q, 0 );
    return item;
}

void qUpdateScore( Queue *q, int idx, uint32_t score ) {
    int ip = ( idx - 1 ) / 2;
    int il = 2*idx + 1;
    int ir = 2*idx + 2;

    q->nodes[ idx ]->score = score;
    if      (                  qLt( q, idx, ip ) ) qHeapUp( q, idx );
    else if ( il < q->count && qGt( q, idx, il ) ) qHeapDown( q, idx );
    else if ( ir < q->count && qGt( q, idx, ir ) ) qHeapDown( q, idx );

}
void check( bool success, char *format, ... );
bool tryGetDataFromFile( char *inFileName, Maze *outWarehouse );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Maze mz;
    check( tryGetDataFromFile( argv[ 1 ], &mz ), "Error: Could not read data from %s.", argv[ 1 ] );
    char **grid = mz.grid;
    int X = mz.X;
    int Y = mz.Y;
    int D = 4;

    // Create scrore grid and initialize the priority queue.
    Queue q;
    qInit( &q, mz.X, mz.Y );
    Node *nodes = malloc( mz.X * mz.Y * 4 * sizeof( Node ) );
    for ( uint8_t y = 0 ; y < Y ; y++ )
    for ( uint8_t x = 0 ; x < X ; x++ )
    for ( uint8_t d = 0 ; d < D ; d++ )
    {
        int idx = y*(D*X) + x*D + d;
        nodes[ idx ] = (Node){ UINT32_MAX, x, y, d, grid[y][x] };
        if ( x == mz.sx && y == mz.sy && d == DIR_E ) {
            nodes[ idx ].score = 0;
        }
        if ( grid[ y ][ x ] != '#') qEnqueue( &q, &nodes[ idx ] );
    }
    assert( qIsValid( &q ) );

    uint32_t finalScore = 0;
    while ( !qIsEmpty( &q ) ) {
        Node *n = qDequeue( &q );
        uint8_t nx = (uint8_t)(n->x + dirs[n->dir][0] );
        uint8_t ny = (uint8_t)(n->y + dirs[n->dir][1] );

        //printf( "Trying: (%d, %d, %d) = %d\n", n->x, n->y, n->dir, n->score );

        // Are we done?
        if ( nx == mz.ex && ny == mz.ey ) {
            finalScore = n->score + 1;
            break;
        }

        // Find the next node;
        Node *next = &nodes[ ny*X*D + nx*D + n->dir ];
        int nIdx;
        if ( qFind( &q, next, &nIdx ) ) {
            uint32_t score = n->score + 1;
            if ( score < next->score ) qUpdateScore( &q, nIdx, score );
            assert( qIsValid( &q ) );
        }

        // Try turning right, CW
        uint8_t dir = ( n->dir + 1 ) % 4;
        next = &nodes[ n->y*X*D + n->x*D + dir ];
        if ( qFind( &q, next, &nIdx ) ) {
            uint32_t score = n->score + 1000;
            if ( score < next->score ) qUpdateScore( &q, nIdx, score );
            assert( qIsValid( &q ) );
        }

        // Try turning left, CCW
        dir = ( n->dir + 3 ) % 4;
        next = &nodes[ n->y*X*D + n->x*D + dir ];
        if ( qFind( &q, next, &nIdx ) ) {
            uint32_t score = n->score + 1000;
            if ( score < next->score ) qUpdateScore( &q, nIdx, score );
            assert( qIsValid( &q ) );
        }
    }

    printf( "%d\n", finalScore );
    return 0;
}

bool tryGetDataFromFile( char *inFileName, Maze *outMaze ) {
    Maze *mz = outMaze;
    FILE *f = fopen( inFileName, "rb" );
    if ( f == NULL ) return false;

    fseek( f, 0, SEEK_END );
    int B = (int)ftell( f );
    fseek( f, 0, SEEK_SET );

    check( mz->bytes = malloc( B + 1 ), "Error: Malloc failed." );

    check( fread( mz->bytes, 1, B, f ), "Error: Failed to read file contents." );
    fclose( f );
    mz->bytes[ B ] = '\0';

    char *eol = strchr( mz->bytes, '\r' );
    int X = (int)( eol - mz->bytes );
    check ( X < 0xff, "Size of X is too large." );
    mz->X = (uint8_t) X;
    int stride = mz->X + 2;
    int Y = ( B + 2 ) / stride;
    check ( Y < 0xff, "Size of Y is too large." );
    mz->Y = (uint8_t) Y;
    
    check( mz->grid = malloc( mz->Y * sizeof( char* ) ), "Error: Malloc failed." );
    for ( int i = 0 ; i < mz->Y ; i++ ) mz->grid[ i ] = mz->bytes + i * stride;

    for ( uint8_t y = 0 ; y < mz->Y ; y++ )
    for ( uint8_t x = 0 ; x < mz->X ; x++ )
    {
        if ( mz->grid[ y ][ x ] == 'S' ) {
            mz->sx = x;
            mz->sy = y;
        }
        if ( mz->grid[ y ][ x ] == 'E' ) {
            mz->ex = x;
            mz->ey = y;
        }
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