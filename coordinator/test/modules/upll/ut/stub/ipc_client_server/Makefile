#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that includes ipc stubs to run  unit tests for UPLL module.
##

CPPFLAGS = -g

OBJS=../stub_objs

SRC_DIR = ../../../../../../modules
STUB_DIR =..
INCLUDE =   -I$(STUB_DIR)/include \
				    -I$(STUB_DIR)/include/cxx \
            -I$(STUB_DIR)/include/cxx/pfcxx \
            -I$(STUB_DIR)/dal/include \
	    			-I$(SRC_DIR)/upll \
            -I$(SRC_DIR)/../core/libs/libpfc_ipc \
	    			-I$(SRC_DIR)/../dist/target/objs/core_include \
            -I$(SRC_DIR)/../dist/target/objs/core/include \
 	    			-I$(SRC_DIR)/../dist/target/objs/include \
	    			-I$(SRC_DIR)/../core/include \
	    			-I$(SRC_DIR)/../include/ \
            -I$(SRC_DIR)/upll/include \
	    			-I$(SRC_DIR)/dal \

all: compile

clean :
	@(cd $(OBJS); rm -rf $(OBJS)/ipc_client.o ipc_server.o)

DEPENDENT_FILES = ipc_server.cc ipc_client.cc module.cc pfc_alarm_api.cc
OBJ1 = $(patsubst %.cc,$(OBJS)/%.o,$(DEPENDENT_FILES))

$(OBJ1):$(OBJS)/%.o : ./%.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

compile : $(OBJ1)

	      

