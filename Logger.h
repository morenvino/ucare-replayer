/** @author morenvino@uchicago.edu */

#ifndef UCARE_LOGGER_H	
#define UCARE_LOGGER_H	

/* ===========================================================================
 * Header
 * ===========================================================================*/

#include <string>
#include <stdexcept>
#include <cstdio>
#include <cstdarg>

namespace ucare {

/* ===========================================================================
 * Class
 * ===========================================================================*/

/** A simple logger. */
class Logger {
public:
	/** Open log. */
	Logger(std::string filename) {
		file = fopen(filename.c_str(), "w");
		if (file == NULL) {
			fprintf(stderr, "error: opening file %s: %m!\n", filename.c_str());
			throw std::runtime_error("error: opening file");
		}
	}

	/** Close log. */
	~Logger() { fclose(file); }

	/** Output log. */
	inline void printf(const char * format, ...) {
		va_list args;
		va_start(args, format);
		vfprintf(file, format, args);
		//vprintf(format, args);        
		//fflush(file);
		va_end(args);	 
	}

private:
	FILE *file; /// Log file
}; // class Logger

} // namespace ucare

#endif // UCARE_LOGGER_H	
