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


def test_audit_vtn_vterm_vtermif():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
      print "TEST 1 :Controller Create Failed"
      exit(1)

    print "TEST 1: Test Audit VTenant with one VTerminal one VTERMIF"
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)

    retval=vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
      print "VTN Create Failed"
      exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
      print "VTERM Create Failed"
      exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
      print "After Create VTERM Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
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
    retval = controller.wait_until_state('ControllerFirst',"down")

    if retval != 0:
      print "controller state change failed"
      exit(1)
    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
      print "VTERMIF Create Failed"
      exit(1)


    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
      print "controller valid_ip update failed"
      exit(1)
   # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
      print "After Create VTERMIF Validate Failed"
      exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
      print "VTERMIF Delete Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
      print "After Delete VTERMIF Validate Failed"
      exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
      print "VTERM/VTN Delete Failed"
      exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
      print "After Delete VTERM Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
      print "VTN Validate Failed after VTERM Deleted"
      exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
      print "VTN Delete Failed in coordinator"
      exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
      print "CONTROLLER delete failed"
      exit(1)
    print "VTN->VTERM->VTERMIF AUDIT TEST SUCCESS"

def test_audit_vtn_multi_vterm_vtermif():
    print "CREATE Controller"
    print "VTNONE->VTERMONE->VTERMIFONE/VTERMIFTHREE"
    print "VTNONE->VTERMTWO->VTERMIFTWO/VTERMIFFOUR"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
      print "TEST 3 :Controller Create Failed"
      exit(1)

    print "TEST 3 : Audit One vtn and Two VTerminals with Two Interfaces each"
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
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

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
      print "VTERMIF1 Create Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',position=0)
    if retval != 0:
      print "VTERMIF1 Validate Failed"
      exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermTwo','VTermIfOne')
    if retval != 0:
      print "VTERM2->VTERMIF1 Create Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfOne','ControllerFirst',position=0)
    if retval != 0:
      print "VTERM2->VTERMIF2 Validate Failed"
      exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"down")
    if retval != 0:
      print "controller state change failed"
      exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
      print "VTERM1->VTERMIF1 Delete Failed"
      exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermTwo','VTermIfOne')
    if retval != 0:
      print "VTERM2->VTERMIF1 Delete Failed"
      exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfThree')
    if retval != 0:
      print "VTERMIF3 Create Failed"
      exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermTwo','VTermIfTwo')
    if retval != 0:
      print "VTERM2->VTERMIF2 Create Failed"
      exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfThree','ControllerFirst',position=0)
    if retval != 0:
      print "VTERMIF3 Validate Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',position=0)
    if retval != 0:
      print "VTERM2->VTERMIF2 Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',position=0)
    if retval != 0:
      print "VTERMONE Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',position=0)
    if retval != 0:
      print "VTERMTWO Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
      print "VTN Validate Failed"
      exit(1)


    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfThree')
    if retval != 0:
      print "VTERM1->VTERMIF3 Delete Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
      print "VTERM1->VTERMIF1 Validate Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfThree','ControllerFirst',presence="no",position=0)
    if retval != 0:
      print "VTERM1->VTERMIF3 Validate Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
      print "VTERM2->VTERMIF1 Validate Failed"
      exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermTwo','VTermIfTwo')
    if retval != 0:
      print "VTERM2->VTERMIF2 Delete Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
      print "VTERM2->VTERMIF2 Validate Failed"
      exit(1)
    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
      print "VTERM1/VTN Delete Failed"
      exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermTwo')
    if retval != 0:
      print "VTERM2/VTN Delete Failed"
      exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
      print "VTERM1/VTN Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
      print "VTERM2/VTN Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
      print "VTN Validate Failed"
      exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
      print "VTN Delete Failed in coordinator"
      exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
      print "CONTROLLER delete failed"
      exit(1)
    print "VTN1->VTERM1->VTERMIF1/VTERMIF3 AND VTN1->VTERM2->VTERMIF1/VTERMIF2 AUDIT TEST SUCCESS"

def test_audit_vtn_vterm_vtermif_portmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
      print "TEST 4 :Controller Create Failed"
      exit(1)

    print "TEST 4 : Test Audit with VTenant one VTerminal one VTERMIF and One PORTMAP"
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)
    retval=vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
      print "VTN Create Failed"
      exit(1)

    retval=vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
      print "VTERM Create Failed"
      exit(1)


    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"down")
    if retval != 0:
      print "controller state change failed"
      exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
      print "VTERMIF Create Failed"
      exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermOne','VTermIfOne');
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
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "controller state change failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="yes");
    if retval != 0:
      print "Portmap Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
      print "After Create VTERM Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
      print "VTN Validate Failed"
      exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
      print "Portmap Delete Failed"
      exit(1)

    retval = vtermif_portmap.delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
      print "VTERMIF Delete Failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
      print "After Delete VTERMIF Validate Failed"
      exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
      print "VTERM/VTN Delete Failed"
      exit(1)

    retval=vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence="no")
    if retval != 0:
      print "After Delete VTERM Validate Failed"
      exit(1)

    retval=vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
      print "VTN Validate Failed after VTERM Deleted"
      exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
      print "VTN Delete Failed in coordinator"
      exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
      print "CONTROLLER delete failed"
      exit(1)
    print "VTN->VTERM->VTERMIF->PORTMAP AUDIT TEST SUCCESS"



# Main Block
if __name__ == '__main__':
   print '*****VTERMIF Portmap AUDIT cases ******'
   test_audit_vtn_vterm_vtermif()
#   test_audit_vtn_multi_vterm_vtermif()
   test_audit_vtn_vterm_vtermif_portmap()


else:
   print "VTN VTERM Loaded as Module"
