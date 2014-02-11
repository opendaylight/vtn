#!/usr/bin/python

#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller, vtn_vbr
import vbrif_portmap, vtn_testconfig, pexpect

VTNVBRDATA = vtn_testconfig.VTNVBRDATA
CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA
MININETDATA = vtn_testconfig.MININETDATA

coordinator_url = vtn_testconfig.coordinator_url
def_header = vtn_testconfig.coordinator_headers
controller_headers = vtn_testconfig.controller_headers
controller_url_part = vtn_testconfig.controller_url_part



def create_mininet_topology(mininet_blockname, controller_blockname, topology_depth_str):

    mininet_ip = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['mininet_ipaddr']
    mininet_user = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['mininet_username']
    ssh  = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['ssh']
    ctr_remote  = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['ctr_remote']
    topo_tree  = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['topo_tree']
    controller_ipaddr = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['ipaddr']

    topology = ssh +' ' +  mininet_user +' '+ mininet_ip +' '+ ctr_remote +controller_ipaddr +' '+ topo_tree +topology_depth_str
    ssh_newkey = 'Are you sure you want to continue connecting'
    print topology

    child = pexpect.spawn(topology)
    index = child.expect([ssh_newkey, 'password:', pexpect.EOF, pexpect.TIMEOUT])
    if index == 0:
        print "I say yes"
        child.sendline('yes')
        index = child.expect([ssh_newkey, 'password:', pexpect.EOF, pexpect.TIMEOUT])
    if index == 1:
        print "sending password"
        child.sendline('mininet')
        print "connection OK"
        return child
    elif index == 2:
        print "2:connection timeout"
        return child
    elif index == 3:
        print "3:connection timeout"
        return child

def close_topology(child):
    if child.isalive() == True :
        print "process alive\n"
        child.sendline('exit')
        child.sendline('exit')
        print "Exiting Topology"
        child.close()
        time.sleep(5)
    else:
        print "child process already closed"

def ping_mininet(child, src_host_str, dst_host_str):
    ping_cmd = src_host_str +' '+'ping -c5'+' '+dst_host_str
    if child.isalive() == True :
        print "process alive\n"
        child.sendline(ping_cmd)
        index = child.expect(['Unreachable', '64 bytes'])
        if index == 0:
            print "PING FAIL"
            return 1
        else:
            print "PING PASS"
            return 0
    else:
        print "child process closed cannot ping"
        return 1

def test_vtn_mininet_ping_demo1():

    print """TEST 1 : One vtn and one VBridge with Two Interfaces one Portmap each
    send packets between two configured host(H1,H3)"""
    print "CREATE Controller"
    print "VTNONE->VBRONE->VBRIFONE->PORTMAP"
    print "VTNONE->VBRONE->VBRIFTWO->PORTMAP"

    child = create_mininet_topology('MININETONE', 'ControllerFirst', '2')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        close_topology(child)
        exit(1)

    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 1 :Controller Create Failed"
        close_topology(child)
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
        print "controller state change failed"
        exit(1)

    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        close_topology(child)
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne', 'VbrOne', 'ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_vbrif('VtnOne', 'VbrOne', 'VbrIfOne')
    if retval != 0:
        print "VBRIFONE Create Failed"
        close_topology(child)
        exit(1)
    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfOne', 'ControllerFirst', position=0)
    if retval != 0:
        print "VBRIFONE Validate Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_vbrif('VtnOne', 'VbrOne', 'VbrIfTwo')
    if retval != 0:
        print "VBRIFTWO Create Failed"
        close_topology(child)
        exit(1)
    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfTwo', 'ControllerFirst', position=1)
    if retval != 0:
        print "VBRIFTWO Validate Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_portmap('VtnOne', 'VbrOne', 'VbrIfOne', vlan_tagged=0);
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_portmap('VtnOne', 'VbrOne', 'VbrIfTwo', vlan_tagged=0);
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne', 'VbrOne', 'VbrIfOne', 'ControllerFirst', presence="yes");
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne', 'VbrOne', 'VbrIfTwo', 'ControllerFirst', presence="yes");
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        close_topology(child)
        exit(1)

    retval = ping_mininet (child,'h1','h3')
    if retval != 0:
        print "MININET PING FAILED"

    close_topology(child)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne', 'VbrOne', 'ControllerFirst')
    if retval != 0:
        print "VBR Validate Failed"
        close_topology(child)
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne', 'ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne', 'VbrOne', 'VbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfOne', 'ControllerFirst', presence="no", position=0)
    if retval != 0:
        print "After Delete VBRIFONE Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne', 'VbrOne', 'VbrIfTwo')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfTwo', 'ControllerFirst', presence="no", position=1)
    if retval != 0:
        print "After Delete VBRIFTWO Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne', 'VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne', 'VbrOne', 'ControllerFirst', presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne', 'ControllerFirst', presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "MININET PING DEMO 1 TEST SUCCESS"

def test_vtn_mininet_ping_demo2():

    print """TEST 2 : One vtn and one VBridge with Two Interfaces one Portmap each
    send packets between two configured host(H2,H4)"""
    print "CREATE Controller"
    print "VTNONE->VBRONE->VBRIFONE->PORTMAP"
    print "VTNONE->VBRONE->VBRIFTWO->PORTMAP"

    child = create_mininet_topology('MININETONE', 'ControllerFirst', '2')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        close_topology(child)
        exit(1)

    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 1 :Controller Create Failed"
        close_topology(child)
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
        print "controller state change failed"
        exit(1)

    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        close_topology(child)
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne', 'VbrOne', 'ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_vbrif('VtnOne', 'VbrOne', 'VbrIfThree')
    if retval != 0:
        print "VBRIFONE Create Failed"
        close_topology(child)
        exit(1)
    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfThree', 'ControllerFirst', position=0)
    if retval != 0:
        print "VBRIFONE Validate Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_vbrif('VtnOne', 'VbrOne', 'VbrIfFour')
    if retval != 0:
        print "VBRIFTWO Create Failed"
        close_topology(child)
        exit(1)
    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfFour', 'ControllerFirst', position=0)
    if retval != 0:
        print "VBRIFTWO Validate Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_portmap('VtnOne', 'VbrOne', 'VbrIfThree', vlan_tagged=0);
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_portmap('VtnOne', 'VbrOne', 'VbrIfFour', vlan_tagged=0);
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne', 'VbrOne', 'VbrIfThree', 'ControllerFirst', presence="yes");
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne', 'VbrOne', 'VbrIfFour', 'ControllerFirst', presence="yes");
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        close_topology(child)
        exit(1)

    retval = ping_mininet (child,'h2','h4')
    if retval != 0:
        print "MININET PING FAILED"

    close_topology(child)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne', 'VbrOne', 'ControllerFirst')
    if retval != 0:
        print "VBR Validate Failed"
        close_topology(child)
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne', 'ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        close_topology(child)
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne', 'VbrOne', 'VbrIfThree')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfThree', 'ControllerFirst', presence="no", position=0)
    if retval != 0:
        print "After Delete VBRIFONE Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne', 'VbrOne', 'VbrIfFour')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfFour', 'ControllerFirst', presence="no", position=0)
    if retval != 0:
        print "After Delete VBRIFTWO Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne', 'VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne', 'VbrOne', 'ControllerFirst', presence="no")
    if retval != 0:
        print "After Delete VBR Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne', 'ControllerFirst', presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "MININET PING DEMO 2 TEST SUCCESS"

# Main Block
if __name__ == '__main__':
    print '*****MININET TESTS******'
    test_vtn_mininet_ping_demo1()
    test_vtn_mininet_ping_demo2()
else:
    print "MININET Loaded as Module"
