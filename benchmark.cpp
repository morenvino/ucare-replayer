/** 
 * @author morenvino@uchicago.edu
 * @file   Direct i/o benchmark. 
 *         Direct i/o part is adapted from tiratatp@uchicago.edu
 *         http://pastebin.com/yR36MsKR  
 */

/* ===========================================================================
 * Headers
 * ===========================================================================*/

//#define _GNU_SOURCE // for C compiler
#include <iostream>
#include <vector>
#include <thread>

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
#include <pthread.h>

#include <linux/fs.h>

#include "Config.h"

using std::thread;
using std::vector;
using std::ostream;
using std::endl;
using mv::Config;

/* ===========================================================================
 * Consts
 * ===========================================================================*/

#define MEM_ALIGN   512  // Memory alignment

/* ===========================================================================
 * Types
 * ===========================================================================*/

typedef struct { int f, b; void *buf; } arg_t;

/* ===========================================================================
 * Functions
 * ===========================================================================*/

static inline int log_nothing(const char *, ...) { return 0; }
static void * seq_write_time(void *arg); 
static void * rand_write_time(void *arg);

/* ===========================================================================
 * Global variables
 * ===========================================================================*/

auto log = printf; // log using printf by default 

/* ===========================================================================
 * Main
 * ===========================================================================*/

int main(int argc, char **argv) 
{
    // read config file
    Config config;
    auto blocksize_seq = config.get<int>("blocksize_seq");
    auto duration_seq  = config.get<int>("duration_seq");
    auto nthreads_seq  = config.get<int>("nthreads_seq");
    auto blocksize_rnd = config.get<int>("blocksize_rnd");
    auto duration_rnd  = config.get<int>("duration_rnd");
    auto nthreads_rnd  = config.get<int>("nthreads_rnd");
    auto start_delay   = config.get<int>("start_delay");
    auto interactive   = config.get<bool>("interactive");
    auto dev           = config.get<const char *>("disk_guest");

    // parse arg
    if (argc > 1) nthreads_rnd = atoi(argv[1]);

    // validate config / arg
    if ((blocksize_rnd % MEM_ALIGN != 0) || (blocksize_seq % MEM_ALIGN != 0)) {
        fprintf(stderr, "Block size has to be multiple of 512 bytes\n");
        return 1;
    }
    if (strstr(dev, "/dev/sda")) { // avoid accidentally writing to system 
        fprintf(stderr, "Error trying to write to system partition %s\n", dev);
        return 1;        
    }

    // initialize
    srand(time(NULL));
    if (not interactive) log = log_nothing;

    log("Aligning memory of %d bytes\n", blocksize_seq);
    void *buf; auto b = blocksize_seq; 
    int err = posix_memalign(&buf, MEM_ALIGN, b);
    if (err) {
        fprintf(stderr, "Error aligning memory\n");
        return 1;    
    }
    memset(buf, 0, b);

    log("Opening device %s\n", dev);
    int f = open(dev, O_WRONLY | O_SYNC | O_DIRECT);
    if (f == -1) {
        fprintf(stderr, "Error opening device %s\n", dev);
        return 1;   
    }

    log("Test writing %d bytes to %s\n", b, dev);
    err = write(f, buf, b);
    if (err == -1) {
        fprintf(stderr, "Error writing to device %s\n", dev);
        return 1;
    } 

/*
    vector<thread> threads(2);
    for (auto& t : threads) t = thread([] { // launch threads
        log("1\n");
        int x = 0;
    });   
    
    for (auto& t : threads) t.join(); // wait for all threads to finish
*/
        
    /*
    arg_t arg = {f, b, buf};    
    
    pthread_t t1, t2;
    //pthread_t threads[16];
    //int numthreads = sizeof(threads) / sizeof(threads[0]);
    
    printf("Starting benchmark for sequential write\n");
    pthread_create(&t1, NULL, seq_write_time, &arg);
    // pthread_create(&t3, NULL, seq_write_time, &arg);

    //printf("Starting benchmark for random write\n");
    // pthread_create(&t2, NULL, rand_write_time, &arg);
    //pthread_create(&t3, NULL, rand_write_time, &arg);
    //for (int i = 0; i < numthreads; ++i)
    //   pthread_create(&threads[i], NULL, rand_write_time, &arg);

    pthread_join(t1, NULL); 
    // pthread_join(t2, NULL);
    //pthread_join(t3, NULL);
    //for (int i = 0; i < numthreads; ++i)
     //  pthread_join(threads[i], NULL);

    for (int numthreads = 1; numthreads <= 16; numthreads *= 2) {
        pthread_t threads[numthreads];
        //int numthreads = sizeof(threads) / sizeof(threads[0]);
    
        printf("Starting benchmark for random write\n");
        // pthread_create(&t2, NULL, rand_write_time, &arg);
        //pthread_create(&t3, NULL, rand_write_time, &arg);
        for (int i = 0; i < numthreads; ++i)
            pthread_create(&threads[i], NULL, rand_write_time, &arg);

        sleep(1); // make sure rand_write() is started

        printf("Starting benchmark for sequential write\n");
        pthread_create(&t1, NULL, seq_write_time, &arg);
        // pthread_create(&t3, NULL, seq_write_time, &arg);

        pthread_join(t1, NULL); 
        // pthread_join(t2, NULL);
        //pthread_join(t3, NULL);
        for (int i = 0; i < numthreads; ++i)
            pthread_join(threads[i], NULL);
    }
    */

    printf("Done\n");
    close(f);
    return 0;
}

/* ===========================================================================
 * Functions
 * ===========================================================================*/

/*
void * seq_write_time(void *arg) 
{
    arg_t *args = (arg_t *)arg;
    int f = args->f, b = args->b; void *buf = args->buf;

    struct timeval begin, end; double duration = 0;
    clock_t begincpu, endcpu; double durationcpu = 0;

    gettimeofday(&begin, NULL);
    begincpu = clock(); // start timer
    
    int err; int itercount = 0;
    while (duration < DURATION) {
        err = write(f, buf, b);
        if (err == -1) {
            fprintf(stderr, "Error writing to device\n");
            return 0;
        }
        ++itercount;

        endcpu = clock(); // end timer
        gettimeofday(&end, NULL);

        durationcpu = (double)(endcpu - begincpu) / CLOCKS_PER_SEC; 
        duration = (end.tv_sec-begin.tv_sec) + (end.tv_usec-begin.tv_usec)*1e-6;    
    }
    
    printf("\nBenchmark result for sequential write\n");
    printf("Wrote: %0.2f MB\n", (b * 1e-6) * itercount);
    printf("Time : %0.2f s \n", duration);
    printf("Cpu  : %0.2f s \n", durationcpu);
    printf("Avg  : %0.2f MB/s\n", ((b * 1e-6) / duration) * itercount);
    return 0;
}
*/

/*
void * rand_write_time(void *arg) {
    arg_t *args = (arg_t *)arg;
    int f = args->f; void *buf = args->buf;

    uint64_t numblocks;
    int err = ioctl(f, BLKGETSIZE, &numblocks); // get num of blocks in the disk
    if (err) {
        fprintf(stderr, "Error getting number of blocks\n");
        return 0;       
    }

    struct timeval begin, end; double duration = 0;
    clock_t begincpu, endcpu; double durationcpu = 0;

    gettimeofday(&begin, NULL);
    begincpu = clock(); // start timer

    off64_t offset; int itercount = 0;
    while (duration < DURATION) {
        offset = (off64_t) numblocks * random() / RAND_MAX;
        err = lseek(f, MEM_ALIGN * offset, SEEK_SET);
        if (err == -1) {
            fprintf(stderr, "Error seeking offset: %m\n");
            return 0;       
        }
        err = write(f, buf, RANDWR_SZ);
        ++itercount;
        
        endcpu = clock(); // end timer
        gettimeofday(&end, NULL);

        durationcpu = (double)(endcpu - begincpu) / CLOCKS_PER_SEC; 
        duration = (end.tv_sec-begin.tv_sec) + (end.tv_usec-begin.tv_usec)/1000000.0;
    }
    /*
    printf("\nBenchmark result for random write\n");
    printf("Wrote: %d IOP\n", itercount);
    printf("Time : %0.2f s \n", duration);
    printf("Cpu  : %0.2f s \n", durationcpu);
    printf("Avg  : %0.2f IOP/s\n", itercount / duration);
    // * /
    return 0;
}
*/
