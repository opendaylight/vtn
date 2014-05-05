#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

#! /usr/bin/python

import requests, json, collections, sys, time, subprocess, controller, vtn_vbr
import vtn_testconfig, pexpect, mininet_test
import resp_code

SWITCHDATA = vtn_testconfig.SWITCHDATA
VTNVBRDATA = vtn_testconfig.VTNVBRDATA
CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA
MININETDATA = vtn_testconfig.MININETDATA
PORTDATA = vtn_testconfig.PORTDATA

coordinator_url = vtn_testconfig.coordinator_url
def_header = vtn_testconfig.coordinator_headers
controller_headers = vtn_testconfig.controller_headers
coordinator_headers = vtn_testconfig.coordinator_headers
controller_url_part = vtn_testconfig.controller_url_part

def validate_switch_at_physical(switch_blockname,controller_blockname,presence="yes",position=0):
    ctr_url=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['get_url']
    get_url=vtn_testconfig.ReadValues(SWITCHDATA,"GETURL")['sw_detail_url']
    switch_id=vtn_testconfig.ReadValues(SWITCHDATA,switch_blockname)['switch_id']
    url =  coordinator_url + ctr_url + get_url
    print url
    r=requests.get(url, headers=coordinator_headers,auth=('admin','adminpass'))
    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1

    data=json.loads(r.content)
    print data

    if data['switches'] == []:
        print "no switches"
        return 0

    switch_content = data['switches'][position]
    print switch_content['switch_id']

    if switch_content['switch_id'] != switch_id:
        if presence == "yes":
            return 1
        else:
            return 0
    else:
        if presence == "yes":
            return 0
        else:
            return 1

def update_switch(switch_blockname, controller_blockname):
    print "update switch"
    ctr_url = vtn_testconfig.ReadValues(SWITCHDATA, "GETURL")['controller_url']
    switch_url = vtn_testconfig.ReadValues(SWITCHDATA, 'GETURL')['switch_url']
    switch_id = vtn_testconfig.ReadValues(SWITCHDATA, switch_blockname)['switch_id']
    default_url = vtn_testconfig.ReadValues(SWITCHDATA, "GETURL")['default_url']
    description = vtn_testconfig.ReadValues(SWITCHDATA, switch_blockname)['description']
    test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
    test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['port']
    url = 'http://'+test_controller_ipaddr+':'+test_controller_port+'/'+ctr_url+ switch_url + switch_id + default_url + description
    print url

    r = requests.put(url,headers=controller_headers,auth=('admin','admin'))
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS:
        return 1
    else:
        return 0

def validate_update_switch(switch_blockname, controller_blockname,presence="yes", position = 0):
    print "validate update switch"
    ctr_url=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['get_url']
    switch_url = vtn_testconfig.ReadValues(SWITCHDATA, 'GETURL')['url']
    switch_id = vtn_testconfig.ReadValues(SWITCHDATA, switch_blockname)['switch_id']
    description = vtn_testconfig.ReadValues(SWITCHDATA, switch_blockname)['description']

    url = coordinator_url + ctr_url + '/switches/' +switch_id +switch_url
    print url

    r = requests.get(url,headers=def_header)
    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1

    data=json.loads(r.content)
    print data

    if data['switch'] == [ ] or data['switch'] == None:
        print "no switches"
        return 1
    switch_content = data['switch']
    print switch_content['description']
    print description

    if switch_content['description'] == description:
        if presence == "yes":
            return 0
        else:
            return 1

def validate_switch_port(switch_blockname, port_blockname, controller_blockname, child, presence="yes", position=0):
    print "validate switch port details"
    ctr_url=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['get_url']
    switch_id = vtn_testconfig.ReadValues(SWITCHDATA, switch_blockname)['switch_id']
    port_id = vtn_testconfig.ReadValues(PORTDATA, port_blockname)['port_id']
    port_url = vtn_testconfig.ReadValues(PORTDATA, 'URL')['port_url']

    url = coordinator_url + ctr_url + '/switches/' + switch_id + port_url
    print url

    r = requests.get(url, headers=coordinator_headers,auth=('admin','adminpass'))
    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1

    data=json.loads(r.content)

    if presence == "no":
        print data['ports']
        if data['ports'] == []:
            print "no ports"
            return 0
    else:
        port_content = data['ports'][position]
        print port_content['port_id']
        if port_content['port_id'] != port_id:
            return 1
        else:
            return 0

def verify_switch_port(switch_blockname, port_blockname, controller_blockname, child, ctr_child, presence="yes", position=0):
    retval = validate_switch_port(switch_blockname, port_blockname, controller_blockname, child, presence, position)
    if retval != 0:
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        print "validate", switch_blockname, port_blockname, "failed"
        exit(1)

def verify_multictr_switch_port(switch_blockname, port_blockname, controller_blockname, child1, child2, ctr1_child, ctr2_child, presence="yes", position=0):
    retval = validate_switch_port(switch_blockname, port_blockname, controller_blockname, child1, presence, position)
    if retval != 0:
        mininet_test.close_topology(child1)
        mininet_test.close_topology(child2)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        print "validate", switch_blockname, port_blockname, "failed"
        exit(1)

def validate_logical_port(port_blockname, controller_blockname, child, presence = "yes", position = 0):
    print "validate logical port details"
    ctr_url=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['get_url']
    logical_port_id = vtn_testconfig.ReadValues(PORTDATA, port_blockname)['logical_port_id']
    domain_id = vtn_testconfig.ReadValues(PORTDATA, 'URL')['domain_id']
    lp_url = vtn_testconfig.ReadValues(PORTDATA, 'URL')['lp_port_url']

    url = coordinator_url + ctr_url + '/domains/' + domain_id + lp_url
    print url

    r = requests.get(url, headers=coordinator_headers,auth=('admin','adminpass'))
    print r.status_code

    if presence == "no":
      if r.status_code == resp_code.RESP_NOT_FOUND:
          return 0

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1

    data=json.loads(r.content)

    if presence == "no":
        print data['logical_ports']
        if data['ports'] == []:
          print "no ports"
          return 0
    else:
        port_content = data['logical_ports'][position]
        print port_content, '\n'
        print port_content['logical_port_id']
        if port_content['logical_port_id'] != logical_port_id:
              return 1
        else:
              return 0

def verify_logical_port(port_blockname, controller_blockname, child, ctr_child, presence = "yes", position = 0):
    retval = validate_logical_port(port_blockname, controller_blockname, child, presence, position)
    if retval != 0:
        print "validate", port_blockname,"failed"
        mininet_test.close_topology(child),
        stop_controller(ctr_child)
        exit(1)

def verify_multictr_logical_port(port_blockname, controller_blockname, child1, child2, ctr1_child, ctr2_child, presence = "yes", position = 0):
    retval = validate_logical_port(port_blockname, controller_blockname, child1, presence, position)
    if retval != 0:
        print "validate", port_blockname,"failed"
        mininet_test.close_topology(child1)
        mininet_test.close_topology(child2)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

def link_down(child, switch, host):
    cmd = 'link' + ' ' + switch + ' ' + host + ' ' + 'down'
    print cmd
    if child.isalive() == True :
        print "process alive\n"
        child.sendline(cmd)
        print "Toggling link of", switch, host, "passed"
        return 0
    else:
        print "child process closed"
        return 1

def validate_link_down(controller_blockname, switch_blockname, port_blockname, status, position=0):
    print "validate link_down"
    ctr_url=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['get_url']
    controller_id = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['controller_id']
    switch_id = vtn_testconfig.ReadValues(SWITCHDATA, switch_blockname)['switch_id']
    port_url = vtn_testconfig.ReadValues(PORTDATA, 'URL')['port_url']

    url = coordinator_url + ctr_url + '/switches/' + switch_id + port_url
    print url

    r = requests.get(url, headers=coordinator_headers,auth=('admin','adminpass'))
    print r.status_code

    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1

    data=json.loads(r.content)

    port_content = data['ports'][position]
    print port_content, '\n'
    if port_content['operstatus'] != status:
        return 1
    else:
        return 0

def insync(delay):
    for i in range(delay):
        time.sleep(1)
        sys.stdout.write("\rWaiting for in sync with controller..%ds.." %i)
        sys.stdout.flush()

def start_controller(controller_blockname):
    ctr_ipaddr = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['ipaddr']
    machine_username = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['machine_username']
    ssh = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['ssh']
    ssh_newkey = 'Are you sure you want to continue connecting'

    cmd = ssh + ' ' + machine_username + ' ' + ctr_ipaddr
    print cmd

    child = pexpect.spawn(cmd)
    index = child.expect([ssh_newkey, 'password:', pexpect.EOF, pexpect.TIMEOUT])
    if index == 0:
        print "I say yes"
        child.sendline('yes')
        index = child.expect([ssh_newkey, 'password:', pexpect.EOF, pexpect.TIMEOUT])
    if index == 1:
        print "sending password"
        child.sendline('uncunc')
        print "connection OK"
        child.sendline("cd opendaylight")
        child.expect('opendaylight]#')
        cmd = './run.sh -virt vtn'
        child.sendline(cmd)
        print('Starting controller')
        return child
    elif index == 2:
        print "2:connection timeout"
        return child
    elif index == 3:
        print "3:connection timeout"
        return child

def stop_controller(child):
    if child.isalive() == True :
        print "process alive\n"
        child.sendline('exit')
        child.expect('(y/n; default=y)')
        child.sendline('y')
        print "Controller Stoppped"
        child.close()
    else:
        print "child process already closed"

#Test cases are being started from here

def test(ctr_child):
    child=mininet_test.create_mininet_topology('MININETONE','ControllerFirst','2')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(60)

    retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst','yes',0)
    if retval != 0:
        print "switch1 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchTwo', 'ControllerFirst','yes',1)
    if retval != 0:
        print "switch2 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchThree', 'ControllerFirst','yes',2)
    if retval != 0:
        print "switch3 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = update_switch('SwitchOne', 'ControllerFirst')
    if retval != 0:
        print "switch1 update failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = update_switch('SwitchTwo', 'ControllerFirst')
    if retval != 0:
        print "switch2 update failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = update_switch('SwitchThree', 'ControllerFirst')
    if retval != 0:
        print "switch3 update failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(60)

    retval = validate_update_switch('SwitchOne', 'ControllerFirst',position = 0)
    if retval != 0:
        print "switch1 validate update failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_update_switch('SwitchTwo', 'ControllerFirst',position = 1)
    if retval != 0:
        print "switch2 validate update failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_update_switch('SwitchThree', 'ControllerFirst',position = 2)
    if retval != 0:
        print "switch3 validate update failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    verify_switch_port('SwitchOne', 'PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchOne', 'PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)

    verify_switch_port('SwitchTwo', 'PortThree', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchTwo', 'PortFour', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchTwo', 'PortFive', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchThree', 'PortSix', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchThree', 'PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchThree', 'PortEight', 'ControllerFirst', child, ctr_child, 'yes', 2)

    insync(40)
    verify_logical_port('PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_logical_port('PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_logical_port('PortThree', 'ControllerFirst', child, ctr_child, 'yes', 2)
    verify_logical_port('PortFour', 'ControllerFirst', child, ctr_child, 'yes', 3)
    verify_logical_port('PortFive', 'ControllerFirst', child, ctr_child, 'yes', 4)
    verify_logical_port('PortSix', 'ControllerFirst', child, ctr_child, 'yes', 5)
    verify_logical_port('PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 6)
    verify_logical_port('PortEight', 'ControllerFirst', child, ctr_child, 'yes', 7)

    mininet_test.close_topology(child)
    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
       print "CONTROLLER delete failed"
       stop_controller(ctr_child)
       exit(1)
    print "PHYSICAL READ TEST SUCCESS"

def test_physical_read_1(ctr_child):
    print "TEST 1"
    #create mininet topology
    child = mininet_test.create_mininet_topology('MININETONE', 'ControllerFirst', '2')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        print "Controller Create Failed"
        exit(1)
# Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst', "up")
    if retval != 0:
       mininet_test.close_topology(child)
       stop_controller(ctr_child)
       print "Controller state check failed"
       exit(1)

#stop and start vtn
    print "stopping vtn"
    subprocess.Popen("/usr/local/vtn/bin/vtn_stop", stdout=subprocess.PIPE)
    time.sleep(5)
    print "starting vtn"
    subprocess.Popen("/usr/local/vtn/bin/vtn_start", stdout=subprocess.PIPE)
    time.sleep(5)

#Adding controller again, and cheking the physical table
    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)
# Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst', "up")
    if retval != 0:
        print "Controller state check failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(60)

    retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst', 'yes', 0)
    if retval != 0:
        print "switch1 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchTwo', 'ControllerFirst', 'yes', 1)
    if retval != 0:
        print "switch2 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchThree', 'ControllerFirst', 'yes', 2)
    if retval != 0:
        print "switch3 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    verify_switch_port('SwitchOne', 'PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchOne', 'PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)

    verify_switch_port('SwitchTwo', 'PortThree', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchTwo', 'PortFour', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchThree', 'PortFive', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchThree', 'PortSix', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchThree', 'PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchThree', 'PortEight', 'ControllerFirst', child, ctr_child, 'yes', 2)

    insync(40)
    verify_logical_port('PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_logical_port('PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)

    verify_logical_port('PortThree', 'ControllerFirst', child, ctr_child, 'yes', 2)
    verify_logical_port('PortFour', 'ControllerFirst', child, ctr_child, 'yes', 3)
    verify_logical_port('PortFive', 'ControllerFirst', child, ctr_child, 'yes', 4)

    verify_logical_port('PortSix', 'ControllerFirst', child, ctr_child, 'yes', 5)
    verify_logical_port('PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 6)
    verify_logical_port('PortEight', 'ControllerFirst', child, ctr_child, 'yes', 7)

#close Topology
    mininet_test.close_topology(child)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
      stop_controller(ctr_child)
      print "CONTROLLER delete failed"
      exit(1)

    print "test_physical_read_1 SUCESS"

def test_physical_read_2(ctr_child):
    print "TEST 2"
#create mininet topology
    child = mininet_test.create_mininet_topology('MININETONE', 'ControllerFirst', '2')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)
# Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst', "up")
    if retval != 0:
        print "Controller state check failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(60)

    retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst', 'yes', 0)
    if retval != 0:
        print "switch1 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchTwo', 'ControllerFirst', 'yes', 1)
    if retval != 0:
        print "switch2 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchThree', 'ControllerFirst', 'yes', 2)
    if retval != 0:
        print "switch3 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    verify_switch_port('SwitchOne', 'PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchOne', 'PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)

    verify_switch_port('SwitchTwo', 'PortThree', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchTwo', 'PortFour', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchTwo', 'PortFive', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchThree', 'PortSix', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchThree', 'PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchThree', 'PortEight', 'ControllerFirst', child, ctr_child, 'yes', 2)

    insync(40)
    verify_logical_port('PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_logical_port('PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)

    verify_logical_port('PortThree', 'ControllerFirst', child, ctr_child, 'yes', 2)
    verify_logical_port('PortFour', 'ControllerFirst', child, ctr_child, 'yes', 3)
    verify_logical_port('PortFive', 'ControllerFirst', child, ctr_child, 'yes', 4)

    verify_logical_port('PortSix', 'ControllerFirst', child, ctr_child, 'yes', 5)
    verify_logical_port('PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 6)
    verify_logical_port('PortEight', 'ControllerFirst', child, ctr_child, 'yes', 7)

#Making the link down of S3 H1
    retval = link_down(child, 's3', 'h3')
    if retval != 0:
        print "Toggling of link s3 h3 down failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(40)
#checking for link down
    retval = validate_link_down('ControllerFirst', 'SwitchThree', 'PortSix', 'down', 0)
    if retval != 0:
        print "s3 h3 is not down"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

#close Topology
    mininet_test.close_topology(child)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        stop_controller(ctr_child)
        exit(1)

    print "test_physical_read_2 SUCESS"

def test_physical_read_3(ctr_child):
    print "TEST 3"
#create mininet topology
    child = mininet_test.create_mininet_topology('MININETONE', 'ControllerFirst', '2')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)
# Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst', "up")
    if retval != 0:
        print "Controller state check failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(60)

    retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst', 'yes', 0)
    if retval != 0:
        print "switch1 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchTwo', 'ControllerFirst', 'yes', 1)
    if retval != 0:
        print "switch2 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchThree', 'ControllerFirst', 'yes', 2)
    if retval != 0:
        print "switch3 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    verify_switch_port('SwitchOne', 'PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchOne', 'PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)

    verify_switch_port('SwitchTwo', 'PortThree', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchTwo', 'PortFour', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchTwo', 'PortFive', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchThree', 'PortSix', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchThree', 'PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchThree', 'PortEight', 'ControllerFirst', child, ctr_child, 'yes', 2)

    insync(40)
    verify_logical_port('PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_logical_port('PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_logical_port('PortThree', 'ControllerFirst', child, ctr_child, 'yes', 2)
    verify_logical_port('PortFour', 'ControllerFirst', child, ctr_child, 'yes', 3)
    verify_logical_port('PortFive', 'ControllerFirst', child, ctr_child, 'yes', 4)
    verify_logical_port('PortSix', 'ControllerFirst', child, ctr_child, 'yes', 5)
    verify_logical_port('PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 6)
    verify_logical_port('PortEight', 'ControllerFirst', child, ctr_child, 'yes', 7)

    mininet_test.close_topology(child)

#create mininet topology with change in topology tree
    child = mininet_test.create_mininet_topology('MININETONE', 'ControllerFirst', '3')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(80)

    retval = validate_switch_at_physical('SwitchFour', 'ControllerFirst', 'yes', 3)
    if retval != 0:
        print "switch4 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchFive', 'ControllerFirst', 'yes', 4)
    if retval != 0:
        print "switch5 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchSix', 'ControllerFirst', 'yes', 5)
    if retval != 0:
        print "switch6 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchSeven', 'ControllerFirst', 'yes', 6)
    if retval != 0:
        print "switch7 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    verify_switch_port('SwitchFour', 'PortNine', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchFour', 'PortTen', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchFour', 'PortEleven', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchFive', 'PortTwelve', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchFive', 'PortThirteen', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchFive', 'PortFourteen', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchSix', 'PortFifteen', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchSix', 'PortSixteen', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchSix', 'PortSeventeen', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchSeven', 'PortEighteen', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchSeven', 'PortNineteen', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchSeven', 'PortTwenty', 'ControllerFirst', child, ctr_child, 'yes', 2)

    insync(40)
    verify_logical_port('PortNine', 'ControllerFirst', child, ctr_child, 'yes', 8)
    verify_logical_port('PortTen', 'ControllerFirst', child, ctr_child, 'yes', 9)
    verify_logical_port('PortEleven', 'ControllerFirst', child, ctr_child, 'yes', 10)
    verify_logical_port('PortTwelve', 'ControllerFirst', child, ctr_child, 'yes', 11)
    verify_logical_port('PortThirteen', 'ControllerFirst', child, ctr_child, 'yes', 12)
    verify_logical_port('PortFourteen', 'ControllerFirst', child, ctr_child, 'yes', 13)
    verify_logical_port('PortFifteen', 'ControllerFirst', child, ctr_child, 'yes', 14)
    verify_logical_port('PortSixteen', 'ControllerFirst', child, ctr_child, 'yes', 15)
    verify_logical_port('PortSeventeen', 'ControllerFirst', child, ctr_child, 'yes', 16)
    verify_logical_port('PortEighteen', 'ControllerFirst', child, ctr_child, 'yes', 17)
    verify_logical_port('PortNineteen', 'ControllerFirst', child, ctr_child, 'yes', 18)
    verify_logical_port('PortTwenty', 'ControllerFirst', child, ctr_child, 'yes', 19)

#close Topology
    mininet_test.close_topology(child)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        stop_controller(ctr_child)
        exit(1)

    print "test_physical_read_3 SUCESS"

def test_physical_read_4(ctr_child):
    print "TEST 4"
#create mininet topology
    child = mininet_test.create_mininet_topology('MININETONE', 'ControllerFirst', '3')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    print "CREATE Controller"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)
# Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst', "up")
    if retval != 0:
        print "Controller state check failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(60)

    retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst', 'yes', 0)
    if retval != 0:
        print "switch1 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchTwo', 'ControllerFirst', 'yes', 1)
    if retval != 0:
        print "switch2 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchThree', 'ControllerFirst', 'yes', 2)
    if retval != 0:
        print "switch3 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchFour', 'ControllerFirst', 'yes', 3)
    if retval != 0:
        print "switch4 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchFive', 'ControllerFirst', 'yes', 4)
    if retval != 0:
        print "switch5 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchSix', 'ControllerFirst', 'yes', 5)
    if retval != 0:
        print "switch6 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchSeven', 'ControllerFirst', 'yes', 6)
    if retval != 0:
        print "switch7 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    verify_switch_port('SwitchOne', 'PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchOne', 'PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)

    verify_switch_port('SwitchTwo', 'PortThree', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchTwo', 'PortFour', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchTwo', 'PortFive', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchThree', 'PortSix', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchThree', 'PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchThree', 'PortEight', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchFour', 'PortNine', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchFour', 'PortTen', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchFour', 'PortEleven', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchFive', 'PortTwelve', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchFive', 'PortThirteen', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchFive', 'PortFourteen', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchSix', 'PortFifteen', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchSix', 'PortSixteen', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchSix', 'PortSeventeen', 'ControllerFirst', child, ctr_child, 'yes', 2)

    verify_switch_port('SwitchSeven', 'PortEighteen', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_switch_port('SwitchSeven', 'PortNineteen', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_switch_port('SwitchSeven', 'PortTwenty', 'ControllerFirst', child, ctr_child, 'yes', 2)

    insync(40)
    verify_logical_port('PortOne', 'ControllerFirst', child, ctr_child, 'yes', 0)
    verify_logical_port('PortTwo', 'ControllerFirst', child, ctr_child, 'yes', 1)
    verify_logical_port('PortThree', 'ControllerFirst', child, ctr_child, 'yes', 2)
    verify_logical_port('PortFour', 'ControllerFirst', child, ctr_child, 'yes', 3)
    verify_logical_port('PortFive', 'ControllerFirst', child, ctr_child, 'yes', 4)
    verify_logical_port('PortSix', 'ControllerFirst', child, ctr_child, 'yes', 5)
    verify_logical_port('PortSeven', 'ControllerFirst', child, ctr_child, 'yes', 6)
    verify_logical_port('PortEight', 'ControllerFirst', child, ctr_child, 'yes', 7)
    verify_logical_port('PortNine', 'ControllerFirst', child, ctr_child, 'yes', 8)
    verify_logical_port('PortTen', 'ControllerFirst', child, ctr_child, 'yes', 9)
    verify_logical_port('PortEleven', 'ControllerFirst', child, ctr_child, 'yes', 10)
    verify_logical_port('PortTwelve', 'ControllerFirst', child, ctr_child, 'yes', 11)
    verify_logical_port('PortThirteen', 'ControllerFirst', child, ctr_child, 'yes', 12)
    verify_logical_port('PortFourteen', 'ControllerFirst', child, ctr_child, 'yes', 13)
    verify_logical_port('PortFifteen', 'ControllerFirst', child, ctr_child, 'yes', 14)
    verify_logical_port('PortSixteen', 'ControllerFirst', child, ctr_child, 'yes', 15)
    verify_logical_port('PortSeventeen', 'ControllerFirst', child, ctr_child, 'yes', 16)
    verify_logical_port('PortEighteen', 'ControllerFirst', child, ctr_child, 'yes', 17)
    verify_logical_port('PortNineteen', 'ControllerFirst', child, ctr_child, 'yes', 18)
    verify_logical_port('PortTwenty', 'ControllerFirst', child, ctr_child, 'yes', 19)

    mininet_test.close_topology(child)

#create mininet topology with change in topology tree
    child = mininet_test.create_mininet_topology('MININETONE', 'ControllerFirst', '2')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    insync(80)

    verify_switch_port('SwitchFour', 'PortNine', 'ControllerFirst', child, ctr_child, 'no', 0)
    verify_switch_port('SwitchFive', 'PortTwelve', 'ControllerFirst', child, ctr_child, 'no', 0)
    verify_switch_port('SwitchSix', 'PortFifteen', 'ControllerFirst', child, ctr_child, 'no', 0)
    verify_switch_port('SwitchSeven', 'PortEighteen', 'ControllerFirst', child, ctr_child, 'no', 0)

#close Topology
    mininet_test.close_topology(child)

    print "DELETE CONTROLLER"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER delete failed"
        stop_controller(ctr_child)
        exit(1)

    print "test_physical_read_4 SUCESS"

def test_multi_ctr_multi_mininet(ctr1_child, ctr2_child):
    child1 = mininet_test.create_mininet_topology('MININETONE','ControllerFirst','2')
    if child1.isalive() == True :
        print "Topology1 creation Success!!!"
    else:
        print "Topology1 creation Failed"
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    print "CREATE Controller1"
    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller1 Create Failed"
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    insync(60)

    child2 = mininet_test.create_mininet_topology('MININETTWO','ControllerSecond','2')
    if child2.isalive() == True :
        print "Topology2 creation Success!!!"
    else:
        print "Topology2 creation Failed"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    print "CREATE Controller2"
    retval = controller.add_controller_ex('ControllerSecond')
    if retval != 0:
        print "Controller2 Create Failed"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    insync(60)

    retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst','yes',0)
    if retval != 0:
        print "switch1 validate failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchTwo', 'ControllerFirst','yes',1)
    if retval != 0:
        print "switch2 validate failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchThree', 'ControllerFirst','yes',2)
    if retval != 0:
        print "switch3 validate failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchOne', 'ControllerSecond','yes',0)
    if retval != 0:
        print "switch1 validate failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchTwo', 'ControllerSecond','yes',1)
    if retval != 0:
        print "switch2 validate failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_switch_at_physical('SwitchThree', 'ControllerSecond','yes',2)
    if retval != 0:
        print "switch3 validate failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = update_switch('SwitchOne', 'ControllerFirst')
    if retval != 0:
        print "switch1 update failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = update_switch('SwitchTwo', 'ControllerFirst')
    if retval != 0:
        print "switch2 update failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = update_switch('SwitchThree', 'ControllerFirst')
    if retval != 0:
        print "switch3 update failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    insync(60)

    retval = validate_update_switch('SwitchOne', 'ControllerFirst', position = 0)
    if retval != 0:
        print "switch1 validate update failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_update_switch('SwitchTwo', 'ControllerFirst', position = 1)
    if retval != 0:
        print "switch2 validate update failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_update_switch('SwitchThree', 'ControllerFirst', position = 2)
    if retval != 0:
        print "switch3 validate update failed for ctr1"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = update_switch('SwitchOne', 'ControllerSecond')
    if retval != 0:
        print "switch1 update failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = update_switch('SwitchTwo', 'ControllerSecond')
    if retval != 0:
        print "switch2 update failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = update_switch('SwitchThree', 'ControllerSecond')
    if retval != 0:
        print "switch3 update failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    insync(60)

    retval = validate_update_switch('SwitchOne', 'ControllerSecond', position = 0)
    if retval != 0:
        print "switch1 validate update failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_update_switch('SwitchTwo', 'ControllerSecond', position = 1)
    if retval != 0:
        print "switch2 validate update failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    retval = validate_update_switch('SwitchThree', 'ControllerSecond', position = 2)
    if retval != 0:
        print "switch3 validate update failed for ctr2"
        mininet_test.close_topology(child2)
        mininet_test.close_topology(child1)
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    verify_multictr_switch_port('SwitchOne', 'PortOne', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 0)
    verify_multictr_switch_port('SwitchOne', 'PortTwo', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child,'yes', 1)

    verify_multictr_switch_port('SwitchTwo', 'PortThree', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 0)
    verify_multictr_switch_port('SwitchTwo', 'PortFour', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 1)
    verify_multictr_switch_port('SwitchTwo', 'PortFive', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 2)

    verify_multictr_switch_port('SwitchThree', 'PortSix', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 0)
    verify_multictr_switch_port('SwitchThree', 'PortSeven', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 1)
    verify_multictr_switch_port('SwitchThree', 'PortEight', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child,'yes', 2)

    verify_multictr_switch_port('SwitchOne', 'PortOne', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 0)
    verify_multictr_switch_port('SwitchOne', 'PortTwo', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 1)

    verify_multictr_switch_port('SwitchTwo', 'PortThree', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 0)
    verify_multictr_switch_port('SwitchTwo', 'PortFour', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 1)
    verify_multictr_switch_port('SwitchTwo', 'PortFive', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child,  'yes', 2)

    verify_multictr_switch_port('SwitchThree', 'PortSix', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 0)
    verify_multictr_switch_port('SwitchThree', 'PortSeven', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 1)
    verify_multictr_switch_port('SwitchThree', 'PortEight', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 2)

    insync(40)
    verify_multictr_logical_port('PortOne', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 0)
    verify_multictr_logical_port('PortTwo', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 1)
    verify_multictr_logical_port('PortThree', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 2)
    verify_multictr_logical_port('PortFour', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 3)
    verify_multictr_logical_port('PortFive', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 4)
    verify_multictr_logical_port('PortSix', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 5)
    verify_multictr_logical_port('PortSeven', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 6)
    verify_multictr_logical_port('PortEight', 'ControllerFirst', child1, child2, ctr1_child, ctr2_child, 'yes', 7)

    verify_multictr_logical_port('PortOne', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 0)
    verify_multictr_logical_port('PortTwo', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 1)
    verify_multictr_logical_port('PortThree', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 2)
    verify_multictr_logical_port('PortFour', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 3)
    verify_multictr_logical_port('PortFive', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 4)
    verify_multictr_logical_port('PortSix', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 5)
    verify_multictr_logical_port('PortSeven', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 6)
    verify_multictr_logical_port('PortEight', 'ControllerSecond', child1, child2, ctr1_child, ctr2_child, 'yes', 7)

    mininet_test.close_topology(child2)
    mininet_test.close_topology(child1)
    print "DELETE CONTROLLER1"
    retval=controller.delete_controller_ex('ControllerFirst')
    if retval != 0:
        print "CONTROLLER1 delete failed"
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    print "DELETE CONTROLLER2"
    retval=controller.delete_controller_ex('ControllerSecond')
    if retval != 0:
        print "CONTROLLER2 delete failed"
        stop_controller(ctr1_child)
        stop_controller(ctr2_child)
        exit(1)

    print "MULTI-CONTROLLER MULTI-MININET PHYSICAL READ TEST SUCCESS"

def test_audit_physical_read(ctr1_child):
    print "****CREATE Controller with valid IP****"
    child = mininet_test.create_mininet_topology('MININETONE', 'ControllerFirst', '3')
    if child.isalive() == True :
        print "Topology creation Success!!!"
    else:
        print "Topology creation Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr1_child)
        exit(1)

    retval = controller.add_controller_ex('ControllerFirst')
    if retval != 0:
        print "Controller Create Failed"
        mininet_test.close_topology(child)
        stop_controller(ctr1_child)
        exit(1)
    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
        print "controller state change failed"
        mininet_test.close_topology(child)
        stop_controller(ctr1_child)
        exit(1)

    insync(50)
    retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst', 'yes', 0)
    if retval != 0:
        print "switch1 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr1_child)
        exit(1)

    print "****UPDATE Controller IP to invalid****"
    test_invalid_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['invalid_ipaddr']
    retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_invalid_ipaddr)
    if retval != 0:
        print "controller invalid_ip update failed"
        mininet_test.close_topology(child)
        stop_controller(ctr1_child)
        exit(1)

    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"down")
    if retval != 0:
       mininet_test.close_topology(child)
       stop_controller(ctr1_child)
       print "controller state change failed"
       exit(1)

    insync(50)
    print "****UPDATE Controller IP to valid****"
    test_controller_ipaddr= vtn_testconfig.ReadValues(CONTROLLERDATA,'ControllerFirst')['ipaddr']
    retval = controller.update_controller_ex('ControllerFirst',ipaddr=test_controller_ipaddr)
    if retval != 0:
        print "controller valid_ip update failed"
        mininet_test.close_topology(child)
        stop_controller(ctr1_child)
        exit(1)
    # Delay for AUDIT
    retval = controller.wait_until_state('ControllerFirst',"up")
    if retval != 0:
        print "controller state change failed"
        mininet_test.close_topology(child)
        stop_controller(ctr1_child)
        exit(1)

    insync(60)
    retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst', 'yes', 0)
    if retval != 0:
        print "switch1 validate failed"
        mininet_test.close_topology(child)
        stop_controller(ctr_child)
        exit(1)

    print "SWITCH READ IN AUDIT SCENARIO IS SUCCESS"

# Main Block
if __name__ == '__main__':
    print '*****Switch TESTS******'
#Start controller1
    ctr1_child = start_controller('ControllerFirst')
    insync(150)
    test(ctr1_child)
    test_physical_read_1(ctr1_child)
    test_physical_read_2(ctr1_child)
    test_physical_read_3(ctr1_child)
    test_physical_read_4(ctr1_child)
    test_audit_physical_read(ctr1_child)
#Start controller2
    ctr2_child = start_controller('ControllerSecond')
    insync(150)
    test_multi_ctr_multi_mininet(ctr1_child, ctr2_child)
    stop_controller(ctr1_child)
    stop_controller(ctr2_child)
else:
    print "Switch Loaded as Module"
