#!/usr/bin/python

#
# Copyright (c) 2013-2016 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller
import vtn_testconfig, vtn_vbr

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA

def test_vtn_vbr_audit_multi_controller_1():
    print """TEST 1 : create VTN and VBR when controller is up
             change the controller status to down delete VTN and VBR
             trigger Audit"""
    print "****CREATE multi-Controller with valid IP****"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "****Create VTN****"
    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    print "****Create VBR****"
    retval = vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)


    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
        print "controller1 invalid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['invalid_ipaddr']
    retval = controller.update_controller_ex('ControllerSecond',ipaddr=test_invalid_ipaddr)
    if retval != 0:
        print "controller2 invalid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "****Delete VBR****"
    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3/VTN Delete Failed"
        exit(1)

    print "****Delete VTN****"
    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
        print "controller1 valid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['ipaddr']
    retval = controller.update_controller_ex('ControllerSecond',ipaddr=test_controller_ipaddr)
    if retval != 0:
        print "controller2 valid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR1 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VBR3 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    print "DELETE CONTROLLER2"
    retval = controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"

    print "VTN->VBR Multi-Controller AUDIT TEST SUCCESS"


def test_vtn_vbr_audit_multi_controller_2():
    print """TEST 2 : create VTN and VBR when controller is down
             change the controller status to up trigger Audit"""
    print "****CREATE Controller with valid IP****"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
        print "controller1 invalid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['invalid_ipaddr']
    retval = controller.update_controller_ex('ControllerSecond',ipaddr=test_invalid_ipaddr)
    if retval != 0:
        print "controller2 invalid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "****Create VTN****"
    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    print "****Create VBR****"
    retval = vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
        print "controller1 valid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['ipaddr']
    retval = controller.update_controller_ex('ControllerSecond',ipaddr=test_controller_ipaddr)
    if retval != 0:
        print "controller2 valid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Validate Failed"
        exit(1)

    print "****Delete VBR****"
    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    iretval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3/VTN Delete Failed"
        exit(1)

    print "****Delete VTN****"
    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)


    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR1 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VBR3 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)


    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval = controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN->VBR Multi-controller AUDIT TEST SUCCESS"

def test_multi_vtn_with_vbr_multi_controller_audit_test():
    print "TEST 3 : 2 Tenants with one VBridge each in multi-controller and test AUDIT"
    print "CONTROLLER1->VTNONE->VBRONE"
    print "CONTROLLER1->VTNTWO->VBRONE"
    print "CONTROLLER2->VTNTHREE->VBRONE"
    print "CONTROLLER2->VTNFOUR->VBRONE"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN1 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN2 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vtn('VtnThree')
    if retval != 0:
        print "VTN3 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vtn('VtnFour')
    if retval != 0:
        print "VTN4 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1/VTN1 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnTwo','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1/VTN2 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnThree','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3/VTN3 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnFour','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3/VTN4 Create Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN1 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTN2 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnThree','ControllerSecond',position=0)
    if retval != 0:
        print "VTN3 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnFour','ControllerSecond',position=0)
    if retval != 0:
        print "VTN4 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1/VTN1 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1/VTN2 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnThree','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3/VTN3 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnFour','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR4/VTN4 Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
        print "controller1 invalid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['invalid_ipaddr']
    retval = controller.update_controller_ex('ControllerSecond',ipaddr=test_invalid_ipaddr)
    if retval != 0:
        print "controller2 invalid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "****DELETE VBR****"
    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN1 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrOne')
    if retval != 0:
        print "VBR1/VTN2 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnThree','VbrThree')
    if retval != 0:
        print "VBR3/VTN3 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnFour','VbrThree')
    if retval != 0:
        print "VBR3/VTN4 Delete Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
        print "controller1 valid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['ipaddr']
    retval = controller.update_controller_ex('ControllerSecond',ipaddr=test_controller_ipaddr)
    if retval != 0:
        print "controller2 valid_ip update failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR1/VTN1 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN1 Validate Failed"
        exit(1)


    retval =  vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR1/VTN2 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN2 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnThree','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VBR3/VTN3 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN3 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vbr_at_controller('VtnFour','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VBR3/VTN4 Validate Failed"
        exit(1)

    retval =  vtn_vbr.validate_vtn_at_controller('VtnFour','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN4 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN1 Delete Failed in coordinator"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnTwo')
    if retval != 0:
        print "VTN2 Delete Failed in coordinator"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnThree')
    if retval != 0:
        print "VTN3 Delete Failed in coordinator"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnFour')
    if retval != 0:
        print "VTN4 Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval = controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "MULTI VTN->VBR MULTI_CONTROLLER AUDIT TEST SUCCESS"

# Main Block
if __name__ == '__main__':
    print '*****MULTI-CONTROLLER VTN VBR AUDIT TESTS******'
    test_vtn_vbr_audit_multi_controller_1()
    test_vtn_vbr_audit_multi_controller_2()
    test_multi_vtn_with_vbr_multi_controller_audit_test()
else:
    print "VTN VBR Audit Loaded as Module"

