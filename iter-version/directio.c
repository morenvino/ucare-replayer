
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <limits.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/fs.h>

#define MAX_ITER 128 //512

int main(int argc, char **argv) 
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dev> <byte>\n", argv[0]);
        return 1;
    }
    int b = atoi(argv[2]);
    if (b % 512 != 0) {
        fprintf(stderr, "Byte <byte> has to be multiple of 512 bytes\n");
        return 1;
    }
    char *dev = argv[1];
    if (strstr(dev, "/dev/sda")) { // avoid accidentally writing to system
        fprintf(stderr, "Error trying to write to system partition %s\n", dev);
        return 1;        
    }
    
    srand(time(NULL));

    void *buf;
    int err = posix_memalign(&buf, 512, b);
    if (err) {
        fprintf(stderr, "Error aligning memory\n");
        return 1;    
    }
    memset(buf, 0, b);
    //memset(buf, 0xAB, b);
    int i;
    //char *buffer = buf;
    //for (i = 0; i < b; ++i) buffer[i] = i;
    
    printf("Opening device %s\n", dev);
    int f = open(dev, O_WRONLY | O_SYNC | O_DIRECT);
    if (f == -1) {
        fprintf(stderr, "Error opening device %s\n", dev);
        return 1;   
    }

    printf("Test writing %d bytes to %s\n", b, dev);
    err = write(f, buf, b);
    if (err == -1) {
        fprintf(stderr, "Error writing to device %s\n", dev);
        return 1;
    }    
    
    struct timeval begin, end; double duration;
    clock_t begincpu, endcpu; double durationcpu;
    
    printf("\n");
    printf("Starting benchmark for sequential write\n");
    
    gettimeofday(&begin, NULL);
    begincpu = clock(); // start timer
    
    for (i = 0; i < MAX_ITER; ++i) {
        err = write(f, buf, b);
        if (err == -1) {
            fprintf(stderr, "Error writing to device %s\n", dev);
            return 1;
        }    
    }
    
    endcpu = clock(); // end timer
    gettimeofday(&end, NULL);

    durationcpu = (double)(endcpu - begincpu) / CLOCKS_PER_SEC; 
    duration = (end.tv_sec-begin.tv_sec) + (end.tv_usec-begin.tv_usec)*1e-6;
    
    printf("Wrote: %0.2f MB\n", (b * 1e-6) * MAX_ITER);
    printf("Time : %0.2f s \n", duration);
    printf("Cpu  : %0.2f s \n", durationcpu);
    printf("Avg  : %0.2f MB/s\n", ((b * 1e-6) / duration) * MAX_ITER);
    
    printf("\n");
    printf("Starting benchmark for random write\n");
    uint64_t numblocks;
    err = ioctl(f, BLKGETSIZE, &numblocks); // get num of blocks in the disk
    if (err) {
        fprintf(stderr, "Error getting number of blocks\n");
        return 1;       
    }

    gettimeofday(&begin, NULL);
    begincpu = clock(); // start timer

    off64_t offset;
    for (i = 0; i < MAX_ITER; ++i) {
        offset = (off64_t) numblocks * random() / RAND_MAX;
        err = lseek(f, 512 * offset, SEEK_SET); // why only 512 works?
        if (err == -1) {
            fprintf(stderr, "Error seeking offset: %m\n");
            return 1;       
        }
        err = write(f, buf, /*b*/ 4096); // knot asked to use 4096
    }

    endcpu = clock(); // end timer
    gettimeofday(&end, NULL);

    durationcpu = (double)(endcpu - begincpu) / CLOCKS_PER_SEC; 
    duration = (end.tv_sec-begin.tv_sec) + (end.tv_usec-begin.tv_usec)/1000000.0;
    
    printf("Wrote: %d IOP\n", MAX_ITER);
    printf("Time : %0.2f s \n", duration);
    printf("Cpu  : %0.2f s \n", durationcpu);
    printf("Avg  : %0.2f IOP/s\n", MAX_ITER / duration);

    printf("Done\n");
    close(f);
    return 0;
}
