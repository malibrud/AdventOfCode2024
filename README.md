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