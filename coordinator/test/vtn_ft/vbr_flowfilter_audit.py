#!/usr/bin/python

#
# Copyright (c) 2014 NEC Corporation
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


def test_vbr_flowfilter_audit_1():

    print "****CREATE Controller with valid IP****"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    print """TEST 1 : create VTN and VBR FLOWFILTER when controller is down
             change the controller status to up trigger Audit and validate"""
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)
    print "****Create VTN****"
    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    print "****Create VBR****"
    retval = vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
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

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
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

    retval=flowfilter.create_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
    if retval != 0:
        print "VBRFlowFilter Create Failed"
        exit(1)

    retval=flowfilter.create_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
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

    retval=flowfilter.validate_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne', presence='yes', position=0)
    if retval != 0:
        print "VBRFlowFilterEntry Validate Failed at Co-ordinator"
        exit(1)

    retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
    if retval != 0:
        print "FlowFilter validation Failed at Controller"
        exit(1)

    retval = flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
    if retval != 0:
       print "VBRFlowFilterEntry deletete Failed"
       exit(1)
    retval = flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
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

    print "****Delete VTN****"
    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
    if retval != 0:
      print "FlowFilter validation Failed after deleting"
      exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
      print "VTN Validate Failed"
      exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VBR->FLOWFILTER AUDIT_1 TEST SUCCESS"

def test_vbr_flowfilter_audit_2():

    print "****CREATE Controller with valid IP****"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)

    print """TEST 2 : create VTN FLOWFILTER when controller is up
             delete the flowfilter change the controller status to
             down trigger Audit and validate"""

    print "****Create VTN****"
    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    print "****Create VBR****"
    retval = vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
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

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
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

    retval = flowfilter.validate_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne', presence='yes', position=0)
    if retval != 0:
        print "VBRFlowFilterEntry Validate Failed at Co-ordinator"
        exit(1)

    retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='yes', position=0)
    if retval != 0:
        print "FlowFilter validation Failed"
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

    retval=flowfilter.delete_flowfilter_entry('VtnOne|VbrOne', 'FlowfilterOne')
    if retval != 0:
        print "VTNFlowFilterEntry delete Failed"
        exit(1)

    retval=flowfilter.delete_flowfilter('VtnOne|VbrOne', 'FlowfilterOne')
    if retval != 0:
        print "VTNFlowFilter delete Failed"
        exit(1)

    retval=flowlistentry.delete_flowlistentry('FlowlistOne', 'FlowlistentryOne')
    if retval != 0:
      print "FlowilistEntry deletete Failed"
      exit(1)

    retval=flowlistentry.delete_flowlist('FlowlistOne')
    if retval != 0:
      print "Flowilist deletete Failed"
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

    retval=flowfilter.validate_flowfilter_at_controller('VtnOne|VbrOne', 'ControllerFirst', 'FlowfilterOne', presence='no', position=0)
    if retval != 0:
        print "FlowFilter validation Failed after deleting"
        exit(1)

    print "****Delete VTN****"
    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
         print "CONTROLLER delete failed"
         exit(1)
    print "VBR->FLOWFILTER AUDIT_2 SUCCESS"


# Main Block
if __name__ == '__main__':
    print '*****VBR AUDIT FLOWFILTER TESTS******'
    test_vbr_flowfilter_audit_1()
    test_vbr_flowfilter_audit_2()
else:
    print "VBR FLOWFILTER Audit Loaded as Module"

