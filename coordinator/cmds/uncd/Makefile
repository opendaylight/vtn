#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of UNC daemon.
##

# A short description about this daemon.
# - Value must be a quoted string.
# - This must be defined before daemon.mk is included.
DAEMON_DESC	= "UNC daemon"

# List of module names to be loaded.
# This must be defined before daemon.mk is included.
DAEMON_MODULES	= launcher tc usess

# Use UNC daemon specific configuration.
DAEMON_CONF_IN	= uncd.conf.in

include ../daemon.mk

##
## Targets:
##
## make all (default target)
##	Build all binary distributions.
##
## make install
##	Execute "make all" and install binary distributions into the
##	installation directory.
##
## make clean
##	Remove binary distributions built by "make all".
##
## make clobber
##	Execute "make clean" and remove header dependency files.
##
