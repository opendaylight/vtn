/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCXX_SYNCH_HH
#define	_PFCXX_SYNCH_HH

/*
 * Definitions for thread synchronization utilities in the C++ world.
 */

namespace pfc {
namespace core {

/*
 * Mutex
 */
class Mutex
{
public:
    /*
     * Create a new mutex object.
     */
    Mutex(void)
    {
    }

    /*
     * Destructor of a mutex object.
     */
    ~Mutex(void)
    {
    }

    /*
     * Acquire the mutex lock.
     */
    inline void
    lock(void)
    {
    }

    inline int
    trylock(void)
    {
      return 1;
    }

    inline int
    timedlock(const pfc_timespec_t &timeout)
    {
      return 1;
    }

    inline void
    unlock(void)
    {
    }

protected:
    explicit Mutex(bool recursive)
    {
    }
};

class ScopedMutex
{
public:
    explicit ScopedMutex(Mutex &m)
    {
    }

    ~ScopedMutex()
    {
    }

    inline void
    unlock(void)
    {
    }

};

class ReadWriteLock
{
public:
    ReadWriteLock(void)
    {
    }

    ~ReadWriteLock(void)
    {
    }

    inline void
    rdlock(void)
    {
    }

    inline int
    tryrdlock(void)
    {
      return 1;
    }

    inline int
    timedrdlock(const pfc_timespec_t &timeout)
    {
        return 1;
    }

    inline void
    wrlock(void)
    {
    }

    inline int
    trywrlock(void)
    {
        return 1;
    }

    inline int
    timedwrlock(const pfc_timespec_t &timeout)
    {
        return 1;
    }

    inline void
    unlock(void)
    {
    }

};

class Semaphore
{
public:
    explicit Semaphore(unsigned int value)
    {
    }

    ~Semaphore(void)
    {
    }

    inline void
    post(void)
    {
    }

    inline void
    wait(void)
    {
    }

    inline int
    trywait(void)
    {
        return 1;
    }

    inline int
    timedwait(const pfc_timespec_t &timeout)
    {
        return 1;
    }

    inline int
    getvalue(void)
    {
        return 1;
    }

};

class Condition
{
 public:
  Condition(void)
  {
  }

  ~Condition(void)
  {
  }

  inline void
      signal(void)
      {
      }

  inline void
      broadcast(void)
      {
      }
  inline void
      wait(Mutex &m)
      {
      }

  inline int
      timedwait(Mutex &m, const pfc_timespec_t &timeout)
      {
        return 1;
      }

 private:
  /* PFC cond object. */
  pfc_cond_t  _cond;
};

}	// core
}	// pfc

#endif	/* !_PFCXX_SYNCH_HH */
