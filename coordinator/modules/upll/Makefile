#
# Copyright (c) 2012-2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of upll module.
##

include ../defs.mk

CXX_OPT = -O0

# We need to append build directory to header search path explicitly
# for build.cc.
EXTRA_INCDIRS	= .. ../dal

CXX_SOURCES	= ipct_st.cc kt_util.cc ipc_util.cc key_tree.cc dbconn_mgr.cc \
	ipc_client_handler.cc \
	ipc_event_queue.cc \
	task_sched.cc \
	config_yield.cc \
	momgr_intf.cc \
	ctrlr_mgr.cc \
	config_svc.cc \
	config_lock.cc config_mgr.cc read_bulk.cc tx_mgr.cc tclib_intf_impl.cc tx_update_util.cc \
  $(VTN_SOURCES) \
  $(POM_SOURCES)

VTN_SOURCES	= momgr_util.cc momgr_impl.cc \
  vtn_momgr.cc \
	vnode_momgr.cc vnode_child_momgr.cc \
  vbr_portmap_util.cc \
	vbr_portmap_momgr.cc \
  vbr_momgr.cc vbr_if_momgr.cc \
	vrt_momgr.cc vrt_if_momgr.cc \
  vlink_momgr.cc vlanmap_momgr.cc iproute_momgr.cc \
  dhcprelay_server_momgr.cc dhcprelay_if_momgr.cc \
	nwm_momgr.cc nwm_host_momgr.cc \
	vunk_momgr.cc vunk_if_momgr.cc \
        vtep_momgr.cc vtep_if_momgr.cc \
        vtep_grp_momgr.cc vtepgrp_mem_momgr.cc \
        vtunnel_momgr.cc vtunnel_if_momgr.cc \
				vtn_dataflow_momgr.cc vterminal_momgr.cc vterm_if_momgr.cc \
        unified_nw_momgr.cc unw_label_momgr.cc unw_label_range_momgr.cc vtn_unified_momgr.cc \
       unw_spine_domain_momgr.cc domain_check_util.cc
POM_SOURCES	= policingprofile_momgr.cc policingprofile_entry_momgr.cc \
              flowlist_momgr.cc flowlist_entry_momgr.cc \
							vbr_flowfilter_momgr.cc vbr_flowfilter_entry_momgr.cc \
              vbr_if_flowfilter_momgr.cc vbr_if_flowfilter_entry_momgr.cc \
						 	vrt_if_flowfilter_momgr.cc vrt_if_flowfilter_entry_momgr.cc \
						 	vtn_flowfilter_momgr.cc vtn_flowfilter_entry_momgr.cc \
              vbr_policingmap_momgr.cc vbr_if_policingmap_momgr.cc vterm_if_policingmap_momgr.cc \
							vtn_policingmap_momgr.cc \
              vterm_if_flowfilter_momgr.cc vterm_if_flowfilter_entry_momgr.cc

include ../rules.mk

##
## Targets:
##
## make all (default target)
##	Build all binary distributions.
##
## make install
##	Execute "make all" and install binary distributions into the
##	installation directory.
##
## make clean
##	Remove binary distributions built by "make all".
##
## make clobber
##	Execute "make clean" and remove header dependency files.
##
