#include "io.h"

#include "lib/time.h"

#include "ctype.h"
#include "stdarg.h"
#include "string.h"

/* defined in io.S */
extern void uart_write ( char * s );
extern int uart_read ( );

/* function that are not public ourside */
void buffer_enqueue ( const char * str );

int printf ( const char * format, ... )
{
    const char * current = format;
    char temp_buffer[50];
    int output_char_counts = 0;

    va_list arguments;
    format_t output_format;

    va_type_value value;

    va_start ( arguments, format );

    do
    {
        if ( *current == '%' )
        {
            output_format = parse_format ( &current );

            /* retrive argument */
            switch ( output_format.type )
            {
                case LONG_INT:
                case INT:
                    value.d = va_arg ( arguments, int );
                    itoa ( value.d, temp_buffer, 10 );
                    break;
                case BINARY:
                    value.d = va_arg ( arguments, int );
                    itoa ( value.d, temp_buffer, 2 );
                    break;
                case OCTAL:
                    value.d = va_arg ( arguments, int );
                    itoa ( value.d, temp_buffer, 8 );
                    break;
                case HEXADECIMAL_LOWER:
                    value.d        = va_arg ( arguments, int );
                    temp_buffer[0] = '0';
                    temp_buffer[1] = 'x';
                    itoa ( value.d, temp_buffer + 2, 16 );
                    str_to_lower ( temp_buffer );
                    break;
                case HEXADECIMAL_UPPER:
                case POINTER:
                    value.d        = va_arg ( arguments, int );
                    temp_buffer[0] = '0';
                    temp_buffer[1] = 'X';
                    itoa ( value.d, temp_buffer + 2, 16 );
                    break;
                case DOUBLE:
                    value.f = va_arg ( arguments, double );
                    ftoa ( value.f, temp_buffer, output_format.after_point_length );
                    break;
                case CHAR:
                    value.c        = va_arg ( arguments, int );
                    temp_buffer[0] = value.c;
                    temp_buffer[1] = '\0';
                    break;
                case STRING:
                    value.s = va_arg ( arguments, char * );
                    break;
                default:
                    break;
            }

            /* output signed if needed */
            if ( output_format.signed_flag && output_format.type == INT )
            {
                value.d = va_arg ( arguments, int );

                output_char_counts++;
                buffer_enqueue ( value.d > 0 ? "+" : "-" );
            }

            /* padding zero or space */
            if ( output_format.int_length != 0 && output_format.int_length > strlen ( temp_buffer ) && !output_format.is_justify_left )
            {
                while ( output_format.int_length - strlen ( temp_buffer ) > 0 )
                {
                    if ( output_format.is_zero_padding )
                        buffer_enqueue ( "0" );
                    else
                        buffer_enqueue ( " " );

                    output_char_counts++;

                    output_format.int_length--;
                }
            }

            if ( output_format.type == INT && output_format.signed_flag && value.d < 0 )
            {
                output_char_counts += strlen ( temp_buffer + 1 );
                buffer_enqueue ( temp_buffer + 1 );
            }
            else if ( output_format.type == STRING )
            {
                output_char_counts += strlen ( value.s );
                buffer_enqueue ( value.s );
                ;
            }
            else
            {
                output_char_counts += strlen ( temp_buffer );
                buffer_enqueue ( temp_buffer );
            }

            /* padding space at the end*/
            if ( output_format.int_length != 0 && output_format.int_length > strlen ( temp_buffer ) && output_format.is_justify_left )
            {
                while ( output_format.int_length - strlen ( temp_buffer ) > 0 )
                {
                    output_char_counts++;
                    buffer_enqueue ( " " );

                    output_format.int_length--;
                }
            }
        }
        else
        {
            temp_buffer[0] = *current;
            temp_buffer[1] = '\0';

            output_char_counts++;
            buffer_enqueue ( temp_buffer );

            current++;
        }

    } while ( *current != '\0' );

    return output_char_counts;
}

char * gets ( char * str )
{
    int i = 0;
    int c;

    while ( 1 )
    {
        while ( ( c = uart_read ( ) ) != -1 )
            ;
        str[i++] = c;

        if ( c == '\n' )
        {
            str[i] = '\0';
            break;
        }
    }

    return str;
}

void buffer_enqueue ( const char * str )
{
    static char buffer[4096];
    static int buffer_count = 0;

    int current_str_output = 0;

    while ( current_str_output < strlen ( str ) )
    {
        if ( buffer_count + strlen ( str + current_str_output ) > 4000 )
        {
            strncpy ( buffer + buffer_count, str + current_str_output, 4000 - buffer_count );
            current_str_output += ( 4000 - buffer_count );

            buffer[4000] = '\0';

            uart_write ( buffer );
            buffer_count = 0;
        }
        else
        {
            strcpy ( buffer + buffer_count, str + current_str_output );

            buffer_count += strlen ( str + current_str_output );
            current_str_output += strlen ( str + current_str_output );
        }
    }

    /* if there still something in the queue, and the buffer contain \n */
    /* flush the buffer */
    if ( buffer_count > 0 && strchr ( buffer, '\n' ) != NULL )
    {
        uart_write ( buffer );
        buffer_count = 0;
    }
}

format_t parse_format ( const char ** input )
{
    format_t format = {
        .is_zero_padding    = 0,
        .signed_flag        = 0,
        .int_length         = 0,
        .after_point_length = 6,
        .is_justify_left    = 0,
    };

    // move on, skip '%'
    ( *input )++;

    while ( 1 )
    {
        if ( ( **input ) == '+' )
        {
            format.signed_flag = 1;
            ( *input )++;
        }
        else if ( ( **input ) == '-' )
        {
            format.is_justify_left = 1;
            ( *input )++;
        }
        else if ( isdigit ( **input ) )
        {
            // lenght control start with 0
            if ( ( **input ) == '0' )
            {
                ( *input )++;
                format.is_zero_padding = 1;
            }

            format.int_length = atoi ( *input );

            while ( isdigit ( **input ) )
                ( *input )++;
        }
        else if ( ( **input ) == '.' )
        {
            ( *input )++;
            format.after_point_length = atoi ( *input );

            while ( isdigit ( **input ) )
                ( *input )++;
        }
        else if ( isalpha ( **input ) )
        {
            switch ( **input )
            {
                case 'd':
                case 'i':
                    format.type = INT;
                    break;
                case 'f':
                    format.type = DOUBLE;
                    break;
                case 'x':
                    format.type = HEXADECIMAL_LOWER;
                    break;
                case 'X':
                    format.type = HEXADECIMAL_UPPER;
                    break;
                case 'o':
                    format.type = OCTAL;
                    break;
                case 'b':
                    format.type = BINARY;
                    break;
                case 'c':
                    format.type = CHAR;
                    break;
                case 's':
                    format.type = STRING;
                    break;
                case 'p':
                    format.type = POINTER;
                    break;
                default:
                    break;
            }
            ( *input )++;
        }
        else
        {
            break;
        }
    }

    return format;
}