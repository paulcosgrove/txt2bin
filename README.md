txt2bin
=======

Txt2bin is a program for creating binary files using ASCII. 

Syntax
======

To output a byte, simply write its value in hex. For example:

    0x40

The '0x' prefix is optional, so we could also write:

    40

To output several bytes, separate the values with whitespace:

    40 41 42 43
    44 45

Comments begin with a '#' and stop at the end of a line:

    # A comment line on its own
    01 02   # An inline comment

Txt2bin can also output 16 and 32-bit values. These will be output in 
the current byte order mode (little-endian by default).

    1234
    12345678

The byte order mode can be changed using the words BIG_ENDIAN and 
LITTLE_ENDIAN:

    BIG_ENDIAN      # Set the byte order mode to big-endian
    
    DEAD            # Output a 16-bit big-endian value
    DEADBEEF        # Output a 32-bit big-endian value

    LITTLE_ENDIAN   # Set the current byte order mode to little-endian
    
    DEAD            # Output a 16-bit little-endian value
    DEADBEEF        # Output a 32-bit little-endian value

To set the byte order for a value, irrespective of the byte order 
mode, prefix the value with either a '^' (big-endian) or a '_' (little-endian).

    LITTLE_ENDIAN   # Set current byte order mode to little-endian

    DEADBEEF        # Output a 32-bit little-endian value
    ^DEADBEEF       # Output a 32-bit big-endian value
    _DEAFBEEF       # Output a 32-bit little-endian value
    
    BIG_ENDIAN      # Set the current byte order mode to big-endian

    DEADBEEF        # Output a 32-bit big-endian value
    ^DEADBEEF       # Output a 32-bit big-endian value
    _DEAFBEEF       # Output a 32-bit little-endian value

    LITTLE_ENDIAN   # Switch default byte order back to little-endian

Finally, text strings can also be output by enclosing the text in quotes:

    "BM"            # Output 42h and 4Dh

