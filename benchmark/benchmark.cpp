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

/* ===========================================================================
 * Functions
 * ===========================================================================*/

static inline int log_nothing(const char *, ...) { return 0; }

/* ===========================================================================
 * Global variables
 * ===========================================================================*/


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
    if (argc > 1) nthreads_rnd = atoi(argv[1]); // prioritize argv over config

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
    auto log = printf; // log using printf by default 
    if (not interactive) log = log_nothing;

    // print configuration
    log("blocksize_seq : %d\n", blocksize_seq);
    log("duration_seq  : %d\n", duration_seq);
    log("nthreads_seq  : %d\n", nthreads_seq);
    log("blocksize_rnd : %d\n", blocksize_rnd);
    log("duration_rnd  : %d\n", duration_rnd);
    log("nthreads_rnd  : %d\n", nthreads_rnd);
    log("start_delay   : %d\n", start_delay);
    log("interactive   : %d\n", interactive);
    log("dev           : %s\n", dev);

    // prepare for direct i/o
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
        fprintf(stderr, "Error opening device '%s'\n", dev);
        return 1;   
    }

    log("Test writing %d bytes to %s\n", b, dev);
    err = write(f, buf, b);
    if (err == -1) {
        fprintf(stderr, "Error writing to device %s\n", dev);
        return 1;
    } 

    // start benchmark
    log("Starting benchmark for random write\n");
    vector<thread> threads_rnd(nthreads_rnd);
    for (auto& t : threads_rnd) t = thread([&] { // launch random write threads
        uint64_t numblocks;
        int err = ioctl(f, BLKGETSIZE, &numblocks); // get num of blocks in the disk
        if (err) {
            fprintf(stderr, "Error getting number of blocks\n");
            return;       
        }

        struct timeval begin, end; double duration = 0;
        clock_t begincpu, endcpu; double durationcpu = 0;

        gettimeofday(&begin, NULL);
        begincpu = clock(); // start timer

        off64_t offset; int itercount = 0;
        while (duration < duration_rnd) {
            offset = (off64_t) numblocks * random() / RAND_MAX;
            err = lseek(f, MEM_ALIGN * offset, SEEK_SET);
            if (err == -1) {
                fprintf(stderr, "Error seeking offset: %m\n");
                return;       
            }
            err = write(f, buf, blocksize_rnd);
            ++itercount;
            
            endcpu = clock(); // end timer
            gettimeofday(&end, NULL);

            durationcpu = (double)(endcpu - begincpu) / CLOCKS_PER_SEC; 
            duration = (end.tv_sec-begin.tv_sec) + (end.tv_usec-begin.tv_usec)/1000000.0;
        }
        log("\nBenchmark result for random write\n");
        log("Wrote: %d IOP\n", itercount);
        log("Time : %0.2f s \n", duration);
        log("Cpu  : %0.2f s \n", durationcpu);
        log("Avg  : %0.2f IOP/s\n", itercount / duration);
    });
    
    sleep(start_delay); // give time for random write to start 
    
    log("Starting benchmark for sequential write\n");
    vector<thread> threads_seq(nthreads_seq);
    for (auto& t : threads_seq) t = thread([&] { // launch seq write threads
        struct timeval begin, end; double duration = 0;
        clock_t begincpu, endcpu; double durationcpu = 0;

        gettimeofday(&begin, NULL);
        begincpu = clock(); // start timer
        
        int err; int itercount = 0;
        while (duration < duration_rnd) {
            err = write(f, buf, b);
            if (err == -1) {
                fprintf(stderr, "Error writing to device\n");
                return;
            }
            ++itercount;

            endcpu = clock(); // end timer
            gettimeofday(&end, NULL);

            durationcpu = (double)(endcpu - begincpu) / CLOCKS_PER_SEC; 
            duration = (end.tv_sec-begin.tv_sec) + (end.tv_usec-begin.tv_usec)*1e-6;    
        }
        
        log("\nBenchmark result for sequential write\n");
        log("Wrote: %0.2f MB\n", (b * 1e-6) * itercount);
        log("Time : %0.2f s \n", duration);
        log("Cpu  : %0.2f s \n", durationcpu);
        log("Avg  : %0.2f MB/s\n", ((b * 1e-6) / duration) * itercount);
        if (not interactive) printf("%0.2f\n", ((b * 1e-6) / duration) * itercount);
    });
    
    for (auto& t : threads_seq) t.join(); // wait for all threads to finish
    for (auto& t : threads_rnd) t.join(); // wait for all threads to finish

    // clean up    
    log("Done\n");
    close(f);
    return 0;
}

/* ===========================================================================
 * Functions
 * ===========================================================================*/
