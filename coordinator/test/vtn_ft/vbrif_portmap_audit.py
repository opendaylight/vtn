#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#


#! /usr/bin/python

import requests, json, collections, time, controller, vtn_vbr
import vtn_testconfig, vbrif_portmap

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA

#####AUDIT TEST CASE#################

def test_audit_vtn_vbr_vbrif():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 1 :Controller Create Failed"
        exit(1)

    print "TEST 1: Test Audit VTenant with one VBridge one VBRIF"
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"down")

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Create Failed"
        exit(1)


    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VBRIF Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VBR->VBRIF AUDIT TEST SUCCESS"

def test_audit_multi_vbrif():

    print "CREATE Controller"
    print "VTNONE->VBRONE->VBRIFONE"
    print "VTNONE->VBRONE->VBRIFTWO"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 2 :Controller Create Failed"
        exit(1)

    print "TEST 2 : Test audit One vtn and one VBridge with Two Interfaces"
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"down")

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIFONE Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIFTWO Create Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=1)
    if retval != 0:
        print "VBRIFTWO Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIFONE Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIFONE Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=1)
    if retval != 0:
        print "After Delete VBRIFTWO Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VBR->VBRIF1/VBRIF2 AUDIT TEST SUCCESS"

def test_audit_vtn_multi_vbr_vbrif():
    print "CREATE Controller"
    print "VTNONE->VBRONE->VBRIFONE/VBRIFTHREE"
    print "VTNONE->VBRTWO->VBRIFTWO/VBRIFFOUR"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 3 :Controller Create Failed"
        exit(1)

    print "TEST 3 : Audit One vtn and Two VBridges with Two Interfaces each"
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")
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

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBR2->VBRIF2 Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"down")

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrTwo','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIF1 Delete Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBR2->VBRIF2 Create Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfThree','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBR2->VBRIF2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRONE Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=1)
    if retval != 0:
        print "VBRTWO Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)


    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfThree')
    if retval != 0:
        print "VBR1->VBRIF3 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfThree','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1->VBRIF3 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR2->VBRIF1 Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBR2->VBRIF2 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=1)
    if retval != 0:
        print "VBR2->VBRIF2 Validate Failed"
        exit(1)
    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR1/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR2/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=1)
    if retval != 0:
        print "VBR2/VTN Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN1->VBR1->VBRIF1/VBRIF3 AND VTN1->VBR2->VBRIF1/VBRIF2 AUDIT TEST SUCCESS"

def test_audit_vtn_vbr_vbrif_portmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 4 :Controller Create Failed"
        exit(1)

    print "TEST 4 : Test Audit with VTenant one VBridge one VBRIF and One PORTMAP"
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")
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
    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VBRIF Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"down")

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Create Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
    if retval != 0:
        print "Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no");
    if retval != 0:
        print "After Delete Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VBR->VBRIF->PORTMAP AUDIT TEST SUCCESS"


def test_audit_vtn_multi_vbr_vbrif_portmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 5 :Controller Create Failed"
        exit(1)

    print "TEST 5 : VTenant with one VBridge Two VBRIF and One PORTMAP each"
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")
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
        print "VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=1)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"down")

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="yes",position=1);
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)


    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"down")

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=1);
    if retval != 0:
        print "After Delete VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=1)
    if retval != 0:
        print "After Delete VBRIF2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN!->VBR1->VBRIF1->PORTMAP AND VTN1->VBR1->VBRIF2->PORTMAP AUDIT TEST SUCCESS"

def test_audit_vtn_vbr_multi_vbrif_portmap():
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 6 :Controller Create Failed"
        exit(1)

    print "TEST 6 : VTenant with one VBridge Two VBRIF1/VBRIF2 and One PORTMAP in VBRIF1"
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")
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
        print "VBRIF1 Create Failed"
        exit(1)

    retval=vbrif_portmap.create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=1)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"down")

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.create_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    controller.wait_until_state('ControllerFirst',"up")

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=vbrif_portmap.delete_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=vbrif_portmap.validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=1)
    if retval != 0:
        print "After Delete VBRIF2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed after VBR Deleted"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN1->VBR1->VBRIF1->PORTMAP(DELETE) AND VTN1->VBR1->VBRIF2->PORTMAP(CREATE) AUDIT TEST SUCCESS"


# Main Block
if __name__ == '__main__':
    print '*****VBRIF Portmap AUDIT cases ******'
    test_audit_vtn_vbr_vbrif()
    test_audit_multi_vbrif()
    test_audit_vtn_multi_vbr_vbrif()
    test_audit_vtn_vbr_vbrif_portmap()
    test_audit_vtn_multi_vbr_vbrif_portmap()
    test_audit_vtn_vbr_multi_vbrif_portmap()

else:
    print "VTN VBR Loaded as Module"
