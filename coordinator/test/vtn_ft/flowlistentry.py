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

FLOWLISTDATA=vtn_testconfig.FLOWLISTDATA
CONTROLLERDATA=vtn_testconfig.CONTROLLERDATA
FLOWLISTENTRYDATA=vtn_testconfig.FLOWLISTENTRYDATA

coordinator_url=vtn_testconfig.coordinator_url
def_header=vtn_testconfig.coordinator_headers
controller_headers=vtn_testconfig.controller_headers
controller_url_part=vtn_testconfig.controller_url_part


def create_flowlist(blockname):
  test_vtn_name=vtn_testconfig.ReadValues(FLOWLISTDATA,blockname)['flowlist_name']
  flowlist_url=vtn_testconfig.ReadValues(FLOWLISTDATA,'FLOWLISTURL')['url']
  url= coordinator_url + flowlist_url
  print url

  vtn_add = collections.defaultdict(dict)

  vtn_add['flowlist']['fl_name']=test_vtn_name
  r = requests.post(url,data=json.dumps(vtn_add),headers=def_header)
  print r.status_code
  if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
    return 1
  else:
    return 0


def delete_flowlist(blockname):
  test_vtn_name=vtn_testconfig.ReadValues(FLOWLISTDATA,blockname)['flowlist_name']
  url= coordinator_url + '/flowlists/' + test_vtn_name + '.json'
  print url

  r = requests.delete(url,headers=def_header)
  print r.status_code
  if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
    return 1
  else:
    return 0



def validate_flowlist_at_controller(flowlist_blockname, controller_blockname, presence="yes",position=0):
  test_vtn_name=vtn_testconfig.ReadValues(FLOWLISTDATA,flowlist_blockname)['flowlist_name']
  test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
  test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['restconf_port']
  test_vtn_url=vtn_testconfig.ReadValues(FLOWLISTDATA,'FLOWLISTURL')['ctr_url']

  url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_vtn_url
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
  return 0
#  if presence == "no":
#    print data['flowlist']
#  if data['flowlist'] == []:
#    return 0

  vtn_content=data['vtn-flow-condition'][position]

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
      return 0
    else:
      return 1

def create_flowlistentry(flowlist_blockname,flowlistentry_blockname,controller_blockname):
  test_flowlist_name=vtn_testconfig.ReadValues(FLOWLISTDATA,flowlist_blockname)['flowlist_name']
  test_flentry_seqnum=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['seqnum']
  test_macethertype=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['macethertype']
  test_ipdstaddr=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipdstaddr']
  test_ipdstaddrprefix=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipdstaddrprefix']
  test_ipsrcaddr=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipsrcaddr']
  test_ipsrcaddrprefix=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipsrcaddrprefix']
  test_ipproto=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipproto']
  test_ipdscp=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipdscp']
  test_icmptypenum=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['icmptypenum']
  test_icmpcodenum=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['icmpcodenum']
  flowlist_url=vtn_testconfig.ReadValues(FLOWLISTDATA,flowlist_blockname)['flowlist_url']
  flowlist_entry_url=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['flowlistentry_url']

  url=coordinator_url+flowlist_entry_url+'/flowlistentries'
  print url

  vbr_add = collections.defaultdict(dict)
  vbr_add['flowlistentry']['seqnum']=test_flentry_seqnum
  vbr_add['flowlistentry']['macethertype']=test_macethertype
  vbr_add['flowlistentry']['ipdstaddr']=test_ipdstaddr
  vbr_add['flowlistentry']['ipdstaddrprefix']=test_ipdstaddrprefix
  vbr_add['flowlistentry']['ipsrcaddr']=test_ipsrcaddr
  vbr_add['flowlistentry']['ipsrcaddrprefix']=test_ipsrcaddrprefix
  vbr_add['flowlistentry']['ipproto']=test_ipproto
  vbr_add['flowlistentry']['ipdscp']=test_ipdscp
  vbr_add['flowlistentry']['icmptypenum']=test_icmptypenum
  vbr_add['flowlistentry']['icmpcodenum']=test_icmpcodenum
  print vbr_add

  r = requests.post(url,data=json.dumps(vbr_add),headers=def_header)
  print r.status_code
  if r.status_code != resp_code.RESP_CREATE_SUCCESS:
      return 1
  else:
      return 0

def update_flowlist_entry(flowlist_blockname, flowlistentry_blockname):
  test_flowlist_name=vtn_testconfig.ReadValues(FLOWLISTDATA,flowlist_blockname)['flowlist_name']
  test_flentry_seqnum=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['seqnum']
  test_macethertype=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['macethertype']
  test_ipdstaddr=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipdstaddr']
  test_ipdstaddrprefix=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipdstaddrprefix']
  test_ipsrcaddr=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipsrcaddr']
  test_ipsrcaddrprefix=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipsrcaddrprefix']
  test_ipproto=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipproto']
  test_ipdscp=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['ipdscp']
  test_icmptypenum=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['icmptypenum']
  test_icmpcodenum=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['icmpcodenum']
  flowlist_url=vtn_testconfig.ReadValues(FLOWLISTDATA,flowlist_blockname)['flowlist_url']
  flowlist_entry_url=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['flowlistentry_url']

  url=coordinator_url+flowlist_entry_url+'/flowlistentries/' + test_flentry_seqnum + '.json'


  print url

  vbr_add = collections.defaultdict(dict)
  vbr_add['flowlistentry']['macethertype']=test_macethertype
  vbr_add['flowlistentry']['ipdstaddr']=test_ipdstaddr
  vbr_add['flowlistentry']['ipdstaddrprefix']=test_ipdstaddrprefix
  vbr_add['flowlistentry']['ipsrcaddr']=test_ipsrcaddr
  vbr_add['flowlistentry']['ipsrcaddrprefix']=test_ipsrcaddrprefix
  vbr_add['flowlistentry']['ipproto']=test_ipproto
  vbr_add['flowlistentry']['ipdscp']=test_ipdscp
  vbr_add['flowlistentry']['icmptypenum']=test_icmptypenum
  vbr_add['flowlistentry']['icmpcodenum']=test_icmpcodenum

  print vbr_add

  r = requests.put(url, data=json.dumps(vbr_add), headers=def_header)
  print r.status_code
  if r.status_code != resp_code.RESP_UPDATE_SUCCESS and r.status_code != resp_code.RESP_UPDATE_SUCCESS_U14:
      return 1
  else:
      return 0


def delete_flowlistentry(flowlist_blockname,flowlistentry_blockname):
  test_vtn_name=vtn_testconfig.ReadValues(FLOWLISTDATA,flowlist_blockname)['flowlist_name']
  test_flentry_seqnum=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['seqnum']

  url= coordinator_url + '/flowlists/' + test_vtn_name + '/flowlistentries/' + test_flentry_seqnum + '.json'

  print url

  r = requests.delete(url,headers=def_header)
  print r.status_code
  if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
      return 1
  else:
      return 0

def validate_flowlist_entry(flowlist_blockname, flowlistentry_blockname, controller_blockname, presence="yes",position=0):
  test_vtn_name=vtn_testconfig.ReadValues(FLOWLISTDATA,flowlist_blockname)['flowlist_name']
  test_flentry_seqnum=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['seqnum']
  test_controller_ipaddr=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['ipaddr']
  test_controller_id=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['controller_id']
  test_controller_port=vtn_testconfig.ReadValues(CONTROLLERDATA,controller_blockname)['port']
  test_flowlist_url=vtn_testconfig.ReadValues(FLOWLISTDATA,'FLOWLISTURL')['ctr_url']
  test_flowlistentry_url=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,'FLOWLISTENTRYURL')['ctr_url']
  flowlist_entry_url=vtn_testconfig.ReadValues(FLOWLISTENTRYDATA,flowlistentry_blockname)['flowlistentry_url']

  url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_flowlist_url

  url='http://'+test_controller_ipaddr+':'+test_controller_port+controller_url_part+test_flowlist_url+'/'+test_vtn_name+test_vbr_url
  print url
  r = requests.get(url,headers=controller_headers,auth=('admin','admin'))
  print r.status_code

  if presence == "no":
      if r.status_code == resp_code.RESP_NOT_FOUND:
          return 0

      if r.status_code != resp_code.RESP_GET_SUCCESS:
          return 1


  data=json.loads(r.content)

  vtn_content=data['flowlistentries'][position]

  print vtn_content

  if vtn_content == None:
       if presence == "yes":
            return 0
       else:
            return 1


  if vtn_content['flowlist_name'] != test_vtn_name:
        if presence == "yes":
            return 1
        else:
            return 0
  else:
         if presence == "yes":
            return 0

  if vtn_content['seqnum'] != test_flentry_seqnum:
          if presence == "yes":
             return 1
          else:
             return 0
  else:
          if presence == "yes":
             return 0


def test_vtn_vbr():

  print "CREATE Controller"
  retval = controller.add_controller_ex('ControllerFirst')
  if retval != 0:
    print "Controller Create Failed"
    exit(1)

  retval=create_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowlist Create Failed"
    exit(1)


  retval=create_flowlistentry('FlowlistOne','FlowlistentryOne','ControllerFirst')
  if retval != 0:
    print "Flowlistentry Create Failed"
    exit(1)


  retval = delete_flowlist('FlowlistOne')
  if retval != 0:
    print "Flowlist Delete Failed----"
    exit(1)


  retval = delete_flowlistentry('FlowlistOne','FlowlistientryOne')
  if retval != 0:
    print "Flowlistentry Delete Failed----"
    exit(1)

print "FLOWLIST SUCCESS";

# Main Block
if __name__ == '__main__':
    print '*****CONTROLLER TESTS******'

else:
      print "VTN VBR Loaded as Module"

