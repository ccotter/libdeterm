
#include <inc/io.h>
#include <inc/types.h>
#include <inc/string.h>
#include <inc/syscall.h>
#include <inc/fs.h>

/* Illegal direct access to console. For debugging purposes. */
#define UL(x) ((unsigned long)x)
static ssize_t write(int fd, const void *buf, size_t count)
{
    return (ssize_t)syscall3(SYS_write, UL(fd), UL(buf), UL(count));
}
#undef UL

void _putch(int i)
{
    char ch = (char)i;
    write(2, &ch, 1);
    /*putch_buffer[putch_idx] = (char)i;
    if (sizeof(putch_buffer) == ++putch_idx)
    {
        write(1, putch_buffer, sizeof(putch_buffer));
    }*/
}

// Get an unsigned int of various possible sizes from a varargs list,
// depending on the lflag parameter.
static unsigned long long
getuint(va_list *ap, int lflag)
{
    if (lflag >= 2)
        return va_arg(*ap, unsigned long long);
    else if (lflag)
        return va_arg(*ap, unsigned long);
    else
        return va_arg(*ap, unsigned int);
}

// Same as getuint but signed - can't use getuint
// because of sign extension
static long long
getint(va_list *ap, int lflag)
{
    if (lflag >= 2)
        return va_arg(*ap, long long);
    else if (lflag)
        return va_arg(*ap, long);
    else
        return va_arg(*ap, int);
}
static void
printnum(void (*putch)(int), unsigned long num, unsigned base, int width, int padc)
{
    // first recursively print all preceding (more significant) digits
    if (num >= base) {
        printnum(putch, num / base, base, width - 1, padc);
    } else {
        // print any needed pad characters before first digit
        while (--width > 0)
            putch(padc);
    }

    // then print this (the least significant) digit
    putch("0123456789abcdef"[num % base]);
}

static void
vprintfmt(void (*putch)(int), const char *fmt, va_list ap)
{
    register const char *p;
    register int ch, err;
    unsigned long long num;
    int base, lflag, width, precision, altflag;
    char padc;

    while (1) {
        while ((ch = *(unsigned char *) fmt++) != '%') {
            if (ch == '\0')
                return;
            putch(ch);
        }

        // Process a %-escape sequence
        padc = ' ';
        width = -1;
        precision = -1;
        lflag = 0;
        altflag = 0;
    reswitch:
        switch (ch = *(unsigned char *) fmt++) {

        // flag to pad on the right
        case '-':
            padc = '-';
            goto reswitch;
            
        // flag to pad with 0's instead of spaces
        case '0':
            padc = '0';
            goto reswitch;

        // width field
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            for (precision = 0; ; ++fmt) {
                precision = precision * 10 + ch - '0';
                ch = *fmt;
                if (ch < '0' || ch > '9')
                    break;
            }
            goto process_precision;

        case '*':
            precision = va_arg(ap, int);
            goto process_precision;

        case '.':
            if (width < 0)
                width = 0;
            goto reswitch;

        case '#':
            altflag = 1;
            goto reswitch;

        process_precision:
            if (width < 0)
                width = precision, precision = -1;
            goto reswitch;

        // long flag (doubled for long long)
        case 'l':
            lflag++;
            goto reswitch;

        // character
        case 'c':
            putch(va_arg(ap, int));
            break;

        // string
        case 's':
            if ((p = va_arg(ap, char *)) == NULL)
                p = "(null)";
            if (width > 0 && padc != '-')
                for (width -= strnlen(p, precision); width > 0; width--)
                    putch(padc);
            for (; (ch = *p++) != '\0' && (precision < 0 || --precision >= 0); width--)
                if (altflag && (ch < ' ' || ch > '~'))
                    putch('?');
                else
                    putch(ch);
            for (; width > 0; width--)
                putch(' ');
            break;

        // (signed) decimal
        case 'd':
            num = getint(&ap, lflag);
            if ((long long) num < 0) {
                putch('-');
                num = -(long long) num;
            }
            base = 10;
            goto number;

        // unsigned decimal
        case 'u':
            num = getuint(&ap, lflag);
            base = 10;
            goto number;

        // (unsigned) octal
        case 'o':
            // Replace this with your code.
         num = getuint(&ap, lflag);
         base = 8;
         goto number;
            break;

        // pointer
        case 'p':
            putch('0');
            putch('x');
            num = (unsigned long long)
                (uintptr_t) va_arg(ap, void *);
            base = 16;
            goto number;

        // (unsigned) hexadecimal
        case 'x':
            num = getuint(&ap, lflag);
            base = 16;
        number:
            printnum(putch, num, base, width, padc);
            break;

        // escaped '%' character
        case '%':
            putch(ch);
            break;
            
        // unrecognized escape sequence - just print it literally
        default:
            putch('%');
            for (fmt--; fmt[-1] != '%'; fmt--)
                /* do nothing */;
            break;
        }
    }
}

int iprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(_putch, fmt, ap);
    va_end(ap);
    return 0;
}

/* We buffer characters from printf calls and call dfs_write(__stdout) when
   the buffer is filled. */
#define PRINT_BUFFER_SIZE 512
struct printbuf
{
    char buf[PRINT_BUFFER_SIZE];
    int idx;
};
struct printbuf pb;
/* vprintfmt writes characters to our printbuf. */
static void putch_printf(int ch)
{
    pb.buf[pb.idx] = (char)ch;
    ++pb.idx;
    if (PRINT_BUFFER_SIZE - 1 == pb.idx)
    {
        dfs_write(1, pb.buf, PRINT_BUFFER_SIZE);
        pb.idx = 0;
    }
}
void flush_printf_buffer(void)
{
    dfs_write(1, pb.buf, pb.idx);
    pb.idx = 0;
}

/* This is the correct way to "print" data. */
int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(putch_printf, fmt, ap);
    va_end(ap);
    return 0;
}

/* There should be on the stack if I had to worry about multithreading, but since
   everything is "deterministic" I won't worry. Though, I should make these stack
   arguments since that is a cleaner solution... */
static int snprintf_idx;
static int snprintf_size;
static char *snprintf_buf;
static void putch_snprintf(int ch)
{
    if (snprintf_size - 1 <= snprintf_idx)
        return; /* snprintf overflow. */
    snprintf_buf[snprintf_idx++] = (char)ch;
}
int snprintf(char *buf, ssize_t size, const char *fmt, ...)
{
    /* va_list ap;
    snprintf_buf = buf;
    snprintf_idx = 0;
    snprintf_size = size;
    va_start(ap, fmt);
    vprintfmt(putch_snprintf, fmt, ap);
    va_end(ap);
    snprintf_buf[snprintf_idx] = 0; / * Always NULL terminate. * /
    return snprintf_idx; */
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
    snprintf_buf = str;
    snprintf_idx = 0;
    snprintf_size = size;
    vprintfmt(putch_snprintf, fmt, ap);
    snprintf_buf[snprintf_idx] = 0; /* Always NULL terminate. */
    return snprintf_idx;
}

/* Return to parent if __stdin is empty. */
int getchar(void)
{
    int n;
    char c;
begin:
    n = dfs_read(0, &c, 1);
    if (0 == n)
    {
        io_sync(1);
        n = dfs_read(0, &c, 1);
    }
    if (n == 1)
        return (int)c;
    else if (n < 0)
        return EOF;
    else
        return EOF;
}

int io_sync(int needinput)
{
    /* Get the current offset of stdin to remember where we were since our parent
       will overwrite our stdin file descriptor offset. */
    long off = dfs_tell(0);
    if (needinput)
        dfs_setstate(DPROC_INPUT);
    flush_printf_buffer();
    dret();
    dfs_seek(0, off, DSEEK_SET);
    dfs_setstate(DPROC_READY);
    return 0;
}

