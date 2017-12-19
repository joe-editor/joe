/* Check width of each unicode character by printing character and reading back cursor position.
   This lets you check what the terminal emulator is doing.

   Use it like this: ./check-widths 2>widths.txt

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <poll.h>
#include <sys/fcntl.h>
#include <termios.h>

#define TO_CHAR_OK(c) ((char)(c))

ptrdiff_t utf8_encode(char *buf,int c)
{
	if (c < 0x80) {
		buf[0] = TO_CHAR_OK(c);
		buf[1] = 0;
		return 1;
	} else if(c < 0x800) {
		buf[0] = TO_CHAR_OK((0xc0|(c>>6)));
		buf[1] = TO_CHAR_OK((0x80|(c&0x3F)));
		buf[2] = 0;
		return 2;
	} else if(c < 0x10000) {
		buf[0] = TO_CHAR_OK((0xe0|(c>>12)));
		buf[1] = TO_CHAR_OK((0x80|((c>>6)&0x3f)));
		buf[2] = TO_CHAR_OK((0x80|((c)&0x3f)));
		buf[3] = 0;
		return 3;
	} else if(c < 0x200000) {
		buf[0] = TO_CHAR_OK((0xf0|(c>>18)));
		buf[1] = TO_CHAR_OK((0x80|((c>>12)&0x3f)));
		buf[2] = TO_CHAR_OK((0x80|((c>>6)&0x3f)));
		buf[3] = TO_CHAR_OK((0x80|((c)&0x3f)));
		buf[4] = 0;
		return 4;
	} else if(c < 0x4000000) {
		buf[0] = TO_CHAR_OK((0xf8|(c>>24)));
		buf[1] = TO_CHAR_OK((0x80|((c>>18)&0x3f)));
		buf[2] = TO_CHAR_OK((0x80|((c>>12)&0x3f)));
		buf[3] = TO_CHAR_OK((0x80|((c>>6)&0x3f)));
		buf[4] = TO_CHAR_OK((0x80|((c)&0x3f)));
		buf[5] = 0;
		return 5;
	} else {
		buf[0] = TO_CHAR_OK((0xfC|(c>>30)));
		buf[1] = TO_CHAR_OK((0x80|((c>>24)&0x3f)));
		buf[2] = TO_CHAR_OK((0x80|((c>>18)&0x3f)));
		buf[3] = TO_CHAR_OK((0x80|((c>>12)&0x3f)));
		buf[4] = TO_CHAR_OK((0x80|((c>>6)&0x3f)));
		buf[5] = TO_CHAR_OK((0x80|((c)&0x3f)));
		buf[6] = 0;
		return 6;
	}
}

int full_read(unsigned char *p, int total, int timeout)
{
    int amount_read = 0;
    ssize_t len;
    struct pollfd item;

    item.fd = fileno(stdin);
    item.events = POLLIN;
    item.revents = 0;

    while (total) {
        if (poll(&item, 1, timeout) < 1)
            return amount_read;
        len = read(fileno(stdin), p, total);
        if (len > 0) {
            total -= len;
            amount_read += len;
            p += len;
        }
    }

    return amount_read;
}

int get_line(char *buf, int timeout)
{
    char c;
    for (;;) {
        int x;
        for (x = 0; x != timeout; ++x)
            if (1 == full_read(&c, 1, 32))
                break;
        if (x == timeout)
            return -1;
        *buf++ = c;
        if (c == 'R') {
            *buf++ = 0;
            return 0;
        }
    }
    
}

int main()
{
    struct termios org_attr;
    struct termios attr;
    char buf[16];
    int x;
    int first = -1;
    int last = -1;
    int wid = -1;

    if (tcgetattr(fileno(stdin), &attr)) {
        printf("Couldn't read termios data\n");
        return -1;
    }
    org_attr = attr;

    /* Set up terminal */
    /* Force modes we always want */
    attr.c_iflag |= (IGNBRK | IGNPAR);

#ifdef IUCLC /* not POSIX */
    attr.c_iflag &= ~(BRKINT | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | IUCLC | IXANY);
#else
    attr.c_iflag &= ~(BRKINT | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | IXANY);
#endif

    attr.c_cflag |= (CREAD);

#ifdef XCASE /* not POSIX */
    attr.c_lflag &= ~(XCASE | ECHONL | ECHOCTL | ECHOPRT | ECHOK | NOFLSH);
#else
    attr.c_lflag &= ~(ECHONL | ECHOCTL | ECHOPRT | ECHOK | NOFLSH);
#endif

    attr.c_lflag |= (ECHOE | ECHOKE);

    /* Modes we usually want */
    attr.c_lflag &= ~(ECHO | ICANON);
//    attr.c_oflag &= ~(ONLCR);
    attr.c_iflag &= ~(ICRNL);
    if (tcsetattr(fileno(stdin), TCSADRAIN, &attr)) {
        printf("Couldn't set termios data\n");
        return -1;
    }


    for (x = 1; x != 0x110000; ++x) {
//    for (x = 1; x != 0x10000; ++x) {
        int line, col;
        utf8_encode(buf, x);
        printf("\r%s|\033[6n", buf); fflush(stdout);
        // Should get back ESC [ line ; col R
        get_line(buf, 100);
        if (buf[0] == 27) buf[0] = 'E';
        sscanf(buf,"E[%d;%dR", &line, &col);
        printf("\r\n%x: %s %d %d\n", x, buf, line, col);

        col -= 2;
        
        if (col == wid) {
            last = x;
        } else {
            if (first != -1) {
                fprintf(stderr, "%x..%x %d\n", first, last, wid);
                first = last = -1;
                wid = -1;
            } 
            if (col != 1) {
                first = last = x;
                wid = col;
            }
        }
    }
    if (first != -1) {
        fprintf(stderr, "%x..%x %d\n", first, last, wid);
    }

    tcsetattr(fileno(stdin), TCSADRAIN, &org_attr);
}
