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