#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that installs Apache Tomcat binary distribution.
##

include ../../build/config.mk

ifdef	JAVA_EXT_MK

SUBDIRS		= bootstrap conf ext launcher lib unc-addon

include $(CORE_BLDDIR)/subdirs.mk

else	# !JAVA_EXT_MK

all install clean clobber:

.PHONY:	all install clean clobber

endif	# JAVA_EXT_MK
