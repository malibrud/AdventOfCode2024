#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_BYTES 3450
typedef struct {
    int X, Y;
    int N;
    int Nu;
    int GWH; // Grid width and height.
    uint8_t x[ MAX_BYTES ];
    uint8_t y[ MAX_BYTES ];
    char *_grid;
    char **grid;
} Bytes;

void check( bool success, char *format, ... );
bool tryGetDataFromFile( char *inFileName, Bytes *outComputer );

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

typedef struct _Node {
    uint32_t f; // Total score, g+h
    uint32_t g; // Total cost to get to this node.
    uint32_t h; // Heuristic to get to end node.
    uint8_t x;
    uint8_t y;
} Node;

// Priority queue for 'Node' elements ordered by 'score' with the min at the top.
// Inspired by: https://www.geeksforgeeks.org/c-program-to-implement-priority-queue/
typedef struct {
    Node **nodes;
    int   count;
    int   max;
} Queue;

void qInit( Queue *q, int N ) {
    q->max = N;
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
    return q->nodes[ ia ]->f < q->nodes[ ib ]->f;
}

bool qGt( Queue *q, int ia, int ib ) {
    return q->nodes[ ia ]->f > q->nodes[ ib ]->f;
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

void qUpdate( Queue *q, int idx ) {
    int ip = ( idx - 1 ) / 2;
    int il = 2*idx + 1;
    int ir = 2*idx + 2;

    if      (                  qLt( q, idx, ip ) ) qHeapUp( q, idx );
    else if ( il < q->count && qGt( q, idx, il ) ) qHeapDown( q, idx );
    else if ( ir < q->count && qGt( q, idx, ir ) ) qHeapDown( q, idx );

}

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Bytes b = {};
    check( tryGetDataFromFile( argv[ 1 ], &b ), "Error: Could not read data from %s.", argv[ 1 ] );
    int X = b.X;
    int Y = b.Y;

    Queue open, closed;
    qInit( &open,   X*Y );
    qInit( &closed, X*Y );

    Node *flatNodes = malloc( sizeof( Node ) * X * Y );
    Node **nGrid = malloc( sizeof( Node* ) * Y );
    for ( int i = 0 ; i < Y ; i++ ) nGrid[ i ] = flatNodes + X*i;
    for ( uint8_t y = 0 ; y < Y ; y++ )
    for ( uint8_t x = 0 ; x < X ; x++ )
    {
        Node *n = &nGrid[ y ][ x ];
        n->g = UINT32_MAX;
        n->h = UINT32_MAX;
        n->f = UINT32_MAX;
        n->x = x;
        n->y = y;
    }

    // Initialize the starting node
    nGrid[ 0 ][ 0 ].g = 0;
    nGrid[ 0 ][ 0 ].h = X + Y - 2;
    nGrid[ 0 ][ 0 ].f = X + Y - 2;
    qAppend( &open, &nGrid[0][0] );

    // Do the A* algorithm to find the end.
    int steps = 0;
    while( !qIsEmpty( &open ) ) {
        Node *q = qDequeue( &open );

        for ( int d = 0 ; d < 4 ; d++ ) {
            int nx = q->x + dirs[ d ][ 0 ];
            int ny = q->y + dirs[ d ][ 1 ];

            // Don't fall off of the grid.
            if ( nx < 0 || nx >= X ) continue;
            if ( ny < 0 || ny >= Y ) continue;

            // Are we at the destination?
            if ( nx == b.X-1 && ny == b.Y-1 ) {
                steps = q->g + 1;
                goto DONE;
            }

            // Are we hitting a barrier?
            if ( b.grid[ ny ][ nx ] == '#' ) continue;

            // Determine score for successor and update/add to the open set.
            Node *s = &nGrid[ ny ][ nx ];
            uint32_t g = q->g + 1;
            uint32_t h = abs( nx - ( X - 1 ) ) + abs( ny - ( Y - 1 ) );
            uint32_t f = g + h;

            if ( g < s->g ) {
                s->f = f;
                s->g = g;
                s->h = h;
                int idx;
                if ( qFind( &open, s, &idx ) ) {
                    qUpdate( &open, idx );
                }
                else {
                    qEnqueue( &open, s );
                }
            }
        }
    }
    DONE:
    printf( "%d\n", steps );

    return 0;
}

bool tryGetDataFromFile( char *inFileName, Bytes *outBytes ) {
    Bytes *b = outBytes;
    FILE *f = fopen( inFileName, "r" );
    if ( f == NULL ) return false;

    // Read the X-Y pairs
    while( !feof( f ) ) {
        assert( b->N < MAX_BYTES );
        int x, y;
        check( 2 == fscanf( f, "%d,%d\n", &x, &y ), "Failed to read x,y pair" );
        b->x[ b->N ] = (uint8_t)x;
        b->y[ b->N ] = (uint8_t)y;
        b->N++;
    }
    fclose(f);

    // Determine file dependent values, i.e. test vs. data.
    if ( strstr( inFileName, "test.txt") ) {
        b->X = 6+1;
        b->Y = 6+1;
        b->Nu = 12;
    } else
    if ( strstr( inFileName, "data.txt") ) {
        b->X = 70+1;
        b->Y = 70+1;
        b->Nu = 1024;
    }
    else assert( false );

    // Initialize the grid
    int stride = b->X + 1;
    int B = stride * b->Y + 1;
    b->_grid = malloc( B );
    b->grid  = malloc( b->Y * sizeof( void* ) );
    check( b->_grid && b->grid, "Malloc failed" );

    memset( b->_grid, '.', stride * b->Y );
    for ( int y = 0 ; y < b->Y ; y++ ) {
        b->grid[ y ] = b->_grid + y*stride;
        b->grid[ y ][ b->X ] = '\n';
    }
    b->_grid[ stride * b->Y ] = '\0';
    for ( int i = 0 ; i < b->Nu ; i++ ) {
        int x = b->x[ i ];
        int y = b->y[ i ];
        b->grid[ y ][ x ] = '#';
    }

    printf( "%s", b->_grid );
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