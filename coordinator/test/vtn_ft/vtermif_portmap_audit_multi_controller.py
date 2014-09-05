#!/usr/bin/python

#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller, vtn_vterm
import vtn_testconfig, vtermif_portmap

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA

def test_audit_vtn_vterm_vtermif_multi_controller():

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

    print "TEST 1: Test Audit VTenant with one VTerminal one VTERMIF with multi-controller"

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
        print "VTERM3 Create Failed"
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

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller1 invalid_ip update failed"
     exit(1)
    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst', "down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerSecond',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller2 invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond', "down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermThree','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Create Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller1 valid_ip update failed"
     exit(1)
    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['ipaddr']
    retval=controller.update_controller_ex('ControllerSecond',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller2 valid_ip update failed"
     exit(1)
    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfTwo','ControllerSecond')
    if retval != 0:
        print "After Create VTERMIF2 Validate Failed"
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

    print "VTN->VTERM->VTERMIF AUDIT WITH MULTI_CONTROLLER TEST SUCCESS"



def test_audit_vtn_vterm_vtermif_portmap_multi_controller():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 4 :Controller1 Create Failed"
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

    print "TEST 4 : VTenant with one VTerminal one VTERMIF and One PORTMAP with multi-controller"

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
        print "VTERM3 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_vtermif('VtnOne','VTermThree','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Create Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERMIF1 Validate Failed"
        exit(1)

    retval=vtermif_portmap.validate_vtermif_at_controller('VtnOne','VTermThree','VTermIfTwo','ControllerSecond')
    if retval != 0:
        print "After Create VTERMIF2 Validate Failed"
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller1 invalid_ip update failed"
     exit(1)
   # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_invalid_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['invalid_ipaddr']
    retval=controller.update_controller_ex('ControllerSecond',ipaddr=test_invalid_ipaddr)
    if retval != 0:
     print "controller2 invalid_ip update failed"
     exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"down")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "Portmap1 Create Failed"
        exit(1)

    retval=vtermif_portmap.create_portmap('VtnOne','VTermThree','VTermIfTwo');
    if retval != 0:
        print "Portmap2 Create Failed"
        exit(1)

    print "****UPDATE Controller IP to Valid****"
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval=controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller1 valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
      print "Controller state check failed"
      exit(1)

    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerSecond')['ipaddr']
    retval=controller.update_controller_ex('ControllerSecond',ipaddr=test_controller_ipaddr)
    if retval != 0:
     print "controller2 valid_ip update failed"
     exit(1)
   # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
      print "Controller state check failed"
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

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "Portmap1 Delete Failed"
        exit(1)

    retval=vtermif_portmap.delete_portmap('VtnOne','VTermThree','VTermIfTwo');
    if retval != 0:
        print "Portmap2 Delete Failed"
        exit(1)

#    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence="no");
#    if retval != 0:
#        print "After Delete Portmap1 Validate Failed"
#        exit(1)

#    retval=vtermif_portmap.validate_vtermif_portmap_at_controller('VtnOne','VTermThree','VTermIfTwo','ControllerSecond',presence="no");
#    if retval != 0:
#        print "After Delete Portmap2 Validate Failed"
#        exit(1)

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
        print "VTERM3/VTN Delete Failed"
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

    print "VTN->VTERM->VTERMIF->PORTMAP MULTI-CONTROLER TEST SUCCESS"


# Main Block
if __name__ == '__main__':
    print '*****MULTI-CONTOLLRER VTERMIF Portmap AUDIT cases ******'
    test_audit_vtn_vterm_vtermif_multi_controller()
    test_audit_vtn_vterm_vtermif_portmap_multi_controller()

else:
    print "VTN VTERM Loaded as Module"
