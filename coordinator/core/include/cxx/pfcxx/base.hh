/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCXX_BASE_HH
#define	_PFCXX_BASE_HH

#include <exception>

/*
 * Common definitions for code written in C++ language.
 * This header file is designed to be included from pfc/base.h.
 */

/*
 * PFCXX_CATCH_ALL()
 *	Macro to catch any throwable.
 *	Code to catch unexpected throwable can be written in subsequent block
 *	statement. Note that this macro must be placed at the end of try-catch
 *	block sequence,
 *
 * Usage:
 *	try {
 *	    ...
 *	}
 *	catch (const std::exception &ex) {
 *	    Code to catch standard exception.
 *	}
 *	PFCXX_CATCH_ALL() {
 *	    Code to catch unexpected throwable.
 *	}
 */
#ifdef	PFC_USE_GLIBC

/*
 * POSIX thread in GNU libc uses exception to cancel or terminate a thread,
 * and it must NOT be caught in application code.
 */
#ifdef	PFCXX_HAVE_FORCE_UNWIND

/*
 * abi::__forced_unwind is used to unwind thread's stack. We must rethrow it.
 */
#include <cxxabi.h>

#define	PFCXX_CATCH_ALL()                       \
    catch (const abi::__forced_unwind &) {      \
        throw;                                  \
    }                                           \
    catch (...)

#else	/* !PFCXX_HAVE_FORCE_UNWIND */

/*
 * There is no simple way to detect glibc's internal exception.
 * We give up.
 */
#define	PFCXX_CATCH_ALL()	if (0)

#endif	/* PFCXX_HAVE_FORCE_UNWIND */

#else	/* !PFC_USE_GLIBC */
#define	PFCXX_CATCH_ALL()	catch (...)
#endif	/* PFC_USE_GLIBC */

/*
 * PFCXX_TRY_ON_RELEASE()
 * PFCXX_CATCH_ON_RELEASE(ex)
 * PFCXX_CATCH_ALL_ON_RELEASE()
 *	Execute statement with catching any exception on release build to
 *	improve robustness.
 *
 * Usage:
 *	PFCXX_TRY_ON_RELEASE() {
 *		statement;
 *	}
 *	PFCXX_CATCH_ON_RELEASE(ex) {
 *		exception-catch-statement;
 *	}
 *	PFCXX_CATCH_ALL_ON_RELEASE() {
 *		catch-all-statement;
 *	}
 *
 *	On release build, `statement' is executed with catching exception.
 *	If `statement' throws an std::exception or sub class of std::exception,
 *	`exception-catch-statement' will be executed. `ex' is a variable name
 *	which keeps reference to std::exception, and it can be used in
 *	`exception-catch-statement'.
 *	`catch-all-statement' will be executed if `statement' throws an
 *	exception which is not a sub class of std::exception.
 *
 *	On debug build, `statement' is executed without catching exception.
 *	So `exception-catch-statement' and `catch-all-statement' will be
 *	invalidated by the C++ compiler.
 */
#ifdef	PFC_VERBOSE_DEBUG

/*
 * Execute `statement' without catching any exception.
 * So program will be aborted if `statement' throws an exception.
 *
 * Remarks:
 *	PFCXX_CATCH_ON_RELEASE(ex) needs to declare `ex' as variable in order
 *	to avoid compilation error.
 */
#define	PFCXX_TRY_ON_RELEASE()		if (true)
#define	PFCXX_CATCH_ON_RELEASE(ex)	for (const std::exception ex; false;)
#define	PFCXX_CATCH_ALL_ON_RELEASE()	if (false)

#else	/* !PFC_VERBOSE_DEBUG */

/* Catch any exception to improve robustness. */
#define	PFCXX_TRY_ON_RELEASE()		try
#define	PFCXX_CATCH_ON_RELEASE(ex)	catch (const std::exception &ex)
#define	PFCXX_CATCH_ALL_ON_RELEASE()	PFCXX_CATCH_ALL()

#endif	/* PFC_VERBOSE_DEBUG */

#endif	/* !_PFCXX_BASE_HH */
