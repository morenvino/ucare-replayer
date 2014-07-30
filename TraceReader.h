/** @author morenvino@xxxxxxxx.xxx */

#ifndef UCARE_TRACE_READER_H	
#define UCARE_TRACE_READER_H	

/* ===========================================================================
 * Header
 * ===========================================================================*/

#include <cstdio>
#include <stdexcept>

namespace ucare {

/* ===========================================================================
 * Class
 * ===========================================================================*/

/** An I/O trace event. */
struct TraceEvent {
	double time;
	size_t bcount;
	size_t size;
	size_t blkno;
	int    flags;
}; // struct TraceEvent

/** A simple trace reader. */
class TraceReader {
public:
	/** Open trace. */
	TraceReader(const char *filename) {
		input = fopen(filename, "r");
		if (input == NULL) {
			fprintf(stderr, "error: opening file %s: %m!\n", filename);
			throw std::runtime_error("error: opening file");
		}
	}

	/** Close trace. */
	~TraceReader() { fclose(input); }

	/** Read trace. */
	inline bool read(TraceEvent& event) {
		return fscanf(input, "%lf %*d %ld %ld %d", 
			&event.time, &event.blkno, &event.bcount, &event.flags) == 4;
	}

private:
	FILE *input; /// Trace file
}; // class TraceReader

} // namespace ucare

#endif // UCARE_TRACE_READER_H	
