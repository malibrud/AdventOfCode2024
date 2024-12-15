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