#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

#ifndef X_MATH_HPP
#include "x_math.hpp"
#endif

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#undef x_vsprintf

//////////////////////////////////////////////////////////////////////////////
// COMMENT:
//---------------------------------------------------------------------------
// TempBufferPtr is now being passed around because it needs to be thread safe
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// FLAGS FOR THE VSPRINTF

// NUMERIC VALUES
#define LONGDBL			0x01        // long double; unimplemented
#define LONGINT			0x02        // long integer
#define QUADINT			0x04        // quad integer
#define SHORTINT		0x08        // short integer
#define NUMBERMASK		0x0F

// OTHER FLAGS
#define ALT				0x10        // alternate form
#define HEXPREFIX		0x20        // add 0x or 0X prefix
#define LADJUST			0x40        // left adjustment
#define ZEROPAD			0x80        // zero (as opposed to blank) pad

// CONSTANTS
#define SPF_LONG_MAX    2147483647
#define WORKSIZE		128         // space for %c, %[diouxX], %[eEfgG]

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//==========================================================
// vsprintf_pow
//==========================================================
static
f64 vsprintf_pow( f64 x, s32 p )
{
    f64 r;

    if (p == 0) return 1.0;
    if (x == 0.0) return 0.0;

    if (p < 0)
    {
        p = -p;
        x = 1.0 / x;
    }

    r = 1.0;
    for(;;)
    {
        if (p & 1) r *= x;
        if ((p >>= 1) == 0) return r;
        x *= x;
    }
}

//================================================================
// IsINF
//----------------------------------------------------------------
// Check to see if it is infinite
//================================================================
static
s32 IsINF( f64 x )
{
    return x != x;
}

//================================================================
// IsNAN
//----------------------------------------------------------------
// Check to see if it is NOT a number
//================================================================
static
s32 IsNAN( f64 x )
{
    return x != x;
}

//=========================================================
// dtoa
//---------------------------------------------------------
// fmtbase - String where the output is going to go
//          (Make sure that the string is atleast 24 bytes long)
// fpnum   - number which is going to be converted
// cvt     - what type of conversion needs
// width   - total width of the output number
// prec    - how many digits of precision
//=========================================================
static
s32 dtoa( char* fmtbase, f64 fpnum, char cvt, s32 width, s32 prec )
{
    char    fwork[WORKSIZE];
    char*   fw;

    // for integer part
    char    iwork[WORKSIZE];
    char*   iworkend;
    char*   iw;

    // for exponent part
    char    ework[16];
    char*   eworkend;
    char*   ew;

    // other variables
    s32     is_neg;
    f64     powprec;
    f64     rounder;
    s32     f_fmt;
    s32     iwidth;
    s32     fwidth;
    s32     ewidth;

    // arrange everything in returned string variables
    char*   fmt;
    s32     i;
    s32     showdot;
    s32     fmtwidth;
    s32     pad;

    ASSERT(fmtbase);
    //ASSERT(width > 0);
    ASSERT(prec >= 0);

    // initialize some variables
    fw        = fwork;

    // set up the integer part
    iworkend  = &iwork[sizeof(iwork) - 1];
    iw        = iworkend;
    *iw       = 0;

    // setup the exponent part
    eworkend  = &ework[sizeof(ework) - 1];
    ew        = eworkend;
    *ew       = 0;

    if (IsINF(fpnum))
    {
        char* inffmt  = fmtbase;
        char* inffmtp = inffmt;

        if (fpnum < 0) *inffmtp++ = '-';
        x_strcpy(inffmtp, "Inf");
        return (s32)(inffmt - fmtbase);
    }

    if (IsNAN(fpnum))
    {
        char* nanfmt = fmtbase;
        x_strcpy(nanfmt, "NaN");
        return (s32)(nanfmt - fmtbase);
    }

    // grab sign & make non-negative
    is_neg = fpnum < 0;
    if (is_neg) fpnum = -fpnum;

    // precision matters

    // can't have more prec than supported
    if (prec > WORKSIZE - 2) prec = WORKSIZE - 2;

    if (prec == 6) powprec = 1.0e6;
    else
        powprec = vsprintf_pow( 10.0, prec );

    rounder = 0.5 / powprec;

    f_fmt = cvt == 'f' ||
          ((cvt == 'g') && (fpnum == 0.0 || (fpnum >= 1e-4 && fpnum < powprec)));

    iwidth = 0;
    fwidth = 0;
    ewidth = 0;

    if (f_fmt)  // fixed format
    {
        f64 ipart;
        f64 fpart = x_modf(fpnum, &ipart);

        // convert fractional part
        if (fpart >= rounder || cvt != 'g')
        {
            f64  ifpart;
            s32     i;
            f64  ffpart;

            fpart += rounder;
            if (fpart >= 1.0)
            {
                ipart += 1.0;
                fpart -= 1.0;
            }

            ffpart = fpart;

            for (i = 0; i < prec; ++i)
            {
                ffpart = x_modf(ffpart * 10.0, &ifpart);
                *fw++ = (char)('0' + (s32)(ifpart));
                ++fwidth;
            }

            if (cvt == 'g')  // inhibit trailing zeroes if g-fmt
            {
                char* p;
                for ( p = fw - 1; p >= fwork && *p == '0'; --p)
                {
                    *p = 0;
                    --fwidth;
                }
            }
        }

        // convert integer part
        if (ipart == 0.0)
        {
            if (cvt != 'g' || fwidth < prec || fwidth < width)
            {
                *--iw = '0'; ++iwidth;
            }
        }
        else
            if (ipart <= (f64)(SPF_LONG_MAX)) // a useful speedup
            {
                s32 li = (s32)(ipart);
                while (li != 0)
                {
                    *--iw = (char)('0' + (li % 10));
                    li = li / 10;
                    ++iwidth;
                }
            }
            else // the slow way
            {
                while (ipart > 0.5)
                {
                    f64 ff = x_modf(ipart / 10.0, &ipart);
                    ff = (ff + 0.05) * 10.0;
                    *--iw = (char)('0' + (s32)(ff));
                    ++iwidth;
                }
            }

        // g-fmt: kill part of frac if prec/width exceeded
        if (cvt == 'g')
        {
            s32 m = prec;
            s32 adj;

            if (m < width) m = width;

            adj = iwidth + fwidth - m;
            if (adj > fwidth) adj = fwidth;

            if (adj > 0)
            {
                char* f;
                for (f = &fwork[fwidth-1]; f >= fwork && adj > 0; --adj, --f)
                {
                    char ch = *f;

                    --fwidth;
                    *f = 0;

                    if (ch > '5') // properly round: unavoidable propagation
                    {
                        char* p;
                        s32 carry = 1;

                        for ( p = f - 1; p >= fwork && carry; --p)
                        {
                            ++*p;

                            if (*p > '9') *p = '0';
                            else carry = 0;
                        }

                        if (carry)
                        {
                            for (p = iworkend - 1; p >= iw && carry; --p)
                            {
                                ++*p;
                                if (*p > '9') *p = '0';
                                else carry = 0;
                            }

                            if (carry)
                            {
                                *--iw = '1';
                                ++iwidth;
                                --adj;
                            }
                        }
                    }
                }
            }
        }
    }
    else  // e-fmt
    {
        f64     almost_one;
        f64     ipart;
        f64     fpart;
        f64     ffpart;
        f64     ifpart;
        s32     i;
        char    eneg;
        s32     exp;

        // normalize
        exp = 0;

        while (fpnum >= 10.0)
        {
            fpnum *= 0.1;
            ++exp;
        }

        almost_one = 1.0 - rounder;

        while (fpnum > 0.0 && fpnum < almost_one)
        {
            fpnum *= 10.0;
            --exp;
        }

        fpart = x_modf(fpnum, &ipart);

        if (cvt == 'g')     // used up one digit for int part...
        {
            --prec;
            powprec /= 10.0;
            rounder = 0.5 / powprec;
        }

        // convert fractional part -- almost same as above
        if (fpart >= rounder || cvt != 'g')
        {
            fpart += rounder;

            if (fpart >= 1.0)
            {
                fpart -= 1.0;
                ipart += 1.0;

                if (ipart >= 10.0)
                {
                    ++exp;
                    ipart /= 10.0;
                    fpart /= 10.0;
                }
            }

            ffpart = fpart;

            for (i = 0; i < prec; ++i)
            {
                ffpart = x_modf(ffpart * 10.0, &ifpart);
                *fw++ = (char)('0' + (s32)(ifpart));
                ++fwidth;
            }

            if (cvt == 'g')  // inhibit trailing zeroes if g-fmt
            {
                char* p;

                for ( p = fw - 1; p >= fwork && *p == '0'; --p)
                {
                    *p = 0;
                    --fwidth;
                }
            }
        }


        // convert exponent

        eneg = exp < 0;
        if (eneg) exp = - exp;

        while (exp > 0)
        {
            *--ew = (char)('0' + (exp % 10));
            exp /= 10;
            ++ewidth;
        }

        while (ewidth < 2)  // ensure at least 2 zeroes
        {
            *--ew = '0';
            ++ewidth;
        }

        *--ew = eneg ? '-' : '+';
        *--ew = 'e';

        ewidth += 2;

        // convert the one-digit integer part
        *--iw = (char)('0' + (s32)(ipart));
        ++iwidth;

    }

    // arrange everything in returned string
    showdot = cvt != 'g' || fwidth > 0;
    fmtwidth = is_neg + iwidth + showdot + fwidth + ewidth;
    pad = width - fmtwidth;

    if (pad < 0) pad = 0;

    //fmtbase = (char *)malloc(fmtwidth + pad + 1); // NOW IS PAST AS A PARAMETER
    fmt = fmtbase;

    for (i = 0; i < pad; ++i) *fmt++ = ' ';

    if (is_neg) *fmt++ = '-';

    for (i = 0; i < iwidth; ++i) *fmt++ = *iw++;

    if (showdot)
    {
        *fmt++ = '.';
        fw = fwork;
        for (i = 0; i < fwidth; ++i) *fmt++ = *fw++;
    }

    for (i = 0; i < ewidth; ++i) *fmt++ = *ew++;

    *fmt = 0;


    return (s32)(fmt - fmtbase);
}

//================================================================
// ULtoA
//----------------------------------------------------------------
// Convert an unsigned long to ASCII for printf purposes, returning
// a pointer to the first character of the string representation.
// Octal numbers can be forced to have a leading zero; hex numbers
// use the given digits.
//----------------------------------------------------------------
// val      - Numeric value to be converted
// endp     - String to be fill in fromt the end
// base     - base of the number (10, 8, 16)
// octzero  - flag to add to the oct numbers the 0 in front
// xdigs    - Hexadecimal string array of number 0,1,2,3,4,5,9,A,or a, ..etc
//================================================================
#define to_digit(c) ((c) - '0')
#define is_digit(c) ((u32)to_digit(c) <= 9)
#define to_char(n)  ((char)((n) + '0'))

static
char* ULtoA( u32 val, char* endp, s32 base, xbool octzero, char* xdigs )
{
    char *cp = endp;
    s32 sval;

    ASSERT(endp);
//    ASSERT(xdigs);

    // Handle the three cases separately, in the hope of getting
    // better/faster code.

    switch (base)
    {
    case 10:
            if (val < 10)
            {   // many numbers are 1 digit
                *--cp = to_char(val);
                return (cp);
            }


            // On many machines, unsigned arithmetic is harder than
            // signed arithmetic, so we do at most one unsigned mod and
            // divide; this is sufficient to reduce the range of
            // the incoming value to where signed arithmetic works.

            if (val > SPF_LONG_MAX)
            {
                *--cp = to_char(val % 10);
                sval = (s32)(val / 10);
            }
            else
            {
                sval = (s32)val;
            }

            do
            {
                *--cp = to_char(sval % 10);
                sval /= 10;
            } while (sval != 0);

            break;

    case 8:
            do
            {
                *--cp = to_char(val & 7);
                val >>= 3;
            } while (val);

            if (octzero && *cp != '0') *--cp = '0';

            break;

    case 16:
            do
            {
                *--cp = xdigs[val & 15];
                val >>= 4;
            } while (val);

            break;

    default:            /* oops */
        return NULL;
    }

    return (cp);
}

//================================================================
// WriteToBuffer
//================================================================
static
void WriteToBuffer(char** TempBufferPtr, char* String, s32 Size )
{
    s32 i;

    ASSERT(String);
    ASSERT(Size>=0);

    for (i=0; i < Size; i++)
    {
        **TempBufferPtr = String[i];
        (*TempBufferPtr)++;
    }
}

//================================================================
// PadBuffer
//----------------------------------------------------------------
// Choose PADSIZE to trade efficiency vs. size.  If larger printf
// fields occur frequently, increase PADSIZE and make the initialisers
// below longer.
//================================================================
#define PADSIZE  16     // pad chunk size

static
void PadBuffer( char** TempBufferPtr, s32 HowMany, char With )
{
    s32 i;
    char* Type;
    static char Blanks[PADSIZE] =
     {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    static char Zeroes[PADSIZE] =
     {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

    // Find what type we need to path with
    Type = (With == ' ') ? Blanks : Zeroes;

    if ( HowMany > 0 )
    {
        for (i = HowMany; i > PADSIZE; i -= PADSIZE)
        {
            WriteToBuffer( TempBufferPtr, Type, PADSIZE );
        }

        WriteToBuffer( TempBufferPtr, Type, i );
    }
}

//================================================================
// GetSignARG
//----------------------------------------------------------------
// To extend shorts properly, we need both signed and unsigned
// argument extraction methods.
//================================================================
static
s32 GetSignARG( x_va_list* ap, s32 flags )
{
    ASSERT(ap);
    ASSERT(*ap);

    switch (flags & NUMBERMASK)
    {
    case LONGDBL:   ASSERT(0); //return x_va_arg(ap, long);
    case LONGINT:   return x_va_arg(*ap, s32); // This may need to change for the N64
    case QUADINT:   return x_va_arg(*ap, s32); // This may need to change for the N64
    case SHORTINT:  return x_va_arg(*ap, s16);
    }

    return x_va_arg(*ap, s32);
}

//================================================================
// GetUnSignARG
//----------------------------------------------------------------
// To extend shorts properly, we need both signed and unsigned
// argument extraction methods.
//================================================================
static
u32 GetUnSignARG( x_va_list* ap, s32 flags )
{
    ASSERT(ap);
    ASSERT(*ap);

    switch (flags & NUMBERMASK)
    {
    case LONGDBL:   ASSERT(0); //return x_va_arg(ap, long);
    case LONGINT:   return x_va_arg(*ap, u32);     // This may need to change for the N64
    case QUADINT:   return x_va_arg(*ap, u32);     // This may need to change for the N64
    case SHORTINT:  return x_va_arg(*ap, u16);
    }

    return x_va_arg(*ap, u32);
}

//================================================================
// VFSPrintf
//----------------------------------------------------------------
// format               - formated string
// ap                   - argument pointer
//================================================================
#define DEFPREC  6      // number of precision for the real numbers

s32 x_vsprintf( char* TempBufferPtr, const char *format, x_va_list ap )
{
    char*           fmt;            // format string */
    char            ch;             // character from fmt */
    s32             n;              // handy integer (short term usage)
    char*           cp;             // handy char pointer (short term usage)
    s32             flags;          // flags as above
    s32             ret;            // return value accumulator
    s32             width;          // width from format (%8d), or 0
    s32             prec;           // precision from format (%.3d), or -1
    char            sign;           // sign prefix (' ', '+', '-', or \0)
    f64             _double;        // double precision arguments %[eEfgG]
    u32             ulval = 0;      // integer arguments %[diouxX]
    u32             uqval = 0;      // %q integers ************************ This may need to change for N64
    s32             base;           // base for [diouxX] conversion
    s32             dprec;          // a copy of prec if [diouxX], 0 otherwise
    s32             realsz;         // field size expanded by dprec, sign, etc
    s32             size;           // size of converted field or string
    char*           xdigs = NULL;   // digits for [xX] conversion
    char            buf[WORKSIZE];  // space for %c, %[diouxX], %[eEfgG]
    char            ox[2];          // space for 0x hex-prefix

    ASSERT(format);
    ASSERT(ap);

    // Initialize variables
    fmt = (char *)format;
    ret = 0;

    // Scan the format for conversions (`%' character).
    for (;;)
    {
        // Find the first "interesting symbol"
        for (cp = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
        {
            // Empty
        }

        // Write all the caracters before the "interesting symbol"
        if ((n = fmt - cp) != 0)
        {
            WriteToBuffer(&TempBufferPtr, cp, n);
            ret += n;
        }

        // are we done?
        if (ch == '\0') goto done;

        // skip over '%'
        fmt++;

        // Get ready for formating the info
        flags = 0;
        dprec = 0;
        width = 0;
        prec = -1;
        sign = '\0';

rflag:
        ch = *fmt++;

reswitch:
        switch (ch)
        {
            case ' ':

                // ``If the space and + flags both appear, the space
                // flag will be ignored.''
                //  -- ANSI X3J11
                if (!sign) sign = ' ';
                goto rflag;

            case '#':
                flags |= ALT;
                goto rflag;

            case '*':
                 // ``A negative field width argument is taken as a
                 // - flag followed by a positive field width.''
                 // -- ANSI X3J11
                 // They don't exclude field widths read from args.

                if ( (width = x_va_arg(ap, s32)) >= 0 ) goto rflag;

                width = -width;

                /////////>>>>>>>>>>>>>>>>>> FALLTHROUGH <<<<<<<<<<<<<<//////////
            case '-':
                flags |= LADJUST;
                goto rflag;

            case '+':
                sign = '+';
                goto rflag;

            case '.':
                if ((ch = *fmt++) == '*')
                {
                    n = x_va_arg(ap, s32);
                    prec = n < 0 ? -1 : n;
                    goto rflag;
                }

                n = 0;

                while ( is_digit(ch) )
                {
                    n = 10 * n + to_digit(ch);
                    ch = *fmt++;
                }

                prec = n < 0 ? -1 : n;
                goto reswitch;


            case '0':
                 // ``Note that 0 is taken as a flag, not as the
                 // beginning of a field width.''
                 // -- ANSI X3J11
                flags |= ZEROPAD;
                goto rflag;

            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                n = 0;
                do
                {
                    n = 10 * n + to_digit(ch);
                    ch = *fmt++;
                } while (is_digit(ch));

                width = n;
                goto reswitch;

            case 'L':
                flags |= LONGDBL;
                goto rflag;

            case 'h':
                flags |= SHORTINT;
                goto rflag;

            case 'l':
                flags |= LONGINT;
                goto rflag;

            case 'q':
                flags |= QUADINT;
                goto rflag;

            case 'c':
                *(cp = buf) = (char)x_va_arg(ap, s32);
                size = 1;
                sign = '\0';
                break;

            case 'D':
                flags |= LONGINT;
                /////////>>>>>>>>>>>>>>>>>> FALLTHROUGH <<<<<<<<<<<<<<//////////

            case 'd':
            case 'i':

                if (flags & QUADINT)
                {
                    s32 Temp = x_va_arg( ap, s32 );

                    if( Temp < 0 )
                    {
                        Temp = -Temp;
                        sign = '-';
                    }
                    
                    uqval = (u32)Temp;
                }
                else
                {
                    s32 Temp = GetSignARG( (x_va_list*)&ap, flags );
                    if( Temp < 0 )
                    {
                        Temp = -Temp;
                        sign = '-';
                    }
                    
                    ulval = (u32)(Temp);
                }

                base = 10;
                goto number;

            case 'g':
            case 'G':
                if (prec == 0) prec = 1;

            case 'e':
            case 'E':
            case 'f':
                /////////>>>>>>>>>>>>>>>>>> FALLTHROUGH <<<<<<<<<<<<<<//////////

                if (sign == '+') 
                {
                    WriteToBuffer(&TempBufferPtr, &sign, 1);
                    width--;
                    ret++;
                }

                if (prec == -1) prec = DEFPREC;

                if (flags & LONGDBL) _double = (f64)x_va_arg(ap, f64); // long double f64 N64?
                else
                    _double = x_va_arg(ap, f64);

                if (flags & LADJUST) width = -width;

                // right-adjusting zero padding
                size = dtoa(buf, _double, ch, width, prec);

                // check whether we have to pad or not
                if (flags & ZEROPAD)
                {
                    s32 i;
                    for (i=0; buf[i] == ' '; i++)
                    {
                        buf[i] = '0';
                    }
                }

                WriteToBuffer(&TempBufferPtr, buf, size);

                if (flags & LADJUST) PadBuffer(&TempBufferPtr, -width - size, ' ');

                // finally, adjust ret
                ret += width > size ? width : size;

                continue;

                    ///////////////////// FLOATING_POINT //////////////////////
            case 'n':

                if (flags & QUADINT) *x_va_arg(ap, s32 *) = ret;  //********* N64 s32->s64
                else
                if (flags & LONGINT) *x_va_arg(ap, s32 *) = ret;
                else
                if (flags & SHORTINT) *x_va_arg(ap, s16 *) = (s16)ret;
                else
                    *x_va_arg(ap, s32 *) = ret;

                // no output
                continue;

            //////////////////////////////// THIS IS NOT ANSI STANDARD
            //case 'O':
            //    flags |= LONGINT;
            //    /////////>>>>>>>>>>>>>>>>>> FALLTHROUGH <<<<<<<<<<<<<<//////////

            case 'o':
                if (flags & QUADINT) uqval = (u32)x_va_arg(ap, s32); //****** N64 s32->s64
                else
                    ulval = GetUnSignARG( (x_va_list*)&ap, flags );

                base = 8;
                goto nosign;

            case 'p':
                 // "The argument shall be a pointer to void.  The
                 // value of the pointer is converted to a sequence
                 // of printable characters, in an implementation-
                 // defined manner." 
                 // -- ANSI X3J11

                ulval = (u32)x_va_arg(ap, void *);
                base  = 16;
                xdigs = "0123456789abcdef";
                flags = (flags & ~QUADINT); // | HEXPREFIX; Not prefixes
                ch    = 'x';
                if (prec < 0) prec = 8;     // make sure that the precision is at 8

                goto nosign;

            case 's':

                cp = x_va_arg(ap, char*);
                ASSERT( cp != NULL );
                if (prec >= 0)
                {
                    // can't use strlen; can only look for the
                    // NULL in the first `prec' characters, and
                    // strlen() will go further.
                    char *p = (char*)x_memchr(cp, 0, (s32)prec);

                    if (p != NULL)
                    {
                        size = p - cp;
                        if (size > prec) size = prec;
                    }
                    else
                        size = prec;
                }
                else
                    size = x_strlen(cp);

                sign = '\0';
                break;

            case 'U':
                flags |= LONGINT;
                /////////>>>>>>>>>>>>>>>>>> FALLTHROUGH <<<<<<<<<<<<<<//////////

            case 'u':
                if (flags & QUADINT) uqval = (u32)x_va_arg(ap, s32);  //************ N64 s32->s64
                else
                    ulval = GetUnSignARG( (x_va_list*)&ap, flags );

                base = 10;
                goto nosign;

            case 'X': xdigs = "0123456789ABCDEF"; goto hex;
            case 'x': xdigs = "0123456789abcdef"; hex:


                if (flags & QUADINT) uqval = x_va_arg(ap, u32);  //************ N64 s32->s64
                else
                    ulval = GetUnSignARG( (x_va_list*)&ap, flags );

                base = 16;

                // leading 0x/X only if non-zero
                if (flags & ALT && (flags & QUADINT ? uqval != 0 : ulval != 0))
                    flags |= HEXPREFIX;

                // unsigned conversions
    nosign:     sign = '\0';

                // ``... diouXx conversions ... if a precision is
                // specified, the 0 flag will be ignored.''
                //  -- ANSI X3J11

    number:     if ((dprec = prec) >= 0) flags &= ~ZEROPAD;
                
                // ``The result of converting a zero value with an
                // explicit precision of zero is no characters.''
                // -- ANSI X3J11
                cp = buf + WORKSIZE;

                if (flags & QUADINT)
                {
                    if (uqval != 0 || prec != 0)
                    {
                        cp = ULtoA(uqval, cp, base, flags & ALT, xdigs);
                    }
                }
                else
                {
                    if (ulval != 0 || prec != 0)
                    {
                        cp = ULtoA(ulval, cp, base,flags & ALT, xdigs);
                    }
                }

                size = buf + WORKSIZE - cp;
                break;

            default:    // "%?" prints ?, unless ? is NUL
                if (ch == '\0') goto done;

                // pretend it was %c with argument ch
                cp = buf;
                *cp = ch;
                size = 1;
                sign = '\0';

                break;
            }

            // All reasonable formats wind up here.  At this point, `cp'
            // points to a string which (if not flags&LADJUST) should be
            // padded out to `width' places.  If flags&ZEROPAD, it should
            // first be prefixed by any sign or other prefix; otherwise,
            // it should be blank padded before the prefix is emitted.
            // After any left-hand padding and prefixing, emit zeroes
            // required by a decimal [diouxX] precision, then print the
            // string proper, then emit zeroes required by any leftover
            // floating precision; finally, if LADJUST, pad with blanks.


            // Compute actual size, so we know how much to pad.
            // size excludes decimal prec; realsz includes it.
            realsz = dprec > size ? dprec : size;
            if (sign) realsz++;
            else
                if (flags & HEXPREFIX) realsz += 2;

            // right-adjusting blank padding
            if ((flags & (LADJUST|ZEROPAD)) == 0) PadBuffer(&TempBufferPtr, width - realsz, ' ');

            // prefix
            if (sign)
            {
                WriteToBuffer(&TempBufferPtr, &sign, 1);
            }
            else
                if (flags & HEXPREFIX)
                {
                    ox[0] = '0';
                    ox[1] = ch;
                    WriteToBuffer(&TempBufferPtr, ox, 2);
                }

            // right-adjusting zero padding
            if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
                PadBuffer(&TempBufferPtr, width - realsz, '0');

            // leading zeroes from decimal precision
            PadBuffer(&TempBufferPtr, dprec - size, '0');

            // writte the integer number
            WriteToBuffer(&TempBufferPtr, cp, size);

            // left-adjusting padding (always blank)
            if (flags & LADJUST) PadBuffer(&TempBufferPtr, width - realsz, ' ');

            // finally, adjust ret
            ret += width > realsz ? width : realsz;
    }

done:
    WriteToBuffer(&TempBufferPtr, "\0", 1);

    return (ret);
}

//================================================================
//================================================================

#if 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//================================================================
// Printf
//================================================================
static
s32 Printf(const char *Format, ...)
{
    x_va_list ap;
    s32       Res;
    char      String[128];

    ASSERT (Format);

    x_va_start(ap, Format);
    Res = VSPrintF(String, Format, ap );
    x_va_end(ap);

    printf ("%s", String);
    return Res;
}

//================================================================
// SPrintf
//================================================================
static
s32 SPrintf(char* Buff, const char *Format, ...)
{
    x_va_list ap;
    s32     Res;

    ASSERT (Format);
    ASSERT (Buff);

    x_va_start(ap, Format);
    Res = x_vsprintf(Buff, Format, ap );
    x_va_end(ap);

    return Res;
}

//================================================================
// SPrintf
//================================================================
void test (char* fmt, ...)
{
    char Buff1[100];
    char Buff2[100];
    int  Size1;
    int  Size2;
    x_va_list ap;

    x_va_start( ap, fmt );

    Size1 = vsprintf (Buff1, fmt, ap );
    Size2 = VSPrintF (Buff2, fmt, ap );

    if( (Size1 != Size2) || (strcmp (Buff1, Buff2) != 0) )
    {
        printf ("------------------------------------\n");
        printf ("MS [%s] %d\n", Buff1, Size1 );
        printf ("IE [%s] %d\n", Buff2, Size2 );
    }
    else
    {
        // Output to the screen
        printf ("OK: %s\n", Buff1);
    }

    x_va_end(ap);
}


//================================================================
// main
//================================================================
void main (void)
{
    test( "%f",         0.1234 );
    test( "%010.4f",    123.456 );
    test( "%+010.4f",   123.456 );
    test( "%+09d",      123456 );
    test( "%s",         "STRING" ); 
    test( "%-20s",      "ABCDEFGHIJKLM" );
    test( "%+09x",      123456 );
    test( "%+09X",      123456 );
    test( "%+09o",      123456 );
    test( "%+09O",      123456 );
    test( "%+09Z",      123456 );
    test( "%boom" );
    test( "%e",         0.1234 );
    test( "%010.4e",    123.456 );
    test( "%+010.4e",   123.456 );


    test( "%*.*s",      20, 10, "ABCDEFGHIJKLM" );
    test( "%-*.*s",     20, 10, "ABCDEFGHIJKLM" );
    test( "%*s",        20, "ABCDEFGHIJKLM" );

    test( "%*d",        2, 123456 );
    test( "%*d",        20, 123456 );

    test( "%0*.*f",     10, 4, 123.456 );
    test( "%0*.*f",     10, 4, 123.456 );

    test( "0x%p",       main );
    test( "0x%P",       main );
    test( "%d%d%d",     2, 123, 456 );

    {
        int i ;
        union
        {
            int A[2];
            double B;
        } J;

        for (i=0; i < 10000; i++)
        {
            J.A[0] = rand();
            J.A[1] = rand();            
            test ("%e",  J.B );
            test ("%f",  J.B );
        }
    }

}

//================================================================

#endif
