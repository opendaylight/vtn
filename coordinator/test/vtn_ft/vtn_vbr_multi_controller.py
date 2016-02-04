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

VTNVBRDATA = vtn_testconfig.VTNVBRDATA
CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA

coordinator_url = vtn_testconfig.coordinator_url
def_header = vtn_testconfig.coordinator_headers
controller_headers = vtn_testconfig.controller_headers
controller_url_part = vtn_testconfig.controller_url_part

def test_vtn_vbr_multi_controller():
    print "TEST 1 : 1 Tenants with 1 VBridge each on different controllers"
    print "CONTROLLER1->VTNONE->VBRONE"
    print "CONTROLLER2->VTNONE->VBRTWO"
    print "CREATE ControllerFirst"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "CREATE ControllerSecond"
    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBROne Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBRTwo Validate Failed"
        exit(1)

    retval  =  vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VtnVBRTwo Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VtnVBR Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)


    print "DELETE CONTROLLER1"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "DELETE CONTROLLER2"
    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "MULTI CONTROLLER VTN->VBR TEST SUCCESS"

def test_vtn_multi_vbr_multi_controller():
    print "TEST 2 : 1 Tenants with 2 VBridge each on different controllers"
    print "CONTROLLER1->VTNONE->VBRONE->VBRTWO"
    print "CONTROLLER2->VTNTWO->VBRTHREE->VBRFOUR"
    print "CREATE ControllerFirst"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller1 Create Failed"
        exit(1)
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "CREATE ControllerSecond"
    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "Controller2 Create Failed"
        exit(1)
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vbr.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VBR2 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnTwo','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnTwo','VbrFour','ControllerSecond')
    if retval != 0:
        print "VBR4 Create Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN1 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerSecond')
    if retval != 0:
        print "VTN2 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBR2 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrThree','ControllerSecond',position=0)
    if retval != 0:
        print "VBR3 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrFour','ControllerSecond')
    if retval != 0:
        print "VBR4 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN1 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR2/VTN1 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrThree')
    if retval != 0:
        print "VBR3/VTN2 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrFour')
    if retval != 0:
        print "VBR4/VTN2 Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR1/VBR1 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR2/VTN1 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VBR3/VTN2 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrFour','ControllerSecond',presence="no")
    if retval != 0:
     print "VBR4/VTN2 Validate Failed"
     exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst', presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerSecond', presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnTwo')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER1"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "DELETE CONTROLLER2"
    retval = controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "MULTI CONTROLLER MULTI VBR VTN->VBR TEST SUCCESS"

def test_multi_vtn_with_vbr_multi_controller():
    print "TEST 3 : 2 Tenants with one VBridge each"
    print "CONTROLLER1->VTNONE->VBRONE"
    print "CONTROLLER1->VTNTWO->VBRONE"
    print "CONTROLLER2->VTNONE->VBRONE"
    print "CONTROLLER2->VTNTWO->VBRONE"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller1 Create Failed"
        exit(1)
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "Controller2 Create Failed"
        exit(1)
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vbr.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnTwo','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR2 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    retval = vtn_vbr.create_vbr('VtnTwo','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR4 Create Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN1/CONTROLLER1 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTN2/CONTROLLER2 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "VTN1/CONTROLLER2 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerSecond',position=0)
    if retval != 0:
        print "VTN2/CONTROLLER2 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1/VTN1 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1/VTN2 Validate Failed "
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR2/VTN1 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR2/VTN2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN1 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR2 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR2/VTN1 Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VBR2/VTN1 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrThree')
    if retval != 0:
        print "VBR2/VTN2 Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VBR2/VTN2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnTwo')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
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

    print "MULTI VTN->VBR MULTI-CONTROLLER TEST SUCCESS"


# Main Block
if __name__ == '__main__':
  print '*****MULTI-CONTOLLER VTN_VBR TESTS******'
  test_vtn_vbr_multi_controller()
  test_vtn_multi_vbr_multi_controller()
  test_multi_vtn_with_vbr_multi_controller()

else:
  print "VTN VBR Loaded as Module"

