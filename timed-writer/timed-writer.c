/*
    timed-writer : writes an line to a file every so many seconds

    Copyright (C) 2022 Brendon Caligari <caligari@cypraea.co.uk>

    License: GNU Affero General Public License
        https://www.gnu.org/licenses/agpl-3.0.en.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define INTERVAL_DEFAULT    5
#define INTERVAL_MIN        1
#define INTERVAL_MAX        60*60
#define ITERATION_MAX       666             // safety bailout
#define BUF_MAX             1024

void fail_bad_args(char *progname) {
    fprintf(stderr, "%s: Invalid arguments\n", progname);
    exit(1);
}

void usage(char *progname) {
    printf("Usage: %s FILENAME [INTERVAL]\n", progname);
    printf("Writes a line to FILENAME every %d or [%d, %d] integral seconds\n",
        INTERVAL_DEFAULT,
        INTERVAL_MAX);
    printf("Example: %s /mnt/myfile.txt 10\n", progname);
    exit(0);
}

int line_writer(const char *filename, unsigned int interval) {
    char str_buf[BUF_MAX];
    size_t str_len;
    ssize_t written, ws;
    int _errno;
    int fd;
    struct timeval wall_clock_before;
    struct timeval wall_clock_after;
    double wall_clock_delta;
    if ((fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC|O_SYNC, (mode_t) 0666)) == -1) {
        _errno = errno;
        fprintf(stderr, "Unable to open %s : open(2) returned %d (%s)\n",
            filename,
            _errno,
            strerror(_errno));
        return 1;
    }
    for (int iter = 0; iter < ITERATION_MAX; iter++) {
        printf("\nWriting sequence %d\n", iter);
        sprintf(str_buf, "%d\n", iter);
        str_len = strlen(str_buf);
        gettimeofday(&wall_clock_before, NULL);
        ws = write(fd, str_buf, str_len);
        _errno = errno;
        if ((ws == -1) || (ws != (ssize_t) str_len)) {
            fprintf(stderr, "write(2) returned %d (%s)\n",
                filename,
                _errno,
                strerror(_errno));
        }
        gettimeofday(&wall_clock_after, NULL);
        if (ws != (ssize_t) str_len)
            printf("Wrote %d instead of %d!!\n", (int) ws, (int) str_len);
        wall_clock_delta = 
            ((double) wall_clock_after.tv_sec + ((double) wall_clock_after.tv_usec / 1000000)) -
            ((double) wall_clock_before.tv_sec + ((double) wall_clock_before.tv_usec / 1000000));
        printf("write(2) took approx %.1lf seconds\n", wall_clock_delta);
        sleep(interval);
    }
    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    long interval = (long) INTERVAL_DEFAULT;
    if ((argc == 1) || 
        (argc == 2) &&
        ((strcmp(argv[1], "-h") == 0) ||
         (strcmp(argv[1], "--help") == 0))) {
             usage(argv[0]);
         }
    if (argc == 3) {
        interval = atol(argv[2]);
        if ((interval == 0) ||
            (interval > (long) INTERVAL_MAX) ||
            (interval < (long) INTERVAL_MIN))
            fail_bad_args(argv[0]);
    }
    if (argc > 3)
        fail_bad_args(argv[0]);
    printf("Writing to %s with %lu second intervals\n", argv[0], interval);
    line_writer(argv[1], (unsigned int) interval);
}