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
VBRIFDATA=vtn_testconfig.VBRIFDATA

coordinator_url=vtn_testconfig.coordinator_url
def_header=vtn_testconfig.coordinator_headers
controller_headers=vtn_testconfig.controller_headers
controller_url_part=vtn_testconfig.controller_url_part



def create_vbrif(vtn_blockname,vbr_blockname,vbrif_blockname):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    vbrif_create_url=vtn_testconfig.ReadValues(VBRIFDATA,'VBRIFURL')['url']
    vbrif_name=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_name']
    description=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['description']
    admin_status=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['admin_status']


    url= coordinator_url + vtn_url + vbr_url + vbrif_create_url
    print url

    vbrif_add = collections.defaultdict(dict)

    vbrif_add['interface']['if_name']=vbrif_name
    vbrif_add['interface']['description']=description
    vbrif_add['interface']['admin_status']=admin_status
    print vbrif_add

    r = requests.post(url,data=json.dumps(vbrif_add),headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0

def update_vbrif(vtn_blockname,vbr_blockname,vbrif_blockname):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    vbrif_name=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_name']
    vbrif_url=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['url']
    updatedescription=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['updatedescription']
    adminstatus=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['adminstatus']

    url= coordinator_url + vtn_url + vbr_url + vbrif_url +vbrif_name + '.json'
    print url


    vbrif_add = collections.defaultdict(dict)

    vbrif_add['interface']['if_name']=vbrif_name
    vbrif_add['interface']['description']=updatedescription
    vbrif_add['interface']['adminstatus']=adminstatus

    print vbrif_add

    r = requests.put(url,data=json.dumps(vbrif_add),headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_UPDATE_SUCCESS and r.status_code != resp_code.RESP_UPDATE_SUCCESS_U14:
       return 1
    else:
       return 0


def delete_vbrif(vtn_blockname,vbr_blockname,vbrif_blockname):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    vbrif_url=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_url']
    url= coordinator_url + vtn_url + vbr_url + vbrif_url + '.json'
    print url

    r = requests.delete(url,headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0



def validate_vbrif_at_controller(vtn_blockname, vbr_blockname, vbrif_blockname,
                                                                  controller_blockname, presence="yes",position=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VTNURL')['ctr_url']
    test_vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['ctrl_url']
    test_vbrif_ctr_url=vtn_testconfig.ReadValues(VBRIFDATA,'VBRIFURL')['ctr_url']
    test_vbrif_name=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_name']
    test_vbrif_adminstatus=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['adminstatus']
    print test_vbr_url
    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+test_vbrif_ctr_url+'/'+test_vbrif_name
    print url
    r = requests.get(url,headers=controller_headers,auth=('admin','admin'))

    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'vbridge interface name : '+test_vbrif_name+' is removed'
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
    vtn_content=data['vinterface'][position]

    print vtn_content['name']
    if vtn_content == None:
        if presence == "yes":
            return 0
        else:
            return 1

    if vtn_content['name'] != test_vbrif_name:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            print 'vbridge interface name : '+vtn_content['name']+' is present'
            return 0
        else:
            return 1

def validate_updatevbrif_at_controller(vtn_blockname, vbr_blockname, vbrif_blockname,
                                                                  controller_blockname, presence="yes",position=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VTNURL')['ctr_url']
    test_vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['ctrl_url']
    test_vbrif_ctr_url=vtn_testconfig.ReadValues(VBRIFDATA,'VBRIFURL')['ctr_url']
    test_vbrif_name=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_name']
    test_vbrif_adminstatus=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['adminstatus']
    print test_vbr_url
    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+test_vbrif_ctr_url+'/'+test_vbrif_name
    print url
    r = requests.get(url,headers=controller_headers,auth=('admin','admin'))

    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'vbridge interface name : '+test_vbrif_name+' is removed'
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        print 'log 1 *************'
        return 1


    data=json.loads(r.content)
    print data
    if presence == "no":
        print data['vinterface']
        if data['vinterface'] == []:
            return 0
    print position
    vtn_content=data['vinterface'][position]
    print vtn_content

    print "ENABLED Value"
    print vtn_content['vinterface-config']['enabled']
    print "Test Value"
    print test_vbrif_adminstatus
    if vtn_content == None:
        if presence == "yes":
            return 0
        else:
            return 1

    if vtn_content['vinterface-config']['enabled'] != test_vbrif_adminstatus:
        if presence == "yes":
            return 0
        else:
            print 'log 2 *************'
            return 1
    else:
        if presence == "yes":
            return 0
        else:
            print 'log 3 *************'
            return 1

def create_portmap(vtn_blockname,vbr_blockname,vbrif_blockname,vlan_tagged=1):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    vbrif_url=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_url']
    vbrif_name=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_name']
    description=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['description']
    admin_status=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['admin_status']
    logical_port_id=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['logical_port_id']
    vlan_id=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vlan_id']
    tagged=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['tagged']
    portmap_url=vtn_testconfig.ReadValues(VBRIFDATA,'PORTMAPURL')['url']



    url= coordinator_url + vtn_url + vbr_url + vbrif_url + portmap_url
    print url

    vbrif_add = collections.defaultdict(dict)

    vbrif_add['portmap']['logical_port_id']=logical_port_id
    if vlan_tagged == 1:
      vbrif_add['portmap']['vlan_id']=vlan_id
      vbrif_add['portmap']['tagged']=tagged
    print json.dumps(vbrif_add)
    r = requests.put(url,data=json.dumps(vbrif_add),headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_UPDATE_SUCCESS and r.status_code != resp_code.RESP_UPDATE_SUCCESS_U14:
        return 1
    else:
        return 0

def delete_portmap(vtn_blockname,vbr_blockname,vbrif_blockname):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']
    vbrif_url=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_url']
    url= coordinator_url + vtn_url + vbr_url + vbrif_url + '/portmap.json'
    print url

    r = requests.delete(url,headers=def_header)
    print r.status_code
    print "delete portmap sucess"

    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0

def validate_vbrif_portmap_at_controller(vtn_blockname, vbr_blockname, vbrif_blockname,
                                                                  controller_blockname, presence="yes",position=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VTNURL')['ctr_url']
    test_vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VBRURL')['ctr_url']
    test_vbrif_url=vtn_testconfig.ReadValues(VBRIFDATA,'VBRIFURL')['ctr_url']
    test_vbrif_name=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['vbrif_name']
    test_vbrif_admin_status=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['admin_status']
    test_vbrif_logicalid=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['logical_port_id']
    test_vbrif_node_id=vtn_testconfig.ReadValues(VBRIFDATA,vbrif_blockname)['node_id']

    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+'/'+test_vbr_name+test_vbrif_url+'/'+test_vbrif_name+'/port-map-config'
    print url
    r = requests.get(url,headers=controller_headers,auth=('admin','admin'))

    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'Portmap config is removed'
            return 0
        if r.status_code == resp_code.RESP_DELETE_SUCCESS or r.status_code == resp_code.RESP_DELETE_SUCCESS_U14:
            return 0
    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data=json.loads(r.content)
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
    if node_content != test_vbrif_logicalid:
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

def test_vtn_vbr_vbrif():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 1 :Controller Create Failed"
        exit(1)

    print "TEST 1 : VTenant with one VBridge one VBRIF"
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VBRIF Validate Failed"
        exit(1)



    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
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
    print "VTN->VBR->VBRIF TEST SUCCESS"

def test_multi_vbrif():

    print "CREATE Controller"
    print "VTNONE->VBRONE->VBRIFONE"
    print "VTNONE->VBRONE->VBRIFTWO"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 2 :Controller Create Failed"
        exit(1)

    print "TEST 2 : One vtn and one VBridge with Two Interfaces"
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIFONE Create Failed"
        exit(1)



    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIFONE Validate Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIFTWO Create Failed"
        exit(1)


    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIFTWO Validate Failed"
        exit (1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIFONE Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
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
    print "VTN->VBR->VBRIF1/VBRIF2 TEST SUCCESS"

def test_multi_vbr_vbrif():

    print "CREATE Controller"
    print "VTNONE->VBRONE->VBRIFONE"
    print "VTNONE->VBRTWO->VBRIFTWO"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 3 :Controller Create Failed"
        exit(1)

    print "TEST 3 : One vtn and Two VBridges with One Interfaces each"
  # Delay for AUDIT
    time.sleep(15)
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIFONE Create Failed"
        exit(1)


    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "VBRIFONE Validate Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIFTWO Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst')
    if retval != 0:
        print "VBRIFTWO Validate Failed"
        exit(1)
    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRONE Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBRTWO Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIFONE Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1->VBRIFONE Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBR1->VBRIFTWO Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR2->VBRIFTWO Validate Failed"
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

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=0)
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
    print "VTN1->VBR1->VBRIF1 AND VTN1->VBR2->VBRIF2 TEST SUCCESS"

def test_multi_vtn_vbr_vbrif():

    print "TEST 4 :CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    print "TEST 4 : Two Vtn and Two Vbridges one Interfaces each "
  # Delay for AUDIT
    time.sleep(15)
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnTwo','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR2 Create Failed"
        exit(1)

    retval=create_vbrif('VtnTwo','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VTN2->VBR2->VBRIF2 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Validate Failed"
        exit(1)



    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VTN2->VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VTN1 Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnTwo','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VTN2->VBR2->VBRIF2 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN1->VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN2->VBR2->VBRIF2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VTN1->VBR1 Validate Failed"
        exit(1)


    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VTN2->VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "Before VBR1 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "Before VBR2 Delete VTN2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VTN1->VBR1 Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrTwo')
    if retval != 0:
        print "VTN2->VBR2 Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN1->VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN2->VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After VBR1 Delete VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After VBR1 Delete VTN1 Validate Failed"
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
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN1->VBR1->VBRIF1 and VTN2->VBR2->VBRIF2 TEST SUCCESS"

def test_vtn_multi_vbr_vbrif():
    print "CREATE Controller"
    print "VTNONE->VBRONE->VBRIFONE/VBRIFTHREE"
    print "VTNONE->VBRTWO->VBRIFTWO/VBRIFFOUR"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 5 :Controller Create Failed"
        exit(1)

    print "TEST 5 : One vtn and Two VBridges with Two Interfaces each"
  # Delay for AUDIT
    time.sleep(15)
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIF1 Validate Failed"
        exit(1)
    retval=create_vbrif('VtnOne','VbrOne','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Create Failed"
        exit(1)
    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfThree','ControllerFirst',position=0)
    if retval != 0:
        print "VBRIF3 Validate Failed"
        exit(1)
    retval=create_vbrif('VtnOne','VbrTwo','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIF1 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBR2->VBRIF2 Validate Failed"
        exit(1)
    retval=create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBR2->VBRIF2 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBR2->VBRIF2 Validate Failed"
        exit(1)
    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "VBRONE Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBRTWO Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBR1->VBRIF1 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfThree')
    if retval != 0:
        print "VBR1->VBRIF3 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1->VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfThree','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR1->VBRIF3 Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrTwo','VbrIfOne')
    if retval != 0:
        print "VBR2->VBRIF1 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VBR2->VBRIF1 Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBR2->VBRIF2 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0)
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

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=0)
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
    print "VTN1->VBR1->VBRIF1/VBRIF3 AND VTN1->VBR2->VBRIF1/VBRIF2 TEST SUCCESS"

def test_vtn_vbr_vbrif_portmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 6 :Controller Create Failed"
        exit(1)

    print "TEST 6 : VTenant with one VBridge one VBRIF and One PORTMAP"
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VBRIF Validate Failed"
        exit(1)


    retval=create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Create Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
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

    retval=delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no");
    if retval != 0:
       print "After Delete Portmap Validate Failed"
       exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
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
    print "VTN->VBR->VBRIF->PORTMAP TEST SUCCESS"

def test_vtn_multi_vbr_vbrif_portmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 7 :Controller Create Failed"
        exit(1)

    print "TEST 7 : VTenant with one VBridge Two VBRIF and One PORTMAP each"
  # Delay for AUDIT
    time.sleep(15)
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)


    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="yes",position=0);
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

    retval=delete_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
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
    print "VTN1->VBR1->VBRIF1->PORTMAP AND VTN1->VBR1->VBRIF2->PORTMAP TEST SUCCESS"

def test_vtn_vbr_multi_vbrif_portmap():
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 8 :Controller Create Failed"
        exit(1)

    print "TEST 8 : VTenant with one VBridge Two VBRIF/VBRIF2 and One PORTMAP in VBRIF1"
  # Delay for AUDIT
    time.sleep(15)
    retval=vtn_vbr.create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=vtn_vbr.create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR1 Create Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)


    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)
    retval=create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
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
    print "VTN1->VBR1->VBRIF1->PORTMAP AND VTN1->VBR1->VBRIF2 TEST SUCCESS"

def test_vtn_multi_vbr_single_vbrif_portmap():
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 9 :Controller Create Failed"
        exit(1)

    print "TEST 9 : VTenant with Two VBridge Two VBRIF and One PORTMAP each"
  # Delay for AUDIT
    time.sleep(15)
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrTwo','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrTwo','VbrIfTwo');
    if retval != 0:
        print "VbrIfTwo Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF2 Portmap Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
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
    print "VTN1->VBR1->VBRIF1->PORTMAP AND VTN1->VBR2->VBRIF2->PORTMAP TEST SUCCESS"

def test_multi_vtn_vbr_vbrif_portmap():
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 10 :Controller Create Failed"
        exit(1)

    print "TEST 10 : Two VTenant with Two VBridge Two VBRIF and One PORTMAP each"
  # Delay for AUDIT
    time.sleep(15)
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=create_vbrif('VtnTwo','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=create_portmap('VtnTwo','VbrTwo','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTN2 Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=delete_portmap('VtnTwo','VbrTwo','VbrIfTwo');
    if retval != 0:
        print "VbrIfTwo Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF2 Portmap Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnTwo','VbrTwo','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnTwo','VbrTwo','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF2 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnTwo','VbrTwo')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnTwo','VbrTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN1 Validate Failed after VBR Deleted"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "VTN2 Validate Failed after VBR Deleted"
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
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN1->VBR1->VBRIF1->PORTMAP AND VTN2->VBR2->VBRIF2->PORTMAP TEST SUCCESS"

def test_vtn_vbr_multiple_vbrif_portmap():
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 11 :Controller Create Failed"
        exit(1)

    print "TEST 11 : VTenant with Two VBridge Four VBRIF and One PORTMAP each"
  # Delay for AUDIT
    time.sleep(15)
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Create Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Create Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrTwo','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Create Failed"
        exit(1)

    retval=create_vbrif('VtnOne','VbrTwo','VbrIfFour')
    if retval != 0:
        print "VBRIF4 Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF2 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF3 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBRIF4 Validate Failed"
        exit(1)
    retval=create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "VBRIF1 Portmap Create Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "VBRIF2 Portmap Create Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrTwo','VbrIfThree');
    if retval != 0:
        print "VBRIF3 Portmap Create Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrTwo','VbrIfFour');
    if retval != 0:
        print "VBRIF4 Portmap Create Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',presence="yes",position=0);
    if retval != 0:
        print "VBRIF4 Portmap Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "After Create VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF1 Portmap Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrOne','VbrIfTwo');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF2 Portmap Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrTwo','VbrIfThree');
    if retval != 0:
        print "VbrIfThree Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF3 Portmap Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrTwo','VbrIfFour');
    if retval != 0:
        print "VbrIfFour Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',presence="no",position=0);
    if retval != 0:
        print "After Delete VBRIF4 Portmap Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF1 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfTwo')
    if retval != 0:
        print "VBRIF2 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrTwo','VbrIfThree')
    if retval != 0:
        print "VBRIF3 Delete Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrTwo','VbrIfFour')
    if retval != 0:
        print "VBRIF4 Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF1 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF2 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfThree','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF3 Validate Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrTwo','VbrIfFour','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBRIF4 Validate Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval = vtn_vbr.delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR1 Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no",position=0)
    if retval != 0:
        print "After Delete VBR2 Validate Failed"
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
    print "VTN1->VBR1->VBRIF1/VBRIF2->PORTMAP AND VTN2->VBR2->VBRIF3/VBRIF4->PORTMAP TEST SUCCESS"


def test_vbrif_update():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 12 :Controller Create Failed"
        exit(1)
    print "UPDATE TEST  : VTenant with one VBridge one VBRIF"
    print "TEST  : VTenant with one VBridge one VBRIF UPDATE SCENARIO"
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VBRIF Validate Failed"
        exit(1)

    retval=update_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
      print "VBRIF Update Failed"
      exit(1)


    retval=validate_updatevbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After update  VBRIF Validate Failed"

        exit(1)



    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
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
    print "VTN->VBR->VBRIF Update TEST SUCCESS"


def test_vbrif_update_portmap():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "TEST 13 :Controller Create Failed"
        exit(1)

    print "TEST 13 : VTenant with one VBridge one VBRIF and One UPDATE PORTMAP"
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

    retval=create_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Create Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Create VBRIF Validate Failed"
        exit(1)

    retval=create_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Create Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="yes");
    if retval != 0:
        print "Portmap Validate Failed"
        exit(1)

    retval=update_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
      print "VBRIF Update Failed"
      exit(1)

    retval=validate_updatevbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst')
    if retval != 0:
        print "After Update VBRIF Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "After Create VBR Validate Failed"
        exit(1)

    retval=vtn_vbr.validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=delete_portmap('VtnOne','VbrOne','VbrIfOne');
    if retval != 0:
        print "Portmap Delete Failed"
        exit(1)

    retval=validate_vbrif_portmap_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no");
    if retval != 0:
        print "After Delete Portmap Validate Failed"
        exit(1)

    retval = delete_vbrif('VtnOne','VbrOne','VbrIfOne')
    if retval != 0:
        print "VBRIF Delete Failed"
        exit(1)

    retval=validate_vbrif_at_controller('VtnOne','VbrOne','VbrIfOne','ControllerFirst',presence="no",position=0)
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
    print "VTN->VBR->VBRIF->Update PORTMAP TEST SUCCESS"


# Main Block
if __name__ == '__main__':
    print '*****VBRIF TESTS******'
    test_vtn_vbr_vbrif()
    test_multi_vbrif()
    test_multi_vbr_vbrif()
    test_multi_vtn_vbr_vbrif()
    test_vtn_multi_vbr_vbrif()
    test_vtn_vbr_vbrif_portmap()
    test_vtn_multi_vbr_vbrif_portmap()
    test_vtn_vbr_multi_vbrif_portmap()
    test_vtn_multi_vbr_single_vbrif_portmap()
    test_multi_vtn_vbr_vbrif_portmap()
    test_vtn_vbr_multiple_vbrif_portmap()
    test_vbrif_update()
    test_vbrif_update_portmap()

else:
    print "VBRIF_PORTMAP Loaded as Module"
