#!/usr/bin/python

#
# Copyright (c) 2014-2016 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, controller
import vtn_testconfig
import resp_code

VTNVTERMDATA = vtn_testconfig.VTNVTERMDATA
CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA

coordinator_url = vtn_testconfig.coordinator_url
def_header = vtn_testconfig.coordinator_headers
controller_headers = vtn_testconfig.controller_headers
controller_url_part = vtn_testconfig.controller_url_part



def create_vtn(blockname):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA, blockname)['vtn_name']
    test_vtn_description = vtn_testconfig.ReadValues(VTNVTERMDATA, blockname)['description']
    vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA, 'VTNURL')['url']
    url= coordinator_url + vtn_url
    print url

    vtn_add = collections.defaultdict(dict)

    vtn_add['vtn']['vtn_name'] = test_vtn_name
    vtn_add['vtn']['description'] = test_vtn_description
    r = requests.post(url, data = json.dumps(vtn_add), headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0


def delete_vtn(blockname):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA, blockname)['vtn_name']
    test_vtn_description = vtn_testconfig.ReadValues(VTNVTERMDATA, blockname)['description']
    vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA, blockname)['vtn_url']
    url= coordinator_url + vtn_url + '.json'
    print url

    r = requests.delete(url, headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0



def validate_vtn_at_controller(vtn_blockname, controller_blockname, presence = "yes", position = 0):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA, vtn_blockname)['vtn_name']
    test_controller_ipaddr = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['ipaddr']
    test_controller_port = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['restconf_port']
    test_vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA, 'VTNURL')['ctr_url']

    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name
    print url
    r = requests.get(url, headers = controller_headers, auth = ('admin', 'admin'))

    print r.status_code

    if presence  == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'vtenant name : '+test_vtn_name+' is removed'
            return 0
    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data = json.loads(r.content)


    if presence == "no":
        print data['vtn']
        if data['vtn'] == []:
            return 0

    vtn_content = data['vtn'][position]
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



def create_vterm(vtn_blockname, vterm_blockname, controller_blockname):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA, vtn_blockname)['vtn_name']
    test_vterminal_name = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_blockname)['vterminal_name']
    test_vterm_controller_id = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['controller_id']
    test_vterm_description = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_blockname)['description']
    test_vterm_domain_id  = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_blockname)['domain_id']
    test_vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA, vtn_blockname)['vtn_url']
    vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA, 'VTERMURL')['url']


    url= coordinator_url + test_vtn_url + vterm_url

    print "url is ", url

    vterm_add = collections.defaultdict(dict);
    vterm_add['vterminal']['vterminal_name'] = test_vterminal_name
    vterm_add['vterminal']['controller_id'] = test_vterm_controller_id
    vterm_add['vterminal']['domain_id'] = test_vterm_domain_id
    vterm_add['vterminal']['description'] = test_vterm_description


    r = requests.post(url, data = json.dumps(vterm_add), headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0



def delete_vterm(vtn_blockname, vterm_blockname):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA, vtn_blockname)['vtn_name']
    test_vterminal_name = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_blockname)['vterminal_name']
    test_vterm_description = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_blockname)['description']
    test_vterm_domain_id  = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_blockname)['domain_id']
    test_vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA, vtn_blockname)['vtn_url']
    vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_blockname)['vterm_url']

    url= coordinator_url + test_vtn_url + vterm_url + '.json'

    print url

    r = requests.delete(url, headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0




def validate_vterm_at_controller(vtn_blockname, vterm_blockname, controller_blockname, presence = "yes", position = 0):
    test_vtn_name = vtn_testconfig.ReadValues(VTNVTERMDATA, vtn_blockname)['vtn_name']
    test_vterminal_name = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_blockname)['vterminal_name']
    test_controller_ipaddr = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['ipaddr']
    test_controller_id = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['controller_id']
    test_controller_port = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['restconf_port']
    test_vtn_url = vtn_testconfig.ReadValues(VTNVTERMDATA, 'VTNURL')['ctr_url']
    test_vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA, 'VTERMURL')['ctr_url']

    url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url+'/'+test_vtn_name+test_vterm_url+'/'+test_vterminal_name
    print url
    r = requests.get(url, headers = controller_headers, auth = ('admin', 'admin'))
    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            print 'vterminal name : '+test_vterminal_name+' is removed'
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1


    data = json.loads(r.content)

    vtn_content = data['vterminal'][position]

    print vtn_content

    if vtn_content == None:
        if presence == "yes":
            return 0
        else:
            return 1


    if vtn_content['name'] != test_vterminal_name:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            print 'vterminal name : '+vtn_content['name']+' is present'
            return 0
        else:
            return 1



def test_vtn_vterm():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    print "TEST 1 : SIMPLE VTenant with one VTerminal"
  # Delay for AUDIT
    time.sleep(15)
    retval = create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = create_vterm('VtnOne', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)


    retval = validate_vtn_at_controller('VtnOne', 'ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)


    retval = validate_vterm_at_controller('VtnOne', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval = delete_vterm('VtnOne', 'VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed----"
        exit(1)

    retval = validate_vterm_at_controller('VtnOne', 'VTermOne', 'ControllerFirst', presence = "no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval = validate_vtn_at_controller('VtnOne', 'ControllerFirst', presence = "no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)

    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)
    print "VTN->VTERM TEST SUCCESS"


def test_multi_vtn_with_vterm_test():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    time.sleep(15)
    print "TEST 2 : 2 Tenants with one VTerminal each"
    print "VTNONE->VTERMONE"
    print "VTNTWO->VTERMONE"

    retval = create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = create_vtn('VtnTwo')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = create_vterm('VtnOne', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = create_vterm('VtnTwo', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = validate_vtn_at_controller('VtnOne', 'ControllerFirst', position = 0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = validate_vtn_at_controller('VtnTwo', 'ControllerFirst', position = 0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnOne', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnTwo', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)


    retval = delete_vterm('VtnOne', 'VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnOne', 'VTermOne', 'ControllerFirst', presence = "no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval = validate_vtn_at_controller('VtnOne', 'ControllerFirst', presence = "no")
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vterm('VtnTwo', 'VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnTwo', 'VTermOne', 'ControllerFirst', presence = "no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval = validate_vtn_at_controller('VtnTwo', 'ControllerFirst', presence = "no")
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
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "MULTI VTN->VTERM TEST SUCCESS"


def test_vtn_multi_vterm_test():

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        exit(1)

    time.sleep(15)
    print "TEST 3 : 1 Tenants with two VTerminal"
    print "VTNONE->VTERMONE"
    print "VTNONE->VTERMTWO"

    retval = create_vtn('VtnOne')
    if retval != 0:
        print "VTN Create Failed"
        exit(1)

    retval = create_vterm('VtnOne', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VTERM Create Failed"
        exit(1)

    retval = create_vterm('VtnOne', 'VTermTwo', 'ControllerFirst')
    if retval != 0:
        print "VTERM Create Failed"
        exit(1)

    retval = validate_vtn_at_controller('VtnOne', 'ControllerFirst', position = 0)
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnOne', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VTERMOne Validate Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnOne', 'VTermTwo', 'ControllerFirst', position = 0)
    if retval != 0:
        print "VTERMTwo Validate Failed"
        exit(1)

    retval = delete_vterm('VtnOne', 'VTermTwo')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnOne', 'VTermTwo', 'ControllerFirst', presence = "no")
    if retval != 0:
        print "VtnVTERMTwo Validate Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnOne', 'VTermOne', 'ControllerFirst')
    if retval != 0:
        print "VtnVTERM Validate Failed"
        exit(1)

    retval = validate_vtn_at_controller('VtnOne', 'ControllerFirst')
    if retval != 0:
        print "VTN Validate Failed"
        exit(1)

    retval = delete_vterm('VtnOne', 'VTermOne')
    if retval != 0:
        print "VTERM/VTN Delete Failed"
        exit(1)

    retval = validate_vterm_at_controller('VtnOne', 'VTermOne', 'ControllerFirst', presence = "no")
    if retval != 0:
        print "VTERM Validate Failed"
        exit(1)

    retval = validate_vtn_at_controller('VtnOne', 'ControllerFirst', presence = "no")
    if retval != 0:
          print "VTN Validate Failed"
          exit(1)

    retval = delete_vtn('VtnOne')
    if retval != 0:
        print "VTN Delete Failed in coordinator"
        exit(1)


    print "DELETE CONTROLLER"
    retval = controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        exit(1)

    print "MULTI VTERM IN ONE VTN->VTERM TEST SUCCESS"

# Main Block
if __name__ == '__main__':
    print '*****CONTROLLER TESTS******'
    test_vtn_vterm()
    test_multi_vtn_with_vterm_test()
    test_vtn_multi_vterm_test()

else:
    print "VTN VTERM Loaded as Module"

