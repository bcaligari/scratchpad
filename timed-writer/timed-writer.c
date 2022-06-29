/*
    timed-writer : writes an line to a file every so many seconds

    Copyright (C) 2022 Brendon Caligari <caligari@cypraea.co.uk>

    License: GNU Affero General Public License
        https://www.gnu.org/licenses/agpl-3.0.en.html

    TODO:
        - signal handling for controlled termination
        - cleanup code because this is embarrassing
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/times.h>
#include <limits.h>
#include <time.h>
#include <getopt.h>

#define INTERVAL_DEFAULT    5
#define INTERVAL_MIN        1
#define INTERVAL_MAX        60*60
#define ITERATION_MAX       666
#define FAILURE_MAX         100
#define FAILURE_DEFAULT     5
#define BUF_MAX             1024


void usage(char *progname) {
    printf("Usage: %s [-s SLEEP ] [-c MAX_ITER] [-l] FILENAME\n", progname);
    printf("       %s -h\n", progname);
    printf("\n");
    printf("Writes a line to FILENAME with SLEEP seconds between writes\n");
    printf("\n");
    printf("        -s SLEEP     : seconds sleep after each iteration (default: %d; bounds: [%d, %d])\n",
        INTERVAL_DEFAULT,
        INTERVAL_MIN,
        INTERVAL_MAX);
    printf("        -c MAX_ITER  : limit iterations to MAX_ITER (def: %d)\n",
        ITERATION_MAX);
    printf("        -f MAX_FAIL  : limit consecutive write() failures to MAX_FAIL <= %d (def: %d; inf: 0)\n",
        FAILURE_MAX,
        FAILURE_DEFAULT);
    printf("        -l           : place LOCK_EX on FILENAME\n");
    printf("\n");
    printf("Example: %s /mnt/myfile.txt\n", progname);
    printf("         %s -s 5 -c 100 -l /mnt/myexlusive.txt\n", progname);
    printf("\n");
}


int line_writer(const char *filename,
                unsigned int interval,
                int excl_lock,
                unsigned int iterations,
                unsigned int failmax) {
    char str_buf[BUF_MAX];
    size_t str_len;
    ssize_t written, ws;
    int _errno;
    int fd;
    unsigned int failures = 0;
    struct timeval wall_clock_before;
    struct timeval wall_clock_after;
    double wall_clock_delta;
    struct tms times_before;
    struct tms times_after;
    double user_times_delta;
    double sys_times_delta;

    printf("Filename: %s\n", filename);
    printf("Exclusive lock: %s\n", excl_lock ? "on" : "off");
    printf("Sleep after each write: %u\n", interval);
    printf("Max iterations: %u\n", iterations);
    printf("Max consecutive write fails: %u\n", failmax);

    if ((fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC|O_SYNC, (mode_t) 0666)) == -1) {
        _errno = errno;
        fprintf(stderr, "Unable to open %s : open() returned %d (%s)\n",
            filename,
            _errno,
            strerror(_errno));
        return 1;
    }

    if (excl_lock) {
        if (flock(fd, LOCK_EX) == -1) {
            _errno = errno;
            fprintf(stderr, "Unable to place lock on %s : flock() returned %d (%s)\n",
                filename,
                _errno,
                strerror(_errno));
            return 1;
        }
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        printf("\nWriting sequence %d\n", iter);
        sprintf(str_buf, "%d\n", iter);
        str_len = strlen(str_buf);
        gettimeofday(&wall_clock_before, NULL);
        times(&times_before);
        ws = write(fd, str_buf, str_len);
        _errno = errno;
        times(&times_after);
        gettimeofday(&wall_clock_after, NULL);
        if (ws == -1) {
            fprintf(stderr, "write() failed with errno %d (%s)\n",
                _errno,
                strerror(_errno));
            if (failmax > 0) {
                failures++;
                if (failures == failmax) {
                    fprintf(stderr, "Reached max failcount ... bye!\n");
                    exit(EXIT_FAILURE);
                }
            }
        } else
            failures = 0;
        if ((ws != -1) && (ws != (ssize_t) str_len))
            printf("write() returned %d instead of %d. Interrupted?!!\n", (int) ws, (int) str_len);
        wall_clock_delta = 
            ((double) wall_clock_after.tv_sec + ((double) wall_clock_after.tv_usec / 1000000)) -
            ((double) wall_clock_before.tv_sec + ((double) wall_clock_before.tv_usec / 1000000));
        user_times_delta =
            ((double) (times_after.tms_utime - times_before.tms_utime) / sysconf(_SC_CLK_TCK));
        sys_times_delta =
            ((double) (times_after.tms_stime - times_before.tms_stime) / sysconf(_SC_CLK_TCK));
        printf("write() took approx %.2lf seconds (user: %.2lf; sys: %.2lf)\n",
            wall_clock_delta,
            user_times_delta,
            sys_times_delta);
        sleep(interval);
    }
    close(fd);
    return 0;
}


int main(int argc, char *argv[]) {
    long interval = (long) INTERVAL_DEFAULT;
    long iterations = (long) ITERATION_MAX;
    long failmax = (long) FAILURE_DEFAULT;
    int excl_lock = 0;
    int opt;

    while ((opt = getopt(argc, argv, "s:c:f:lh")) != -1) {
        switch(opt) {
        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);
        case 's':                   // sleep time between iterations
            interval = atol(optarg);
            if ((interval == 0) ||
                (interval > (long) INTERVAL_MAX) ||
                (interval < (long) INTERVAL_MIN)) {
                    fprintf(stderr, "Invalid sleep time: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
            break;
        case 'c':                   // maximum iterations
            iterations = atol(optarg);
            if ((iterations <= 0) ||
                (interval > (long) ITERATION_MAX)) {
                    fprintf(stderr, "Invalid max iterations: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
            break;
        case 'f':                   // maximum consecutive write fails
            failmax = atol(optarg);
            if ((failmax < 0) ||
                (failmax > (long) FAILURE_MAX)) {
                    fprintf(stderr, "Invalid max consecutive write failures: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
            break;
        case 'l':
            excl_lock = 1;
            break;
        default:
            fprintf(stderr, "Command line gibberish, try -h\n");
            exit(EXIT_FAILURE);
        }
    }
    if (optind + 1 != argc) {
        fprintf(stderr, "Expecting one, and only one, FILENAME\n");
        exit(EXIT_FAILURE);
    }

    line_writer(argv[optind],
                (unsigned int) interval,
                excl_lock,
                (unsigned int) iterations,
                (unsigned int) failmax);
}