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

VTNVTERMDATA = vtn_testconfig.VTNVTERMDATA
CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA

coordinator_url = vtn_testconfig.coordinator_url
def_header = vtn_testconfig.coordinator_headers
controller_headers = vtn_testconfig.controller_headers
controller_url_part = vtn_testconfig.controller_url_part

def test_vtn_vterm_multi_controller():
    print "TEST 1 : 1 Tenants with 1 VTerminal each on different controllers"
    print "CONTROLLER1->VTNONE->VTERMONE"
    print "CONTROLLER2->VTNONE->VTERMTWO"
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

    retval = vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM Create Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERMOne Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERMTwo Validate Failed"
        exit(1)

    retval  =  vtn_vterm.delete_vterm('VtnOne','VTermThree')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VtnVTERMTwo Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VtnVTERM Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
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

    print "MULTI CONTROLLER VTN->VTERM TEST SUCCESS"

def test_vtn_multi_vterm_multi_controller():
    print "TEST 2 : 1 Tenants with 2 VTerminal each on different controllers"
    print "CONTROLLER1->VTNONE->VTERMONE->VTERMTWO"
    print "CONTROLLER2->VTNTWO->VTERMTHREE->VTERMFOUR"
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
        print "VTERM1 Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTERM2 Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnTwo','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM3 Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnTwo','VTermFour','ControllerSecond')
    if retval != 0:
        print "VTERM4 Create Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerSecond')
    if retval != 0:
        print "VTN2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTERM2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermThree','ControllerSecond',position=0)
    if retval != 0:
        print "VTERM3 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermFour','ControllerSecond')
    if retval != 0:
        print "VTERM4 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM1/VTN1 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermTwo')
    if retval != 0:
        print "VTERM2/VTN1 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermThree')
    if retval != 0:
        print "VTERM3/VTN2 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermFour')
    if retval != 0:
        print "VTERM4/VTN2 Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM1/VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM2/VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VTERM3/VTN2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermFour','ControllerSecond',presence="no")
    if retval != 0:
     print "VTERM4/VTN2 Validate Failed"
     exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst', presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerSecond', presence="no")
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

    print "MULTI CONTROLLER MULTI VTERM VTN->VTERM TEST SUCCESS"

def test_multi_vtn_with_vterm_multi_controller():
    print "TEST 3 : 2 Tenants with one VTerminal each"
    print "CONTROLLER1->VTNONE->VTERMONE"
    print "CONTROLLER1->VTNTWO->VTERMONE"
    print "CONTROLLER2->VTNONE->VTERMONE"
    print "CONTROLLER2->VTNTWO->VTERMONE"
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
        print "VTERM1 Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnTwo','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM2 Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM3 Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnTwo','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM4 Create Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN1/CONTROLLER1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTN2/CONTROLLER2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "VTN1/CONTROLLER2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerSecond',position=0)
    if retval != 0:
        print "VTN2/CONTROLLER2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM1/VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM1/VTN2 Validate Failed "
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM2/VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM2/VTN2 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTERM2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermThree')
    if retval != 0:
        print "VTERM2/VTN1 Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VTERM2/VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermThree')
    if retval != 0:
        print "VTERM2/VTN2 Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VTERM2/VTN2 Validate Failed"
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
        print "CONTROLLER1 delete failed"
        exit(1)

    retval = controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "MULTI VTN->VTERM MULTI-CONTROLLER TEST SUCCESS"


# Main Block
if __name__ == '__main__':
  print '*****MULTI-CONTOLLER VTN_VTERM TESTS******'
  test_vtn_vterm_multi_controller()
  test_vtn_multi_vterm_multi_controller()
  test_multi_vtn_with_vterm_multi_controller()

else:
  print "VTN VTERM Loaded as Module"

