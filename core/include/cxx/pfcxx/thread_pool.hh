/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFCXX_THREAD_POOL_HH
#define _PFCXX_THREAD_POOL_HH

/*
 * Definitions for C++ thread pool management that's a
 * wrapper for PFC core library for C lang
 */

#include <string>
#include <pfc/tpool.h>

namespace pfc {
namespace core {

/*
 * Class for thread pools.
 * C lang thread pool function (pfc_tpool_*) is called internally.
 *
 * It has only a static member function for now.
 */
class ThreadPool
{
  public:
    /*
     * create thread pool
     */
    static int create(const std::string &name);

    /*
     * create thread pool
     */
    static int create(const char *name_p);

    /*
     * destroy thread pool
     */
    static int destroy(const std::string &name,
                       const pfc_timespec_t *timeout = NULL);

    /*
     * destroy thread pool
     */
    static int destroy(const char *name_p,
                       const pfc_timespec_t *timeout = NULL);

  private:
    ThreadPool();
};

}       // core
}       // pfc

#endif  /* !_PFCXX_THREAD_POOL_HH */
