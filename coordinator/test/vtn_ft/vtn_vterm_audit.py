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
import vtn_testconfig, vtn_vterm

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA


def test_vtn_vterm_audit_1():

    print "****CREATE Controller with valid IP****"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    print """TEST 1 : create VTN and VTERM when controller is up
             change the controller status to down delete VTN and VTERM
             trigger Audit"""
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)
    print "****Create VTN****"
    retval = vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    print "****Create VTERM****"
    retval = vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)


    retval =  vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)


    retval =  vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM Validate Failed"
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

    print "****Delete VTERM****"
    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    print "****Delete VTN****"
    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
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

    retval =  vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval =  vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)


    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VTERM AUDIT TEST SUCCESS"

def test_vtn_vterm_audit_2():

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

    print """TEST 2 : create VTN and VTERM when controller is down
             change the controller status to up trigger Audit"""
    print "****Create VTN****"
    retval = vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    print "****Create VTERM****"
    retval = vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
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

    retval =  vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)


    retval =  vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)


    print "****Delete VTERM****"
    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    print "****Delete VTN****"
    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)


    retval =  vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval =  vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)


    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VTERM AUDIT TEST SUCCESS"

def test_multi_vtn_with_vterm_audit_test():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)
    print "TEST 6 : 2 Tenants with one VTerminal each and test AUDIT"
    print "VTNONE->VTERMONE"
    print "VTNTWO->VTERMONE"

    retval = vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vterm.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnTwo','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval =  vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval =  vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval =  vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval =  vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM Validate Failed"
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

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
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

    retval =  vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval =  vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)


    retval =  vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval =  vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)


    retval = vtn_vterm.delete_vtn('VtnTwo')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "MULTI VTN->VTERM AUDIT TEST SUCCESS"

# Main Block
if __name__ == '__main__':
    print '*****VTN VTERM AUDIT TESTS******'
    test_vtn_vterm_audit_1()
    test_vtn_vterm_audit_2()
    test_multi_vtn_with_vterm_audit_test()
else:
    print "VTN VTERM Audit Loaded as Module"

