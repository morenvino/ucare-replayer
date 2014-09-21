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
#include <aio.h>

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
 * Type
 * ===========================================================================*/

// Add IoEvent into aiocb
struct AioCB /*: public aiocb*/ {
	aiocb cb; 
	TraceEvent event; // Io Event
	Timer::Timepoint beginTime; // Time right before we submit the IO
	Logger *log; // Reference to shared log 
}; // struct AioCB


/* ===========================================================================
 * Function
 * ===========================================================================*/

// Perform IO in async manner
static inline void 
performIoAsync(int fd, void *buf, TraceEvent const& io, Logger& logger);

// Callback when IO's completed
static void onIoCompleted(sigval_t sigval);

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
		static int count = 0;
		while (trace.read(event)) {
			event.time = event.time * 1000; // to microseconds
			event.size = event.bcount * BYTE_PER_BLOCK;
			event.offset = event.blkno * BYTE_PER_BLOCK;
			queue.push(event);

			if (count++ == 10) break;
		}
		readDone = true;
		queue.notifyAll();
		Timer::delay<seconds>(1); 
		queue.notifyAll(); // notify worker we're done  
	});	
	//queue.waitUntilFull(); // wait until at least queue's full

	printf("Start replaying trace\n");
	thread timerThread([&] {
		Logger logger(logDir + traceFile + "1");
		
		Timer timer; // mark the beginning 
		while (!readDone or !queue.empty()) { 
			TraceEvent event;
			if (not queue.pop(event)) continue; // retry 
			
			long currentTime = timer.elapsedTime(), nextIoTime = event.time;
			while (currentTime < nextIoTime) { // we're early
				currentTime = timer.elapsedTime(); // busy-waiting
			}

			performIoAsync(fd, buf, event, logger);
		}
	});


	fileThread.join(); // wait for all threads to finish
	timerThread.join(); 

	//printf("Late count: %d\n", lateCount.load());
	//Logger logger(logDir + traceFile + to_string(0));
	//logger.printf("%d\n", lateCount.load());
	printf("Done\n");
	return 0;
}

/* ===========================================================================
 * Function
 * ===========================================================================*/

static inline void 
performIoAsync(int fd, void *buf, TraceEvent const& io, Logger &logger) {
	auto our =  new AioCB; // keep cb alive till callback's done
	//auto cb =  (aiocb*)malloc(sizeof(Aiocb)); // keep cb alive till callback's done
	aiocb *cb = &our->cb;

	cb->aio_fildes = fd;
	cb->aio_buf = buf;
	//cb->aio_nbytes = io.size;
	cb->aio_nbytes = 512;
	//cb->aio_offset = io.offset;
	cb->aio_offset = 0;

	cb->aio_sigevent.sigev_notify = SIGEV_THREAD;
	cb->aio_sigevent.sigev_notify_function = onIoCompleted;
	cb->aio_sigevent.sigev_value.sival_ptr = cb;

	our->event = io;	
	our->log = &logger; 
	our->beginTime = Timer::now();
	
	int error = 0;
	if (io.flags == 0)
		error = aio_write(cb);
	else if (io.flags == 1)
		error = aio_read(cb);

	if (error) {
		fprintf(stderr, "Error performing i/o: %m when size:%ld offset:%ld\n", 
			cb->aio_nbytes, cb->aio_offset);
	}
	else
		printf("not error \n"); 

	fprintf(stderr, "%ld,%ld,%ld,%d\n", 
		(size_t)io.time, io.blkno, io.bcount, io.flags);

}

static void onIoCompleted(sigval_t sigval) {
	printf("onIoCompleted\n");
	auto request = (aiocb *)sigval.sival_ptr;
	auto our = (AioCB *)sigval.sival_ptr;
	long latency = Timer::elapsedTimeSince(our->beginTime); 
	
	int error = aio_error(request);
	if (error) {
		fprintf(stderr, "Error completing i/o:%d\n", error);
		delete request;
		return;
	}
	
	int count = aio_return(request);
	if (count < (int)request->aio_nbytes) { // does this happen with device? 
		fprintf(stderr, "Warning I/O completed:%d but requested:%ld\n", 
			count, request->aio_nbytes);
		// TODO: submit the remaining io
	}

	//TraceEvent& io = our->event;
	//fprintf(*our->log, "%ld,%ld,%ld,%d,%ld,%lf\n", 
	//	(size_t)io.time, io.blkno, io.bcount, io.flags, latency, (double)io.size/latency);

	delete request;
}
