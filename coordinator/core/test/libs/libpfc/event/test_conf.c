/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "test_conf.h"

int
test_sysconf_init(const char *conffile)
{
	pfc_refptr_t *rconf;
	int err;

	if (pfc_sysconf_open() != NULL)
		return 0;

	rconf = pfc_refptr_string_create(conffile);
	err = pfc_sysconf_init(rconf);

	pfc_refptr_put(rconf);

	return err;
}
