#!/usr/bin/python

#
# Copyright (c) 2013-2016 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller, vtn_vbr
import vtn_testconfig, vbrif_portmap

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA
VTNVBRDATA=vtn_testconfig.VTNVBRDATA
VBRIFDATA=vtn_testconfig.VBRIFDATA

coordinator_url=vtn_testconfig.coordinator_url
def_header=vtn_testconfig.coordinator_headers
controller_headers=vtn_testconfig.controller_headers
controller_url_part=vtn_testconfig.controller_url_part


def test_vtn_vbr_vbrif_multi_controller():

    print "TEST 1 : VTenant with one VBridge one VBRIF with multi-controller"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 1 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 1 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1/VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBRIF1/VBR3 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VBRIF1/VBR1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond')
    if retval != 0:
        print "After Create VBRIF1/VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "After Create VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF/VBR1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBRIF/VBR3 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF/VBR1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF/VBR3 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR1 Deleted at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR3 Deleted at controller2"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)
    print "VTN->VBR->VBRIF MULTI-CONTROLLER TEST SUCCESS"

def test_multi_vbrif_muti_controller():

    print "TEST 2 : One vtn and one VBridge with Two Interfaces with multi-controller"
    print "VTNONE->VBRONE->VBRIFONE"
    print "VTNONE->VBRONE->VBRIFTWO"
    print "VTNONE->VBRTHREE->VBRIFONE"
    print "VTNONE->VBRTHREE->VBRIFTWO"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 2 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 2 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)


    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIFONE/VBR1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIFTWO/VBR1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBRIFONE/VBR3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBRIFTWO/VBR3 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIFONE/VBR1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIFTWO/VBR1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',position=0)
    if retval != 0:
        print "VBRIFONE/VBR3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',position=0)
    if retval != 0:
        print "VBRIFTWO/VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controler1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VTN1->VBR3->VBRIF1 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIFONE/VBR1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIFONE/VBR3 Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VTN1->VBR1->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VTN1->VBR3->VBRIF2 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIFTWO/VBR1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIFTWO/VBR3 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN->VBR->VBRIF1/VBRIF2 MULTI_CONTROLLER TEST SUCCESS"


def test_multi_vbr_vbrif_multi_controller():

    print "TEST 3 : One vtn and Two VBridges with One Interfaces each with multi-controller"
    print "CONTROLLER1->VTNONE->VBRONE->VBRIFONE"
    print "CONTROLLER1->VTNONE->VBRTWO->VBRIFONE"
    print "CONTROLLER2->VTNONE->VBRTHREE->VBRIFONE"
    print "CONTROLLER2->VTNONE->VBRFOUR->VBRIFONE"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 3 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 3 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBRONE Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VBRTWO Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBRTHREE Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrFour','ControllerSecond')
    if retval != 0:
        print "VBRFOUR Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIFONE/VBR1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfOne')
    if retval != 0:
        print "VBRIFONE/VBR2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBRIFONE/VBR3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrFour','VbrIfThree')
    if retval != 0:
        print "VBRIFONE/VBR4 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "VBRIFONE/VBR1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "VBRIFONE/VBR2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond')
    if retval != 0:
        print "VBRIFONE/VBR3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfThree','ControllerSecond')
    if retval != 0:
        print "VBRIFONE/VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRONE Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBRTWO Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',position=0)
    if retval != 0:
        print "VBRTHREE Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrFour','ControllerSecond',position=0)
    if retval != 0:
        print "VBRFOUR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controler1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed controller2"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIFONE Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrTwo','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIFTWO Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR3->VBRIFONE Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrFour','VbrIfThree')
    if retval != 0:
        print "VBR4->VBRIFTWO Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1->VBRIFONE Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR2->VBRIFTWO Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR3->VBRIFONE Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR4->VBRIFTWO Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR2 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrFour')
    if retval != 0:
        print "VBR4 Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR2/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR3/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR4/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN1->VBR1->VBRIF1 AND VTN1->VBR2->VBRIF2 MULTI_CONTROLLER TEST SUCCESS"

def test_multi_vtn_vbr_vbrif_multi_controller():

    print "TEST 4 : Two Vtn and Two Vbridges one Interfaces each with multi-controller "
    print "TEST 4 :CREATE Controller"
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
      print "TEST 4 :Controller2 Create Failed"
      exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN2 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnTwo','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR2 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VTN1->VBR3 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnTwo','VbrFour','ControllerSecond')
    if retval != 0:
        print "VTN1->VBR4 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnTwo','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VTN2->VBR2->VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfThree')
    if retval != 0:
        print "VTN1->VBR3->VBRIF3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnTwo','VbrFour','VbrIfFour')
    if retval != 0:
        print "VTN2->VBR4->VBRIF4 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond')
    if retval != 0:
        print "VTN1->VBR3->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnTwo','VbrFour','VbrIfFour','ControllerSecond')
    if retval != 0:
        print "VTN1->VBR4->VBRIF4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VTN2->VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VTN1->VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrFour','ControllerSecond')
    if retval != 0:
        print "VTN2->VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed at controler1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed at controller2"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed at controller2"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnTwo','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VTN2->VBR2->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfThree')
    if retval != 0:
        print "VTN1->VBR3->VBRIF3 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnTwo','VbrFour','VbrIfFour')
    if retval != 0:
        print "VTN2->VBR3->VBRIF4 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN2->VBR2->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTN1->VBR3->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTN2->VBR4->VBRIF4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VTN2->VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VTN1->VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrFour','ControllerSecond')
    if retval != 0:
        print "VTN2->VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "Before VBR1 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "Before VBR2 Delete VTN2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "Before VBR3 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerSecond',position=0)
    if retval != 0:
        print "Before VBR4 Delete VTN2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VTN1->VBR1 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrTwo')
    if retval != 0:
        print "VTN2->VBR2 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VTN1->VBR3 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrFour')
    if retval != 0:
        print "VTN2->VBR4 Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN1->VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN2->VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN1->VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrFour','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN2->VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After VBR1 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After VBR2 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After VBR3 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After VBR4 Delete VTN1 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN1 Delete Failed in coordinator"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnTwo')
    if retval != 0:
        print "VTN2 Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN1->VBR1->VBRIF1 and VTN2->VBR2->VBRIF2 MULTI_CONTROLLER TEST SUCCESS"

def test_vtn_multi_vbr_vbrif_muli_controller():
    print "TEST 5 : One vtn and Two VBridges with Two Interfaces each with multi-controller"
    print "CONTROLLER1->VTNONE->VBRONE->VBRIFONE/VBRIFTHREE"
    print "CONTROLLER1->VTNONE->VBRTWO->VBRIFTWO/VBRIFFOUR"
    print "CONTROLLER2->VTNONE->VBRTHREE->VBRIFONE/VBRIFTHREE"
    print "CONTROLLER3->VTNONE->VBRFOUR->VBRIFTWO/VBRIFFOUR"
    print "CREATE Controller"

    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 5 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 5 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VBR2 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrFour','ControllerSecond')
    if retval != 0:
        print "VBR4 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBR1->VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfThree')
    if retval != 0:
        print "VBR2->VBRIF3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfFour')
    if retval != 0:
        print "VBR2->VBRIF4 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBR3->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR3->VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrFour','VbrIfThree')
    if retval != 0:
        print "VBR4->VBRIF3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrFour','VbrIfFour')
    if retval != 0:
        print "VBR4->VBRIF4 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBR1>VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',position=0)
    if retval != 0:
        print "VBR2->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',position=0)
    if retval != 0:
        print "VBR2->VBRIF4 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',position=0)
    if retval != 0:
        print "VBR3->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',position=0)
    if retval != 0:
        print "VBR3>VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfThree','ControllerSecond',position=0)
    if retval != 0:
        print "VBR4->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',position=0)
    if retval != 0:
        print "VBR4->VBRIF4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRONE Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBRTWO Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',position=0)
    if retval != 0:
        print "VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrFour','ControllerSecond',position=0)
    if retval != 0:
        print "VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBR1->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrTwo','VbrIfThree')
    if retval != 0:
        print "VBR2->VBRIF3 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrTwo','VbrIfFour')
    if retval != 0:
        print "VBR2->VBRIF4 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBR3->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR3->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrFour','VbrIfThree')
    if retval != 0:
        print "VBR4->VBRIF3 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrFour','VbrIfFour')
    if retval != 0:
        print "VBR4->VBRIF4 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR2->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR2->VBRIF4 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR3->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR3->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR4->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR4->VBRIF4 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR2/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrFour')
    if retval != 0:
        print "VBR4/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR2/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR3/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VBR4/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN1->VBR1->VBRIF1/VBRIF3 AND VTN1->VBR2->VBRIF1/VBRIF2 MULTI_CONTROLLER TEST SUCCESS"

def test_vtn_vbr_vbrif_portmap_multi_controller():

    print "TEST 6 : VTenant with one VBridge one VBRIF and One PORTMAP with multi-controller"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 6 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 6 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfTwo');
    if retval != 0:
        print "Portmap2 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond')
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
    if retval != 0:
        print "Portmap1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="yes");
    if retval != 0:
        print "Portmap2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "After Create VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap1 Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrThree','VbrIfTwo');
    if retval != 0:
        print "Portmap2 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no");
    if retval != 0:
        print "After Delete Portmap1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerSecond',presence="no");
    if retval != 0:
        print "After Delete Portmap2 Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR2/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted at controller2"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN->VBR->VBRIF->PORTMAP MULTI-CONTROLER TEST SUCCESS"

def test_vtn_multi_vbr_vbrif_portmap_multi_controller():

    print "TEST 7 : VTenant with one VBridge Two VBRIF and One PORTMAP each with multi-controller"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 7 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 7 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBR1->VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR2->VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBR1->VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "VBR1->VBRIF2 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfOne');
    if retval != 0:
        print "VBR2->VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfTwo');
    if retval != 0:
        print "VBR2->VBRIF2 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR2->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR2->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBR1->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBR1->VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBR2->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBR2->VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBR1->VBRIF1 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "VBR1->VBRIF2 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrThree','VbrIfOne');
    if retval != 0:
        print "VBR2->VBRIF1 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrThree','VbrIfTwo');
    if retval != 0:
        print "VBR2->VBRIF2 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR1>VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR1->VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR1->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR2->VBRIF2 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBR1->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR2->VBRIF2 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2->VBRIF2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR2/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted at controller2"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN1->VBR1->VBRIF1->PORTMAP AND VTN1->VBR1->VBRIF2->PORTMAP TEST SUCCESS"


def test_vtn_vbr_multi_vbrif_portmap_multi_controller():

    print "TEST 8 : VTenant with one VBridge Two VBRIF/VBRIF2 and One PORTMAP in VBRIF1 with multi-controller"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 8 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 8 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBR1->VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR2->VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR2->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR2->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBR1->VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfOne');
    if retval != 0:
        print "VBR2->VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBR1->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBR2->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBR1->VBRIF1 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrThree','VbrIfOne');
    if retval != 0:
        print "VBR2->VBRIF1 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR1->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR2->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBR1->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR2->VBRIF2 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2->VBRIF2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR2/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR1 Deleted"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR2 Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN1->VBR1->VBRIF1->PORTMAP AND VTN1->VBR1->VBRIF2 MULTI_CONTROLLER TEST SUCCESS"

def test_vtn_multi_vbr_single_vbrif_portmap_multi_controller():

    print "TEST 9 : VTenant with Two VBridge Two VBRIF and One PORTMAP each multi_controller"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 9 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 9 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VBR2 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrFour','ControllerSecond')
    if retval != 0:
        print "VBR4 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrFour','VbrIfFour')
    if retval != 0:
        print "VBRIF4 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBRIF4 Validate Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfThree');
    if retval != 0:
        print "VBRIF3 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrFour','VbrIfFour');
    if retval != 0:
        print "VBRIF4 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBRIF4 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrTwo','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrThree','VbrIfThree');
    if retval != 0:
        print "VBRIF3 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrFour','VbrIfFour');
    if retval != 0:
        print "VBRIF4 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF4 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrFour','VbrIfFour')
    if retval != 0:
        print "VBRIF4 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF4 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR2/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrFour')
    if retval != 0:
        print "VBR4/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted at controller2"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN1->VBR1->VBRIF1->PORTMAP AND VTN1->VBR2->VBRIF2->PORTMAP MULTI-CONTROLLER TEST SUCCESS"


def test_multi_vtn_vbr_vbrif_portmap_multi_controller():

    print "TEST 10 : Two VTenant with Two VBridge Two VBRIF and One PORTMAP each with multi-controller"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 10 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 10 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnTwo','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VBR2 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnTwo','VbrFour','ControllerSecond')
    if retval != 0:
        print "VBR4 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnTwo','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnTwo','VbrFour','VbrIfFour')
    if retval != 0:
        print "VBRIF4 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnTwo','VbrFour','VbrIfFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBRIF4 Validate Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnTwo','VbrTwo','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfThree');
    if retval != 0:
        print "VBRIF3 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnTwo','VbrFour','VbrIfFour');
    if retval != 0:
        print "VBRIF4 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnTwo','VbrFour','VbrIfFour','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBRIF4 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN1 Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTN2 Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "VTN1 Validate Failed at controller2"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerSecond',position=0)
    if retval != 0:
        print "VTN2 Validate Failed at controller2"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBR1->Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnTwo','VbrTwo','VbrIfTwo');
    if retval != 0:
        print "VBR2->Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrThree','VbrIfThree');
    if retval != 0:
        print "VBR3->Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnTwo','VbrFour','VbrIfFour');
    if retval != 0:
        print "VBR4->Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnTwo','VbrFour','VbrIfFour','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF4 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnTwo','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnTwo','VbrFour','VbrIfFour')
    if retval != 0:
        print "VBRIF4 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnTwo','VbrFour','VbrIfFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF4 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrTwo')
    if retval != 0:
        print "VBR2/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrFour')
    if retval != 0:
        print "VBR4/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN1 Validate Failed after VBR Deleted at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN2 Validate Failed after VBR Deleted at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTN1 Validate Failed after VBR Deleted at controller2"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTN2 Validate Failed after VBR Deleted at controller2"
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
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)
    print "VTN1->VBR1->VBRIF1->PORTMAP AND VTN2->VBR2->VBRIF2->PORTMAP MULTI-CONTROLLER TEST SUCCESS"

def test_vtn_vbr_multiple_vbrif_portmap_multi_controller():

    print "TEST 11 : VTenant with Two VBridge Four VBRIF and One PORTMAP each with multi-controller"
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 11 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 11 :Controller2 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VBR2 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR3 Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrFour','ControllerSecond')
    if retval != 0:
        print "VBR4 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfFour')
    if retval != 0:
        print "VBRIF4 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBR3->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR3->VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrFour','VbrIfThree')
    if retval != 0:
        print "VBR3->VBRIF3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrFour','VbrIfFour')
    if retval != 0:
        print "VBR4->VBRIF4 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR2->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR2->VBRIF4 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR3->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR3->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR4->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR4->VBRIF4 Validate Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBR1->VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "VBR1->VBRIF2 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfThree');
    if retval != 0:
        print "VBR2->VBRIF3 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrTwo','VbrIfFour');
    if retval != 0:
        print "VBR2->VBRIF4 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfOne');
    if retval != 0:
        print "VBR3->VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrThree','VbrIfTwo');
    if retval != 0:
        print "VBR3->VBRIF2 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrFour','VbrIfThree');
    if retval != 0:
        print "VBR4->VBRIF3 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrFour','VbrIfFour');
    if retval != 0:
        print "VBR4->VBRIF4 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBR1->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBR1->VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBR2->VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBR2->VBRIF4 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBR3->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBR3->VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrFour','VbrIfThree','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBR4->VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VBR4->VBRIF4 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBR1->VBRIF1->Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "VBR1->VBRIF1->Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrTwo','VbrIfThree');
    if retval != 0:
        print "VBR2->VBRIF3->VbrIfThree Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrTwo','VbrIfFour');
    if retval != 0:
        print "VBR2->VBRIF4->VbrIfFour Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrThree','VbrIfOne');
    if retval != 0:
        print "VBR3->VBRIF1->Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrThree','VbrIfTwo');
    if retval != 0:
        print "VBR3->VBRIF2->Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrFour','VbrIfThree');
    if retval != 0:
        print "VBR4->VBRIF3 Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrFour','VbrIfFour');
    if retval != 0:
        print "VBR4->VBRIF4-> Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR1->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR1->VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR2->VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR2->VBRIF4 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR3->VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR3->VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrFour','VbrIfThree','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR4->VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',presence="no",position=0);
    if retval != 0:
        print "After Delete VBR4->VBRIF4 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBR1->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrTwo','VbrIfThree')
    if retval != 0:
        print "VBR2->VBRIF3 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrTwo','VbrIfFour')
    if retval != 0:
        print "VBR2->VBRIF4 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfOne')
    if retval != 0:
        print "VBR3->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrThree','VbrIfTwo')
    if retval != 0:
        print "VBR3->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrFour','VbrIfThree')
    if retval != 0:
        print "VBR4->VBRIF3 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrFour','VbrIfFour')
    if retval != 0:
        print "VBR4->VBRIF4 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2->VBRIF4 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR3->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrThree','VbrIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR3->VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR4->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrFour','VbrIfFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR4->VBRIF4 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR2/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR3/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrFour')
    if retval != 0:
        print "VBR4/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR3 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR4 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted at controler1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted at controler2"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN1->VBR1->VBRIF1/VBRIF2->PORTMAP AND VTN2->VBR2->VBRIF3/VBRIF4->PORTMAP MULTI-CONTROLLER TEST SUCCESS"

# Main Block
if __name__ == '__main__':
    print '*****MULTI-CONTROLLER VBRIF TESTS******'
    test_vtn_vbr_vbrif_multi_controller()
    test_multi_vbrif_muti_controller()
    test_multi_vbr_vbrif_multi_controller()
    test_multi_vtn_vbr_vbrif_multi_controller()
    test_vtn_multi_vbr_vbrif_muli_controller()
    test_vtn_vbr_vbrif_portmap_multi_controller()
    test_vtn_multi_vbr_vbrif_portmap_multi_controller()
    test_vtn_vbr_multi_vbrif_portmap_multi_controller()
    test_vtn_multi_vbr_single_vbrif_portmap_multi_controller()
    test_multi_vtn_vbr_vbrif_portmap_multi_controller()
    test_vtn_vbr_multiple_vbrif_portmap_multi_controller()

else:
    print "VBRIF_PORTMAP Loaded as Module"
