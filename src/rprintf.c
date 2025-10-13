/*---------------------------------------------------*/
/* Public Domain version of printf (embedded-friendly)*/
/* Rud Merriam (original), lightly fixed/extended     */
/*---------------------------------------------------*/

#include "rprintf.h"

/*---------------------------------------------------*/
/* Minimal helpers & state                           */
/*---------------------------------------------------*/
static func_ptr out_char;
static int do_padding;
static int left_flag;
static int len;
static int num1;
static int num2;
static char pad_character;

size_t strlen(const char *str) {
    unsigned int l = 0;
    while (str && str[l] != '\0') l++;
    return l;
}

/* correct tolower: only convert 'A'..'Z' */
int tolower(int c) {
    if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
    return c;
}

int isdig(int c) { return (c >= '0' && c <= '9'); }

/*---------------------------------------------------*/
/* Padding & string emit                             */
/*---------------------------------------------------*/
static void padding(const int l_flag)
{
    int i;
    if (do_padding && l_flag && (len < num1))
        for (i = len; i < num1; i++)
            out_char(pad_character);
}

static void outs(charptr lp)
{
    if (lp == NULL) lp = "(null)";

    /* pad on left if needed */
    len = (int)strlen(lp);
    padding(!left_flag);

    /* emit (optionally width-limited via num2) */
    while (*lp && num2--)
        out_char(*lp++);

    /* pad on right if needed */
    len = (int)strlen(lp);
    padding(left_flag);
}

/*---------------------------------------------------*/
/* Unsigned number to string with base               */
/*---------------------------------------------------*/
static void outnum_u(unsigned long num, int base)
{
    char outbuf[32];
    charptr cp = outbuf;
    const char digits[] = "0123456789ABCDEF";

    /* build number backwards in outbuf */
    do {
        *cp++ = digits[num % (unsigned)base];
        num /= (unsigned)base;
    } while (num > 0);

    *cp-- = 0;

    /* padding on left */
    len = (int)strlen(outbuf);
    padding(!left_flag);

    /* emit */
    while (cp >= outbuf)
        out_char(*cp--);

    /* padding on right */
    padding(left_flag);
}

/* signed decimal with optional leading '-' */
static void outnum_s(long val)
{
    unsigned long u;
    int neg = (val < 0);

    if (neg) val = -val;
    u = (unsigned long)val;

    if (!do_padding || left_flag) {
        /* no left padding to worry about: print sign first if needed */
        if (neg) out_char('-');
        outnum_u(u, 10);
        return;
    }

    /* When left-padding with width, we need to account for the '-' in width */
    /* Compute the decimal digits length */
    char tmp[32];
    char *p = tmp;
    unsigned long t = u;
    do { *p++ = (char)('0' + (t % 10)); t /= 10; } while (t);
    int digits = (int)(p - tmp);

    int needed = digits + (neg ? 1 : 0);
    len = needed;

    /* left pad (spaces or zeros) */
    if (do_padding && !left_flag && (len < num1)) {
        int padcount = num1 - len;
        while (padcount--) out_char(pad_character);
    }

    if (neg) out_char('-');

    /* print number */
    while (p != tmp) out_char(*--p);
}

/*---------------------------------------------------*/
/* Parse integer from format                         */
/*---------------------------------------------------*/
static int getnum(charptr* linep)
{
    int n = 0;
    charptr cp = *linep;
    while (isdig((int)*cp))
        n = n*10 + ((*cp++) - '0');
    *linep = cp;
    return n;
}

/*---------------------------------------------------*/
/* printf core                                       */
/*---------------------------------------------------*/
void esp_printf(const func_ptr f_ptr, charptr ctrl, ...)
{
    va_list args;
    va_start(args, *ctrl);
    esp_vprintf(f_ptr, ctrl, args);
    va_end(args);
}

void esp_vprintf(const func_ptr f_ptr, charptr ctrl, va_list argp)
{
    int long_flag;
    int dot_flag;
    char ch;

    out_char = f_ptr;

    for (; *ctrl; ctrl++) {

        if (*ctrl != '%') { out_char(*ctrl); continue; }

        /* reset flags for this conversion */
        dot_flag   =
        long_flag  =
        left_flag  =
        do_padding = 0;
        pad_character = ' ';
        num2 = 32767;

try_next:
        ch = *(++ctrl);

        if (isdig((int)ch)) {
            if (dot_flag) {
                num2 = getnum(&ctrl);
            } else {
                if (ch == '0') pad_character = '0';
                num1 = getnum(&ctrl);
                do_padding = 1;
            }
            ctrl--;
            goto try_next;
        }

        switch (tolower((int)ch)) {
        case '%':
            out_char('%'); 
            continue;

        case '-':
            left_flag = 1; 
            break;

        case '.':
            dot_flag = 1; 
            break;

        case 'l':
            long_flag = 1; 
            break;

        case 'i':
        case 'd': {
            if (long_flag || ch == 'D') {
                long v = va_arg(argp, long);
                outnum_s(v);
            } else {
                int v = va_arg(argp, int);
                outnum_s((long)v);
            }
            continue;
        }

        case 'u': { /* unsigned decimal */
            unsigned long v = long_flag ? va_arg(argp, unsigned long)
                                        : (unsigned long)va_arg(argp, unsigned int);
            outnum_u(v, 10);
            continue;
        }

        case 'x': { /* unsigned hex */
            unsigned long v = long_flag ? va_arg(argp, unsigned long)
                                        : (unsigned long)va_arg(argp, unsigned int);
            outnum_u(v, 16);
            continue;
        }

        case 'p': { /* pointer as 0x.... */
            unsigned long v = (unsigned long)va_arg(argp, void*);
            out_char('0'); out_char('x');
            outnum_u(v, 16);
            continue;
        }

        case 's':
            outs(va_arg(argp, charptr));
            continue;

        case 'c':
            out_char(va_arg(argp, int));
            continue;

        case '\\':
            switch (*ctrl) {
            case 'a': out_char(0x07); break;
            case 'h': out_char(0x08); break;
            case 'r': out_char(0x0D); break;
            case 'n': out_char(0x0D); out_char(0x0A); break;
            default:  out_char(*ctrl); break;
            }
            ctrl++;
            break;

        default:
            /* unknown specifier: ignore and continue */
            continue;
        }
        goto try_next;
    }

    /* argp was provided by caller; no va_end here */
}
/*---------------------------------------------------*/
