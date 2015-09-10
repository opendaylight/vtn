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
import vtn_testconfig, vtn_vbr, flowfilter, flowlistentry
import resp_code


def test_vtn_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VTN->FLOWFILTER TEST"
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

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VTNFlowFilterEntry Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne', 'ControllerFirst', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne', 'ControllerFirst', 'VTNFlowfilterOne', presence='no', position=0)
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
  print "VTN->FLOWFILTER TEST SUCCESS"

def update_vtn_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 2 : VTN->UPDATE FLOWFILTER TEST "
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

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VTNFlowFilterEntry Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne', 'ControllerFirst', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation Failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne', 'VTNFlowfilterOne', 'UpdateVTNFlowfilter')
  if retval != 0:
    print "VTNFlowFilterEntry Update Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne', 'VTNFlowfilterOne|UpdateVTNFlowfilter', presence='yes', position=0)
  if retval != 0:
    print "VTNFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne', 'ControllerFirst', 'VTNFlowfilterOne', presence='no', position=0)
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
  print "VTN->UPDATE FLOWFILTER TEST SUCCESS"


def negative_vtn_flowfilter():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)
  print "NEGATIVE FLOWFILTER TEST"
  print "TEST 3 : VTN->Negative FLOWFILTER TEST "
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

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VTNFlowFilterEntry Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne', 'ControllerFirst', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation Failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne', 'VTNFlowfilterOne', 'NegativeVTNFlowfilter')
  if retval != 1:
    print "VTNFlowFilterEntry Negative test case  Failed at priority and dscp "
    print "Because at priority range up to 63 and dscp range up to 7 its more than its showing bad request so test has faild"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VTNFlowFilterEntry Updatation Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.delete_flowfilter_entry('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilterEntry deletete Failed"
    exit(1)

  retval=flowfilter.delete_flowfilter('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilter deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
  if retval != 0:
    print "FlowilistEntry deletete Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne', 'ControllerFirst', 'VTNFlowfilterOne', presence='no', position=0)
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
  print "VTN->UPDATE FLOWFILTER TEST SUCCESS"

def test_vtn_flowfilter_bulkdelete():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VTN->FLOWFILTER TEST"
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

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=flowlistentry.create_flowlist('FlowlistOne')
  if retval != 0:
    print "FlowList Create Failed"
    exit(1)

  retval=flowlistentry.create_flowlistentry('FlowlistOne', 'FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "FlowlistEntry Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne', 'VTNFlowfilterOne')
  if retval != 0:
    print "VTNFlowFilterEntry Create Failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VTNFlowFilterEntry Validate Failed at Co-ordinator"
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne', 'ControllerFirst', 'VTNFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation Failed"
    exit(1)

  retval=vtn_vbr.delete_vtn('VtnOne')
  if retval != 0:
    print "DELETE VTN Failed"
    exit(1)

  retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence='no')
  if retval != 0:
    print "VTN Validate Failed"
    exit(1)

  retval=flowlistentry.delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowilist deletete Failed"
    exit(1)

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "Buld Delete VTN->FLOWFILTER TEST SUCCESS"


# Main Block
if __name__ == '__main__':
    print '*****VTN FLOWFILTER TESTS******'
    test_vtn_flowfilter()
    print '*****UPDATE FLOWFILTER TESTS****'
    update_vtn_flowfilter()
    print '*******NEGATIVE FLOWFILTER TESTS****'
    negative_vtn_flowfilter()
    print '*******VTN FLOWFILTER TESTS BULK****'
    test_vtn_flowfilter_bulkdelete()
else:
    print 'VTN_FLOWFILTER LOADED AS MODULE'
