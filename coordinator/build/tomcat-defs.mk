#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configuration for Apache Tomcat installation.
##

ifndef	UNC_BUILD_TOMCAT_DEFS_MK_INCLUDED

UNC_BUILD_TOMCAT_DEFS_MK_INCLUDED	:= 1

ifdef	JAVA_EXT_MK

# Root directory of Tomcat binary distribution.
TOMCAT_ROOT	= $(PREFIX)/tomcat

# Tomcat library directories.
TOMCAT_LIBDIR	= $(TOMCAT_ROOT)/lib
TOMCAT_SHLIBDIR	= $(TOMCAT_ROOT)/shared/lib

# Application base directory.
TOMCAT_APPDIR	= $(TOMCAT_ROOT)/webapps

# Name of JAR files which provides Tomcat APIs.
TOMCAT_JARFILES	= $(TOMCAT_SERVLET_API_JAR) $(TOMCAT_JSP_API_JAR)

# Relative path from TOMCAT_LIBDIR to INST_JARDIR.
TOMCAT_RJARDIR	= ../../lib/java/jar

endif	# JAVA_EXT_MK

endif	# !UNC_BUILD_TOMCAT_DEFS_MK_INCLUDED
