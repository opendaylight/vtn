#!/usr/bin/python

#
# Copyright (c) 2014-2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller
import vtn_testconfig, vtn_vbr, vbrif_portmap, flowfilter, flowlistentry
import resp_code


def test_vbrif_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBRIF->FLOWFILTER TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->FLOWFILTER TEST SUCCESS"

def update_vbrif_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBRIF->Update FLOWFILTER TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree', 'VbrIfThree')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfThree');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', 'UpdateFlowfilter')
  if retval != 0:
    print "VBRFIFlowFilterEntry Update Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne|UpdateFlowfilter', presence='yes', position=0)
  if retval != 0:
    print "VBRFIFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)


  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->UPDATE FLOWFILTER TEST SUCCESS"


def negative_vbrif_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBRIF->Negative test scenario FLOWFILTER TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree', 'VbrIfThree')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfThree');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', 'NegativeFlowfilter')
  if retval != 1:
    print "VBRFIFlowFilterEntry Negative test case DSCP,Priority  Failed"
    print "Because  DSCP range was up to (0-63) and priority range (0-7) its more than higher its showing Bad command so test failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print retval
    print "VBRFIFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)


  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->UPDATE FLOWFILTER TEST SUCCESS"

def test_vbrif_flowfilter_out():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBRIF->FLOWFILTER TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneOut', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)


  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneOut', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->FLOWFILTER TEST SUCCESS"

def update_vbrif_flowfilter_out():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBRIF->Update FLOWFILTER TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneOut', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree', 'VbrIfThree')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfThree');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut', 'UpdateFlowfilterOne')
  if retval != 0:
    print "VBRFIFlowFilterEntry Update Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut|UpdateFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFIFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)


  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->UPDATE FLOWFILTER TEST SUCCESS"


def negative_vbrif_flowfilter_out():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBRIF->Negative test scenario FLOWFILTER TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneOut', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree', 'VbrIfThree')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfThree');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut', 'NegativeFlowfilterOne')
  if retval != 1:
    print "VBRFIFlowFilterEntry Negative test case DSCP,Priority  Failed"
    print "Because  DSCP range was up to (0-63) and priority range (0-7) its more than higher its showing Bad command so test failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut', presence='yes', position=0)
  if retval != 0:
    print retval
    print "VBRFIFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)


  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneOut', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->UPDATE FLOWFILTER TEST SUCCESS"

def test_vbrif_flowfilter_pass():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 3 : VBRIF->FLOWFILTER PASS TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOnePass', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOnePass', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->FLOWFILTER PASS TEST SUCCESS"

def update_vbrif_flowfilter_pass():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 4 : VBRIF->Update FLOWFILTER DROP TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)


  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOnePass', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree', 'VbrIfThree')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfThree');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass', 'UpdateFlowfilterOnePass')
  if retval != 0:
    print "VBRFIFlowFilterEntry Update Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst','UpdateFlowfilterOnePass', presence='yes', position=0)
  if retval != 0:
    print "VBRFIFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOnePass', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->UPDATE FLOWFILTER DROP TEST SUCCESS"

def test_vbrif_flowfilter_drop():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 5 : VBRIF->FLOWFILTER DROP TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)


  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneDrop', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)


  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneDrop', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->FLOWFILTER TEST SUCCESS"

def update_vbrif_flowfilter_drop():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 6 : VBRIF->Update FLOWFILTER PASS TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)


  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneDrop', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerFirst')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree', 'VbrIfThree')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfThree');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop', 'UpdateFlowfilterOneDrop')
  if retval != 0:
    print "VBRFIFlowFilterEntry Update Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst','UpdateFlowfilterOneDrop', presence='yes', position=0)
  if retval != 0:
    print "VBRFIFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)


  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOneDrop', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->UPDATE FLOWFILTER TEST PASS SUCCESS"


def test_vbrif_flowfilter_bulkdelete():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBRIF->FLOWFILTER TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)

  retval=vtn_vbr.create_vtn('VtnOne')
  if retval != 0:
    print "VTN Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
  if retval != 0:
    print "VBR Create Failed"
    exit(1)

  retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
  if retval != 0:
    print "VBRIF Create Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
    print "Portmap Validate Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->FLOWFILTER TEST SUCCESS"


# Main Block
if __name__ == '__main__':
    print '*****VBR FLOWFILTER IN TESTS******'
    test_vbrif_flowfilter()
    print '****** UPDATE FLOWFILTER IN TESTS**********'
    update_vbrif_flowfilter()
    print '*****VBR FLOWFILTER PASS TESTS******'
    test_vbrif_flowfilter_pass()
    print '****** UPDATE FLOWFILTER DROP TESTS**********'
    update_vbrif_flowfilter_pass()
    print '*****VBR FLOWFILTER DROP TESTS******'
    test_vbrif_flowfilter_drop()
    print '****** UPDATE FLOWFILTER PASS TESTS**********'
    update_vbrif_flowfilter_drop()
    print '*****VBR FLOWFILTER OUT TESTS******'
    test_vbrif_flowfilter_out()
    print '*****UPDATE VBR FLOWFILTER OUT TESTS******'
    update_vbrif_flowfilter_out()
    print '*****NEGATIVE VBR FLOWFILTER IN TESTS******'
    negative_vbrif_flowfilter()
    print '*****NEGATIVE VBR FLOWFILTER OUT TESTS******'
    negative_vbrif_flowfilter_out()
    print '*****Delete Bulk VBR FLOWFILTER IN TESTS******'
    test_vbrif_flowfilter_bulkdelete()



else:
    print 'VBR_FLOWFILTER LOADED AS MODULE'
