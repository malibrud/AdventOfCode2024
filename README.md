# Advent of Code Solutions for 2024
**Author**: Mark Brudnak

## Day 01

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

## Day 02

First part was pretty straight forward.  Second part after trying a method to find faults in the part 1 algorithm, I found that 
finding a fault in the first or second position created a lot of complexity and then decided to just brute force each fault case.
Since finding a 'safe' sequence with one element skipped is sufficient, I did not check the whole sequence.