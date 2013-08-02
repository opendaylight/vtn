/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_CMD_CMD_IMPL_H
#define	_PFC_LIBPFC_CMD_CMD_IMPL_H

/*
 * libpfc_cmd internal utilities.
 */

#include <pfc/base.h>
#include <pfc/conf.h>

PFC_C_BEGIN_DECL

/*
 * String literals to access PFC system configuration.
 */
extern const char	conf_options[];
extern const char	conf_pid_file[];
extern const char	conf_ctrl_timeout[];

/*
 * static inline pfc_cfblk_t
 * pfccmd_conf_options(void)
 *	Return configuration block handle associated with the "options" block.
 */
static inline pfc_cfblk_t
pfccmd_conf_options(void)
{
	return pfc_sysconf_get_block(conf_options);
}

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_CMD_CMD_IMPL_H */
