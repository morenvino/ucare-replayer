/** @author morenvino@xxxxxxxx.xxx */

#ifndef UCARE_TRACE_READER_H	
#define UCARE_TRACE_READER_H	

/* ===========================================================================
 * Header
 * ===========================================================================*/

#include <cstdio>
#include <stdexcept>
#include <cstdlib>

namespace ucare {

/* ===========================================================================
 * Class
 * ===========================================================================*/

/** An I/O trace event. */
struct TraceEvent {
	double time;
	size_t bcount; // count in block
	size_t size; // count in bytes
	size_t blkno; // offset in block
	size_t offset; // offset in bytes
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
    char line[50];
    char *tok;
    int field_num;
    if (fgets(line, 50, input) == NULL)
      return 0;
    // first ignored field is disk num; second is IO type (normal, sheltered, cleanup)
    for (tok = strtok(line, " "), field_num=0; tok!=NULL; tok = strtok(NULL, " "), ++field_num) {
      switch (field_num) {
        case 0:
          event.time = strtod(tok, NULL);
          break;
        case 2:
          event.blkno = strtol(tok, NULL, 10);
          break;
        case 3:
          event.bcount = strtol(tok, NULL, 10);
          break;
        case 4:
          event.flags = strtol(tok, NULL, 10); // this is just integer
          break;
        default:
          // ignored
          break;
      }
    }
    //printf("%lf %ld %ld %d\n", event.time, event.blkno, event.bcount, event.flags);
    return 1;
	}

private:
	FILE *input; /// Trace file
}; // class TraceReader

} // namespace ucare

#endif // UCARE_TRACE_READER_H	
