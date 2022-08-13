#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>


#define ITERATIONS_MIN      1
#define ITERATIONS_MAX      100
#define ITERATIONS_DEF      1
#define INTERVAL_MIN        0
#define INTERVAL_MAX        300
#define INTERVAL_DEF        5


int main(int argc, char *argv[])
{
    time_t timep_time;
    time_t timep_gettimeofday;
    time_t timep_clock_gettime;
    struct timeval tv;
    struct timespec tp;
    extern char *tzname[];
    extern long timezone;
    extern int daylight;
    int opt;
    long iterations = (long) ITERATIONS_DEF;
    long interval = (long) INTERVAL_DEF;
    char datebuf[1024];

    while ((opt = getopt(argc, argv, "c:i:h")) != -1) {
        switch(opt) {
        case 'h':
            printf("%s [-c ITERATIONS] [-i INTERVAL]\n", argv[0]);
            printf("        -c ITERATIONS     : default = %d, range = [%d,%d]\n",
                ITERATIONS_DEF, ITERATIONS_MIN, ITERATIONS_MAX);
            printf("        -i INTERVAL       : default = %d, range = [%d,%d]\n",
                INTERVAL_DEF, INTERVAL_MIN, INTERVAL_MAX);
            exit(EXIT_SUCCESS);
            break;
        case 'c':                   // iterations
            iterations = atol(optarg);
            if ((iterations < ITERATIONS_MIN) ||
                (iterations > (long) ITERATIONS_MAX)) {
                    fprintf(stderr, "Invalid number of iterations: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
            break;
        case 'i':                   // interval
            interval = atol(optarg);
            if ((interval < INTERVAL_MIN) ||
                (interval > (long) INTERVAL_MAX)) {
                    fprintf(stderr, "Invalid sleep interval: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
            break;
        }
    }

    for (int i = (int) iterations; i > 0; i--) {
        tzset();
        time(&timep_time);
        gettimeofday(&tv, NULL);
        clock_gettime(CLOCK_REALTIME, &tp);
        timep_gettimeofday = tv.tv_sec;
        timep_clock_gettime = tp.tv_sec;
        strftime(datebuf, 1024, "%FT%T%z", localtime(&timep_time));
        printf("Timezone:          TZ=%s; DST=%s\n", tzname[0], tzname[1]);
        printf("West of UTC:       %d seconds\n", timezone);
        printf("DST rule:          %d\n", daylight);
        printf("time():            %s\n", datebuf);
        strftime(datebuf, 1024, "%FT%T%z", localtime(&timep_gettimeofday));
        printf("gettimeofday():    %s\n", datebuf);
        strftime(datebuf, 1024, "%FT%T%z", localtime(&timep_clock_gettime));
        printf("clock_gettime():   %s\n", datebuf);
        if (i > 1) {
            printf("\n");
            sleep((unsigned int) interval);
        }
    }
    return 0;
}