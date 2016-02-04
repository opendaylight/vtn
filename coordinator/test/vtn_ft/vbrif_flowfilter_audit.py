#!/usr/bin/python

#
# Copyright (c) 2014-2016 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller
import vtn_testconfig, vtn_vbr, vbrif_portmap, flowfilter, flowlistentry
import resp_code

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA

def test_vbrif_flowfilter_audit():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 1 : VBRIF->FLOWFILTER AUDIT TEST"
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

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 0)
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

  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
    exit(1)

  retval=flowfilter.create_flowfilter('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilter Create Failed"
    exit(1)

  retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne')
  if retval != 0:
    print "VBRFlowFilterEntry Create Failed"
    exit(1)

  print "****UPDATE Controller IP to Valid****"
  test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
  if retval != 0:
    print "controller valid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"up")
  if retval != 0:
    print "controller state change failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFlowFilterEntry Validation at Co-ordinator Failed "
    exit(1)

  retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne|VbrIfOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "FlowFilter validation at Controller Failed"
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
  print "VBRIF->FLOWFILTER AUDIT TEST SUCCESS"

def update_vbrif_flowfilter_audit():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 2 : VBRIF->AUDIT Update FLOWFILTER TEST"
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

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 0)
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

  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
    exit(1)

  retval=flowfilter.update_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne', 'UpdateFlowfilter')
  if retval != 0:
    print "VBRFIFlowFilterEntry Update Failed"
    exit(1)
  print "****UPDATE Controller IP to Valid****"
  test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
  if retval != 0:
    print "controller valid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"up")
  if retval != 0:
    print "controller state change failed"
    exit(1)


  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOne|UpdateFlowfilter', presence='yes', position=0)
  if retval != 0:
    print "VBRFIFlowFilterEntry Updatation Validate Failed at Co-ordinator"
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
  print "VBR->UPDATE FLOWFILTER AUDIT TEST SUCCESS"


def test_vbrif_flowfilter_out_audit():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 7 : VBRIF->AUDIT FLOWFILTER TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)
  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
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

  print "****UPDATE Controller IP to Valid****"
  test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
  if retval != 0:
    print "controller valid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"up")
  if retval != 0:
    print "controller state change failed"
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

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 0)
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
  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
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


  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->AUDIT FLOWFILTER TEST SUCCESS"

def update_vbrif_flowfilter_out_audit():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 8 : VBRIF->AUDIT Update FLOWFILTER TEST"
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

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 0)
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
  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
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
  print "****UPDATE Controller IP to Valid****"
  test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
  if retval != 0:
    print "controller valid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"up")
  if retval != 0:
    print "controller state change failed"
    exit(1)

  retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne|VbrIfOne', 'FlowfilterOneOut|UpdateFlowfilterOne', presence='yes', position=0)
  if retval != 0:
    print "VBRFIFlowFilterEntry Updatation Validate Failed at Co-ordinator"
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
  print "VBR->AUDIT UPDATE FLOWFILTER TEST SUCCESS"


def test_vbrif_flowfilter_pass_audit():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 3 : VBRIF->AUDIT FLOWFILTER PASS TEST"
  # Delay for AUDIT
  retval=controller.wait_until_state('ControllerFirst', "up")
  if retval != 0:
    print "Controller state check Failed"
    exit(1)
  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
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
  print "****UPDATE Controller IP to Valid****"
  test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
  if retval != 0:
    print "controller valid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"up")
  if retval != 0:
    print "controller state change failed"
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

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 0)
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
  print "VBR->AUDIT FLOWFILTER PASS TEST SUCCESS"

def update_vbrif_flowfilter_pass_audit():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 4 : VBRIF->Update AUDIT FLOWFILTER DROP TEST"
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

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)

  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
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

  print "****UPDATE Controller IP to Valid****"
  test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
  if retval != 0:
    print "controller valid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"up")
  if retval != 0:
    print "controller state change failed"
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
  print "VBR->AUDIT UPDATE FLOWFILTER DROP TEST SUCCESS"

def test_vbrif_flowfilter_drop_audit():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 5 : VBRIF->AUDIT FLOWFILTER DROP TEST"
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

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 0)
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

  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
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

  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
     print "CONTROLLER delete failed"
     exit(1)
  print "VBR->AUDIT FLOWFILTER TEST SUCCESS"

def update_vbrif_flowfilter_drop_audit():
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  print "TEST 6 : VBRIF->Update AUDIT FLOWFILTER PASS TEST"
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

  retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "VBR Validate Failed"
    exit(1)

  retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst', presence = 'yes', position = 0)
  if retval != 0:
    print "After Create VBRIF Validate Failed"
    exit(1)
  print "****UPDATE Controller IP to invalid****"
  test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
  if retval != 0:
    print "controller invalid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"down")
  if retval != 0:
    print "controller state change failed"
    exit(1)


  print "****UPDATE Controller IP to Valid****"
  test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
  retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
  if retval != 0:
    print "controller valid_ip update failed"
    exit(1)
  # Delay for AUDIT
  retval = controller.wait_until_state('ControllerFirst',"up")
  if retval != 0:
    print "controller state change failed"
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
  print "VBR->AUDIT UPDATE FLOWFILTER TEST PASS SUCCESS"





# Main Block
if __name__ == '__main__':
    print '*****VBR FLOWFILTER TESTS******'
    test_vbrif_flowfilter_audit()
    print '****** UPDATE AUDIT FLOWFILTER IN TESTS**********'
    update_vbrif_flowfilter_audit()
    print '*****VBR AUDIT FLOWFILTER PASS TESTS******'
    test_vbrif_flowfilter_pass_audit()
    print '****** UPDATE AUDIT FLOWFILTER DROP TESTS**********'
    update_vbrif_flowfilter_pass_audit()
    print '*****VBR AUDIT FLOWFILTER DROP TESTS******'
    test_vbrif_flowfilter_drop_audit()
    print '****** UPDATE AUDIT FLOWFILTER PASS TESTS**********'
    update_vbrif_flowfilter_drop_audit()
    print '*****VBR AUDIT FLOWFILTER OUT TESTS******'
    test_vbrif_flowfilter_out_audit()
    print '*****UPDATE VBR FLOWFILTER OUT TESTS******'
    update_vbrif_flowfilter_out_audit()

else:
    print 'VBR_FLOWFILTER LOADED AS MODULE'
