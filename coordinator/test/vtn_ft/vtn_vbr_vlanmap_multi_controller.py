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
import vtn_testconfig, vtn_vbr_vlanmap

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA
VTNVBRDATA=vtn_testconfig.VTNVBRDATA
VLANMAPDATA=vtn_testconfig.VLANMAPDATA

coordinator_url=vtn_testconfig.coordinator_url
def_header=vtn_testconfig.coordinator_headers
controller_headers=vtn_testconfig.controller_headers
controller_url_part=vtn_testconfig.controller_url_part

def test_vtn_vbr_vlanmap_multi_controller():

    print "TEST 1 : VTenant with one VBridge one VLANMAP without vlan_id and logicalport_id with multi-controller"
    print "CREATE Controller1"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 1 :Controller Create Failed"
        exit(1)
    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "CREATE Controller2"
    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 1 :Controller Create Failed"
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
        print "VBR Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrThree','VlanmapTwo',no_vlan=0)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=0)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapTwo','ControllerSecond',no_vlan_id=0)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=0)
    if retval != 0:
        print "VLANMAP update Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrThree','VlanmapTwo',update_id=0)
    if retval != 0:
        print "VLANMAP update Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrThree','VlanmapTwo',no_vlan=0)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapTwo','ControllerSecond',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER1"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "DELETE CONTROLLER2"
    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VBR->VLANMAP no vlan_id with multi-controller TEST SUCCESS"

def test_vtn_vbr_vlanmap_with_vlanid_multi_controller():

    print "TEST 2 : VTenant with one VBridge one VLANMAP with vlan_id and logicalport_id with multi-controller"
    print "CREATE Controller1"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 2 :Controller Create Failed"
        exit(1)
    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "CREATE Controller2"
    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 2 :Controller Create Failed"
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
        print "VBR Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrThree','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapTwo','ControllerSecond',no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=1)
    if retval != 0:
        print "VLANMAP update Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrThree','VlanmapTwo',update_id=1)
    if retval != 0:
        print "VLANMAP update Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrThree','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapTwo','ControllerSecond',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER1"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "DELETE CONTROLLER2"
    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "VTN->VBR->VLANMAP with vlan_id with multi-controller TEST SUCCESS"

def test_vtn_vbr_multi_vlanmap_multi_controller():

    print "TEST 3 : VTenant with one VBridge two VLANMAP with multi-controller"
    print "CREATE Controller1"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 3 :Controller1 Create Failed"
        exit(1)
    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "CREATE Controller2"
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
        print "VBROne Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBRTwo Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP1 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrThree','VlanmapThree',no_vlan=0)
    if retval != 0:
        print "VLANMAP3 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',position=0,no_vlan_id=0)
    if retval != 0:
        print "After Create VLANMAP1 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapThree','ControllerSecond',position=0,no_vlan_id=0)
    if retval != 0:
        print "After Create VLANMAP3 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=0)
    if retval != 0:
        print "VLANMAP1 update Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrThree','VlanmapThree',update_id=0)
    if retval != 0:
        print "VLANMAP3 update Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrOne','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP2 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrThree','VlanmapFour',no_vlan=1)
    if retval != 0:
        print "VLANMAP2 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapTwo','ControllerFirst',position=0,no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP2 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapFour','ControllerSecond',position=0,no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP2 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrOne','VlanmapTwo',update_id=1)
    if retval != 0:
        print "VLANMAP2 upadte Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrThree','VlanmapFour',update_id=1)
    if retval != 0:
        print "VLANMAP2 upadte Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP1 Delete Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrOne','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP2 Delete Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrThree','VlanmapThree',no_vlan=0)
    if retval != 0:
        print "VLANMAP3 Delete Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrThree','VlanmapFour',no_vlan=1)
    if retval != 0:
        print "VLANMAP4 Delete Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP1 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapTwo','ControllerFirst',presence="no",position=1,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP2 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapThree','ControllerSecond',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP3 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapFour','ControllerSecond',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP4 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrThree')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBROne Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBRTwo Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER1"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    print "DELETE CONTROLLER2"
    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN->VBR->TWO VLANMAP with multi_controller TEST SUCCESS"


def test_vtn_vbr_vlanmap_vlanid_multi_controller():

    print "TEST 4 : VTenant with one VBridge one VLANMAP only with vlan_id with multi-controller"
    print "CREATE Controller1"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 4 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "CREATE Controller2"
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
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBROne Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "VBRTwo Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=2)
    if retval != 0:
        print "VLANMAP1 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrThree','VlanmapTwo',no_vlan=3)
    if retval != 0:
        print "VLANMAP2 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=2)
    if retval != 0:
        print "After Create VLANMAP1 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapTwo','ControllerSecond',no_vlan_id=3)
    if retval != 0:
        print "After Create VLANMAP2 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=0)
    if retval != 0:
        print "VLANMAP1 update Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrThree','VlanmapTwo',update_id=1)
    if retval != 0:
        print "VLANMAP2 update Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBROne Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "After Create VBRTwo Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN controller1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN controller2 Validate Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP1 Delete Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrThree','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP2 Delete Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapTwo','ControllerSecond',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
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
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER1"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    print "DELETE CONTROLLER2"
    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN->VBR->VLANMAP with vlan_id with multi-controller TEST SUCCESS"


def test_vtn_vbr_vlanmap_lg_id_multi_controller():

    print "TEST 5 : VTenant with one VBridge one VLANMAP no_vlan_id and logicalport_id with multi-controller"
    print "CREATE Controller1"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 5 :Controller1 Create Failed"
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    print "CREATE Controller2"
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
        print "VBROne Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrTwo','ControllerSecond')
    if retval != 0:
        print "VBRTwo Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=4)
    if retval != 0:
        print "VLANMAP1 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrTwo','VlanmapTwo',no_vlan=5)
    if retval != 0:
        print "VLANMAP2 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=4)
    if retval != 0:
        print "After Create VLANMAP1 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrTwo','VlanmapTwo','ControllerSecond',no_vlan_id=5)
    if retval != 0:
        print "After Create VLANMAP2 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=1)
    if retval != 0:
        print "VLANMAP1 upadte Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrTwo','VlanmapTwo',update_id=2)
    if retval != 0:
        print "VLANMAP2 upadte Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerSecond')
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

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP1 Delete Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrTwo','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP2 Delete Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP1 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrTwo','VlanmapTwo','ControllerSecond',presence="no",position=0,no_vlan_id=2)
    if retval != 0:
        print "After Delete VLANMAP2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTNOne Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR/VTNTwo Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerSecond',presence="no")
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

    print "DELETE CONTROLLER1"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        exit(1)

    print "DELETE CONTROLLER2"
    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)

    print "VTN->VBR->VLANMAP with no_vlan_id and logicalport_id with multi-controller TEST SUCCESS"


def test_vtn_vbr_vlanmap_no_vlanid_multi_controller():

    print "TEST 6 : VTenant with one VBridge one VLANMAP with vlan_id and logicalport_id"
    print                "then update vlan_id to no_vlan_id with multi-controller"
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
        print "VBR3 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP1 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.create_vlanmap('VtnOne','VbrThree','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP2 Create Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP1 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapTwo','ControllerSecond',no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP2 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=2)
    if retval != 0:
        print "VLANMAP1 upadte Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.update_vlanmap('VtnOne','VbrThree','VlanmapTwo',update_id=2)
    if retval != 0:
        print "VLANMAP2 upadte Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrThree','ControllerSecond')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed at controller1"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerSecond')
    if retval != 0:
        print "VTN Validate Failed at controller2"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP1 Delete Failed"
        exit(1)

    retval = vtn_vbr_vlanmap.delete_vlanmap('VtnOne','VbrThree','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP3 Delete Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP1 Validate Failed"
        exit(1)

    retval=vtn_vbr_vlanmap.validate_vlanmap_at_controller('VtnOne','VbrThree','VlanmapTwo','ControllerSecond',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP3 Validate Failed"
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
    print "VTN->VBR->VLANMAP with vlan_id->no_vlan_id with multi-controller TEST SUCCESS"

if __name__ == '__main__':
  print "MULTI-CONTROLLER VLANMAP TEST"
  test_vtn_vbr_vlanmap_multi_controller()
  test_vtn_vbr_vlanmap_with_vlanid_multi_controller()
  test_vtn_vbr_multi_vlanmap_multi_controller()
  test_vtn_vbr_vlanmap_vlanid_multi_controller()
  test_vtn_vbr_vlanmap_lg_id_multi_controller()
  test_vtn_vbr_vlanmap_no_vlanid_multi_controller()
else:
    print "VLANMAP Loaded as Module"
