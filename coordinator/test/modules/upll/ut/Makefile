#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for UPLL.
##


GTEST_SRCROOT = ../../../../modules

CPPFLAGS  +=  

INCLUDE = -Iinclude \
					-Istub/include \
					-Istub/include/cxx \
					-Istub/include/cxx/pfcxx \
					-Istub/include/core_include \
					-Istub \
					-Istub/tclib_module \
					-Istub/capa_module \
					-I$(GTEST_SRCROOT)/capa/include \
					-Istub/dal \
					-Istub/dal/include \
					-I$(GTEST_SRCROOT)/upll \
					-I$(GTEST_SRCROOT)/dal/ \
					-I$(GTEST_SRCROOT) \
					-I$(GTEST_SRCROOT)/../dist/target/objs/include \
					-I$(GTEST_SRCROOT)/../dist/target/objs/core_include \
					-I$(GTEST_SRCROOT)/../dist/target/objs/core/include \
					-I$(GTEST_SRCROOT)/../core/include \
					-I$(GTEST_SRCROOT)/../core/include/cxx \
					-I$(GTEST_SRCROOT)/../include/ \
					-I$(GTEST_SRCROOT)/../include/pfcapi/pfcapi_generic/ \
					-I$(GTEST_SRCROOT)/../include/unc \
					-I$(GTEST_SRCROOT)/upll/include \
					-I$(GTEST_SRCROOT) \
					-I$(GTEST_SRCROOT)/../test/modules/upll/ut \
					-I$(GTEST_SRCROOT)/alarm/include\
        

CXXFLAGS += -g -fprofile-arcs -ftest-coverage -Dprivate=public	-Dprotected=public

SOURCE_FILES = $(GTEST_SRCROOT)/upll/ipc_util.cc \
							 $(GTEST_SRCROOT)/upll/ipct_st.cc  \
							 $(GTEST_SRCROOT)/upll/config_mgr.cc \
							 $(GTEST_SRCROOT)/upll/ctrlr_mgr.cc \
							 $(GTEST_SRCROOT)/upll/key_tree.cc \
							 $(GTEST_SRCROOT)/upll/tclib_intf_impl.cc \
							 $(GTEST_SRCROOT)/upll/momgr_intf.cc \
							 $(GTEST_SRCROOT)/upll/momgr_impl.cc \
							 $(GTEST_SRCROOT)/upll/tx_mgr.cc \
							 $(GTEST_SRCROOT)/upll/config_lock.cc \
							 $(GTEST_SRCROOT)/upll/kt_util.cc \
							 $(GTEST_SRCROOT)/upll/vtn_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vbr_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vbr_if_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vlanmap_momgr.cc \
							 $(GTEST_SRCROOT)/upll/nwm_momgr.cc \
							 $(GTEST_SRCROOT)/upll/nwm_host_momgr.cc \
							 $(GTEST_SRCROOT)/upll/iproute_momgr.cc \
							 $(GTEST_SRCROOT)/upll/dhcprelay_if_momgr.cc \
							 $(GTEST_SRCROOT)/upll/dhcprelay_server_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vrt_if_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vunk_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vunk_if_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vnode_child_momgr.cc \
							 $(GTEST_SRCROOT)/upll/momgr_util.cc \
							 $(GTEST_SRCROOT)/upll/vnode_momgr.cc \
							 $(GTEST_SRCROOT)/upll/read_bulk.cc \
							 $(GTEST_SRCROOT)/upll/vlink_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vrt_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtn_flowfilter_entry_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtn_flowfilter_momgr.cc \
							 $(GTEST_SRCROOT)/upll/flowlist_entry_momgr.cc \
							 $(GTEST_SRCROOT)/upll/flowlist_momgr.cc \
							 $(GTEST_SRCROOT)/upll/policingprofile_entry_momgr.cc \
							 $(GTEST_SRCROOT)/upll/policingprofile_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vbr_flowfilter_entry_momgr.cc\
							 $(GTEST_SRCROOT)/upll/vbr_flowfilter_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vbr_if_flowfilter_entry_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vbr_if_flowfilter_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vbr_if_policingmap_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vbr_policingmap_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vrt_if_flowfilter_entry_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vrt_if_flowfilter_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtn_policingmap_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtep_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtep_if_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtep_grp_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtepgrp_mem_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtunnel_momgr.cc \
							 $(GTEST_SRCROOT)/upll/vtunnel_if_momgr.cc \


SRC_OBJS = $(patsubst $(GTEST_SRCROOT)/upll/%.cc,$(OBJSDIR)/upll/%.o,$(SOURCE_FILES))

UNITTEST_FILES = vbr_if_momgr_ut.cc vtn_momgr_ut.cc vbr_momgr_ut.cc 

OBJSDIR = objs

UT_OBJS_DIR = objs/ut
UPLL_OBJS_DIR = objs/upll

EXEC_NAME =  vbr_if_momgr_ut vbr_momgr_ut vtn_momgr_ut 

UT_LIBS = -L$(GTEST_SRCROOT)/../dist/target/objs/ldlibs -lpthread -lpfc_util -lgtest_main $(UT_OBJS_DIR)/upll_ut.a ./stub/stub_objs/stub.a ./stub/stub_objs/stub.a -lgcov

LD_LIBRARY_PATH= $(GTEST_SRCROOT)/../dist/target/objs/ldlibs
export LD_LIBRARY_PATH

all  : createobjsupll createobjsut stub upll_lib ut_exe
test : 
			./vtn_momgr_ut
			./vbr_momgr_ut
	  	./vbr_if_momgr_ut	
clean :
	@(cd $(OBJSDIR)/ut/;  rm -f upll_ut.a *.o *.gcno *.gcov *.gcda);
	@(cd $(OBJSDIR)/upll/;  rm -f *.o *.gcno *.gcov *.gcda);
	@(cd ./stub/; $(MAKE) clean);
	rm -f $(EXEC_NAME) *.gcov


UT_OBJS = $(patsubst %.cc,$(OBJSDIR)/ut/%.o,$(UNITTEST_FILES))

$(SRC_OBJS): $(OBJSDIR)/upll/%.o : $(GTEST_SRCROOT)/upll/%.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(UT_OBJS): $(OBJSDIR)/ut/%.o : %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

createobjsupll:
	mkdir -p $(UPLL_OBJS_DIR)

createobjsut:
	mkdir -p $(UT_OBJS_DIR)

stub:
	@echo "****** Invoking stub makefile"
	@($(MAKE) -C ./stub/ );

.PHONY: stub upll_lib \

upll_lib : $(UT_OBJS_DIR)/upll_ut.a

$(UT_OBJS_DIR)/upll_ut.a : $(SRC_OBJS)
		$(AR) $(ARFLAGS) $@ $^

compile_ut:$(UT_OBJS)

ut_exe : $(EXEC_NAME)

vtn_momgr_ut : $(UT_OBJS_DIR)/vtn_momgr_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)
vbr_momgr_ut : $(UT_OBJS_DIR)/vbr_momgr_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)
vbr_if_momgr_ut : $(UT_OBJS_DIR)/vbr_if_momgr_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)


install: all
