#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#


#! /usr/bin/python

import requests, json, collections, time, controller, vtn_vbr
import vbrif_portmap, vtn_testconfig, mininet_test, pexpect

VTNVBRDATA = vtn_testconfig.VTNVBRDATA
CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA
MININETDATA = vtn_testconfig.MININETDATA

coordinator_url = vtn_testconfig.coordinator_url
def_header = vtn_testconfig.coordinator_headers
controller_headers = vtn_testconfig.controller_headers
controller_url_part = vtn_testconfig.controller_url_part

def create_multicontroller_mininet_topology(mininet_blockname):

    mininet_ip = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['mininet_ipaddr']
    mininet_user = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['mininet_username']
    ssh  = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['ssh']
    file_name  = vtn_testconfig.ReadValues(MININETDATA, mininet_blockname)['file_name']

    topology = ssh +' ' +  mininet_user +' '+ mininet_ip +' ' +file_name
    ssh_newkey = 'Are you sure you want to continue connecting'
    print topology

    child = pexpect.spawn(topology)
    child.logfile = open("/tmp/mylog", "w")
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

def create_boundary(boundary_blockname, controller1_blockname, controller2_blockname):
    boundary_name = vtn_testconfig.ReadValues(MININETDATA, boundary_blockname)['boundary_id']
    domain1_id = vtn_testconfig.ReadValues(MININETDATA, boundary_blockname)['domain1_id']
    domain2_id = vtn_testconfig.ReadValues(MININETDATA, boundary_blockname)['domain2_id']
    logical_port1_id = vtn_testconfig.ReadValues(MININETDATA, boundary_blockname)['logical_port1_id']
    logical_port2_id = vtn_testconfig.ReadValues(MININETDATA, boundary_blockname)['logical_port2_id']
    boundary_url = vtn_testconfig.ReadValues(MININETDATA, 'URL')['url']
    test_controller1_id = vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                                 controller1_blockname)['controller_id']
    test_controller2_id = vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                                 controller2_blockname)['controller_id']
    url= coordinator_url + boundary_url
    print url

    boundary_add = collections.defaultdict(dict)
    link = collections.defaultdict(dict)
    boundary_add['boundary']['boundary_id'] = boundary_name
    boundary_add['boundary']['link'] = link
    link['controller1_id'] = test_controller1_id
    link['controller2_id'] = test_controller2_id
    link['domain1_id'] = domain1_id
    link['domain2_id'] = domain2_id
    link['logical_port1_id'] = logical_port1_id
    link['logical_port2_id'] = logical_port2_id

    print json.dumps(boundary_add)
    r = requests.post(url, data=json.dumps(boundary_add), headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS:
         return 1
    else:
         return 0

def delete_boundary(blockname):
    boundary_name = vtn_testconfig.ReadValues(MININETDATA, blockname)['boundary_id']
    boundary_url = vtn_testconfig.ReadValues(MININETDATA, blockname)['url']
    url= coordinator_url + boundary_url
    print url

    r = requests.delete(url, headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS:
         return 1
    else:
         return 0

def validate_boundary(blockname, presence="yes", position=0):
    boundary_name = vtn_testconfig.ReadValues(MININETDATA, blockname)['boundary_id']
    boundary_url = vtn_testconfig.ReadValues(MININETDATA, 'URL')['url']
    url= coordinator_url + boundary_url
    print url

    r = requests.get(url, headers=def_header)
    print r.status_code

    if presence == "no":
         if r.status_code == resp_code.RESP_NOT_FOUND:
             return 0
         if r.status_code != resp_code.RESP_GET_SUCCESS:
             return 1

    data = json.loads(r.content)

    if presence == "no":
      print data['boundaries']
      if data['boundaries'] == []:
        return 0

    vtn_content = data['boundaries'][position]

    if vtn_content['boundary_id'] != boundary_name:
          if presence == "yes":
               return 1
          else:
               return 0
    else:
        if presence == "yes":
           return 0
        else:
            return 1

def create_vlink(vlink_blockname, boundary_blockname, vtn_blockname):
    vlk_name = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['link_name']
    vnode1 = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['vnode1_name']
    vnode2 = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['vnode2_name']
    if1_name = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['if1_name']
    if2_name = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['if2_name']
    vlan_id = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['vlan_id']
    vlink_url = vtn_testconfig.ReadValues(MININETDATA, 'URL')['link_url']
    boundary_name = vtn_testconfig.ReadValues(MININETDATA, boundary_blockname)['boundary_id']
    vtn_name = vtn_testconfig.ReadValues(VTNVBRDATA, vtn_blockname)['vtn_name']
    vtn_url = vtn_testconfig.ReadValues(VTNVBRDATA, 'VTNURL')['ctr_url']

    url= coordinator_url + vtn_url +'/' + vtn_name + vlink_url
    print url

    vlink_add = collections.defaultdict(dict)
    boundarymap = collections.defaultdict(dict)

    vlink_add['vlink']['vlk_name'] = vlk_name
    vlink_add['vlink']['vnode1_name'] = vnode1
    vlink_add['vlink']['if1_name'] = if1_name
    vlink_add['vlink']['vnode2_name'] = vnode2
    vlink_add['vlink']['if2_name'] = if2_name
    vlink_add['vlink']['boundary_map'] = boundarymap
    boundarymap['boundary_id'] = boundary_name
    boundarymap['vlan_id'] = vlan_id

    print json.dumps(vlink_add)
    r = requests.post(url, data=json.dumps(vlink_add), headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS:
         return 1
    else:
         return 0

def delete_vlink(vlink_blockname, vtn_blockname):
    vlink_name = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['link_name']
    vlink_url = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['url']
    vtn_name = vtn_testconfig.ReadValues(VTNVBRDATA, vtn_blockname)['vtn_name']
    vtn_url = vtn_testconfig.ReadValues(VTNVBRDATA, 'VTNURL')['ctr_url']
    url= coordinator_url + vtn_url + '/' + vtn_name + vlink_url
    print url

    r = requests.delete(url, headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS:
         return 1
    else:
         return 0

def validate_vlink(vlink_blockname, vtn_blockname, presence="yes", position=0):
    vlink_name = vtn_testconfig.ReadValues(MININETDATA, vlink_blockname)['link_name']
    vlink_url = vtn_testconfig.ReadValues(MININETDATA, 'URL')['link_url']
    vtn_name = vtn_testconfig.ReadValues(VTNVBRDATA, vtn_blockname)['vtn_name']
    vtn_url = vtn_testconfig.ReadValues(VTNVBRDATA, 'VTNURL')['ctr_url']
    url= coordinator_url + vtn_url + '/' + vtn_name + vlink_url
    print url

    r = requests.get(url, headers=def_header)
    print r.status_code

    if presence == "no":
         if r.status_code == resp_code.RESP_NOT_FOUND:
             return 0
         if r.status_code != resp_code.RESP_GET_SUCCESS:
             return 1

    data = json.loads(r.content)

    if presence == "no":
      print data['vlinks']
      if data['vlinks'] == []:
        return 0

    vtn_content = data['vlinks'][position]

    if vtn_content['vlk_name'] != vlink_name:
          if presence == "yes":
               return 1
          else:
               return 0
    else:
        if presence == "yes":
           return 0
        else:
            return 1

def test_multi_ctr_mininet_ping():
    child = create_multicontroller_mininet_topology('MININETONE')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.mininet_test.close_topology(child)
        exit(1)

    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 3 :Controller Create Failed"
        mininet_test.close_topology(child)
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
        print "controller state change failed"
        exit(1)

    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "TEST 3 :Controller Create Failed"
        mininet_test.close_topology(child)
        exit(1)
  # Delay for AUDIT
    retval = controller.wait_until_state('ControllerSecond',"up")
    if retval != 0:
        print "controller state change failed"
        exit(1)

    retval = create_boundary('BOUNDARY', 'ControllerFirst', 'ControllerSecond')
    if retval != 0:
        print "Boundary  Create Failed"
        mininet_test.close_topology(child)
        exit(1)
    retval = validate_boundary('BOUNDARY', presence="yes", position=0)
    if retval != 0:
        print "Boundary  Validate  Failed after create"
        mininet_test.close_topology(child)
        exit(1)

    retval = vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne', 'VbrOne', 'ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vtn_vbr.create_vbr('VtnOne', 'VbrTwo', 'ControllerSecond')
    if retval != 0:
        print "VBRTWO Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_vbrif('VtnOne', 'VbrOne', 'MultiCtrVbrIfOne')
    if retval != 0:
        print "VBRIFONE Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'MultiCtrVbrIfOne', 'ControllerFirst', position=0)
    if retval != 0:
        print "VBRIFONE Validate Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_vbrif('VtnOne', 'VbrOne', 'VbrIfTwo')
    if retval != 0:
        print "VBRIFONE Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfTwo', 'ControllerFirst', position=1)
    if retval != 0:
        print "VBRIFTWO Validate Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_portmap('VtnOne', 'VbrOne', 'MultiCtrVbrIfOne', vlan_tagged=0);
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne', 'VbrOne', 'MultiCtrVbrIfOne', 'ControllerFirst', presence="yes");
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_vbrif('VtnOne', 'VbrTwo', 'MultiCtrVbrIfThree')
    if retval != 0:
        print "VBRIFTHREE Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrTwo', 'MultiCtrVbrIfThree', 'ControllerSecond', position=0)
    if retval != 0:
        print "VBRIFTHREE Validate Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_vbrif('VtnOne', 'VbrTwo', 'VbrIfFour')
    if retval != 0:
        print "VBRIFFOUR Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrTwo', 'VbrIfFour', 'ControllerSecond', position=0)
    if retval != 0:
        print "VBRIFFOUR Validate Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.create_portmap('VtnOne', 'VbrTwo', 'MultiCtrVbrIfThree', vlan_tagged=0);
    if retval != 0:
        print "VBRIF3 Portmap Create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne', 'VbrTwo', 'MultiCtrVbrIfThree', 'ControllerSecond', presence="yes");
    if retval != 0:
        print "VBRIF3 Portmap Validate Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = create_vlink('VLINK', 'BOUNDARY', 'VtnOne')
    if retval != 0:
        print "Vlink create Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = validate_vlink('VLINK', 'VtnOne', presence="yes", position=0)
    if retval != 0:
        print "Vlink validate Failed"
        mininet_test.close_topology(child)
        exit(1)

    retval = mininet_test.ping_mininet (child,'h2','h6')
    if retval != 0:
        print "MININET PING FAILED"

    mininet_test.close_topology(child)

    retval = delete_vlink('VLINK', 'VtnOne')
    if retval != 0:
        print "Vlink Delete Failed"
        exit(1)

    retval = validate_vlink('VLINK', 'VtnOne', presence="no")
    if retval != 0:
        print "After Delete validate vlink Failed"
        exit(1)

    retval = delete_boundary('BOUNDARY')
    if retval != 0:
        print "Delete boundary Failed"
        exit(1)

    retval = validate_boundary('BOUNDARY', presence="no")
    if retval != 0:
        print "After Delete validate boundary Failed"
        exit(1)

    retval = vbrif_portmap.delete_portmap('VtnOne', 'VbrTwo', 'MultiCtrVbrIfThree')
    if retval != 0:
        print "VBRIF3 portmap Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne', 'VbrTwo', 'MultiCtrVbrIfThree', 'ControllerSecond', presence="no");
    if retval != 0:
        print "After Delete VBRIF3 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_portmap('VtnOne', 'VbrOne', 'MultiCtrVbrIfOne')
    if retval != 0:
        print "VBRIF1 portmap Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_portmap_at_controller('VtnOne', 'VbrOne', 'MultiCtrVbrIfOne', 'ControllerFirst', presence="no");
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne', 'VbrOne', 'MultiCtrVbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'MultiCtrVbrIfOne', 'ControllerFirst', presence="no")
    if retval != 0:
        print "After Delete VBRIFONE Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne', 'VbrOne', 'VbrIfTwo')
    if retval != 0:
        print "VTN1->VBR1->VBRIF2 Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrOne', 'VbrIfTwo', 'ControllerFirst', presence="no")
    if retval != 0:
        print "After Delete VBRIFTWO Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne', 'VbrTwo', 'MultiCtrVbrIfThree')
    if retval != 0:
        print "VTN1->VBR2->VBRIF3 Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrTwo', 'MultiCtrVbrIfThree', 'ControllerSecond', presence="no")
    if retval != 0:
        print "After Delete VBRIFTHREE Validate Failed"
        exit(1)

    retval = vbrif_portmap.delete_vbrif('VtnOne', 'VbrTwo', 'VbrIfFour')
    if retval != 0:
        print "VTN1->VBR2->VBRIF4 Delete Failed"
        exit(1)

    retval = vbrif_portmap.validate_vbrif_at_controller('VtnOne', 'VbrTwo', 'VbrIfFour', 'ControllerSecond', presence="no")
    if retval != 0:
        print "After Delete VBRIFFOUR Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne', 'VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne', 'VbrOne', 'ControllerFirst', presence="no")
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne', 'VbrTwo')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.validate_vbr_at_controller('VtnOne', 'VbrTwo', 'ControllerSecond', presence="no")
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne', 'ControllerFirst', presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vbr.validate_vtn_at_controller('VtnOne', 'ControllerSecond', presence="no")
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
        print "CONTROLLER1 delete failed"
        exit(1)

    retval = controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        exit(1)
    print "MININET PING MULTI CONTROLLER TEST SUCCESS"

# Main Block
if __name__ == '__main__':
    print '*****MULTI CONTROLLER MININET TESTS******'
    test_multi_ctr_mininet_ping()
else:
    print "MININET Loaded as Module"
