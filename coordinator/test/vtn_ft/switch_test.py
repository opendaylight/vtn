#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#


#! /usr/bin/python

import requests, json, collections, sys, time, controller, vtn_vbr
import vtn_testconfig, pexpect, mininet_test

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
    if r.status_code == 404:
      return 0

  if r.status_code != 200:
    return 1

  data=json.loads(r.text)
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
  if r.status_code != 201:
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
    if r.status_code == 404:
      return 0

  if r.status_code != 200:
    return 1

  data=json.loads(r.text)
  print data

  if data['switch'] == [ ]:
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
    if r.status_code == 404:
      return 0

  if r.status_code != 200:
    return 1

  data=json.loads(r.text)

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

def verify_switch_port(switch_blockname, port_blockname, controller_blockname, child, presence="yes", position=0):
  retval = validate_switch_port(switch_blockname, port_blockname, controller_blockname, child, presence, position)
  if retval != 0:
    mininet_test.close_topology(child)
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
    if r.status_code == 404:
      return 0

  if r.status_code != 200:
    return 1

  data=json.loads(r.text)

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

def verify_logical_port(port_blockname, controller_blockname, child, presence = "yes", position = 0):
  retval = validate_logical_port(port_blockname, controller_blockname, child, presence, position)
  if retval != 0:
    print "validate", port_blockname,"failed"
    mininet_test.close_topology(child)
    exit(1)

def insync(delay):
    for i in range(delay):
        time.sleep(1)
        sys.stdout.write("\rWaiting for in sync with controller..%ds.." %i)
        sys.stdout.flush()

def test():

  child=mininet_test.create_mininet_topology('MININET','ControllerFirst','2')
  if child.isalive() == True :
      print "Topology creation Success!!!"
  else:
      print "Topology creation Failed"
      mininet_test.close_topology(child)
      exit(1)
  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    mininet_test.close_topology(child)
    exit(1)

  insync(60)

  retval = validate_switch_at_physical('SwitchOne', 'ControllerFirst','yes',0)
  if retval != 0:
    print "switch1 validate failed"
    mininet_test.close_topology(child)
    exit(1)

  retval = validate_switch_at_physical('SwitchTwo', 'ControllerFirst','yes',1)
  if retval != 0:
    print "switch2 validate failed"
    mininet_test.close_topology(child)
    exit(1)

  retval = validate_switch_at_physical('SwitchThree', 'ControllerFirst','yes',2)
  if retval != 0:
    print "switch3 validate failed"
    mininet_test.close_topology(child)
    exit(1)

  retval = update_switch('SwitchOne', 'ControllerFirst')
  if retval != 0:
    print "switch1 update failed"
    mininet_test.close_topology(child)
    exit(1)

  retval = update_switch('SwitchTwo', 'ControllerFirst')
  if retval != 0:
    print "switch2 update failed"
    mininet_test.close_topology(child)
    exit(1)

  retval = update_switch('SwitchThree', 'ControllerFirst')
  if retval != 0:
    print "switch3 update failed"
    mininet_test.close_topology(child)
    exit(1)

  insync(60)

  retval = validate_update_switch('SwitchOne', 'ControllerFirst',position = 0)
  if retval != 0:
    print "switch1 validate update failed"
    mininet_test.close_topology(child)
    exit(1)

  retval = validate_update_switch('SwitchTwo', 'ControllerFirst',position = 1)
  if retval != 0:
    print "switch2 validate update failed"
    mininet_test.close_topology(child)
    exit(1)

  retval = validate_update_switch('SwitchThree', 'ControllerFirst',position = 2)
  if retval != 0:
    print "switch3 validate update failed"
    mininet_test.close_topology(child)
    exit(1)

  verify_switch_port('SwitchOne', 'PortOne', 'ControllerFirst', child, 'yes', 0)
  verify_switch_port('SwitchOne', 'PortTwo', 'ControllerFirst', child, 'yes', 1)

  verify_switch_port('SwitchTwo', 'PortThree', 'ControllerFirst', child, 'yes', 0)
  verify_switch_port('SwitchTwo', 'PortFour', 'ControllerFirst', child, 'yes', 1)
  verify_switch_port('SwitchTwo', 'PortFive', 'ControllerFirst', child, 'yes', 2)

  verify_switch_port('SwitchThree', 'PortSix', 'ControllerFirst', child, 'yes', 0)
  verify_switch_port('SwitchThree', 'PortSeven', 'ControllerFirst', child, 'yes', 1)
  verify_switch_port('SwitchThree', 'PortEight', 'ControllerFirst', child, 'yes', 2)

  insync(20)
  verify_logical_port('PortOne', 'ControllerFirst', child, 'yes', 0)
  verify_logical_port('PortTwo', 'ControllerFirst', child, 'yes', 1)
  verify_logical_port('PortThree', 'ControllerFirst', child, 'yes', 2)
  verify_logical_port('PortFour', 'ControllerFirst', child, 'yes', 3)
  verify_logical_port('PortFive', 'ControllerFirst', child, 'yes', 4)
  verify_logical_port('PortSix', 'ControllerFirst', child, 'yes', 5)
  verify_logical_port('PortSeven', 'ControllerFirst', child, 'yes', 6)
  verify_logical_port('PortEight', 'ControllerFirst', child, 'yes', 7)

  mininet_test.close_topology(child)
  print "DELETE CONTROLLER"
  retval=controller.delete_controller_ex('ControllerFirst')
  if retval != 0:
    print "CONTROLLER delete failed"
    exit(1)
  print "PHYSICAL READ TEST SUCCESS"
# Main Block
if __name__ == '__main__':
  print '*****Switch TESTS******'
  test()
else:
  print "Switch Loaded as Module"
