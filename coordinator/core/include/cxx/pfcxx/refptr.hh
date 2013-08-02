/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCXX_REFPTR_HH
#define	_PFCXX_REFPTR_HH

/*
 * C++ wrapper for refptr object.
 */

#include <pfc/refptr.h>
#include <pfc/debug.h>

namespace pfc {
namespace core {

/*
 * Simple C++ wrapper class for refptr object.
 * Reference counter is incremented on copy, and decremented on delete.
 */
class RefPointer
{
public:
    /*
     * Create an instance that keeps refptr object.
     * This constructor doesn't change the reference counter.
     */
    RefPointer(pfc_refptr_t *ref) : _refptr(ref)
    {
        PFC_ASSERT(ref != NULL && ref->pr_refcnt != 0);
    }

    /*
     * Copy constructor.
     * Reference counter of refptr object is incremented.
     */
    RefPointer(const RefPointer &rp) : _refptr(rp._refptr)
    {
        pfc_refptr_get(_refptr);
    }

    /*
     * Destructor.
     * Reference counter of refptr object is decremented.
     */
    virtual ~RefPointer(void)
    {
        pfc_refptr_put(_refptr);
    }

    /*
     * Return refptr object in this instance.
     */
    inline pfc_refptr_t *
    operator*() const
    {
        return _refptr;
    }

private:
    /* Refptr object. */
    pfc_refptr_t	*_refptr;
};

}	// core
}	// pfc

#endif	/* !_PFCXX_REFPTR_HH */
