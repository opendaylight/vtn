#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that include tclib modules stub to run unit tests for UPLL module.
##




CPPFLAGS = -g 

OBJS=../stub_objs

SRC_DIR = ../../../../../../modules
STUB_DIR =..
INCLUDE =   -I$(STUB_DIR)/include \
				    -I$(STUB_DIR)/include/cxx \
            -I$(STUB_DIR)/include/cxx/pfcxx \
	    			-I$(SRC_DIR)/../core/include \
 	    			-I$(SRC_DIR)/../include \
            -I$(SRC_DIR)/../core/libs/libpfc_ipc \
	    			-I$(SRC_DIR)/../dist/target/objs/core_include \
            -I$(SRC_DIR)/../dist/target/objs/core/include \
 	    			-I$(SRC_DIR)/../dist/target/objs/include \
            -I$(SRC_DIR)/upll/include \
	    			-I$(SRC_DIR)/dal \



all: $(OBJS)/tclib_module.o

clean :
	@(cd $(OBJS); rm -rf $(OBJ) tclib_module.o)

$(OBJS)/tclib_module.o :tclib_module.cc tclib_module.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

	


	      

