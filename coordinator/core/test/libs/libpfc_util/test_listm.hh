/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_LISTM_HH
#define _TEST_LISTM_HH

/*
 * Common definitions for listmodel tests
 */

#include "test.h"


#ifndef PARALLEL_TEST
#define LISTM_RANDOM_SEED     ((uint32_t)1)
#else  /* !PARALLEL_TEST */
#define LISTM_RANDOM_SEED     ((uint32_t)loop_count_of_listmodel_test)
#endif /* !PARALLEL_TEST */

#endif  /* !_TEST_LISTM_BASIC_HH */
