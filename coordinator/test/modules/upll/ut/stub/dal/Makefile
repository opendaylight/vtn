#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that includes dal module as stub to run  unit tests for UPLL module.
##


CPPFLAGS = -g

OBJS=../stub_objs

SRC_DIR = ../../../../../../modules

INCLUDE =   -I$ ../stub/include \
	    -I$ ../stub/include/cxx \
            -I$ ../stub/include/cxx/pfcxx \
            -I$ ../stub/dal/include \
	    -I$ ../stub/dal \
	    -I$(SRC_DIR)/ \
            -I$(SRC_DIR)/../core/libs/libpfc_ipc \
	    -I$(SRC_DIR)/../dist/target/objs/core_include \
            -I$(SRC_DIR)/../dist/target/objs/core/include \
 	    -I$(SRC_DIR)/../dist/target/objs/include \
	    -I$(SRC_DIR)/../core/include \
	    -I$(SRC_DIR)/../include/ \
            -I$(SRC_DIR)/upll/include \
	    -I$(SRC_DIR)/modules \
	    -I$(SRC_DIR)/dal \


all: compile

Dependent_File = dal_odbc_mgr.cc dal_bind_info.cc dal_schema.cc
OBJ1 = $(patsubst %.cc,$(OBJS)/%.o,$(Dependent_File))

clean :
	@(cd $(OBJS); rm -rf *.o)

$(OBJ1):$(OBJS)/%.o : %.cc dal_odbc_mgr.hh dal_bind_info.hh dal_cursor.hh dal_schema.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

compile :$(OBJ1)
	      

