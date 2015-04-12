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
import vtn_testconfig, vtn_vbr, vbrif_portmap,vtn_vterm,vtermif_portmap ,flowfilter, flowlistentry
import resp_code


def test_vbr_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBR->FLOWFILTER TEST"
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
  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)
  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
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
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
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

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
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

def update_vbr_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print 'TEST 2 : VBR->FLOWFILTER TEST: Create Three VBR and VbrFlowFilterEntry with vbrone update the VbrFlowFilterEntry to vbrthree'
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
  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)
  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
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
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
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

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne', 'UpdateFlowfilter')
  if retval != 0:
    print "VBRFlowFilterEntry Update Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne|UpdateFlowfilter', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
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

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrTwo', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
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


def update_vbr_flowlist():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print 'TEST 3 : VBR->FLOWLISTTEST:Update the VbrFlowListEntry'
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

  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)
  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
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
  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst')
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)
  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence="yes", position = 1)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)


  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=flowlistentry.update_flowlist_entry('FlowlistOne', 'UpdateFlowlistentryOne')
  if retval != 0:
    print "VBRFlowListEntry Update Failed"
    exit(1)
#  retval=flowlistentry.validate_flowlist_entry('FlowlistOne', 'UpdateFlowlistentryOne','ControllerFirst', presence='yes', position=0)
#  if retval != 0:
#    print "VBRFlowListEntry Updatation Validate Failed at Co-ordinator"
#    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
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

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrTwo', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
  if retval != 0:
    print "FlowFilter validation Failed after deleting"
    exit(1)

  retval=flowlistentry.validate_flowlist_at_controller('FlowlistOne', 'ControllerFirst', presence='no', position=0)
  if retval != 0:
    print "Flowlist validation Failed after deleting"
    exit(1)

#  retval=flowlistentry.validate_flowlist_entry('FlowlistOne', 'UpdateFlowlistentryOne','ControllerFirst', presence='no', position=0)
#  if retval != 0:
#    print "VBRFlowListEntry Delete Validate Failed at Co-ordinator"
#    exit(1)

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
  print "VBR->UPDATE FLOWLIST TEST SUCCESS"
def negative_vbr_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)
  print "Negative test scenario test cases"
  print 'TEST 3 : VBR->FLOWFILTER TEST: Create Three VBR and VbrFlowFilterEntry with vbrone negative  the VbrFlowFilterEntry to vbrthree'
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
  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)
  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
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
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)


  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
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

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne', 'NegativeFlowfilter')
  if retval != 0:
    print "VBRFlowFilterEntry at negative  priority and dscp has  Failed"
    print "Because flowfilter priority range up 63 and dscp range up to 7 its more than test was faild its showing Bad error"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne|NegativeFlowfilter', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Negative  Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
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

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrTwo', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
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


def test_vbr_flowfilter_pass():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)
  print "action type create pass"
  print "TEST 5 : VBR->FLOWFILTER TEST"
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
  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)
  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
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
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)


  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)


  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOnePass', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOnePass')
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

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOnePass', presence='no', position=0)
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

def update_vbr_flowfilter_pass():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print 'TEST 6 : VBR->FLOWFILTER TEST: Create Three VBR and VbrFlowFilterEntry with vbrone update the VbrFlowFilterEntry to vbrthree'
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
  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)
  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
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
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)


  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)


  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOnePass', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)


  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOnePass', 'UpdateFlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry Update Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst','UpdateFlowfilterOnePass', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOnePass')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOnePass')
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

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrTwo', 'ControllerFirst', 'FlowfilterOnePass', presence='no', position=0)
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

def test_vbr_flowfilter_drop():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBR->FLOWFILTER TEST"
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
  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)
  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
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
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)


  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)


  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOneDrop', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOneDrop')
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

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOneDrop', presence='no', position=0)
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

def update_vbr_flowfilter_drop():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print 'TEST 8 : VBR->FLOWFILTER TEST: Create Three VBR and VbrFlowFilterEntry with vbrone update the VbrFlowFilterEntry to vbrthree'
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
  retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
  if retval != 0:
    print "Portmap Create Failed"
    exit(1)
  retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
  if retval != 0:
    print "Portmap Create Failed"
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
  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)

  retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes");
  if retval != 0:
     print "Portmap Validate Failed"
     exit(1)


  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)


  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOneDrop', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
    exit(1)


  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOneDrop', 'UpdateFlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry Update Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst','UpdateFlowfilterOneDrop', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOneDrop')
  if retval != 0:
    print "VBRFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOneDrop')
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

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrTwo', 'ControllerFirst', 'FlowfilterOneDrop', presence='no', position=0)
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



# Main Block
if __name__ == '__main__':
    print '*****VBR FLOWFILTER IN TESTS******'
    test_vbr_flowfilter()
    print '******UPDATE VBR FLOWFLTER IN TESTS****'
    update_vbr_flowfilter()
    print '******UPDATE VBR FLOWLIST TESTS****'
    update_vbr_flowlist()
    print '*****VBR FLOWFILTER ACTION TYPE PASS TESTS******'
    test_vbr_flowfilter_pass()
    print '******UPDATE VBR FLOWFLTER ACTION TYPE DROP TESTS****'
    update_vbr_flowfilter_pass()
    print '*****VBR FLOWFILTER ACTION TYPE DROP TESTS******'
    test_vbr_flowfilter_drop()
    print '******UPDATE VBR FLOWFLTER ACTION TYPE PASS TESTS****'
    update_vbr_flowfilter_drop()
    print '*****NEGATIVE VBR FLOWFILTER IN  TESTS***'
    negative_vbr_flowfilter()

else:
    print 'VBR_FLOWFILTER LOADED AS MODULE'
