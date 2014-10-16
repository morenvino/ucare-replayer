/** 
 * @author morenvino@xxxxxxxx.xxx
 * @file   SNIA trace replayer adapted from tiratatp@xxxxxxxx.xxx
 */

/* ===========================================================================
 * Headers
 * ===========================================================================*/

//#define _GNU_SOURCE // for C compiler

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>

#include <cstdio> 
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#include "Config.h"
#include "TraceReader.h"
#include "ConcurrentQueue.h"
#include "Timer.h"
#include "Logger.h"

using namespace std;
using namespace chrono;
using namespace ucare;

/* ===========================================================================
 * Const
 * ===========================================================================*/

#define MEM_ALIGN             512  // Memory alignment
#define BYTE_PER_BLOCK        512  // Number of bytes per I/O block
#define LARGEST_REQUEST_SIZE 8192  // Largest request size in blocks

#define DEFAULT_NTHREADS      256
#define DEFAULT_DEVICE       "/dev/sdb"
#define DEFAULT_TRACE_FILE   "in.trace"
#define DEFAULT_LOG_DIR      "./"

/* ===========================================================================
 * Function
 * ===========================================================================*/

static inline void 	
performIo(int fd, void *buf, TraceEvent const& io, Logger &logger);

/* ===========================================================================
 * Main
 * ===========================================================================*/

int main(int argc, char *argv[]) {
	// read config.ini file
	Config config;
	auto nthreads  = config.get<int>("nthreads");
	auto device    = config.get<const char *>("disk_guest");
	auto traceFile = config.get<const char *>("trace_file");
	auto logDir    = config.get<string>("log_dir");
	
	// parse arg, prioritize argv over config
	if (argc > 1) traceFile = argv[1]; 
	if (argc > 2) nthreads = atoi(argv[2]);
	if (strstr(device, "/dev/sda")) { // avoid accidentally writing to system part 
		fprintf(stderr, "Error trying to write to system partition %s\n", device);
		return 1;
	}
	// use default value if not supplied
	if (strcmp(device, "") == 0) device = DEFAULT_DEVICE;
	if (strcmp(traceFile, "") == 0) traceFile = DEFAULT_TRACE_FILE;
	if (strcmp(logDir.c_str(), "") == 0) logDir = DEFAULT_LOG_DIR;
	if (nthreads == 0) nthreads = DEFAULT_NTHREADS;
	
	srand(time(NULL)); 	// initialize seed

	// print configuration
	printf("trace     : %s\n", traceFile);
	printf("nthreads  : %d\n", nthreads);
	printf("device    : %s\n", device);
	printf("log       : %s\n", logDir.c_str());
	printf("precision : %fms\n", Timer::getResolution());

	printf("Opening device %s\n", device);
	int fd = open(device, O_DIRECT | O_RDWR | O_SYNC); 
	if (fd < 0) {
		fprintf(stderr, "Error opening device '%s'\n", device);
		return 1;
	}

	printf("Allocating buffer\n");
	void *buf; 
	if (posix_memalign(&buf, MEM_ALIGN, LARGEST_REQUEST_SIZE * BYTE_PER_BLOCK)) {
		fprintf(stderr, "Error allocating buffer\n");
		return 1;
	}
	//memset(buf, rand() % 256, LARGEST_REQUEST_SIZE * BYTE_PER_BLOCK);

	printf("Opening trace file\n");
	TraceReader trace(traceFile); // open trace file
	ConcurrentQueue<TraceEvent> queue; // queue of trace events
	bool readDone = false; // whether or not we're done reading trace file
	
	printf("Start reading trace\n");
	thread fileThread([&] { // thread to read trace file 
		TraceEvent event;
		while (trace.read(event)) {
			event.time = event.time * 1000; // to microseconds
			event.size = event.bcount * BYTE_PER_BLOCK;
			queue.push(event);
		}
		readDone = true; 
		queue.notifyAll();
		Timer::delay<seconds>(1);  
		queue.notifyAll(); // notify worker we're done  
	});	
	//queue.waitUntilFull(); // wait until at least queue's full
	sleep(3);


	printf("Start replaying trace\n");
	Timer timeBegin;
	vector<thread> workers(nthreads); // generate worker threads
	//atomic<int> lateCount(0), threadId(0); // late I/O count and threadId
	atomic<int> threadId(0); // late I/O count and threadId
	for (auto& t : workers) t = thread([&] { // launch workers 
		int myId = ++threadId; // id for this thread
		//int myLateCount = 0; // local lateCount for this thread 
		Logger logger(logDir + traceFile + to_string(myId));
		
		Timer timer; // mark the beginning of worker thread	
		while (!readDone or !queue.empty()) { 
			TraceEvent event;
			if (not queue.pop(event)) continue; // retry 
			
			performIo(fd, buf, event, logger);
		}
		//lateCount += myLateCount; // update global lateCount
	});

	fileThread.join(); // wait for all threads to finish
	for (auto& t : workers) t.join(); 

	//printf("Late count: %d\n", lateCount.load());
	//Logger logger(logDir + traceFile + to_string(0));
	//logger.printf("%d\n", lateCount.load());
	printf("All IO completed after: %ld s\n", timeBegin.elapsedTime<seconds>());

	printf("Done\n");
	return 0;
}

/* ===========================================================================
 * Function
 * ===========================================================================*/

static inline void 
performIo(int fd, void *buf, TraceEvent const& io, Logger &logger) {
	off64_t start_offset = (off64_t)io.blkno * BYTE_PER_BLOCK; 
	off64_t current_offset = start_offset;
	//lseek64(fd, start_offset, SEEK_SET);

	int ret = 0; size_t total_size = io.size;
	Timer timer;
	while (total_size > 0) {
		if (io.flags == 0)
			//ret = write(fd, buf, total_size);
			ret = pwrite(fd, buf, total_size, current_offset);
		else if (io.flags == 1)
			//ret = read(fd, buf, total_size);
			ret = pread(fd, buf, total_size, current_offset);
		
		if (ret >= 0) {
			total_size -= ret;
			current_offset += ret;
		} else {
			fprintf(stderr, "Error performing i/o: %m\n");
		}
	}
	long latency = timer.elapsedTime();
	logger.printf("%ld,%ld,%ld,%d,%ld,%lf\n", 
		(size_t)io.time, io.blkno, io.bcount, io.flags, latency, (double)io.size/latency);
}
