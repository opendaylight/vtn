/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * conf.c - Internal utilities to access PFC system configuration.
 */

#include <pfc/conf.h>

/*
 * String literals to access PFC system configuration.
 */
const char	conf_options[] PFC_ATTR_HIDDEN = "options";
const char	conf_pid_file[] PFC_ATTR_HIDDEN = "pid_file";
const char	conf_ctrl_timeout[] PFC_ATTR_HIDDEN = "ctrl_timeout";
