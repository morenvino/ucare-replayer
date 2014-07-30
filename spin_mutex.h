/** @author morenvino@xxxxxxxx.xxx */

#ifndef UCARE_SPIN_MUTEX_H
#define UCARE_SPIN_MUTEX_H

/* ===========================================================================
 * Header
 * ===========================================================================*/
#include <atomic>
#include <mutex>

namespace ucare {

/* ===========================================================================
 * Class
 * ===========================================================================*/

/** A simple spinning mutex for spinlock. */
class spin_mutex /** implements BasicLockable */ { 
public:
	/** Create spinning mutex. */
	spin_mutex() : lk(false) {};

	/** Lock mutex. */
	void lock() { while (lk.exchange(true)); } 

	/** Unlock mutex. */
	void unlock() { lk.store(false); }

	/** @return true if mutex is locked. */
	bool is_locked() { return lk.load(); }

private:
	std::atomic<bool> lk; // note: slow with gcc < 4.7 according to http://stackoverflow.com/questions/13135834/stdatomicbool-is-very-slow
}; // class spin_mutex

} // namespace ucare

#endif //UCARE_SPIN_MUTEX_H
