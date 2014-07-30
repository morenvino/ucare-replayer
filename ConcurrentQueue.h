/** @author morenvino@uchicago.edu */

#ifndef UCARE_CONCURRENT_QUEUE_H
#define UCARE_CONCURRENT_QUEUE_H

/* ===========================================================================
 * Header
 * ===========================================================================*/
#include <deque>
#include <mutex>
#include <condition_variable>
#include "spin_mutex.h"

namespace ucare {

/* ===========================================================================
 * Class
 * ===========================================================================*/

/** A simple concurrent queue. */
template<typename T>
class ConcurrentQueue {

	// Choose mutex type
#ifdef USE_SPIN_LOCK // use spin lock instead
	using mutex_t = ucare::spin_mutex;
#else // MUTEX_LOCK
	using mutex_t = std::mutex;
#endif

	/** Default max queue size */
	constexpr static const size_t DEFAULT_MAX_SIZE = 1000;

public:
	/** Create queue with maximum size n. */
	ConcurrentQueue(size_t n = DEFAULT_MAX_SIZE) : max(n) { 
		queue.get_allocator().allocate(max); 
	}

	/** Try push item into queue. Will wait if queue is full. */
	void push(T const& item) {
		std::unique_lock<mutex_t> lock(mutex);
		while (full()) notFull.wait(lock);
		queue.push_back(item);
		notEmpty.notify_one();
	}

	/** Try pop item from queue. Will try to wait if queue is empty. */
	bool pop(T& item) {
		std::unique_lock<mutex_t> lock(mutex);
		if (empty()) notEmpty.wait(lock);
		if (/* still */ empty()) return false;

		item = queue.front(); queue.pop_front();
		notFull.notify_one();
		return true;
	}

	/** @return true if queue is empty. */
	inline bool empty() const { return queue.empty(); }

	/** @return true if queue is full. */
	inline bool full() const { return queue.size() >= max; }

	/** Wait until queue is full. */
	void waitUntilFull() {
		std::unique_lock<mutex_t> lock(mutex);
		while (!full()) notEmpty.wait(lock);
	}

//private: 
	const size_t max;
	std::deque<T> queue;
	mutex_t mutex;
	std::condition_variable notFull, notEmpty;
}; // class ConcurrentQueue 

} // namespace ucare

#endif //UCARE_CONCURRENT_QUEUE_H
