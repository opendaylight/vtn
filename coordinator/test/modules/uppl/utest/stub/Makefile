#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that stubs to run unit tests for UPPL module.
##



STUB_OBJS=stub_objs

createobjs: 
	mkdir -p $(STUB_OBJS)

all: createobjs buildall link_stub
OBJS = $(STUB_OBJS)/odbcm_mgr.o \
       $(STUB_OBJS)/tclib_module.o \
       $(STUB_OBJS)/odbcm_utils_stub.o \
       $(STUB_OBJS)/ipc_client.o \
       $(STUB_OBJS)/ipc_server.o \
       $(STUB_OBJS)/module.o \
       $(STUB_OBJS)/alarm.o \
       $(STUB_OBJS)/_cfdef_uppl_ctr_capability.o  \

	
buildall:

	@echo "****** Invoking tclib_stub makefile"
	@($(MAKE) -C ./tclib_module/ );

	@echo "****** Invoking ODBC makefile"
	@($(MAKE) -C ./ODBC/ )
	@echo "****** Invoking ipc_client_server_stub makefile"
	@($(MAKE) -C ./ipc_client_server/ );
	cp -rf ../../../../../dist/target/objs/modules/uppl/_cfdef_uppl_ctr_capability.o  $(STUB_OBJS)
	
clean: clearall

clearall:
	

	@(cd ./ODBC/ ;$(MAKE) clean)
	@(cd ./ipc_client_server/ ;$(MAKE) clean)
	@(cd ./tclib_module/ ;$(MAKE) clean)
	@(cd $(STUB_OBJS) ;rm -rf stub.a)
        

link_stub :$(STUB_OBJS)/stub.a

$(STUB_OBJS)/stub.a :$(OBJS)
	   $(AR) $(ARFLAGS) $@ $^




