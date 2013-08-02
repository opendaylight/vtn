/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * thread_pool.cc - thread pool management for C++
 */

#include <exception>
#include <pfc/log.h>
#include <pfcxx/thread_pool.hh>

namespace pfc {
namespace core {

/*
 * int
 * ThreadPool::create(const std::string &name)
 *      Create a new thread pool.
 *      It's declared static in header.
 *
 *      Return values:
 *              Zero is returned on success.
 *              Otherwise, error number which indicates the cause of
 *              error is returned.
 */
/* static */
int
ThreadPool::create(const std::string &name)
{
    return ThreadPool::create(name.c_str());
}

/*
 * int
 * ThreadPool::create(const char *name_p)
 *      Create a new thread pool.
 *      It's declared static in header.
 *
 *      Return values:
 *              Zero is returned on success.
 *              Otherwise, error number which indicates the cause of
 *              error is returned.
 */
/* static */
int
ThreadPool::create(const char *name_p)
{
    return pfc_tpool_create(name_p);
}

/*
 * int
 * ThreadPool::destroy(const std::string &name,
 *                     const pfc_timespec_t *timeout)
 *      Destroy the thread pool specified by the name.
 *      It's declared static in header.
 *      See C function (`pfc_tpool_destroy') about blocking and timeout.
 *
 *      Return values:
 *	            Upon successful completion, zero is returned.
 *	            Otherwise error number which indicates the cause of error is
 *	            returned.
 */
/* static */
int
ThreadPool::destroy(const std::string &name,
                    const pfc_timespec_t *timeout)
{
    return pfc_tpool_destroy(name.c_str(), timeout);
}

/*
 * int
 * ThreadPool::destroy(const char *name_p,
 *                     const pfc_timespec_t *timeout)
 *      Destroy the thread pool specified by the name.
 *      It's declared static in header.
 *      See C function (`pfc_tpool_destroy') about blocking and timeout.
 *
 *      Return values:
 *	            Upon successful completion, zero is returned.
 *	            Otherwise error number which indicates the cause of error is
 *	            returned.
 */
/* static */
int
ThreadPool::destroy(const char *name_p,
                    const pfc_timespec_t *timeout)
{
    return pfc_tpool_destroy(name_p, timeout);
}

}       // core
}       // pfc
