# Advent of Code Solutions for 2024
**Author**: Mark Brudnak

Below are my solutions for the 2024 [Advent of Code](https://adventofcode.com/).

## [Day 01](https://adventofcode.com/2024/day/1)

Decided to code in plain ole C today.  Wrote a varargs error check given that I am using fixed length arrays.

```C
void check( bool success, int exitval, char *format, ... ) {
    if ( success ) return;
    va_list args;
    va_start( args, format );
    vprintf( format, args );
    va_end( args ); 
    printf("\n");
    exit( exitval );
}
```

## [Day 02](https://adventofcode.com/2024/day/2)

First part was pretty straight forward.  Second part after trying a method to find faults in the part 1 algorithm, I found that 
finding a fault in the first or second position created a lot of complexity and then decided to just brute force each fault case.
Since finding a 'safe' sequence with one element skipped is sufficient, I did not check the whole sequence.

To parse a secquence of space delimited integers I wrote the following:

```C
bool tryNextInt( char **inoutStr, int *outVal ) {
    while ( isspace( **inoutStr ) ) (*inoutStr)++;
    if ( **inoutStr == '\0' ) return false;

    if ( sscanf( *inoutStr, "%d", outVal ) == 1 ) {
        while ( isdigit( **inoutStr ) ) (*inoutStr)++;
        return true;
    }
    return false;
}
```

## [Day 03](https://adventofcode.com/2024/day/3)

Today's problem was about parsing.  For part 1 I wrote the following function which just scans along until it finds the next valid `mul()` instruction.

```C
bool tryParseMult( char **inoutCursor, int *outProduct ) {
    int a, b;
    while( true ) {
        char *mulPtr = strstr( *inoutCursor, "mul(" );
        if ( mulPtr == NULL ) return false;
        *inoutCursor = mulPtr + 4;

        if ( !tryParseInt( inoutCursor, &a ) ) continue;
        if ( a > 999 )                         continue;
        if ( *(*inoutCursor)++ != ',' )        continue;
        if ( !tryParseInt( inoutCursor, &b ) ) continue;
        if ( a > 999 )                         continue;
        if ( *(*inoutCursor)++ != ')' )        continue;
        *outProduct = a * b;
        return true;
    }
    return false;
}
```

For part 2 I used a traditional scanning appraoch for looking for keyword matches and then either executing the `mul()` or setting the enabled / disabled state.  Most of the heavy lifting was done in the `main()` function:

```C
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
```

In this case `tryParseMult()` no longer searches, but relies on the main loop to find the starting `mul(` character sequence.

```C
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
```

## [Day 04](https://adventofcode.com/2024/day/4)

Pretty simple word search algorithm.  In both parts, the strategy was just to scan over the grid and pattern match.
My approach to pattern matching is exemplified in the part 2 main function:

```C
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
```


## [Day 05](https://adventofcode.com/2024/day/5)

Since the _page_ numbers were two digit numbers (i.e. 01..99 ), i deciced to create a boolean array of size 10,000 where 
the index encoded the two numbers. The thousands and hundreds digits stored the first page and the tens and ones place stored
the second _page_.  To test for validity of an _update_ I went through the _update_ on a pair wise basis.  If there 
was a _rule_ violation, I skipped to the next _update_.

The code for encoding the rules as a boolean array is

```C
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
```

This made the main function relative simple and straight forward.  Note that I use `goto` to break out of the two inner loops.  Here is the main function:

```C
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
```

For part 2, same logic was used to detect violations, but I just used my rule base as a "comparison" to implement a simple bubble sort.  

```C
int main( int argc, char **argv ) {

    // Omitted for brevity... same as part 1

    for ( int u = 0 ; u < nUpdates ; u++ ) {

        //
        // Same code for part 1
        //

    RULE_VIOLATION:
        // Sort the pages using the rules
        for ( int i = 0   ; i < N ; i++ )
        for ( int j = i+1 ; j < N ; j++ )
        {
            // look for rule in reverse order to detect a rule which is violated by the pair.
            int page1 = pages[ i ];
            int page2 = pages[ j ];
            int idx = 100*page2 + page1;
            if ( rules[ idx ] ) {
                pages[ i ] = page2;
                pages[ j ] = page1;
            }
        }

        assert( N % 2 == 1 );
        sum += pages[ N / 2 ];
        continue;
    }

    printf( "%d\n", sum );
    return 0;
}
```

## [Day 06](https://adventofcode.com/2024/day/6)

Day six was a maze navigation puzzle.  Part 1 was pretty straight forward.  I encoded the direction as a vector `(dx, dy)`
which was then rotated by 90 deg if an obstacle was it.  

The main funtion implements the navigation as follows:

```C
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read map from %s.", argv[ 1 ] );
    int X = map.X;
    int Y = map.Y;

    int steps = 1;
    while ( true ) {
        map.map[ map.y ][ map.x ] = 'X';
        int nx = map.x + map.dx;
        int ny = map.y + map.dy;

        if ( nx < 0 || nx == X ) break;
        if ( ny < 0 || ny == Y ) break;
        
        if ( map.map[ ny ][ nx ] == '#' ) {
            rotateBy90deg( &map.dx, &map.dy );
            continue;
        }

        map.x = nx;
        map.y = ny;
        if ( map.map[ ny ][ nx ] != 'X' ) steps++;
    }
    printf( "%d\n", steps );
    return 0;
}
```

In part 2, I was asked to add one additional obstacle for each open position.  I was then to count the number of such 
configuration for which the maze hit an infinite loop.  I chose to code each visited spot with a number starting with ascii `'1'`
and then increment the value by 1 each time it was visited.  In this way, an infinite loop could be detected by a total of 
4 visits (i.e. entering from each direction at least once).  This was all wrapped in a loop which had to reset the map
for every iteration.

```C
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read map from %s.", argv[ 1 ] );
    char* origMap = malloc( map.nRawBytes );
    check ( origMap, "Could not allocate origMap." );

    // Remember original state;
    memcpy( origMap, map._rawData, map.nRawBytes );
    Map map0 = map;
    int X = map.X;
    int Y = map.Y;

    int count = 0;
    for ( int yo = 0 ; yo < Y ; yo++ )
    for ( int xo = 0 ; xo < X ; xo++ )
    {
        // Restore original state.
        memcpy( map._rawData, origMap, map.nRawBytes );
        map = map0;

        char p = map.map[ yo ][ xo ];
        if ( p == '#'  ) continue;
        if ( p == '^'  ) continue;

        // Place the extra obstacle.
        map.map[ yo ][ xo ] = 'O';

        while ( true ) {
            if ( map.map[ map.y ][ map.x ] == '4' ) {
                count++;
                break;
            }

            int nx = map.x + map.dx;
            int ny = map.y + map.dy;

            if ( nx < 0 || nx == X ) break;
            if ( ny < 0 || ny == Y ) break;
            
            char c = map.map[ ny ][ nx ];
            if ( c == '#' || c == 'O' ) {
                rotateBy90deg( &map.dx, &map.dy );
                continue;
            }

            c = map.map[ map.y ][ map.x ];
            if      ( c == '.' ) c = '1';
            else if ( c == '^' ) c = '1';
            else if ( c >= '1' ) c++;
            map.map[ map.y ][ map.x ] = c;

            map.x = nx;
            map.y = ny;
        }
    }

    printf( "%d\n", count );
    return 0;
}
```

## [Day 07](https://adventofcode.com/2024/day/7)

Essence of the problem was to explore all permutations of operators between integers to test against an expected result.
Part 1 was easy because there were two alternatives and the permutations could be represented by a bit mask.  For part 2
I used the same approach but with base 3 numbers.  Because I could not directly mask the bits from a base 3 number, 
extraction of the digits was a little more involved but nonetheless the same as in the base 2 numbering systems.

Part 1 solution looks like this:

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Equation eqns[ MAX_EQNS ];
    int E;
    check( tryGetEqnsFromFile( argv[ 1 ], eqns, &E ), "Error: Could not read equations from %s.", argv[ 1 ] );

    uint64_t sum = 0;
    for ( int e = 0 ; e < E ; e++ ) {
        Equation *eq = eqns + e;
        int *nums = eq->numbers;
        int nOp = eq->count - 1;
        int P = 1 << nOp;
        for ( int mask = 0 ; mask < P ; mask++ )
        {
            uint64_t result = nums[ 0 ];
            for ( int op = 0 ; op < nOp ; op++ ) {
                int num = nums[ op + 1 ];
                if ( mask & ( 1 << op ) ) result += num;
                else                      result *= num;
            }
            if ( result == eq->result ) {
                sum += result;
                break;
            }
        }
    }

    printf( "%lld\n", sum );
    return 0;
}
```

and the part 2 solution looks like this:

```C
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Equation eqns[ MAX_EQNS ];
    int E;
    check( tryGetEqnsFromFile( argv[ 1 ], eqns, &E ), "Error: Could not read equations from %s.", argv[ 1 ] );

    uint64_t sum = 0;
    for ( int e = 0 ; e < E ; e++ ) {
        Equation *eq = eqns + e;
        int *nums = eq->numbers;
        int nOp = eq->count - 1;
        uint64_t P = threeToThePower( nOp );
        for ( uint64_t mask = 0 ; mask < P ; mask++ )
        {
            uint64_t result = nums[ 0 ];
            uint64_t place = 1;
            for ( int op = 0 ; op < nOp ; op++ ) {
                int num = nums[ op + 1 ];
                uint64_t digit = ( mask % ( place * 3 ) ) / place;
                place *= 3;
                if      ( digit == 0 ) result *= num;
                else if ( digit == 1 ) result += num;
                else if ( digit == 2 ) result = shiftByDigitCountIn( result, num ) + num;
            }
            if ( result == eq->result ) {
                sum += result;
                break;
            }
        }
    }

    printf( "%lld\n", sum );
    return 0;
}
```

## [Day 08](https://adventofcode.com/2024/day/8)

This problem was basic vector math.  Find all pairs of like _frequencies_ and find the same points projected linearly
along the line of the two.  Parts 1 and 2 are very similar.  Here I only post the part 2 solution because it is a 
generalization of the part 1 solution.

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read equations from %s.", argv[ 1 ] );
    int Y = map.Y;
    int X = map.X;

    // Find the anti-nodes
    int N = map.rawSize;
    for ( int i = 0 ; i < N ; i++)
    {
        int x1 = i % map.stride;
        int y1 = i / map.stride;
        if ( map.map[y1][x1] == '.' ) continue;
        if ( x1 >= X ) continue;
        char c1 = map.map[y1][x1];
        for ( int j = i + 1 ; j < N ; j++)
        {
            int x2 = j % map.stride;
            int y2 = j / map.stride;
            if ( map.map[y2][x2] != c1 ) continue;
            if ( x2 >= X ) continue;
            int dx = x2 - x1;
            int dy = y2 - y1;
            int x, y;

            // Try antinodes behind x1, y1
            x = x1;
            y = y1;
            int k = 0;
            while ( x >= 0 && x < X && y >= 0 && y < Y ) {
                map.amap[y][x] = '#';
                x = x1 - k*dx;
                y = y1 - k*dy;
                k++;
            } 

            // Try antinodes beyond x2, y2
            x = x2;
            y = y2;
            k = 0;
            while ( x >= 0 && x < X && y >= 0 && y < Y ) {
                map.amap[y][x] = '#';
                x = x2 + k*dx;
                y = y2 + k*dy;
                k++;
            } 
        }
    }

    // Count the anti-nodes.
    int count = 0;
    for ( int y = 0 ; y < Y ; y++)
    for ( int x = 0 ; x < Y ; x++)
    {
        if ( map.amap[y][x] == '#' ) count++;
    }

    printf( "%d\n", count );
    return 0;
}
```

## [Day 09](https://adventofcode.com/2024/day/9)

This was about expanding provided information in to fragmented disk and then compressing it down to fill free space.  For part 1
it was done by _blocks_ and for part 2 is was done by _file_.

Part 1 solution is:

```c
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
```

And part 2 solution is a little more complicated for the compression part.  The compression part only follows:

```c
    // Compress the blocks
    int *src = uncDisk + uncSize - 1;
    while ( src > uncDisk ) {
        while ( *src == -1 && src >= uncDisk ) src--;
        int srcLen = 0;
        int c = *src;
        while( src[ -srcLen ] == c) srcLen++;
        src = src - srcLen + 1;

        int *dst = uncDisk;
        while ( dst < src )
        {
            while ( *dst != -1 && dst <= src ) dst++;
            int dstLen = 0;
            while ( dst[ dstLen ] == -1 ) dstLen++;
            if ( dstLen < srcLen ) {
                dst += dstLen;
                continue;
            }

            for ( int i = 0 ; i < srcLen ; i++ ) 
            {
                dst[ i ] = src[ i ];
                src[ i ] = -1;
            }
            break;
        }
        src--;
    }
```

## [Day 10](https://adventofcode.com/2024/day/10)

This was a map search problem that required finding the count of paths that ended at unique places (part 1) and the total
number of unique paths (part 2).  Part 1 was actually more complex than part 2 in that I had to remember that I visited 
a spot before.  For this reason I am only showing the part 1 solution here.  To get to part 2, you only need to remove
the logic surrounding `VISITED_BIT` AND `VALUE_MASK`.  Because the values are ascii `'0'` to `'9'` I am free to use the 
sixth bit to indicate a visited state.  For each new search, this needs to be reset.  Here is the code.

```c
#define VISITED_BIT 0x40
#define VALUE_MASK  0x3f

typedef struct {
    int X, Y;   // x and y dimensions of the map
    char **map;
    char *rawMap;
    int rawSize;
    int stride;
} Map;

void check( bool success, char *format, ... );
bool tryGetMapFromFile( char *inFileName, Map *outMap );
int getTrailTo9Count( Map *map, int x, int y );

int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read disk map from %s.", argv[ 1 ] );
    int X = map.X;
    int Y = map.Y;

    int count = 0;
    for ( int y = 0 ; y < Y ; y++ )
    for ( int x = 0 ; x < X ; x++ )
    {
        if ( map.map[ y ][ x ] != '0' ) continue;
        count += getTrailTo9Count( &map, x, y );

        // clear the visited bit
        for ( int i = 0 ; i < map.rawSize ; i++ ) {
            map.rawMap[ i ] &= VALUE_MASK;
        }
    }

    printf( "%d\n", count );
    return 0;
}

int getTrailTo9Count( Map *map, int x, int y ) {
    char curVal = map->map[ y ][ x ];
    if ( curVal & VISITED_BIT ) return 0;
    map->map[ y ][ x ] = curVal | VISITED_BIT;

    if ( curVal == '9' ) {
        // printf( "(%d, %d) = %c, count = %d\n", x, y, curVal, 1 );
        return 1;
    }

    int count = 0;
    int dx = 1;
    int dy = 0;
    for ( int i = 0 ; i < 4 ; i++ ) {
        int nx = x + dx;
        int ny = y + dy;
        int ndx = -dy;
        int ndy = +dx;
        dy = ndy;
        dx = ndx;

        if ( nx < 0 || nx >= map->X ) continue;
        if ( ny < 0 || ny >= map->Y ) continue;
        char nxtVal = map->map[ ny ][ nx ];
        if ( nxtVal == curVal + 1 ) count += getTrailTo9Count( map, nx, ny );
    }
    // printf( "(%d, %d) = %c, count = %d\n", x, y, curVal, count );
    return count;
}
```

## [Day 11](https://adventofcode.com/2024/day/11)

Parts 1 and 2 only differed by the depth of the recursion with part 1 being 25 deep and part 2 being 75 deep.  This made
a huge difference in terms of performance.  With part 1 a recursive approach wored in reasonable time, for part 2 this approach
was infeasible due to the long running times.

To improve performance for part 2, I develop a very simple grid based caching scheme where counts were stored in an x,y grid
with x correstonding to the _blinks_ and y correstonding to the _stone_ value.  I only cashed results for the bottom 1,000
stone values as I figured that these would be more common.  Anyway, this is what the main and recursive counting function looked
like:

```c
#define BLINKS 75
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    uint64_t *stones;
    int N;
    check( tryGetStonesFromFile( argv[ 1 ], &stones, &N ), "Error: Could not read %s.", argv[ 1 ] );

    initCache( 1000, BLINKS );

    uint64_t count = 0;
    for ( int i = 0 ; i < N ; i++ ) {
        printf( "Computing count for %lld\n", stones[ i ] );
        count += countStones( stones[ i ], BLINKS );
    }


    printf( "%lld\n", count );
    return 0;
}

int digits( uint64_t val );
uint64_t pow10( int pow );

uint64_t countStones( uint64_t stone, int blinks ) {
    uint64_t count;
    if      ( blinks == 0 ) count = 1;
    else if ( tryGetCount( stone, blinks, &count ) ) return count;
    else if ( stone == 0 ) count = countStones( 1, blinks - 1 );
    else {
        int d = digits( stone );
        if ( d % 2 == 0 ) {
            uint64_t mod = pow10( d / 2 );
            uint64_t lCount = countStones( stone / mod, blinks - 1);
            uint64_t rCount = countStones( stone % mod, blinks - 1);
            count = lCount + rCount ;
        }
        else {
           count = countStones( stone * 2024, blinks - 1 );
        }
    }
    trySetCount( stone, blinks, count );
    return count;
}
```

and the grid based caching scheme had three functions, an init, a get and a set.  This is what the code looked like:

```c
typedef struct {
    uint64_t *cache;
    int S;  // Number of stones / rows
    int B;  // Number of blinks / columns
} Cache;

Cache g_cache;

void initCache( int nStones, int nBlinks ) {
    Cache *c = &g_cache;
    c->S = nStones;
    c->B = nBlinks;
    size_t size = c->S * c->B * sizeof( uint64_t);
    c->cache = malloc( c->S * c->B * sizeof( uint64_t) );
    memset( c->cache, 0xff, size );
}

bool tryGetCount( uint64_t stone, int blinks, uint64_t *outCount ) {
    Cache *c = &g_cache;
    if ( blinks >= c->B ) return false;
    if ( stone  >= c->S ) return false;
    size_t idx = stone * c->B + blinks;
    if ( c->cache[ idx ] != UINT64_MAX ) {
        *outCount = c->cache[ idx ];
        return true;
    }
    return false;
}

void trySetCount( uint64_t stone, int blinks, uint64_t count ) {
    Cache *c = &g_cache;
    if ( blinks >= c->B ) return;
    if ( stone  >= c->S ) return;
    size_t idx = stone * c->B + blinks;
    c->cache[ idx ] = count;
}
```

## [Day 12](https://adventofcode.com/2024/day/12)

Finding the area and perimeter of a connected group of letters in a grid.  For part one I chose to use a recursive approach.
I used the 6th bit (i.e 32) to flag that a cell had been visited.  Flipping this bit changes the letter from upper case
to lowercase, making it easy to see in a debugger.  Here is the solution.

```C
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read disk map from %s.", argv[ 1 ] );
    int X = map.X;
    int Y = map.Y;

    int total = 0;
    for ( int y = 0 ; y < Y ; y++ )
    for ( int x = 0 ; x < X ; x++ )
    {
        if ( map.map[ y ][ x ] & VISITED_BIT ) continue;
        int perimeter = 0;
        int area = 0;
        getPerimeterAndArea( &map, x, y, &perimeter, &area );
        total += perimeter * area;
    }

    printf( "%d\n", total );
    return 0;
}

void getPerimeterAndArea( Map *map, int x, int y, int *outPerim, int *outArea ) {
    int X = map->X;
    int Y = map->Y;
    char **m = map->map;

    assert( 0 <= x && x < X );
    assert( 0 <= y && y < Y );
    assert( ( m[ y ][ x ] & VISITED_BIT)  == 0 );

    char p = m[ y ][ x ] |= VISITED_BIT;
    (*outArea)++;
    
    int dx = 1;
    int dy = 0;

    for ( int i = 0 ; i < 4 ; i++ ) {
        rot90( &dx, &dy );
        int nx = x + dx;
        int ny = y + dy;
        
        // Test for edge of map both horizontal and vertical.
        if ( nx <  0 || nx >= X || ny <  0 || ny >= Y ) {
            (*outPerim)++;
            continue;
        }
        char np = m[ ny ][ nx ];

        // Test to see if the new plot has been visited.
        if ( p == np ) continue;

        // Test to see if this is the boundary of a region;
        if ( p != ( np | VISITED_BIT ) ) {
            (*outPerim)++;
            continue;
        }

        // This new point is in the region, recurse.
        getPerimeterAndArea( map, nx, ny, outPerim, outArea );
    }
}
```


Part 2 was similar, howver finding the side count was a bit challenging.  I deided on a two step process where
the area step marked the borders, then a second step counted the sides.  I used the lower 4 bits of a character
to mark the top, right, bottom and left edges.  Here is the solution:


```C
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Map map;
    check( tryGetMapFromFile( argv[ 1 ], &map ), "Error: Could not read disk map from %s.", argv[ 1 ] );
    initPerimeterMap( &map );
    int X = map.X;
    int Y = map.Y;

    int total = 0;
    for ( int y = 0 ; y < Y ; y++ )
    for ( int x = 0 ; x < X ; x++ )
    {
        if ( map.map[ y ][ x ] & VISITED_BIT ) continue;
        int sides = 0;
        int area = 0;
        resetPerimeterMap( &map );
        getAreaAndMarkPerimeter( &map, x, y, &area );
        getSideCount( &map, &sides );
        total += sides * area;
    }

    printf( "%d\n", total );
    return 0;
}

void rot90( int *x, int *y ) { 
    assert( *x * *y == 0 );
    int nx = -*y;
    int ny = +*x;
    *x = nx;
    *y = ny;
}

void getAreaAndMarkPerimeter( Map *map, int x, int y, int *outArea ) {
    int X = map->X;
    int Y = map->Y;
    char **m = map->map;
    uint8_t **p = map->perim;

    assert( 0 <= x && x < X );
    assert( 0 <= y && y < Y );
    assert( ( m[ y ][ x ] & VISITED_BIT)  == 0 );

    char c = m[ y ][ x ] |= VISITED_BIT;
    (*outArea)++;
    
    int dx = 1;
    int dy = 0;

    for ( int i = 0 ; i < 4 ; i++ ) {
        rot90( &dx, &dy );
        int nx = x + dx;
        int ny = y + dy;

        char side = '\0';
        if ( dx == -1 ) side = L_BIT;
        if ( dx == +1 ) side = R_BIT;
        if ( dy == -1 ) side = T_BIT;
        if ( dy == +1 ) side = B_BIT;
        
        // Test for boundary edges
        if ( nx <  0 || nx >= X || ny <  0 || ny >= Y ) { 
            p[ y ][ x ] |= ' ' + side; 
            continue; 
        }
        uint8_t np = m[ ny ][ nx ];

        // Test to see if the new plot has been visited.
        if ( c == np ) continue;

        // Test to see if this is the boundary of a region;
        if ( c != ( np | VISITED_BIT ) ) {
            p[ y ][ x ] |= ' ' + side; 
            continue;
        }

        // This new point is in the region, recurse.
        getAreaAndMarkPerimeter( map, nx, ny, outArea );
    }
}

int searchDirs[4][2] = {
    {  0,  1 },  // Boundary on right, search down.
    {  1,  0 },  // Boundary on bottom, search right.
    {  0,  1 },  // Boundary on left, search down.
    {  1,  0 },  // Boundary on top, search right.
};

void getSideCount( Map *map, int *outSides ) {
    int X = map->X;
    int Y = map->Y;
    char **m = map->perim;

    for ( int x = 0 ; x < X ; x++ )
    for ( int y = 0 ; y < Y ; y++ )
    {
        char c = m[ y ][ x ];
        if ( c == ' ' ) continue;

        int bit = 1;
        for ( int i = 0 ; i < 4 ; i++, bit <<= 1 ) {
            if ( !( c & bit ) ) continue;
            (*outSides)++;
            int dx = searchDirs[ i ][ 0 ];
            int dy = searchDirs[ i ][ 1 ];

            int nx = x;
            int ny = y;
            while ( nx >= 0 && nx < X && ny >= 0 && ny < Y ) {
                if ( ( m[ ny ][ nx ] & bit ) == 0 ) break;
                m[ ny ][ nx ] &= ~bit;
                nx += dx;
                ny += dy;
            }
        }
    }
}
```

## [Day 13](https://adventofcode.com/2024/day/13)

This was basically to solve a system of equations with integers.  Part 1 was addressable with a namual search
through all possible cases.  Note that since the system was full rank, a search was not necessary, but it 
worked just fine.  Here is the part 1 solution.

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Machine machines[ MAX_MACHINES ];
    int M;
    check( tryGetMachinesFromFile( argv[ 1 ], machines, &M ), "Error: Could not read equations from %s.", argv[ 1 ] );

    uint64_t sum = 0;
    for ( int m = 0 ; m < M ; m++ ) {
        Machine *ma = machines + m;
        for ( int a = 0 ; a * ma->ax <= ma->px ; a++ ) {
            int ax = ma->ax;
            int ay = ma->ay;
            int bx = ma->bx;
            int by = ma->by;
            int px = ma->px;
            int py = ma->py;

            if ( 
                ( px - a * ax ) % bx == 0 &&
                ( py - a * ay ) % by == 0
            ) {
                int b = ( px - a * ax ) / bx;
                if ( a * ay + b * by == py ) {
                    sum += 3 * a + b;
                    break;
                }
            }
        }
    }

    printf( "%lld\n", sum );
    return 0;
}
```

For part 2 the search space greatly expanded to the order fo 10 trillion as the solution.  The brute force
method took way too long.  Since it is a system of equations, I converted everything to [double precision](https://en.wikipedia.org/wiki/IEEE_754)
which can hold 15.9 decimal digits (a quadrillion).  I then used [Gaussian Elimination](https://en.wikipedia.org/wiki/Gaussian_elimination) to solve for the number of button presses.  Before using the solution
I checked two things: (1) that the solutions were positive and (2) that the nearest integer solution still
solved the problem.  Here is the solution:

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Machine machines[ MAX_MACHINES ];
    int M;
    check( tryGetMachinesFromFile( argv[ 1 ], machines, &M ), "Error: Could not read equations from %s.", argv[ 1 ] );

    uint64_t sum = 0;
    for ( int m = 0 ; m < M ; m++ ) {
        Machine *ma = machines + m;
        assert ( ma->ax * ma->by - ma->ay * ma->bx != 0 ); // If determinent is 0, this approach will not work.

        int ax = ma->ax;
        int ay = ma->ay;
        int bx = ma->bx;
        int by = ma->by;
        int64_t px = ma->px + 10000000000000LL;
        int64_t py = ma->py + 10000000000000LL;

        // Cast the problem as a system of equations.
        //   A * x = b
        //
        //   A = [ a11 a12 ] = [ ax ax ]
        //       [ a21 a22 ]   [ ay ay ]
        //
        //   b = [ b1 b2 ]^T = [ px py ]^T
        //
        double a11 = ax;
        double a12 = bx;
        double a21 = ay;
        double a22 = by;
        double b1 = (double)px;
        double b2 = (double)py;
        double x1, x2;

        // Solve by Gaussian elimination
        // Subtract portion of first row from second row to zero out a21.

        // Reduce to Row Echelon Form (REF)
        double f = a21 / a11;
        a21 -= f * a11;
        a22 -= f * a12;
        b2  -= f * b1;

        // Now solve: a22 * x2 = b2
        x2 = b2 / a22;

        // Now solve: a11 * x1 + a12 * x2 = b1
        x1 = ( b1 - a12 * x2 ) / a11;

        // Check our solution with constraints
        if ( x1 < 0 || x2 < 0 ) continue;

        // Convert to closest integer solution and check 
        int64_t a = (int64_t)round( x1 );
        int64_t b = (int64_t)round( x2 );
        if (
            a * ax + b * bx == px &&
            a * ay + b * by == py
        ) {
            sum += 3 * a + b;
        }
    }

    printf( "%lld\n", sum );
    return 0;
}
```

## [Day 14](https://adventofcode.com/2024/day/14)

Robots moving with modular arithmatic.  Pretty striaght forward.  Don't need to put steps in a for loop.
I directly calculated the position after 100 steps.  Solution:

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Robot bots[ MAX_BOTS ];
    int R;  // Number of bots
    int X;  // Map width
    int Y;  // Map height
    check( tryGetRobotsFromFile( argv[ 1 ], bots, &R, &X, &Y ), "Error: Could not read bots from %s.", argv[ 1 ] );
    int hx = X / 2;
    int hy = Y / 2;

    int qul = 0;
    int qur = 0;
    int qll = 0;
    int qlr = 0;
    for ( int r = 0 ; r < R ; r++ ) {
        Robot b = bots[ r ];
        int  x = b.x,   y = b.y;
        int vx = b.vx, vy = b.vy;

        #define N_STEPS 100
        x = ( x + N_STEPS * ( vx + X ) ) % X;
        y = ( y + N_STEPS * ( vy + Y ) ) % Y;
        if      ( x < hx && y < hy ) qul++;
        else if ( x > hx && y < hy ) qur++;
        else if ( x < hx && y > hy ) qll++;
        else if ( x > hx && y > hy ) qlr++;
    }

    int prod = qul * qur * qll * qlr;

    printf( "%d\n", prod );
    return 0;
}
```

For part 2, we were asked to find a picture of a Christmas tree after so many seconds.  I tried a bunch
of things, but what ended up working was to use the quadrant hint from part 1 and look for imbalances
in the quadrant counts.  I chose the compare the max count to the min count.  Here is the solution:

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Robot bots[ MAX_BOTS ];
    int R;  // Number of bots
    int X;  // Map width
    int Y;  // Map height
    check( tryGetRobotsFromFile( argv[ 1 ], bots, &R, &X, &Y ), "Error: Could not read bots from %s.", argv[ 1 ] );
    size_t G = X * Y * sizeof( int );
    int *grid = malloc( G );

    for ( int s = 1 ; ; s++ ) {
        memset( grid, 0, G );
        int hx = X / 2;
        int hy = Y / 2;

        int qul = 0;
        int qur = 0;
        int qll = 0;
        int qlr = 0;

        // Simulate one second for each robot and store count in the grid;
        for ( int r = 0 ; r < R ; r++ ) {
            Robot *b = &bots[ r ];
            b->x = ( b->x + b->vx + X ) % X;
            b->y = ( b->y + b->vy + Y ) % Y;
            int idx = b->y * X + b->x;
            grid[ idx ]++;
            if      ( b->x < hx && b->y < hy ) qul++;
            else if ( b->x > hx && b->y < hy ) qur++;
            else if ( b->x < hx && b->y > hy ) qll++;
            else if ( b->x > hx && b->y > hy ) qlr++;
        }

        int maxq = qul;
        maxq = max( maxq, qur );
        maxq = max( maxq, qll );
        maxq = max( maxq, qlr );

        int minq = qul;
        minq = min( minq, qur );
        minq = min( minq, qll );
        minq = min( minq, qlr );

        int diff = maxq - minq;
        if ( diff > 230 ) {  // Experimentally determined.
            for ( int x = 0 ; x < X+2 ; x++ ) putchar( '-' );
            putchar( '\n' );
            for ( int y = 0 ; y < Y ; y++ ) {
                putchar( '|' );
                for ( int x = 0 ; x < X ; x++ ) {
                    int idx = y * X + x;
                    if ( grid[ idx ] ) putchar( '.' );
                    else putchar( ' ' );
                }
                putchar( '|' );
                putchar( '\n' );
            }
            for ( int x = 0 ; x < X+2 ; x++ ) putchar( '-' );
            putchar( '\n' );
            printf( "%d Seconds (count = %d):\n\n", s, diff );
            if ( 'b' == getchar() ) break;
        }
    }

    return 0;
}
```
and the picture that I found is below:

![Day 14 Image](figures/day14.png)

## [Day 15](https://adventofcode.com/2024/day/15)

The challenge was to push boxes around a map with prescribed set of moves.  For part 1
it was pretty straight forward because on just had to look down a row or column to find a
a valid move.  The solution is as follows:

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Warehouse wh;
    check( tryGetDataFromFile( argv[ 1 ], &wh ), "Error: Could not read data from %s.", argv[ 1 ] );
    char **map = wh.map;

    for ( int m = 0 ; m < wh.M ; m++ ) {
        char move = wh.moves[ m ];
        if ( move <= ' ' ) continue;

        int dx = 0, dy = 0;
        if      ( move == '<' ) dx = -1;
        else if ( move == '>' ) dx = +1;
        else if ( move == '^' ) dy = -1;
        else if ( move == 'v' ) dy = +1;

        int nx = wh.x + dx;
        int ny = wh.y + dy;
        char nc = map[ ny ][ nx ];
        if ( nc == '#' ) {
            // There is a wall in front of us.  Don't move.
        } else
        if ( nc == '.' ) {
            // There is room, move to next position.
            wh.x = nx;
            wh.y = ny;
        } else
        if ( nc == 'O' ) {
            // There is a box in front of us, push the box if possible.
            int x = nx;
            int y = ny;
            char c;
            while ( ( c = map[ y ][ x ] ) != '#' ) {
                if ( c == '.' ) {
                    map[  y ][  x ] = 'O';
                    map[ ny ][ nx ] = '.';
                    wh.x = nx; 
                    wh.y = ny;
                    break;
                }
                x += dx;
                y += dy;
            }
        }
    }

    // Score the final positions of the boxes
    int sum = 0;
    for ( int y = 0 ; y < wh.Y ; y++ )
    for ( int x = 0 ; x < wh.X ; x++ )
    {
        if ( map[ y ][ x ] == 'O' ) sum += 100*y + x;
    }

    printf( "%d\n", sum );
    return 0;
}
```
Part 2 made each box double wide and as such made pushing vertically more challenging as the 
boxes could be staggered.  To handle this I wrote two helper functions to determine if a block can
be pushed and then to actually push the block.   The final solution is quite long.  Here it is:

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Warehouse wh;
    check( tryGetDataFromFile( argv[ 1 ], &wh ), "Error: Could not read data from %s.", argv[ 1 ] );
     char **map = wh.map;

    for ( int m = 0 ; m < wh.M ; m++ ) {
        char move = wh.moves[ m ];
        if ( move <= ' ' ) continue;

        int dx = 0, dy = 0;
        if      ( move == '<' ) dx = -1;
        else if ( move == '>' ) dx = +1;
        else if ( move == '^' ) dy = -1;
        else if ( move == 'v' ) dy = +1;

        int nx = wh.x + dx;
        int ny = wh.y + dy;
        char nc = map[ ny ][ nx ];
        if ( nc == '#' ) {
            // There is a wall in front of us.  Don't move.
        } else
        if ( nc == '.' ) {
            // There is room, move to next position.
            map[ wh.y ][ wh.x ] = '.';
            map[ ny ][ nx ]     = '@';
            wh.x = nx;
            wh.y = ny;
        } else
        if ( nc == '[' || nc == ']' ) {
            // There is a box in front of us, push the box if possible.
            if ( dx != 0 ) pushBoxHoriz( &wh, dx );
            else           pushBoxVert(  &wh, dy );
        }

        // printf( "After %d: %c\n%s\n", m, move, wh.mapStr );
    }

    // Score the final positions of the boxes
    int sum = 0;
    for ( int y = 0 ; y < wh.Y ; y++ )
    for ( int x = 0 ; x < wh.X ; x++ )
    {
        if ( map[ y ][ x ] == '[' ) sum += 100*y + x;
    }

    printf( "%d\n", sum );
    return 0;
}

void pushBoxHoriz( Warehouse *wh, int dx ) {
    int X = wh->X;
    int xInit = wh->x;
    char *row = wh->map[ wh->y ];
    int nx = xInit + dx;
    int x = nx;
    char c;
    while ( x >= 0 && x < X ) {
        c = row[ x ];
        if ( c == '#' ) break;
        if ( c == '.' ) {
            if ( dx == +1 ) {
                int n = x - xInit;
                memmove( row + xInit + dx, row + xInit, n );
            } else
            if ( dx == -1 ) {
                int n = xInit - x;
                memmove( row + x, row + x + 1, n );
            }
            row[ xInit ]      = '.';
            row[ nx ] = '@';
            wh->x = nx;
            break;
        }
        x += dx;
    }
}

bool canPushBoxVert( Warehouse *wh, int x, int y, int dy ) {
    char **map = wh->map;

    int xl = x, xr = x;
    char c = map[ y ][ x ];
    if ( c == '[') {
        xl = x;
        xr = x + 1;
    } else
    if ( c == ']') {
        xl = x - 1;
        xr = x;
    } 
    else assert( false );
    
    char cnl = map[ y + dy ][ xl ];
    char cnr = map[ y + dy ][ xr ];
    if ( cnl == '#' || cnr == '#' ) return false;
    if ( cnl == '.' && cnr == '.' ) return true;

    if ( cnl == '[') return canPushBoxVert( wh, xl, y+dy, dy );

    bool canMove = true;
    if ( cnl == ']' ) canMove = canMove && canPushBoxVert( wh, xl, y+dy, dy );
    if ( cnr == '[' ) canMove = canMove && canPushBoxVert( wh, xr, y+dy, dy );
    return canMove;
}

void moveBoxVert( Warehouse *wh, int x, int y, int dy ) {
    char **map = wh->map;

    int xl = x, xr = x;
    char c = map[ y ][ x ];
    if ( c == '[') {
        xl = x;
        xr = x + 1;
    } else
    if ( c == ']') {
        xl = x - 1;
        xr = x;
    } 
    else assert( false );
    
    int ny = y + dy;
    char cnl = map[ ny ][ xl ];
    char cnr = map[ ny ][ xr ];
    assert( cnl != '#' && cnr != '#' );

    if ( cnl == '[' ) {
        moveBoxVert( wh, xl, ny, dy );
    }
    else { 
        if ( cnl == ']' ) moveBoxVert( wh, xl, ny, dy );
        if ( cnr == '[' ) moveBoxVert( wh, xr, ny, dy );
    }

    map[ ny ][ xl ] = '[';
    map[ ny ][ xr ] = ']';
    map[ y ][ xl ] = '.';
    map[ y ][ xr ] = '.';
}

void pushBoxVert( Warehouse *wh, int dy ) {
    char **map = wh->map;
    int x = wh->x;
    int y = wh->y;
    int ny = y + dy;
    if ( canPushBoxVert( wh, x, ny, dy ) ) {
        moveBoxVert( wh, x, ny, dy );
        map[ y ][ x ]  = '.';
        map[ ny ][ x ] = '@';
        wh->y = ny;
    }
}
```

## [Day 16](https://adventofcode.com/2024/day/16)

Part 1 was a maze navigation puzzle.  Decided to use [Dijkstra's algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm).  To solve this in C, I needed to write my own [Priority Queue](https://en.wikipedia.org/wiki/Priority_queue) which I used the [this excellent article](https://www.geeksforgeeks.org/c-program-to-implement-priority-queue/).  The priority queue code is as follows:

```c
typedef struct _Node {
    uint32_t score;
    uint8_t x;
    uint8_t y;
    uint8_t dir;
    int prevCount;
    struct _Node *prev[ 8 ];
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
```

The implementation of Dijkstra's algorithm for part 1 was as follows:

```c
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
```

For part 2, I just had to remember the previous node and write a function to recursively walk
the path back following the optimal path at all times.  Here is the key part of the code:

```c
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
```

## [Day 17](https://adventofcode.com/2024/day/17)

Challenge was to build and run a virtual machine.  My implementation of the virtual machine is as follows:

```c
#define MAX_PROGRAM 20
#define MAX_OUTPUT 20
typedef struct {
    uint64_t PC;
    uint64_t A, B, C; // Registers
    uint64_t program[ MAX_PROGRAM ];
    int P; // Program length
    uint64_t output[ MAX_OUTPUT ];
    int U; // Output length
} Computer;

uint64_t combo( Computer *c, uint64_t arg ) {
    switch ( arg ) 
    {
    case 0: return arg;
    case 1: return arg;
    case 2: return arg;
    case 3: return arg;
    case 4: return c->A;
    case 5: return c->B;
    case 6: return c->C;
    case 7: assert( false );
    default: assert( false );
    }
    return 0;
}

void reset( Computer *c, uint64_t A, uint64_t B, uint64_t C ) {
    c->PC = 0;
    c->A = A;
    c->B = B;
    c->C = C;
    c->U = 0;
}

void step( Computer *c ) {

    uint64_t op  = c->program[ c->PC++ ];
    uint64_t arg = c->program[ c->PC++ ];

    switch ( op )
    {
    case 0: // adv
        arg = combo( c, arg );
        c->A /= ( 1ull << arg );
        break;
    case 1: // bxl
        c->B ^= arg;
        break;
    case 2: // bst
        arg = combo( c, arg );
        c->B = arg % 8;
        break;
    case 3: // jnz
        if ( c->A != 0ull ) c->PC = arg;
        break;
    case 4: // bxc
        c->B ^= c->C;
        break;
    case 5: // out
        assert( c->U < MAX_OUTPUT );
        arg = combo( c, arg );
        c->output[ c->U++ ] = arg % 8;
        break;
    case 6: // bdv
        arg = combo( c, arg );
        c->B = c->A / ( 1ull << arg );
        break;
    case 7: // cdv
        arg = combo( c, arg );
        c->C = c->A / ( 1ull << arg );
        break;
    default:
        assert( false );
        break;
    }
}

void run( Computer *c ) {
    while ( c->PC < c->P ) {
        step( c );
    }
}

void print( Computer *c ) {
    printf( "PC: %llu, A: %9llu, B: %9llu, C: %9llu OUT: ", 
            c->PC,  c->A,   c->B,   c->C 
    );
    for ( int i = 0 ; i < c->U ; i++ ) printf( "%llu,", c->output[ i ] );
    printf( "\n" );
}
```

For part 1, the task was just to run the program and report the output.  Easy enough.  Part 2 was
to find the value of register A to preset it to in order to have the output equal the program.
Given that the final answer was `247839653009594` i.e. 247 trillion, I had to find a more efficient
way to find this value by means other than brute force.  To do this I followed on-line advice to decompile the 
program.  So the original program was as follows:

```
Register A: 30899381
Register B: 0
Register C: 0

Program: 2,4,1,1,7,5,4,0,0,3,1,6,5,5,3,1
```
Which i translated into the following machine instructions as `c` equivalents as follows:

```
2,4  BST: B = A % 8
1,1  BXL: B = B ^ 1
7,5  CDV: C = A / ( 1 << B )
4,0  BXC: B = B ^ C
0,3  ADV: A = A >> 3
1,6  BXL: B = B ^ 6
5,5  OUT: B --> out
3,0  JNZ: if ( A != 0 ) goto 0
```

after studying the program it is clear that the initial value in `A` is consumed three bits (or one octal digit)
at a time and then used to generate the output.  In the fourth instruction, higher bits of `A` are 
mixed into the result so that higher order bits affect lower order bits, but not the other way around.
This meants two things, (1) the digits could/should be found from the high digit and proceed down to lower digits
 and (2) that it may be necessary to back track if a specific digit could not be found.  I hardcoded 
 a back track of 2 and that seemed sufficient.  The solution to part 2 is as followsL

 ```c
 int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Computer c = {};
    check( tryGetDataFromFile( argv[ 1 ], &c ), "Error: Could not read data from %s.", argv[ 1 ] );
    uint64_t iB = c.B;
    uint64_t iC = c.C;

    uint64_t A = 7ull << 3*(c.P-1 );
    uint64_t digitMask = 7ull;
    for ( int d = c.P-1 ; d >= 0 ; d-- ) {
        uint64_t base = ( A >> 3*d ) & digitMask;
        for ( int v = 0 ; v < 8 ; v++ ) {
            uint64_t place = ( base + v ) % 8;
            A = A & ~(digitMask << 3*d) | place << 3*d;
            reset( &c, A, iB, iC );
            run( &c );
            if ( c.output[ d ] == c.program[ d ] ) {
                printf( "A0: %llu, ", A );
                print( &c );
                goto DIGIT_FOUND;
            }
        }
        d += 2; // backtrack because a match was not found.
        uint64_t place  = ( A >> 3*d ) & digitMask;
        place++;
        A = A & ~(digitMask << 3*d) | place << 3*d;
        DIGIT_FOUND:
        ;
    }
    printf( "%llu\n", A );
    return 0;
}
```

Because everything is in octal bit manipulations were necessary to extract the digits.  Note that this 
solution is not general and will not work in general.  It is specifically tuned to solve my problem.  It wont
even solve the test example.

## [Day 18](https://adventofcode.com/2024/day/18)

This was another path finding problem.  For part 1 I used the code for the priority queue from Day 16
and I added the additional feature of an [A* Search Algorithm](https://en.wikipedia.org/wiki/A*_search_algorithm).  The code is basically the same but the gist of the solution is shown below:


```c
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
```

For part 2, I had to determine the particular obstacle which blocked the path to the end.  For this
I factored out the A* algorithm and then implemented a binary search.  The algorithm is as follows:

```c
int main( int argc, char **argv ) {
    check( argc >= 2, "Usage: %s filename", argv[0] );

    Bytes b = {};
    check( tryGetDataFromFile( argv[ 1 ], &b ), "Error: Could not read data from %s.", argv[ 1 ] );
    int X = b.X;
    int Y = b.Y;

    Queue open;
    qInit( &open, X*Y );

    Node *flatNodes = malloc( sizeof( Node ) * X * Y );
    Node **nGrid = malloc( sizeof( Node* ) * Y );
    for ( int i = 0 ; i < Y ; i++ ) nGrid[ i ] = flatNodes + X*i;

    // Parameters for the binary search.
    int lb = b.Nu;
    int ub = b.N;

    int lowestNotFound = 0;
    while ( lb < ub - 1 ) {
        b.Nu = ( lb + ub ) / 2;
        if ( tryAStar( &b, nGrid, &open ) ) lb = b.Nu;
        else {
            ub = b.Nu;
            lowestNotFound = ub;
        }
    }

    int x = b.x[ lowestNotFound - 1 ];
    int y = b.y[ lowestNotFound - 1 ];
    printf( "%d,%d\n", x, y );
    return 0;
}
```