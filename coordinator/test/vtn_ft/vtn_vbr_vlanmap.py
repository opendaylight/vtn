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
import vtn_testconfig
import resp_code

CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA
VTNVBRDATA=vtn_testconfig.VTNVBRDATA
VLANMAPDATA=vtn_testconfig.VLANMAPDATA

coordinator_url=vtn_testconfig.coordinator_url
def_header=vtn_testconfig.coordinator_headers
controller_headers=vtn_testconfig.controller_headers
controller_url_part=vtn_testconfig.controller_url_part



def create_vlanmap(vtn_blockname,vbr_blockname,vlanmap_blockname,no_vlan=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    vlanmap_create_url=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['url']
    logical_port_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['logical_port_id']
    vlan_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['vlan_id']
    no_vlan_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['no_vlan_id']


    url= coordinator_url + vtn_url + vbr_url + vlanmap_create_url
    print url

    vlan_map_add = collections.defaultdict(dict)

    if no_vlan == 0:
       vlan_map_add['vlanmap']['no_vlan_id']=no_vlan_id
    elif no_vlan == 1:
       vlan_map_add['vlanmap']['vlan_id']=vlan_id
       vlan_map_add['vlanmap']['logical_port_id']=logical_port_id
    elif no_vlan == 2:
      vlan_map_add['vlanmap']['vlan_id']=vlan_id
    else:
      vlan_map_add['vlanmap']['logical_port_id']=logical_port_id
      vlan_map_add['vlanmap']['no_vlan_id']=no_vlan_id

    print json.dumps(vlan_map_add)
    r = requests.post(url,data=json.dumps(vlan_map_add),headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0

def delete_vlanmap(vtn_blockname,vbr_blockname,vlanmap_blockname,no_vlan=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    vlanmap_create_url=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['ctr_url']
    vlan_map_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['vlanmap_id']
    no_vlanmap_id=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['no_vlanmap_id']
    if no_vlan == 1:
       url= coordinator_url + vtn_url + vbr_url + vlanmap_create_url +'/' +vlan_map_id+'.json'
    else:
      url= coordinator_url + vtn_url + vbr_url + vlanmap_create_url +'/' +no_vlanmap_id+'.json'
    print url

    r = requests.delete(url,headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0
def validate_vlanmap_update(vtn_blockname, vbr_blockname, vlanmap_blockname,
                                 controller_blockname, presence="yes",position=0,no_vlan_id=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    test_vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    test_vlanmap_url=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['ctr_url']
    test_node_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['node_id']
    test_node_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['create_any_id']
    test_any_id=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['any_id']
    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+'/'+test_vbr_name+'/vlan-map/'+test_any_id
   # url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+test_vbr_url+test_vlanmap_url
    print url
    r = requests.get(url,headers=controller_headers,auth=('admin','admin'))

    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data=json.loads(r.content)
    print data
    if presence == "no":
        print data['vlan-map']
        if data['vlan-map'] == []:
            return 0
    vtn_content=data['vlan-map'][position]
    if no_vlan_id == 0:
      if vtn_content['id'] != test_any_id:
        print "ANY_ID",test_any_id
        return 1
      else:
        return 0
    print vtn_content['node']['id']
    if vtn_content == None:
        if presence == "yes":
            return 1
        else:
            return 0

    if vtn_content['node']['id'] != test_node_id:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            return 0
        else:
            return 1


def validate_vlanmap_at_controller(vtn_blockname, vbr_blockname, vlanmap_blockname,
                                 controller_blockname, presence="yes",position=0,no_vlan_id=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VTNURL')['ctr_url']
    test_vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VBRURL')['ctr_url']
    test_vlanmap_url=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['ctr_url']
    test_node_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['node_id']
    test_node=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['node']
    test_any_id=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['default_id']
    test_log_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['test_log_id']
    test_vlan_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['test_vlan_id']
    test_create_any_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['create_any_id']

    if no_vlan_id== 0:
       url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+'/'+test_vbr_name+'/vlan-map/'+test_any_id
       print url
    elif no_vlan_id == 1:
       url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+'/'+test_vbr_name+'/vlan-map/'+test_vlan_id
       print url
    elif no_vlan_id == 2:
       url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+'/'+test_vbr_name+'/vlan-map/'+test_create_any_id
    else:
       url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+'/'+test_vbr_name+'/vlan-map/'+test_log_id
       print url

    r = requests.get(url,headers=controller_headers,auth=('admin','admin'))

    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data=json.loads(r.content)
    print data
    if presence == "no":
        print data['vlan-map']
        if data['vlan-map'] == []:
            return 0
    print position
    vtn_content=data['vlan-map'][position]
    if no_vlan_id == 0:
      if vtn_content['map-id'] != test_any_id:
        print test_any_id
        return 1
      else:
        return 0
    if no_vlan_id == 1:
      if vtn_content['map-id'] != test_vlan_id:
        print test_any_id
        return 1
      else:
        return 0
    elif no_vlan_id == 2:
      if vtn_content['map-id'] != test_create_any_id:
        return 1
      else:
        return 0
    print vtn_content['vlan-map-config']['node']
    if vtn_content == None:
        if presence == "yes":
            return 1
        else:
            return 0

    if vtn_content['vlan-map-config']['node']!= test_node:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            return 0
        else:
            return 1

def update_vlanmap(vtn_blockname,vbr_blockname,vlanmap_blockname,update_id=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    vlanmap_create_url=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['ctr_url']
    vlan_map_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['vlanmap_id']
    vlan_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['u_id']
    vlan_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['u_id']
    no_vlan_id=vtn_testconfig.ReadValues(VLANMAPDATA,vlanmap_blockname)['no_vlan_id']
    no_vlanmap_id=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['no_vlanmap_id']
    any_id=vtn_testconfig.ReadValues(VLANMAPDATA,'VLANMAPURL')['any_id']
    if update_id == 1 or update_id == 2:
       url= coordinator_url + vtn_url + vbr_url + vlanmap_create_url +'/' +vlan_map_id+'.json'
    else:
      url= coordinator_url + vtn_url + vbr_url + vlanmap_create_url +'/' +no_vlanmap_id+'.json'

    print url

    vlan_map_add = collections.defaultdict(dict)
    if update_id == 1:
       vlan_map_add['vlanmap']['vlan_id']=vlan_id
    elif update_id == 2:
       vlan_map_add['vlanmap']['no_vlan_id']=no_vlan_id
    else:
       vlan_map_add['vlanmap']['vlan_id']=50

    print json.dumps(vlan_map_add)
    r = requests.put(url,data=json.dumps(vlan_map_add),headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_UPDATE_SUCCESS and r.status_code != resp_code.RESP_UPDATE_SUCCESS_U14:
        return 1
    else:
        return 0

def test_vtn_vbr_vlanmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 1 :Controller Create Failed"
        exit(1)

    print "TEST 1 : VTenant with one VBridge one VLANMAP without vlan_id and logicalport_id"
  # Delay for AUDIT
    time.sleep(20)
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=0)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=0)
    if retval != 0:
        print "VLANMAP update Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
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
    print "VTN->VBR->VLANMAP no vlan_id TEST SUCCESS"


def test_vtn_vbr_vlanmap_with_vlanid():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 2 :Controller Create Failed"
        exit(1)

    print "TEST 2 : VTenant with one VBridge one VLANMAP with vlan_id and logicalport_id"
  # Delay for AUDIT
    time.sleep(15)
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=1)
    if retval != 0:
        print "VLANMAP upadte Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
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
    print "VTN->VBR->VLANMAP with vlan_id TEST SUCCESS"

def test_vtn_vbr_multi_vlanmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 3 :Controller Create Failed"
        exit(1)

    print "TEST 3 : VTenant with one VBridge two VLANMAP"
  # Delay for AUDIT
    time.sleep(15)
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP1 Create Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',position=0,no_vlan_id=0)
    if retval != 0:
        print "After Create VLANMAP1 Validate Failed"
        exit(1)

    retval=update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=0)
    if retval != 0:
        print "VLANMAP update Failed"
        exit(1)

    retval=create_vlanmap('VtnOne','VbrOne','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP2 Create Failed"
        exit(1)


    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapTwo','ControllerFirst',position=0,no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP2 Validate Failed"
        exit(1)

    retval=update_vlanmap('VtnOne','VbrOne','VlanmapTwo',update_id=1)
    if retval != 0:
        print "VLANMAP2 upadte Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP1 Delete Failed"
        exit(1)

    retval = delete_vlanmap('VtnOne','VbrOne','VlanmapTwo',no_vlan=1)
    if retval != 0:
        print "VLANMAP2 Delete Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP1 Validate Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP2 Validate Failed"
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
    print "VTN->VBR->TWO VLANMAP TEST SUCCESS"

def test_vtn_vbr_vlanmap_vlanid():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 4 :Controller Create Failed"
        exit(1)

    print "TEST 4 : VTenant with one VBridge one VLANMAP only with vlan_id"
  # Delay for AUDIT
    time.sleep(15)
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=2)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=2)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=0)
    if retval != 0:
        print "VLANMAP update Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=0)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=0)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
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
    print "VTN->VBR->VLANMAP with vlan_id TEST SUCCESS"

def test_vtn_vbr_vlanmap_lg_id():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 5 :Controller Create Failed"
        exit(1)

    print "TEST 5 : VTenant with one VBridge one VLANMAP no_vlan_id and logicalport_id"
  # Delay for AUDIT
    time.sleep(15)
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=4)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=4)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=1)
    if retval != 0:
        print "VLANMAP upadte Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
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
    print "VTN->VBR->VLANMAP with no_vlan_id and logicalport_id TEST SUCCESS"

def test_vtn_vbr_vlanmap_no_vlanid():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 6 :Controller Create Failed"
        exit(1)

    print "TEST 6 : VTenant with one VBridge one VLANMAP with vlan_id and logicalport_id"
    print           "then update vlan_id to no_vlan_id"
  # Delay for AUDIT
    time.sleep(15)
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=create_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP Create Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',no_vlan_id=1)
    if retval != 0:
        print "After Create VLANMAP Validate Failed"
        exit(1)

    retval=update_vlanmap('VtnOne','VbrOne','VlanmapOne',update_id=2)
    if retval != 0:
        print "VLANMAP upadte Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vlanmap('VtnOne','VbrOne','VlanmapOne',no_vlan=1)
    if retval != 0:
        print "VLANMAP Delete Failed"
        exit(1)

    retval=validate_vlanmap_at_controller('VtnOne','VbrOne','VlanmapOne','ControllerFirst',presence="no",position=0,no_vlan_id=1)
    if retval != 0:
        print "After Delete VLANMAP Validate Failed"
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
    print "VTN->VBR->VLANMAP with vlan_id->no_vlan_id TEST SUCCESS"


if __name__ == '__main__':
  print "VLANMAP TEST"
  test_vtn_vbr_vlanmap()
  test_vtn_vbr_vlanmap_with_vlanid()
  test_vtn_vbr_multi_vlanmap()
  test_vtn_vbr_vlanmap_vlanid()
  test_vtn_vbr_vlanmap_lg_id()
  test_vtn_vbr_vlanmap_no_vlanid()
else:
    print "VLANMAP Loaded as Module"
