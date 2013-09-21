#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run unit tests of UPPL module
##


SRC_DIR = ../../../../modules

CPPFLAGS  += 

INCLUDE = -Iinclude \
          -Istub/ODBC/ \
          -Istub/ODBC/include \
          -Istub/tclib_module \
					-Istub/include \
					-Istub/include/cxx \
					-Istub/include/cxx/pfcxx \
					-Istub/include/core_include \
					-Istub \
          -Istub/clstat \
					-I$(SRC_DIR)/uppl \
	        -I$(SRC_DIR)/clstat \
					-I$(SRC_DIR)/clstat/include \
					-I$(SRC_DIR)/tclib/include \
					-I$(SRC_DIR)../core/include \
					-I$(SRC_DIR) \
					-I$(SRC_DIR)/../dist/target/objs/include \
					-I$(SRC_DIR)/../dist/target/objs/core_include \
					-I$(SRC_DIR)/../dist/target/objs/core/include \
					-I$(SRC_DIR)/../core/include \
					-I$(SRC_DIR)/../core/include/cxx \
					-I$(SRC_DIR)/../include/ \
					-I$(SRC_DIR)/../include/unc \
					-I$(SRC_DIR)/uppl/include \
					-I$(SRC_DIR)/uppl \
					-I$(SRC_DIR)/ \
					-I$(SRC_DIR)/alarm/include \
					-I$(SRC_DIR)/../test/modules/uppl/utest
	
CXXFLAGS +=  -g -fprofile-arcs -ftest-coverage -Dprivate=public	-Dprotected=public


SOURCE_FILES = $(SRC_DIR)/uppl/physicallayer.cc \
							 $(SRC_DIR)/uppl/phy_util.cc \
							 $(SRC_DIR)/uppl/controller_version.cc \
							 $(SRC_DIR)/uppl/unc_state_handler.cc \
							 $(SRC_DIR)/uppl/ipct_util.cc \
							 $(SRC_DIR)/uppl/physical_core.cc \
							 $(SRC_DIR)/uppl/physical_itc.cc \
							 $(SRC_DIR)/uppl/itc_configuration_request.cc \
							 $(SRC_DIR)/uppl/itc_read_request.cc \
							 $(SRC_DIR)/uppl/itc_kt_base.cc \
							 $(SRC_DIR)/uppl/itc_kt_state_base.cc \
							 $(SRC_DIR)/uppl/itc_kt_root.cc \
							 $(SRC_DIR)/uppl/itc_kt_controller.cc \
							 $(SRC_DIR)/uppl/itc_kt_ctr_domain.cc \
							 $(SRC_DIR)/uppl/itc_kt_logicalport.cc \
							 $(SRC_DIR)/uppl/itc_kt_logical_member_port.cc \
							 $(SRC_DIR)/uppl/itc_kt_switch.cc \
							 $(SRC_DIR)/uppl/itc_kt_port.cc \
							 $(SRC_DIR)/uppl/itc_kt_link.cc \
							 $(SRC_DIR)/uppl/itc_kt_boundary.cc \
							 $(SRC_DIR)/uppl/itc_transaction_request.cc \
							 $(SRC_DIR)/uppl/itc_import_request.cc \
							 $(SRC_DIR)/uppl/itc_audit_request.cc \
							 $(SRC_DIR)/uppl/itc_db_config.cc \
							 $(SRC_DIR)/uppl/itc_state_change.cc \
							 $(SRC_DIR)/uppl/ipc_connection_manager.cc \
							 $(SRC_DIR)/uppl/ipc_client_logical_handler.cc \
							 $(SRC_DIR)/uppl/ipc_server_handler.cc \
							 $(SRC_DIR)/uppl/ipc_client_configuration_handler.cc \
							 $(SRC_DIR)/uppl/physical_notification_manager.cc \
							 $(SRC_DIR)/uppl/itc_notification_request.cc \
               $(SRC_DIR)/uppl/odbcm_db_tableschema.cc \

HEADER_FILES = $(SRC_DIR)/uppl/include/controller_version.hh \
							 $(SRC_DIR)/uppl/include/ipc_client_configuration_handler.hh  \
							 $(SRC_DIR)/uppl/include/ipc_client_logical_handler.hh  \
							 $(SRC_DIR)/uppl/include/ipc_connection_manager.hh  \
							 $(SRC_DIR)/uppl/include/ipc_server_handler.hh  \
							 $(SRC_DIR)/uppl/include/ipct_util.hh  \
							 $(SRC_DIR)/uppl/include/itc_audit_request.hh  \
							 $(SRC_DIR)/uppl/include/itc_configuration_request.hh  \
							 $(SRC_DIR)/uppl/include/itc_db_config.hh  \
							 $(SRC_DIR)/uppl/include/itc_import_request.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_base.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_boundary.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_controller.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_ctr_domain.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_link.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_logical_member_port.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_logicalport.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_port.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_root.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_state_base.hh  \
							 $(SRC_DIR)/uppl/include/itc_kt_switch.hh  \
							 $(SRC_DIR)/uppl/include/tc_notification_request.hh  \
							 $(SRC_DIR)/uppl/include/itc_read_request.hh  \
							 $(SRC_DIR)/uppl/include/itc_state_change.hh  \
							 $(SRC_DIR)/uppl/include/itc_transaction_request.hh  \
							 $(SRC_DIR)/uppl/include/odbcm_common.hh  \
							 $(SRC_DIR)/uppl/include/odbcm_connection.hh  \
							 $(SRC_DIR)/uppl/include/odbcm_db_tableschema.hh  \
							 $(SRC_DIR)/uppl/include/odbcm_db_varbind.hh  \
							 $(SRC_DIR)/uppl/include/odbcm_mgr.hh  \
							 $(SRC_DIR)/uppl/include/odbcm_query_factory.hh  \
							 $(SRC_DIR)/uppl/include/odbcm_query_processor.hh  \
							 $(SRC_DIR)/uppl/include/odbcm_utils.hh  \
							 $(SRC_DIR)/uppl/include/physical_common_def.hh  \
							 $(SRC_DIR)/uppl/include/physical_core.hh  \
							 $(SRC_DIR)/uppl/include/physical_itc.hh  \
							 $(SRC_DIR)/uppl/include/physical_itc_req.hh  \
							 $(SRC_DIR)/uppl/include/physicallayer.hh  \
							 $(SRC_DIR)/uppl/include/physical_notification_manager.hh  \
							 $(SRC_DIR)/uppl/include/phy_util.hh  \
							 $(SRC_DIR)/uppl/include/unc_state_handler.hh  \

SRC_OBJS = $(patsubst $(SRC_DIR)/uppl/%.cc,$(OBJSDIR)/uppl/%.o,$(SOURCE_FILES))


UNITTEST_FILES = Domain_ut.cc KtPort.cc KtLogicalMemberPort_ut.cc KtLogicalPort_ut.cc ut_switch.cc Controller_ut.cc KtLink_ut.cc Boundary_ut.cc
STUB_FILE = PhysicalLayerStub.cc 

OBJSDIR = objs

UT_OBJS_DIR = objs/utest

UT_EXE_LIST = KtPort KtLogicalMemberPort_ut KtLogicalPort_ut UTDomain ut_switch UTController KtLink_ut Boundary_ut

LD_LIBRARY_PATH = $(SRC_DIR)/../dist/target/objs/ldlibs
export LD_LIBRARY_PATH

UT_LIBS = -L$(SRC_DIR)/../dist/target/objs/ldlibs -lpthread -lpfc_util -lpfc -lpfcxx -lgtest_main $(UT_OBJS_DIR)/uppl_ut.a ./stub/stub_objs/stub.a -lgcov

createupplobjs:
	mkdir -p $(OBJSDIR)/uppl

createutestobjs:
	mkdir -p $(OBJSDIR)/utest
.NOTPARALLEL: createupplobjs createutestobjs stub uppl_lib ut_exe
all  : createupplobjs createutestobjs stub uppl_lib ut_exe
test : 
			./KtPort 
			./KtLogicalMemberPort_ut 
			./KtLogicalPort_ut 
			./UTDomain
			./ut_switch
			./UTController
			./KtLink_ut
			./Boundary_ut
clean :
	@(cd $(OBJSDIR)/utest/;  rm -f uppl_ut.a *.o *.gcno *.gcov *.gcda);
	@(cd $(OBJSDIR)/uppl/;  rm -f *.o *.gcno *.gcov *.gcda);
	@(cd ./stub/; $(MAKE) clean);
	rm -f $(UT_EXE_LIST) *.gcov


UT_OBJS = $(patsubst %.cc,$(OBJSDIR)/utest/%.o,$(UNITTEST_FILES))

STUB_OBJ = $(patsubst %.cc,$(OBJSDIR)/utest/%.o,$(STUB_FILE))

$(SRC_OBJS):$(OBJSDIR)/uppl/%.o : $(SRC_DIR)/uppl/%.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(UT_OBJS):$(OBJSDIR)/utest/%.o : %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(STUB_OBJ):$(OBJSDIR)/utest/%.o : %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

stub:
	@echo "****** Invoking stub makefile"
	@($(MAKE) -C ./stub all );

.PHONY: stub uppl_lib

uppl_lib : $(UT_OBJS_DIR)/uppl_ut.a

$(UT_OBJS_DIR)/uppl_ut.a : $(SRC_OBJS) $(STUB_OBJ)
		$(AR) $(ARFLAGS) $@ $^

compile_ut:$(UT_OBJS)

ut_exe : $(UT_EXE_LIST)

UTDomain : $(UT_OBJS_DIR)/Domain_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)

KtPort : $(UT_OBJS_DIR)/KtPort.o
	$(CXX) $^ -o $@ $(UT_LIBS)

KtLogicalMemberPort_ut : $(UT_OBJS_DIR)/KtLogicalMemberPort_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)

KtLogicalPort_ut : $(UT_OBJS_DIR)/KtLogicalPort_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)

ut_switch : $(UT_OBJS_DIR)/ut_switch.o
	$(CXX) $^ -o $@ $(UT_LIBS)

UTController : $(UT_OBJS_DIR)/Controller_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)

KtLink_ut : $(UT_OBJS_DIR)/KtLink_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)

Boundary_ut : $(UT_OBJS_DIR)/Boundary_ut.o
	$(CXX) $^ -o $@ $(UT_LIBS)
