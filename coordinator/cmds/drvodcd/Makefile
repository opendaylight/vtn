#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of UNC logical network daemon.
##

# A short description about this daemon.
# - Value must be a quoted string.
# - This must be defined before daemon.mk is included.
DAEMON_DESC	= "UNC Driver Daemon"

# List of module names to be loaded.
# This must be defined before daemon.mk is included.
DAEMON_MODULES	= odcdriver

# Daemon process type.
# This must be defined before daemon.mk is included.
PROCESS_TYPE	= 3

# Starting order.
# This must be defined before daemon.mk is included.
START_ORDER	= 100

# Stopping order.
# This must be defined before daemon.mk is included.
STOP_ORDER	= 10000

# Order of cluster state event delivery.
# It must be defined before daemon.mk is included.
CLEV_ORDER_ACT	= 100

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
