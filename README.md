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

