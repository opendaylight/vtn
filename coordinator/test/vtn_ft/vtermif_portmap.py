#!/usr/bin/python

#
# Copyright (c) 2014-2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller, vtn_vterm
import vtn_testconfig
import resp_code

CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA
VTNVTERMDATA = vtn_testconfig.VTNVTERMDATA
VTERMIFDATA = vtn_testconfig.VTERMIFDATA

coordinator_url = vtn_testconfig.coordinator_url
def_header = vtn_testconfig.coordinator_headers
controller_headers = vtn_testconfig.controller_headers
controller_url_part = vtn_testconfig.controller_url_part



def create_vtermif(vtn_blockname,vterm_blockname,vtermif_blockname):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_name']
    vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_url']
    vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['vterm_url']
    vtermif_create_url = vtn_testconfig.ReadValues(VTERMIFDATA,'VTERMIFURL')['url']
    vtermif_name = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['vtermif_name']
    description = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['description']
    admin_status = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['admin_status']


    url= coordinator_url + vtn_url + vterm_url + vtermif_create_url
    print url

    vtermif_add = collections.defaultdict(dict)

    vtermif_add['interface']['if_name'] = vtermif_name
    vtermif_add['interface']['description'] = description
    vtermif_add['interface']['admin_status'] = admin_status
    r = requests.post(url,data = json.dumps(vtermif_add),headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0


def delete_vtermif(vtn_blockname,vterm_blockname,vtermif_blockname):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_name']
    test_vterminal_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['vterminal_name']
    vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_url']
    vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['vterm_url']
    vtermif_url = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['vtermif_url']
    url= coordinator_url + vtn_url + vterm_url + vtermif_url + '.json'
    print url

    r = requests.delete(url,headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0



def validate_vtermif_at_controller(vtn_blockname, vterm_blockname, vtermif_blockname,
                                                                  controller_blockname, presence = "yes",position = 0):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_name']
    test_vterminal_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['vterminal_name']
    test_controller_ipaddr = vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port = vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA,'VTNURL')['ctr_url']
    test_vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['ctrl_url']
    test_vtermif_url = vtn_testconfig.ReadValues(VTERMIFDATA,'VTERMIFURL')['ctr_url']
    test_vtermif_name = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['vtermif_name']
    test_vtermif_adminstatus = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['admin_status']
    print test_vterm_url
    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vterm_url+test_vtermif_url+'/'+test_vtermif_name
    print url
    r = requests.get(url,headers = controller_headers,auth = ('admin','admin'))

    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'vterminal interface name : '+test_vtermif_name+' is removed'
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data=json.loads(r.content)
    print data
    if presence == "no":
        print data['vinterface']
        if data['vinterface'] == []:
            return 0
    print position
    vtn_content = data['vinterface'][position]

    print vtn_content['name']
    if vtn_content == None:
        if presence == "yes":
            return 0
        else:
            return 1

    if vtn_content['name'] != test_vtermif_name:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            print 'vterminal interface name : '+vtn_content['name']+' is present'
            return 0
        else:
            return 1


def create_portmap(vtn_blockname,vterm_blockname,vtermif_blockname,vlan_tagged = 1):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_name']
    vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_url']
    vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['vterm_url']
    vtermif_url = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['vtermif_url']
    vtermif_name = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['vtermif_name']
    description = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['description']
    admin_status = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['admin_status']
    logical_port_id = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['logical_port_id']
    vlan_id = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['vlan_id']
    tagged = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['tagged']
    portmap_url = vtn_testconfig.ReadValues(VTERMIFDATA,'PORTMAPURL')['url']



    url= coordinator_url + vtn_url + vterm_url + vtermif_url + portmap_url
    print url

    vtermif_add = collections.defaultdict(dict)

    vtermif_add['portmap']['logical_port_id'] = logical_port_id
    if vlan_tagged  == 1:
      vtermif_add['portmap']['vlan_id'] = vlan_id
      vtermif_add['portmap']['tagged'] = tagged
    print json.dumps(vtermif_add)
    r = requests.put(url,data = json.dumps(vtermif_add),headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_UPDATE_SUCCESS and r.status_code != resp_code.RESP_UPDATE_SUCCESS_U14:
        return 1
    else:
        return 0

def delete_portmap(vtn_blockname,vterm_blockname,vtermif_blockname):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_name']
    test_vterminal_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['vterminal_name']
    vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_url']
    vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['vterm_url']
    vtermif_url = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['vtermif_url']
    url= coordinator_url + vtn_url + vterm_url + vtermif_url + '/portmap.json'
    print url

    r = requests.delete(url,headers = def_header)
    print r.status_code
    print "delete portmap sucess"

    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0

def validate_vtermif_portmap_at_controller(vtn_blockname, vterm_blockname, vtermif_blockname,
                                                                  controller_blockname, presence = "yes",position = 0):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vtn_blockname)['vtn_name']
    test_vterminal_name = vtn_testconfig.ReadValues(VTNVTERMDATA,vterm_blockname)['vterminal_name']
    test_controller_ipaddr = vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port = vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA,'VTNURL')['ctr_url']
    test_vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA,'VTERMURL')['ctr_url']
    test_vtermif_url = vtn_testconfig.ReadValues(VTERMIFDATA,'VTERMIFURL')['ctr_url']
    test_vtermif_name = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['vtermif_name']
    test_vtermif_admin_status = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['admin_status']
    test_vtermif_logicalid = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['logical_port_id']
    test_vtermif_node_id = vtn_testconfig.ReadValues(VTERMIFDATA,vtermif_blockname)['node_id']

    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vterm_url+'/'+test_vterminal_name+test_vtermif_url+'/'+test_vtermif_name+'/port-map-config'
    
    print url
    r = requests.get(url,headers = controller_headers,auth = ('admin','admin'))

    print r.status_code

    if presence  == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'Portmap config is removed'
            return 0
        if r.status_code == resp_code.RESP_DELETE_SUCCESS or r.status_code == resp_code.RESP_DELETE_SUCCESS_U14:
            return 0
    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data = json.loads(r.content)
    print data
    
    vtn_content=data['port-map-config']
    print vtn_content

    if vtn_content == None:
        if presence == "yes":
            return 0
        else:
            return 1

    node = vtn_content['node']
    port = vtn_content['port-name']
    node_content = 'PP-'+node+'-'+port
    print node_content
    if node_content != test_vtermif_logicalid:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            print 'Portmap is configured'
            return 0
        else:
            return 1

def test_vtn_vterm_vtermif():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 1 :Controller Create Failed"
        exit(1)

    print "TEST 1 : VTenant with one VTerminal one VTERMIF"
  # Delay for AUDIT
    time.sleep(15)
    retval = vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM Create Failed"
        exit(1)

    retval = create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF Create Failed"
        exit(1)
    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERMIF Validate Failed"
        exit(1)


    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERM Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF Delete Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERMIF Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence = "no")
    if retval != 0:
        print "After Delete VTERM Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence = "no")
    if retval != 0:
        print "VTN Validate Failed after VTERM Deleted"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VTERM->VTERMIF TEST SUCCESS"

def test_multi_vterm_vtermif():

    print "CREATE Controller"
    print "VTNONE->VTERMONE->VTERMIFONE"
    print "VTNONE->VTERMTWO->VTERMIFTWO"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 3 :Controller Create Failed"
        exit(1)

    print "TEST 3 : One vtn and Two VTerminals with One Interfaces each"
  # Delay for AUDIT
    time.sleep(15)
    retval = vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERMONE Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTERMTWO Create Failed"
        exit(1)

    retval = create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIFONE Create Failed"
        exit(1)
    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "VTERMIFONE Validate Failed"
        exit(1)

    retval = create_vtermif('VtnOne','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIFTWO Create Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst')
    if retval != 0:
        print "VTERMIFTWO Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',position = 0)
    if retval != 0:
        print "VTERMONE Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',position = 0)
    if retval != 0:
        print "VTERMTWO Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERM1->VTERMIFONE Delete Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "VTERM1->VTERMIFONE Validate Failed"
        exit(1)

    retval = delete_vtermif('VtnOne','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERM1->VTERMIFTWO Delete Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "VTERM2->VTERMIFTWO Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM1/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermTwo')
    if retval != 0:
        print "VTERM2/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "VTERM1/VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "VTERM2/VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence = "no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN1->VTERM1->VTERMIF1 AND VTN1->VTERM2->VTERMIF2 TEST SUCCESS"

def test_multi_vtn_vterm_vtermif():

    print "TEST 4 :CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    print "TEST 4 : Two Vtn and Two Vterminal one Interfaces each "
  # Delay for AUDIT
    time.sleep(15)
    retval = vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN1 Create Failed"
        exit(1)

    retval = vtn_vterm.create_vtn('VtnTwo')
    if retval != 0:
        print "VTN2 Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1 Create Failed"
        exit(1)

    retval = create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Create Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Validate Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnTwo','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM2 Create Failed"
        exit(1)

    retval = create_vtermif('VtnTwo','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTN2->VTERM2->VTERMIF2 Create Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTN2->VTERM2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTN1 Validate Failed"
        exit(1)

    retval = delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Delete Failed"
        exit(1)

    retval = delete_vtermif('VtnTwo','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTN2->VTERM2->VTERMIF2 Delete Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "VTN1->VTERM1->VTERMIF1 Validate Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "VTN2->VTERM2->VTERMIF2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VTERM1 Validate Failed"
        exit(1)


    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTN2->VTERM2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position = 0)
    if retval != 0:
        print "Before VTERM1 Delete VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',position = 0)
    if retval != 0:
        print "Before VTERM2 Delete VTN2 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTN1->VTERM1 Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermTwo')
    if retval != 0:
        print "VTN2->VTERM2 Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence = "no")
    if retval != 0:
        print "VTN1->VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst',presence = "no")
    if retval != 0:
        print "VTN2->VTERM2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After VTERM1 Delete VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After VTERM1 Delete VTN1 Validate Failed"
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
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN1->VTERM1->VTERMIF1 and VTN2->VTERM2->VTERMIF2 TEST SUCCESS"

def test_vtn_vterm_vtermif_portmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 6 :Controller Create Failed"
        exit(1)

    print "TEST 6 : VTenant with one VTerminal one VTERMIF and One PORTMAP"
  # Delay for AUDIT
    time.sleep(15)
    retval = vtn_vterm.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = vtn_vterm.create_vterm('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "VTERM Create Failed"
        exit(1)

    retval = create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF Create Failed"
        exit(1)
    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERMIF Validate Failed"
        exit(1)

    retval = create_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "Portmap Create Failed"
        exit(1)

    retval = validate_vtermif_portmap_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "yes");
    if retval != 0:
        print "Portmap Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst')
    if retval != 0:
        print "After Create VTERM Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval = delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF Delete Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERMIF Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence = "no")
    if retval != 0:
        print "After Delete VTERM Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence = "no")
    if retval != 0:
        print "VTN Validate Failed after VTERM Deleted"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VTERM->VTERMIF->PORTMAP TEST SUCCESS"

def test_vtn_multi_vterm_single_vtermif_portmap():
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 9 :Controller Create Failed"
        exit(1)

    print "TEST 9 : VTenant with Two VTerminal Two VTERMIF and One PORTMAP each"
  # Delay for AUDIT
    time.sleep(15)
    retval = vtn_vterm.create_vtn('VtnOne')
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

    retval = create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Create Failed"
        exit(1)

    retval = create_vtermif('VtnOne','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Create Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTERMIF1 Validate Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTERMIF2 Validate Failed"
        exit(1)

    retval = create_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "VTERMIF1 Portmap Create Failed"
        exit(1)

    retval = create_portmap('VtnOne','VTermTwo','VTermIfTwo');
    if retval != 0:
        print "VTERMIF2 Portmap Create Failed"
        exit(1)

    retval = validate_vtermif_portmap_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "yes",position = 0);
    if retval != 0:
        print "VTERMIF1 Portmap Validate Failed"
        exit(1)

    retval = validate_vtermif_portmap_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',presence = "yes",position = 0);
    if retval != 0:
        print "VTERMIF2 Portmap Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval = delete_portmap('VtnOne','VTermTwo','VTermIfTwo');
    if retval != 0:
        print "VTermIfTwo Portmap Delete Failed"
        exit(1)

    retval = delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Delete Failed"
        exit(1)

    retval = delete_vtermif('VtnOne','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Delete Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERMIF1 Validate Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermTwo','VTermIfTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERMIF2 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermTwo')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERM2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence = "no")
    if retval != 0:
        print "VTN Validate Failed after VTERM Deleted"
        exit(1)

    retval = vtn_vterm.delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN1->VTERM1->VTERMIF1->PORTMAP AND VTN1->VTERM2->VTERMIF2->PORTMAP TEST SUCCESS"

def test_multi_vtn_vterm_vtermif_portmap():
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 10 :Controller Create Failed"
        exit(1)

    print "TEST 10 : Two VTenant with Two VTerminal Two VTERMIF and One PORTMAP each"
  # Delay for AUDIT
    time.sleep(15)
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

    retval = vtn_vterm.create_vterm('VtnTwo','VTermTwo','ControllerFirst')
    if retval != 0:
        print "VTERM2 Create Failed"
        exit(1)

    retval = create_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Create Failed"
        exit(1)

    retval = create_vtermif('VtnTwo','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Create Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTERMIF1 Validate Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTERMIF2 Validate Failed"
        exit(1)

    retval = create_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "VTERMIF1 Portmap Create Failed"
        exit(1)

    retval = create_portmap('VtnTwo','VTermTwo','VTermIfTwo');
    if retval != 0:
        print "VTERMIF2 Portmap Create Failed"
        exit(1)

    retval = validate_vtermif_portmap_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "yes",position = 0);
    if retval != 0:
        print "VTERMIF1 Portmap Validate Failed"
        exit(1)

    retval = validate_vtermif_portmap_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst',presence = "yes",position = 0);
    if retval != 0:
        print "VTERMIF2 Portmap Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst',position = 0)
    if retval != 0:
        print "After Create VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',position = 0)
    if retval != 0:
        print "VTN1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',position = 0)
    if retval != 0:
        print "VTN2 Validate Failed"
        exit(1)

    retval = delete_portmap('VtnOne','VTermOne','VTermIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval = delete_portmap('VtnTwo','VTermTwo','VTermIfTwo');
    if retval != 0:
        print "VTermIfTwo Portmap Delete Failed"
        exit(1)

    retval = delete_vtermif('VtnOne','VTermOne','VTermIfOne')
    if retval != 0:
        print "VTERMIF1 Delete Failed"
        exit(1)

    retval = delete_vtermif('VtnTwo','VTermTwo','VTermIfTwo')
    if retval != 0:
        print "VTERMIF2 Delete Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnOne','VTermOne','VTermIfOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERMIF1 Validate Failed"
        exit(1)

    retval = validate_vtermif_at_controller('VtnTwo','VTermTwo','VTermIfTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERMIF2 Validate Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnOne','VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.delete_vterm('VtnTwo','VTermTwo')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnOne','VTermOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERM1 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vterm_at_controller('VtnTwo','VTermTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "After Delete VTERM2 Validate Failed"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnOne','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "VTN1 Validate Failed after VTERM Deleted"
        exit(1)

    retval = vtn_vterm.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence = "no",position = 0)
    if retval != 0:
        print "VTN2 Validate Failed after VTERM Deleted"
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
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN1->VTERM1->VTERMIF1->PORTMAP AND VTN2->VTERM2->VTERMIF2->PORTMAP TEST SUCCESS"


# Main Block
if __name__ == '__main__':
    print '*****VTERMIF TESTS******'
    test_vtn_vterm_vtermif()
    test_multi_vterm_vtermif()
    test_multi_vtn_vterm_vtermif()
    test_vtn_vterm_vtermif_portmap()
    test_vtn_multi_vterm_single_vtermif_portmap()
    test_multi_vtn_vterm_vtermif_portmap()



else:
    print "VTERMIF_PORTMAP Loaded as Module"
