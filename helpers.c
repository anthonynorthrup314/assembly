#include <stdio.h>
#include <string.h>

#include "helpers.h"

//// Definitions

BOOL an_parse_int_base(const char* in, int* out, int base)
{
    BOOL prefix = 0; // If it starts with [+-]?0, then true
    int sign = 0; // If it starts with [+-]?, then 1 or -1 resp.
    int value = 0; // The positive value

    int len = strlen(in);
    for (int i = 0; i < len; i++)
    {
        char c = in[i];

        // Parse character value as integer
        int digit = -1;
        if (('0' <= c) && ('9' >= c))
            digit = c - '0';
        else if (('a' <= c) && ('z' >= c))
            digit = c - 'a' + 10;
        else if (('A' <= c) && ('Z' >= c))
            digit = c - 'A' + 10;
        BOOL valid_digit = (0 <= digit) && (base > digit);

        if (0 == i)
        {
            // Check for sign
            if ('-' == c)
                sign = -1;
            else if ('+' == c)
                sign = 1;
            else if (valid_digit)
            {
                prefix = (0 == digit);
                value = digit;
            }
            else
                return 0;
        }
        else if ((1 == i) && (0 != sign))
        {
            // Check for prefix
            if (valid_digit)
            {
                prefix = (0 == digit);
                value = digit;
            }
            else
                return 0;
        }
        else if (((1 == i) && prefix) || ((2 == i) && (0 != sign) && prefix))
        {
            // Check for base modifier
            switch(c)
            {
                case 'b':
                case 'B':
                    base = 2;
                    break;
                case 'h':
                case 'H':
                    base = 8;
                    break;
                case 'x':
                case 'X':
                    base = 16;
                    break;
                default:
                    if (valid_digit)
                        value = value * base + digit;
                    else
                        return 0;
            }
        }
        else
        {
            if (valid_digit)
                value = value * base + digit;
            else
                return 0;
        }
    }

    *out = (-1 == sign) ? -value : value;
    return 1;
}

BOOL an_parse_int(const char* in, int* out)
{
    return an_parse_int_base(in, out, 10);
}
