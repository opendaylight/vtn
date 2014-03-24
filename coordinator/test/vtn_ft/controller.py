#!/usr/bin/python

#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, sys, time
import vtn_testconfig
import resp_code

CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA
coordinator_url=vtn_testconfig.coordinator_url
def_header=vtn_testconfig.coordinator_headers

def is_controller_deleted(controller_url=""):
    url= coordinator_url + controller_url
    print url
    r = requests.get(url,headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1
    data=json.loads(r.content)
    print data['controller']
    if data['controller'] != None:
        return 1
    else:
        return 0


def validate_controller_attributes(controller_id,ipaddr="",version="",
                                   auditstatus="", description="",username="",
                                   password="",operstatus="",type="",
                                   controller_url=""):
    url= coordinator_url + controller_url
    print url
    print def_header
    r = requests.get(url,headers=def_header)
    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1

    data=json.loads(r.content)
    print data
    print data['controller']

    if ipaddr != "":
        if data['controller']['ipaddr'] != ipaddr:
            print "IP Address does not matcch"
            return 1

    if version != "":
        if data['controller']['version'] != version:
            print "Version Mismatch"
            return 1

    if auditstatus != "":
        if data['controller']['auditstatus'] != auditstatus:
            print "Audit Status Mismatch"
            return 1

    if description != "":
        if data['controller']['description'] != description:
            print "Description Mismatch"
            return 1


    if username != "":
        if data['controller']['username'] != username:
            print "username Mismatch"
            return 1

    if password != "":
        if data['controller']['password'] != username:
            print "password Mismatch"
            return 1

    if operstatus != "":
        if data['controller']['operstatus'] != operstatus:
            print "Operstatus is not", operstatus
            return 1

    print "Validation Succeeded"
    return 0


def add_controller(controller_id,type,version,ipaddr="",auditstatus="",
                   description="", username = "",
                   password = "",controller_url=""):
    print "Add Controller"
    print 'controller_id' ,controller_id
    print 'ipaddrd' ,ipaddr
    print  'type' ,type
    print 'version' ,version
    print 'auditstatus' ,auditstatus
    ip_set=0
    desc_set=0
    uname_set=0
    pwd_set=0
    audistst_set=0


    url = coordinator_url + controller_url
    add_data = collections.defaultdict(dict)

    #Add default entried
#  add_data += "'controller_id'" + ':' + controller_id
    add_data['controller']['controller_id'] = controller_id
#  add_data += "," + "'type'" + ':' + type
    add_data['controller']['type'] = type
    add_data['controller']['version'] = version
    #add_data += "," + "'version'" + ':' + version



    if ipaddr != "":
        add_data['controller']['ipaddr']=ipaddr
    else:
        print "Not set IP Address"

    if auditstatus != "":
        add_data['controller']['auditstatus']=auditstatus
    else:
        print "Not set Audit Status"


    if username != "":
        add_data['controller']['username']=username
    else:
        print "No User Name"

    if password != "":
        add_data['controller']['password']=password
    else:
        print "No Password"

    if description != "":
        add_data['controller']['description']=description
    else:
        print 'Empty Description'

    print url
    print add_data
    print json.dumps(add_data)
    r = requests.post(url,data=json.dumps(add_data),headers=def_header)
    print r.status_code
    print r.headers
    print r.content
    if r.status_code == resp_code.RESP_CREATE_SUCCESS:
        return 0
    else:
        return 1



def delete_controller(controller_url=""):
    print "Delete Controller"
    print "Delete for", controller_url
    url= coordinator_url + controller_url
    print url
    r = requests.delete(url,headers=def_header)
    print r.status_code
    print r.headers
    print r.content
    if r.status_code == resp_code.RESP_DELETE_SUCCESS:
        return 0
    else:
        return 1



def update_controller(controller_id,ipaddr="",version="",auditstatus="",
        description="",username="", password="",controller_url=""):
    print "Update Controller"
    print 'controller_id', controller_id

    url= coordinator_url + controller_url
    update_data = collections.defaultdict(dict)
    if ipaddr != "":
        update_data['controller']['ipaddr'] = ipaddr
    else:
        print "IP is not updated"

    if version != "":
        update_data['controller']['version'] =  version
    else:
        print "Version not updated"


    if auditstatus != "":
        update_data['controller']['auditstatus'] = auditstatus
    else:
        print "Audit Status not updated"


    if description != "":
        update_data['controller']['description'] = description
    else:
        print "Description is not updated"

    if username != "":
        update_data['controller']['username'] = username
    else:
        print "Username is not updated"

    if password != "":
        update_data['controller']['password'] = password
    else:
        print "Password is not updated"

    print update_data
    print json.dumps(update_data)
    print url
    r = requests.put(url,data=json.dumps(update_data),headers=def_header)
    print r.status_code
    print r.headers
    print r.content
    if r.status_code == resp_code.RESP_UPDATE_SUCCESS:
        return 0
    else:
        return 1



# Tcase Begin

def test_controller_ops(blockname):

    print "Read from controller.data"
    print  "Read for %s", blockname
    test_ipaddr = vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['ipaddr']
    test_controller_id = vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                                 blockname)['controller_id']
    test_description = vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                                 blockname)['description']
    test_type = vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['type']
    test_version = vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                             blockname)['version']
    test_auditstatus = vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                                 blockname)['auditstatus']
    test_username =  vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                               blockname)['username']
    test_password =  vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                               blockname)['password']
    test_invalusername = vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                                   blockname)['invalidusername']
    test_invalpassword =  vtn_testconfig.ReadValues(CONTROLLERDATA ,
                                                    blockname)['invalidpassword']
    test_url =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['url']
    add_url=vtn_testconfig.ReadValues(CONTROLLERDATA ,'URL')['url']



    print "******ADD CONTROLLER with NO IP Address**********"
    res=add_controller(test_controller_id,test_type,test_version,controller_url=add_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()

    vtn_testconfig.InProgress(delay=15)

    print "******VALIDATE Attributes from Coordinator*******"
    print "validate Mandatory Controller Attributes "
    res = validate_controller_attributes(test_controller_id,type=test_type,version=test_version,operstatus="down",controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()


    print "******Update IP Address************************"
    res = update_controller(test_controller_id,ipaddr=test_ipaddr,controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()

    print "*****VALIDATE IP ADDRESS UPDATE***************"
    res = validate_controller_attributes(test_controller_id,type=test_type,version=test_version,operstatus="down",ipaddr=test_ipaddr,controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()

    print "*****UPDATE AUDITSTATUS***************"
    res = update_controller(test_controller_id,auditstatus="enable",controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()

    vtn_testconfig.InProgress(delay=15)
    print "*****CHECK AUDITSTATUS***************"
    print "*****USES DEF USERNAME and PASSWORD***************"
    res = validate_controller_attributes(test_controller_id,version=test_version,ipaddr=test_ipaddr,operstatus ="up",auditstatus="enable",controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()

    print "*****UPDATE INVALID USERNAME/PASS***************"
    res = update_controller(test_controller_id,username=test_invalusername,password=test_invalpassword,controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()

    print "*****CHECK OPERSTATUS DOWN***************"
#Added to make sure audit operations complete!!
    vtn_testconfig.InProgress(delay=15)
    res = validate_controller_attributes(test_controller_id,version=test_version,ipaddr=test_ipaddr,operstatus ="down",auditstatus="enable",username=test_invalusername,password=test_invalpassword,controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()


    print "*****UPDATE VALID USERNAME/PASS***************"
    res = update_controller(test_controller_id,username=test_username,password=test_password,controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()

    print "*****CHECK OPERSTATUS UP Again***************"
    print "*****USES VALUES SET in ADD COMMAND***************"

#Added to make sure audit operations complete!!
    vtn_testconfig.InProgress(delay=15)
    res = validate_controller_attributes(test_controller_id,version=test_version,ipaddr=test_ipaddr,operstatus ="up",auditstatus="enable",username=test_username,password=test_password,controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()


    print "*****DELETE CONTROLLER***************"
    res = delete_controller(controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()

    print "*****VALIDATE DELETE***************"
    res = is_controller_deleted(controller_url=test_url)
    if res == 0:
        print "Test Success"
    else:
        print "Test Failed!!!"
        exit ()


def add_controller_ex(blockname):
    test_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['ipaddr']
    test_controller_id=vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['controller_id']
    test_description = vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['description']
    test_type = vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['type']
    test_version = vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['version']
    test_auditstatus = vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['auditstatus']
    test_username =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['username']
    test_password =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['password']
    test_invalusername =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['invalidusername']
    test_invalpassword =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['invalidpassword']
    test_url =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['url']
    add_url=vtn_testconfig.ReadValues(CONTROLLERDATA ,'URL')['url']

    res=add_controller(test_controller_id,test_type,test_version,controller_url=add_url,ipaddr=test_ipaddr,auditstatus=test_auditstatus,username=test_username,password=test_password)
    return res


def delete_controller_ex(blockname):
    test_url =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['url']
    return delete_controller(test_url)


def update_controller_ex(blockname,ipaddr="",version="",auditstatus="",
        description="",username="", password=""):
    test_controller_id=vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['controller_id']
    test_url =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['url']
    return update_controller(test_controller_id,ipaddr=ipaddr,version=version,
            auditstatus=auditstatus,description=description,
            username=username,password=password,controller_url=test_url)

#To check the state of the conttroller

def check_controller_state(blockname,state):
    test_url =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['url']
    url = coordinator_url + test_url

    r = requests.get(url,headers=def_header)
    while(1):
        try:
            data = json.loads(r.content)
            break
        except ValueError:
            continue

    if data['controller']['operstatus'] == state:
      return 0
    else:
      return 1

#Wait until the controller state is up/down

def wait_until_state(blockname,state):
    #print "Wait until the controler state is",state
    i = 1
    retval = 1
    test_controller_id = vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['controller_id']
    test_url =  vtn_testconfig.ReadValues(CONTROLLERDATA ,blockname)['url']
    while(retval == 1):
        sys.stdout.write("\rWaiting..%ds.." %i)
        sys.stdout.flush()
        time.sleep(4)
        i+=4
        if i == 45:
          return 1
        retval = check_controller_state(blockname,state)
    return 0

# Main Block
if __name__ == '__main__':
    print '*****CONTROLLER TESTS******'
    test_controller_ops('ControllerFirst')
else:
    print "Controller Loaded as Module"
