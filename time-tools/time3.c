#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main ()
{
    time_t timep;

    for (int i; i < 300; i++) {
        time(&timep);
        printf("Localtime: %s", asctime(localtime(&timep)));
        printf("UTC: %s", asctime(gmtime(&timep)));
        printf("\n");
        sleep(5);
    }
    return 0;
}