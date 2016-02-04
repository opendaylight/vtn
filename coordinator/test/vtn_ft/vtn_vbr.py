#!/usr/bin/python

#
# Copyright (c) 2013-2016 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller
import vtn_testconfig
import resp_code

VTNVBRDATA=vtn_testconfig.VTNVBRDATA
CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA

coordinator_url=vtn_testconfig.coordinator_url
def_header=vtn_testconfig.coordinator_headers
controller_headers=vtn_testconfig.controller_headers
controller_url_part=vtn_testconfig.controller_url_part



def create_vtn(blockname):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,blockname)['vtn_name']
    test_vtn_description=vtn_testconfig.ReadValues(VTNVBRDATA,blockname)['description']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VTNURL')['url']
    url= coordinator_url + vtn_url
    print url

    vtn_add = collections.defaultdict(dict)

    vtn_add['vtn']['vtn_name']=test_vtn_name
    vtn_add['vtn']['description']=test_vtn_description
    r = requests.post(url,data=json.dumps(vtn_add),headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0


def delete_vtn(blockname):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,blockname)['vtn_name']
    test_vtn_description=vtn_testconfig.ReadValues(VTNVBRDATA,blockname)['description']
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,blockname)['vtn_url']
    url= coordinator_url + vtn_url + '.json'
    print url

    r = requests.delete(url,headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0



def validate_vtn_at_controller(vtn_blockname, controller_blockname, presence="yes",position=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VTNURL')['ctr_url']

    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name
    print url
    r = requests.get(url,headers=controller_headers,auth=('admin','admin'))

    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'vtenant name : '+test_vtn_name+' is removed'
            return 0
    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data=json.loads(r.content)


    if presence == "no":
        print data['vtn']
        if data['vtn'] == []:
            return 0

    vtn_content=data['vtn'][position]
    print vtn_content

    if vtn_content == None:
        if presence == "yes":
            return 0
        else:
            return 1

    if vtn_content['name'] != test_vtn_name:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            print 'vtn name : '+vtn_content['name']+' is present'
            return 0
        else:
            return 1



def create_vbr(vtn_blockname,vbr_blockname,controller_blockname):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    test_vbr_controller_id=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['controller_id']
    test_vbr_description=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['description']
    test_vbr_domain_id =vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['domain_id']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VBRURL')['url']


    url= coordinator_url + test_vtn_url + vbr_url

    print "url is ", url

    vbr_add = collections.defaultdict(dict);
    vbr_add['vbridge']['vbr_name']=test_vbr_name
    vbr_add['vbridge']['controller_id']=test_vbr_controller_id
    vbr_add['vbridge']['domain_id']=test_vbr_domain_id
    vbr_add['vbridge']['description']=test_vbr_description


    r = requests.post(url,data=json.dumps(vbr_add),headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0



def delete_vbr(vtn_blockname,vbr_blockname):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    test_vbr_description=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['description']
    test_vbr_domain_id =vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['domain_id']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_url']
    vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_url']

    url= coordinator_url + test_vtn_url + vbr_url + '.json'

    print url

    r = requests.delete(url,headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0




def validate_vbr_at_controller(vtn_blockname, vbr_blockname,controller_blockname, presence="yes",position=0):
    test_vtn_name=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_blockname)['vtn_name']
    test_vbr_name=vtn_testconfig.ReadValues(VTNVBRDATA,vbr_blockname)['vbr_name']
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_id=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['controller_id']
    test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
    test_vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VTNURL')['ctr_url']
    test_vbr_url=vtn_testconfig.ReadValues(VTNVBRDATA,'VBRURL')['ctr_url']

    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vbr_url+'/'+test_vbr_name
    print url
    r = requests.get(url,headers=controller_headers,auth=('admin','admin'))
    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'vbridge name : '+test_vbr_name+' is removed'
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data=json.loads(r.content)

    vtn_content=data['vbridge'][position]

    print vtn_content

    if vtn_content == None:
        if presence == "yes":
            return 0
        else:
            return 1


    if vtn_content['name'] != test_vbr_name:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            print 'vbridge name : '+vtn_content['name']+' is present'
            return 0
        else:
            return 1



def test_vtn_vbr():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    print "TEST 1 : SIMPLE VTenant with one VBridge"
  # Delay for AUDIT
    time.sleep(15)
    retval=create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)


    retval=validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)


    retval=validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval = delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed----"
        exit(1)

    retval=validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval=validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VBR TEST SUCCESS"


def test_multi_vtn_with_vbr_test():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    time.sleep(15)
    print "TEST 2 : 2 Tenants with one VBridge each"
    print "VTNONE->VBRONE"
    print "VTNTWO->VBRONE"

    retval=create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=create_vtn('VtnTwo')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=create_vbr('VtnTwo','VbrOne','ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=validate_vtn_at_controller('VtnTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnTwo','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)


    retval = delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval=validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vbr('VtnTwo','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnTwo','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval=validate_vtn_at_controller('VtnTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)


    retval = delete_vtn('VtnTwo')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "MULTI VTN->VBR TEST SUCCESS"


def test_vtn_multi_vbr_test():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    time.sleep(15)
    print "TEST 3 : 1 Tenants with two VBridge"
    print "VTNONE->VBRONE"
    print "VTNONE->VBRTWO"

    retval=create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval=create_vbr('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=create_vbr('VtnOne','VbrTwo','ControllerFirst')
    if retval != 0:
        print "VBR Create Failed"
        exit(1)

    retval=validate_vtn_at_controller('VtnOne','ControllerFirst',position=0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VBROne Validate Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',position=0)
    if retval != 0:
        print "VBRTwo Validate Failed"
        exit(1)

    retval = delete_vbr('VtnOne','VbrTwo')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnOne','VbrTwo','ControllerFirst',presence="no")
    if retval != 0:
        print "VtnVBRTwo Validate Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst')
    if retval != 0:
        print "VtnVBR Validate Failed"
        exit(1)

    retval=validate_vtn_at_controller('VtnOne','ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vbr('VtnOne','VbrOne')
    if retval != 0:
        print "VBR/VTN Delete Failed"
        exit(1)

    retval=validate_vbr_at_controller('VtnOne','VbrOne','ControllerFirst',presence="no")
    if retval != 0:
        print "VBR Validate Failed"
        exit(1)

    retval=validate_vtn_at_controller('VtnOne','ControllerFirst',presence="no")
    if retval != 0:
          print "VTN Validate Failed"
          exit(1)

    retval = delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)


    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "MULTI VBR IN ONE VTN->VBR TEST SUCCESS"

# Main Block
if __name__ == '__main__':
    print '*****CONTROLLER TESTS******'
    test_vtn_vbr()
    test_multi_vtn_with_vbr_test()
    test_vtn_multi_vbr_test()

else:
    print "VTN VBR Loaded as Module"

