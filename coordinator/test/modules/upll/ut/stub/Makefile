#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that includes stubs to run UPLL test cases.
##

STUB_OBJS=stub_objs

all: createobjs buildall link_stub

createobjs: 
	mkdir -p $(STUB_OBJS)

OBJS = $(STUB_OBJS)/capa_module_stub.o \
       $(STUB_OBJS)/tclib_module.o \
       $(STUB_OBJS)/dal_bind_info.o \
       $(STUB_OBJS)/dal_odbc_mgr.o \
       $(STUB_OBJS)/dal_schema.o \
       $(STUB_OBJS)/ipc_client.o \
       $(STUB_OBJS)/ipc_server.o \
       $(STUB_OBJS)/module.o \
       $(STUB_OBJS)/pfc_alarm_api.o \
	
buildall:

	@echo "****** Invoking tclib_stub makefile"
	@($(MAKE) -C ./tclib_module/ );

	@echo "****** Invoking dal_stub makefile"
	 @($(MAKE) -C ./dal/ )
	@echo "****** Invoking Cap_Stub makefile"
	 @($(MAKE) -C ./capa_module/ );
	
	@echo "****** Invoking ipc_client_server_stub makefile"
	 @($(MAKE) -C ./ipc_client_server/ );
	
clean: clearall

clearall:
	         @(cd ./capa_module/ ;$(MAKE) clean)
		 @(cd ./ipc_client_server/ ;$(MAKE) clean)
		 @(cd ./tclib_module/ ;$(MAKE) clean)
		 @(cd ./dal/ ;$(MAKE) clean)
		 @(cd $(STUB_OBJS) ;rm -rf stub.a)

link_stub :$(STUB_OBJS)/stub.a

$(STUB_OBJS)/stub.a :$(OBJS)
	   $(AR) $(ARFLAGS) $@ $^
