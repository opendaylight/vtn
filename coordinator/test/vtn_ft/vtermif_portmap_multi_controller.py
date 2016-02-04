#!/usr/bin/python

#
# Copyright (c) 2014-2016 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller, vtn_vterm
import vtn_testconfig, vtermif_portmap

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA
VTNVTERMDATA=vtn_testconfig.VTNVTERMDATA
VTERMIFDATA=vtn_testconfig.VTERMIFDATA

coordinator_url=vtn_testconfig.coordinator_url
def_header=vtn_testconfig.coordinator_headers
controller_headers=vtn_testconfig.controller_headers
controller_url_part=vtn_testconfig.controller_url_part


def test_vtn_vterm_vtermif_multi_controller():

    print "TEST 1 : VTenant with one VTerminal one VTERMIF with multi-controller"
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

    retval=vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1/VTERM1 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM3 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermThree','VTermIfOne')
    if retval != 0:
        print "VTERMIF1/VTERM3 Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERMIF1/VTERM1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfOne','ControllerSecond')
    if retval != 0:
        print "After Create VTERMIF1/VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "After Create VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF/VTERM1 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermThree','VTermIfOne')
    if retval != 0:
        print "VTERMIF/VTERM3 Delete Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF/VTERM1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF/VTERM3 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM1/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermThree')
    if retval != 0:
        print "VTERM3/VTN Delete Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VTERM1 Deleted at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VTERM3 Deleted at controller2"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
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
    print "VTN->VTERM->VTERMIF MULTI-CONTROLLER TEST SUCCESS"

def test_multi_vterm_vtermif_multi_controller():

    print "TEST 3 : One vtn and Two VTerminals with One Interfaces each with multi-controller"
    print "CONTROLLER1->VTNONE->VTERMONE->VTERMIFONE"
    print "CONTROLLER1->VTNONE->VTERMTWO->VTERMIFONE"
    print "CONTROLLER2->VTNONE->VTERMTHREE->VTERMIFONE"
    print "CONTROLLER2->VTNONE->VTERMFOUR->VTERMIFONE"
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

    retval=vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERMONE Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTERMTWO Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERMTHREE Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermFour','ControllerSecond')
    if retval != 0:
        print "VTERMFOUR Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIFONE/VTERM1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermTwo','VTermIfOne')
    if retval != 0:
        print "VTERMIFONE/VTERM2 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermThree','VTermIfTwo')
    if retval != 0:
        print "VTERMIFONE/VTERM3 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermFour','VTermIfThree')
    if retval != 0:
        print "VTERMIFONE/VTERM4 Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "VTERMIFONE/VTERM1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "VTERMIFONE/VTERM2 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfTwo','ControllerSecond')
    if retval != 0:
        print "VTERMIFONE/VTERM3 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermFour','VTermIfThree','ControllerSecond')
    if retval != 0:
        print "VTERMIFONE/VTERM4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTERMONE Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTERMTWO Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',position=0)
    if retval != 0:
        print "VTERMTHREE Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermFour','ControllerSecond',position=0)
    if retval != 0:
        print "VTERMFOUR Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controler1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed controller2"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERM1->VTERMIFONE Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermTwo','VTermIfOne')
    if retval != 0:
        print "VTERM2->VTERMIFTWO Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermThree','VTermIfTwo')
    if retval != 0:
        print "VTERM3->VTERMIFONE Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermFour','VTermIfThree')
    if retval != 0:
        print "VTERM4->VTERMIFTWO Delete Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTERM1->VTERMIFONE Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTERM2->VTERMIFTWO Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTERM3->VTERMIFONE Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermFour','VTermIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTERM4->VTERMIFTWO Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM1 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermTwo')
    if retval != 0:
        print "VTERM2 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermThree')
    if retval != 0:
        print "VTERM3 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermFour')
    if retval != 0:
        print "VTERM4 Delete Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTERM1/VTN Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTERM2/VTN Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTERM3/VTN Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTERM4/VTN Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
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

    print "VTN1->VTERM1->VTERMIF1 AND VTN1->VTERM2->VTERMIF2 MULTI_CONTROLLER TEST SUCCESS"

def test_multi_vtn_vterm_vtermif_multi_controller():

    print "TEST 4 : Two Vtn and Two VTermidges one Interfaces each with multi-controller "
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

    retval=vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN1 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN2 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnTwo','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM2 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTN1->VTERM3 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnTwo','VTermFour','ControllerSecond')
    if retval != 0:
        print "VTN1->VTERM4 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnTwo','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTN2->VTERM2->VTERMIF2 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermThree','VTermIfThree')
    if retval != 0:
        print "VTN1->VTERM3->VTERMIF3 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnTwo','VTermFour','VTermIfFour')
    if retval != 0:
        print "VTN2->VTERM4->VTERMIF4 Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfThree','ControllerSecond')
    if retval != 0:
        print "VTN1->VTERM3->VTERMIF3 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnTwo','VTermFour','VTermIfFour','ControllerSecond')
    if retval != 0:
        print "VTN1->VTERM4->VTERMIF4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTN2->VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTN1->VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermFour','ControllerSecond')
    if retval != 0:
        print "VTN2->VTERM4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed at controler1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed at controller2"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed at controller2"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnTwo','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTN2->VTERM2->VTERMIF2 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermThree','VTermIfThree')
    if retval != 0:
        print "VTN1->VTERM3->VTERMIF3 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnTwo','VTermFour','VTermIfFour')
    if retval != 0:
        print "VTN2->VTERM3->VTERMIF4 Delete Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN2->VTERM2->VTERMIF2 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTN1->VTERM3->VTERMIF3 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTN2->VTERM4->VTERMIF4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTN2->VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTN1->VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermFour','ControllerSecond')
    if retval != 0:
        print "VTN2->VTERM4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "Before VTERM1 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "Before VTERM2 Delete VTN2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "Before VTERM3 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerSecond',position=0)
    if retval != 0:
        print "Before VTERM4 Delete VTN2 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTN1->VTERM1 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermTwo')
    if retval != 0:
        print "VTN2->VTERM2 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermThree')
    if retval != 0:
        print "VTN1->VTERM3 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermFour')
    if retval != 0:
        print "VTN2->VTERM4 Delete Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN1->VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN2->VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN1->VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermFour','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN2->VTERM4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After VTERM1 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After VTERM2 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After VTERM3 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After VTERM4 Delete VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN1 Delete Failed in coordinator"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnTwo')
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

    print "VTN1->VTERM1->VTERMIF1 and VTN2->VTERM2->VTERMIF2 MULTI_CONTROLLER TEST SUCCESS"

def test_vtn_vterm_vtermif_portmap_multi_controller():

    print "TEST 6 : VTenant with one VTerminal one VTERMIF and One PORTMAP with multi-controller"
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

    retval=vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM1 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM2 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermThree','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "Portmap1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermThree','VTermIfTwo');
    if retval != 0:
        print "Portmap2 Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfTwo','ControllerSecond')
    if retval != 0:
        print "After Create VTERMIF2 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="yes");
    if retval != 0:
        print "Portmap1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermThree','VTermIfTwo','ControllerSecond',presence="yes");
    if retval != 0:
        print "Portmap2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "After Create VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "Portmap1 Delete Failed"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermThree','VTermIfTwo');
    if retval != 0:
        print "Portmap2 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermThree','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Delete Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF2 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM1/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermThree')
    if retval != 0:
        print "VTERM2/VTN Delete Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VTERM Deleted at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VTERM Deleted at controller2"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
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

    print "VTN->VTERM->VTERMIF->PORTMAP MULTI-CONTROLER TEST SUCCESS"

def test_vtn_multi_vterm_single_vtermif_portmap_multi_controller():

    print "TEST 9 : VTenant with Two VTerminal Two VTERMIF and One PORTMAP each multi_controller"
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

    retval=vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM1 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTERM2 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM3 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermFour','ControllerSecond')
    if retval != 0:
        print "VTERM4 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermThree','VTermIfThree')
    if retval != 0:
        print "VTERMIF3 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermFour','VTermIfFour')
    if retval != 0:
        print "VTERMIF4 Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTERMIF2 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTERMIF3 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermFour','VTermIfFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTERMIF4 Validate Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "VTERMIF1 Portmap Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermTwo','VTermIfTwo');
    if retval != 0:
        print "VTERMIF2 Portmap Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermThree','VTermIfThree');
    if retval != 0:
        print "VTERMIF3 Portmap Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermFour','VTermIfFour');
    if retval != 0:
        print "VTERMIF4 Portmap Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VTERMIF1 Portmap Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VTERMIF2 Portmap Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermThree','VTermIfThree','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VTERMIF3 Portmap Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermFour','VTermIfFour','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VTERMIF4 Portmap Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTERM4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "VTERMIF1 Portmap Delete Failed"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermTwo','VTermIfTwo');
    if retval != 0:
        print "VTERMIF2 Portmap Delete Failed"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermThree','VTermIfThree');
    if retval != 0:
        print "VTERMIF3 Portmap Delete Failed"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermFour','VTermIfFour');
    if retval != 0:
        print "VTERMIF4 Portmap Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermThree','VTermIfThree')
    if retval != 0:
        print "VTERMIF3 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermFour','VTermIfFour')
    if retval != 0:
        print "VTERMIF4 Delete Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF2 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF3 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermFour','VTermIfFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF4 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM1/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermTwo')
    if retval != 0:
        print "VTERM2/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermThree')
    if retval != 0:
        print "VTERM3/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermFour')
    if retval != 0:
        print "VTERM4/VTN Delete Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERM4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VTERM Deleted at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VTERM Deleted at controller2"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
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

    print "VTN1->VTERM1->VTERMIF1->PORTMAP AND VTN1->VTERM2->VTERMIF2->PORTMAP MULTI-CONTROLLER TEST SUCCESS"


def test_multi_vtn_vterm_vtermif_portmap_multi_controller():

    print "TEST 10 : Two VTenant with Two VTerminal Two VTERMIF and One PORTMAP each with multi-controller"
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

    retval=vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vterm.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM1 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnTwo','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTERM2 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermThree','ControllerSecond')
    if retval != 0:
        print "VTERM3 Create Failed"
        exit(1)

    retval=vtn_vterm.create_vterm('VtnTwo','VTermFour','ControllerSecond')
    if retval != 0:
        print "VTERM4 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnTwo','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermThree','VTermIfThree')
    if retval != 0:
        print "VTERMIF3 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnTwo','VTermFour','VTermIfFour')
    if retval != 0:
        print "VTERMIF4 Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTERMIF2 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTERMIF3 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnTwo','VTermFour','VTermIfFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTERMIF4 Validate Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "VTERMIF1 Portmap Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnTwo','VTermTwo','VTermIfTwo');
    if retval != 0:
        print "VTERMIF2 Portmap Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermThree','VTermIfThree');
    if retval != 0:
        print "VTERMIF3 Portmap Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnTwo','VTermFour','VTermIfFour');
    if retval != 0:
        print "VTERMIF4 Portmap Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VTERMIF1 Portmap Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VTERMIF2 Portmap Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermThree','VTermIfThree','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VTERMIF3 Portmap Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnTwo','VTermFour','VTermIfFour','ControllerSecond',presence="yes",position=0);
    if retval != 0:
        print "VTERMIF4 Portmap Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermFour','ControllerSecond',position=0)
    if retval != 0:
        print "After Create VTERM4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN1 Validate Failed at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTN2 Validate Failed at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',position=0)
    if retval != 0:
        print "VTN1 Validate Failed at controller2"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerSecond',position=0)
    if retval != 0:
        print "VTN2 Validate Failed at controller2"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "VTERM1->Portmap Delete Failed"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnTwo','VTermTwo','VTermIfTwo');
    if retval != 0:
        print "VTERM2->Portmap Delete Failed"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermThree','VTermIfThree');
    if retval != 0:
        print "VTERM3->Portmap Delete Failed"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnTwo','VTermFour','VTermIfFour');
    if retval != 0:
        print "VTERM4->Portmap Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnTwo','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermThree','VTermIfThree')
    if retval != 0:
        print "VTERMIF3 Delete Failed"
        exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnTwo','VTermFour','VTermIfFour')
    if retval != 0:
        print "VTERMIF4 Delete Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF2 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF3 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnTwo','VTermFour','VTermIfFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERMIF4 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM1/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermTwo')
    if retval != 0:
        print "VTERM2/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermThree')
    if retval != 0:
        print "VTERM3/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermFour')
    if retval != 0:
        print "VTERM4/VTN Delete Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERM1 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERM2 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermThree','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERM3 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermFour','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "After Delete VTERM4 Validate Failed"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN1 Validate Failed after VTERM Deleted at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN2 Validate Failed after VTERM Deleted at controller1"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTN1 Validate Failed after VTERM Deleted at controller2"
        exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerSecond',presence="no",position=0)
    if retval != 0:
        print "VTN2 Validate Failed after VTERM Deleted at controller2"
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
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)
    print "VTN1->VTERM1->VTERMIF1->PORTMAP AND VTN2->VTERM2->VTERMIF2->PORTMAP MULTI-CONTROLLER TEST SUCCESS"


# Main Block
if __name__ == '__main__':
    print '*****MULTI-CONTROLLER VTERMIF TESTS******'
    test_vtn_vterm_vtermif_multi_controller()
    test_multi_vterm_vtermif_multi_controller()
    test_multi_vtn_vterm_vtermif_multi_controller()
    test_vtn_vterm_vtermif_portmap_multi_controller()
    test_vtn_multi_vterm_single_vtermif_portmap_multi_controller()
    test_multi_vtn_vterm_vtermif_portmap_multi_controller()

else:
    print "VTERMIF_PORTMAP Loaded as Module"
