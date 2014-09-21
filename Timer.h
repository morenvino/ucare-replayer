/** @author morenvino@xxxxxxxx.xxx */

#ifndef UCARE_TIMER_H
#define UCARE_TIMER_H

/* ===========================================================================
 * Header
 * ===========================================================================*/
#include <chrono>
#include <ratio>
#include <thread>

namespace ucare { 

/* ===========================================================================
 * Class
 * ===========================================================================*/

/** Simple high-precision timer utility. */
class Timer {
public:
	using Clock = std::chrono::high_resolution_clock; // highest possible resolution
	using Timepoint = std::chrono::time_point<Clock>;

/* ===========================================================================
 * Static method
 * ===========================================================================*/

	/** @return max clock resolution in ms for the given clock in this machine. */
	static inline double getResolution() {
		using namespace std;
		typedef typename Clock::period P; // time unit
		typedef typename ratio_multiply<P,kilo>::type T;
		return double(T::num)/T::den;
	}

	/** Delay by the specific duration*/
	template <typename Duration = std::chrono::microseconds>
	static inline void delay(size_t duration) {
		using namespace std;
		using namespace chrono;

	#ifdef USE_BUSY_WAITING
		auto startTime = Clock::now();
		while (duration_cast<Duration>(Clock::now()-startTime).count() < duration)
			; // do nothing: just wait
	#else // SLEEP
		this_thread::sleep_for(Duration(duration)); 
	#endif
	}

	/** @return the current point in time. */
	static inline Timepoint now() { return Clock::now(); }

	/** @return the elapsed duration since the given time. */
	template <typename Duration = std::chrono::microseconds>
	static inline long elapsedTimeSince(Timepoint begin) {
		return std::chrono::duration_cast<Duration>(now() - begin).count();
	}

/* ===========================================================================
 * Method
 * ===========================================================================*/

	/** Create a timer. */
	Timer() : begin(now()) {}

	/** @return the elapsed time from beginning. */
	template <typename Duration = std::chrono::microseconds>
	inline long elapsedTime() {
		return std::chrono::duration_cast<Duration>(now() - begin).count();
	}

private:
	Timepoint begin; // mark the time when Timer is created
}; // class Timer

} // namespace ucare

#endif //UCARE_TIMER_H
