/** @author morenvino@uchicago.edu */

#ifndef MV_LOG_H
#define MV_LOG_H

/* ===========================================================================
 * Headers
 * ===========================================================================*/

#include <iostream>

namespace mv {

/* ===========================================================================
 * Functions
 * ===========================================================================*/

/* ===========================================================================
 * Classes
 * ===========================================================================*/

/** A null output stream. */
class NullStream : public std::ostream {
private:
	/** A null stream buffer. */
	class NullBuffer : public std::streambuf {
		public: int overflow(int c) { return c; }
	} nullBuffer;

public:
	/** Ctor. */
	NullStream() : std::ostream(&nullBuffer) {}

}; // class NullStream

} // namespace mv

#endif // MV_LOG_H
