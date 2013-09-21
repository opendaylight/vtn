#
#Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that includes ODBC stub drives the production of unit tests for UPPL module.
##



CPPFLAGS = -g

OBJS=../stub_objs

SRC_DIR = ../../../../../../modules

INCLUDE =   -I$ ../stub/ODBC/include \
            -I$ ../stub/ODBC \
            -I$(SRC_DIR)/ \
            -I$(SRC_DIR)/../core/libs/libpfc_ipc \
            -I$(SRC_DIR)/../dist/target/objs/core_include \
            -I$(SRC_DIR)/../dist/target/objs/core/include \
            -I$(SRC_DIR)/../dist/target/objs/include \
            -I$(SRC_DIR)/../core/include \
            -I$(SRC_DIR)/../include/ \
            -I$(SRC_DIR)/modules \

all: compile

Dependent_File = odbcm_mgr.cc odbcm_utils_stub.cc
OBJ1 = $(patsubst %.cc,$(OBJS)/%.o,$(Dependent_File))

clean :
	@(cd $(OBJS); rm -rf *.o)

$(OBJ1):$(OBJS)/%.o : %.cc odbcm_common.hh odbcm_db_tableschema.hh odbcm_utils_stub.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

compile :$(OBJ1)
