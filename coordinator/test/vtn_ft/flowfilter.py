#!/usr/bin/python

#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import requests, json, collections, time, locale, controller
import vtn_testconfig, vtn_vbr
import resp_code


VTNVBRDATA = vtn_testconfig.VTNVBRDATA
CONTROLLERDATA = vtn_testconfig.CONTROLLERDATA
FLOWFILTERDATA = vtn_testconfig.FLOWFILTERDATA
VBRIFDATA = vtn_testconfig.VBRIFDATA
VTNVTERMDATA = vtn_testconfig.VTNVTERMDATA
VTERMIFDATA = vtn_testconfig.VTERMIFDATA


coordinator_url = vtn_testconfig.coordinator_url
def_header = vtn_testconfig.coordinator_headers
controller_headers = vtn_testconfig.controller_headers
controller_url_part=vtn_testconfig.controller_url_part

vtn_bn = ""
vbr_bn = ""
vbrif_bn = ""
vtermif_bn = ""

def split_blockname(blockname, url):
    count = blockname.count('|')
    vbr_bn = ""
    vbrif_bn = ""
    vtermif_bn = ""
    if count == 3:
        vtermif_bn = blockname.split('|')[3]
        vterm_bn = blockname.split('|')[1]
    if count == 2:
        vbrif_bn = blockname.split('|')[2]
        count = count - 1
    if count == 1:
        vbr_bn = blockname.split('|')[1]
        count = count - 1
    if(vbr_bn):
        vbr_url = vtn_testconfig.ReadValues(VTNVBRDATA, vbr_bn)['vbr_url']
        url = url + vbr_url
    if(vbrif_bn):
        vbr_url = vtn_testconfig.ReadValues(VBRIFDATA, vbrif_bn)['vbrif_url']
        url = url + vbr_url
    if(vtermif_bn):
        vterm_url = vtn_testconfig.ReadValues(VTNVTERMDATA, vterm_bn)['vterm_url']
        vtermif_url = vtn_testconfig.ReadValues(VTERMIFDATA, vtermif_bn)['vtermif_url']
        url = url + vterm_url + vtermif_url
    return url


def create_flowfilter(blockname, ff_blockname):
    vtn_bn = blockname.split('|')[0]
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA, vtn_bn)['vtn_url']
    flowfilter_url = vtn_testconfig.ReadValues(FLOWFILTERDATA, 'FlowfilterURL')['url']
    ff_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['type']
    url = coordinator_url + vtn_url

    url = split_blockname(blockname, url)

    url = url + flowfilter_url

    print url
    flowfilter_add = collections.defaultdict(dict)
    print ff_type
    flowfilter_add['flowfilter']['ff_type'] = ff_type
    print flowfilter_add
    r  =  requests.post(url, data = json.dumps(flowfilter_add), headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0

def delete_flowfilter(blockname, ff_blockname):
    vtn_bn = blockname.split('|')[0]
    vtn_url = vtn_testconfig.ReadValues(VTNVBRDATA, vtn_bn)['vtn_url']
    ff_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['type']

    url =  coordinator_url + vtn_url

    url = split_blockname(blockname, url)
    url = url + '/flowfilters/' + ff_type + '.json'
    print url

    r = requests.delete(url, headers = def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0

def validate_flowfilter_at_controller(blockname, controller_blockname, ff_blockname, presence = 'yes', position = 0):
    vtn_bn = blockname.split('|')[0]
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA, vtn_bn)['vtn_url']
    controller_ipaddr = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['ipaddr']
    controller_port = vtn_testconfig.ReadValues(CONTROLLERDATA, controller_blockname)['port']
    vtn_url = vtn_testconfig.ReadValues(VTNVBRDATA, vtn_bn)['vtn_url']
    ff_type = vtn_testconfig.ReadValues(FLOWFILTERDATA,  ff_blockname)['type']
    index = vtn_testconfig.ReadValues(FLOWFILTERDATA,  ff_blockname)['index']
    condition = vtn_testconfig.ReadValues(FLOWFILTERDATA,  ff_blockname)['condition']
    priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['priority']
    dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['dscp']
    direction = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['direction']

    action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['action_type']
    url = 'http://'+controller_ipaddr+':'+controller_port+controller_url_part+vtn_url

    url = split_blockname(blockname, url)
    url = url + '/flowfilters'
    count = blockname.count('|')
    if count > 0:
        url = url + '/' + ff_type
    print url

    r  =  requests.get(url, headers = controller_headers, auth = ('admin', 'admin'))
    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            return 0
    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1

    data = json.loads(r.content)
    print data

    if presence == "no":
        print data['flowfilter']
        if data['flowfilter'] == []:
            return 0
        else:
            return 0
    else:
      flowfilter_content = data['flowfilter'][position]
      print flowfilter_content
      if flowfilter_content['condition']!=condition:
          return 1
      if flowfilter_content['index']!=int(locale.atof(index)):
          return 1
      ret = validate_filter_type(flowfilter_content)
      if ret != 0:
          return 1
      ret = validate_actions(flowfilter_content, ff_blockname)
      if ret != 0:
          return 1
    return 0

def validate_filter_type(flowfilter_content):
  print "filtertype"
  try:
      if flowfilter_content['filterType']['redirect']['destination']['bridge'] != bridge:
          return 1
      if flowfilter_content['filterType']['redirect']['destination']['interface'] != interface:
          return 1
      if flowfilter_content['filterType']['redirect']['output'] != bool(locale.atof(output)):
          return 1
  except(NameError, KeyError):
      print "Filtertype is either PASS or DROP"
      return 0
  return 0

def validate_actions(flowfilter_content, ff_blockname):
  print "actions"
  length = len(flowfilter_content['actions'])

  for index in range(length):
    print "looping:", index
    ret = 1 
    element = flowfilter_content['actions'][index]
    print element
    if 'dscp' in element:
       ret = validate_dscp(flowfilter_content['actions'][index], ff_blockname);
    if 'vlanpcp' in element:
       ret = validate_priority(flowfilter_content['actions'][index], ff_blockname);
    if 'dlrsrc' in element:
       ret = validate_dlsrc(flowfilter_content['actions'][index], ff_blockname);
    return ret

def validate_dscp(json_array, ff_blockname):
    print "validate_dscp"
    dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA,  ff_blockname)['dscp']
    if json_array['dscp']['dscp'] == int(locale.atof(dscp)):
      print "dscp"
      return 0
    else:
      return 1

def validate_priority(json_array, ff_blockname):
    print "validate_priority"
    priority = vtn_testconfig.ReadValues(FLOWFILTERDATA,  ff_blockname)['priority']
    if json_array['vlanpcp']['priority'] == int(locale.atof(priority)):
      print "priority"
      return 0
    else:
      return 1

def validate_dlsrc(json_array, ff_blockname):
    print "validate_dlsrc"
    dlsrc = vtn_testconfig.ReadValues(FLOWFILTERDATA,  ff_blockname)['dlsrc']
    if json_arrary['dlsrc']['address'] == int(locale.atof(dlsrc)):
      print "dlsrc"
      return 0
    else:
      return 1

def create_flowfilter_entry(blockname, ff_blockname):
    vtn_bn = blockname.split('|')[0]
    vtn_url = vtn_testconfig.ReadValues(VTNVBRDATA, vtn_bn)['vtn_url']
    flowfilter_entry_url = vtn_testconfig.ReadValues(FLOWFILTERDATA,  'FlowfilterEntryURL')['url']
    ff_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['type']

    seqnum = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['seqnum']
    fl_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['fl_name']
    action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['action_type']
    nmg_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['nmg_name']
    priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['priority']
    dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['dscp']

    url = coordinator_url + vtn_url
    url = split_blockname(blockname, url)
    url = url + '/flowfilters/' + ff_type + flowfilter_entry_url
    print url

    flowfilter_entry_add = {'seqnum': seqnum, 'fl_name':fl_name, 'action_type':action_type, 'priority':priority, 'dscp':dscp}
    J_string = dict({'flowfilterentry':flowfilter_entry_add})

    count = blockname.count('|')
    if count > 0 and  action_type == 'redirect':
        vnode_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['vnode_name']
        if_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['if_name']
        direction = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['direction']
        macdstaddr = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['macdstaddr']
        macsrcaddr = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['macsrcaddr']


        redir = {'vnode_name':vnode_name, 'if_name':if_name, 'direction':direction, 'macdstaddr':macdstaddr, 'macsrcaddr':macsrcaddr}

        flowfilter_entry_add = {'seqnum': seqnum, 'fl_name':fl_name, 'action_type':action_type, 'priority':priority, 'dscp':dscp, 'redirectdst':redir}
        flowfilter_entry_add = {'seqnum': seqnum, 'fl_name':fl_name, 'action_type':action_type, 'priority':priority, 'dscp':dscp, 'redirectdst':redir}
        J_string = dict({'flowfilterentry':flowfilter_entry_add})

    print J_string
    r = requests.post(url, data=json.dumps(J_string), headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_CREATE_SUCCESS and r.status_code != resp_code.RESP_CREATE_SUCCESS_U14:
        return 1
    else:
        return 0

def update_flowfilter_entry(blockname, ff_blockname, update_ff_bn):
    vtn_bn = blockname.split('|')[0]
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA, vtn_bn)['vtn_url']
    flowfilter_entry_url = vtn_testconfig.ReadValues(FLOWFILTERDATA, 'FlowfilterEntryURL')['url']
    ff_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['type']
    seqnum = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['seqnum']
    fl_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['fl_name']
    action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['action_type']
    nmg_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['nmg_name']
    dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['dscp']
    priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['priority']

    if update_ff_bn == 'UpdateVTNFlowfilter':
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['priority']
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']


    if update_ff_bn == 'NegativeVTNFlowfilter':
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['priority']        
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']


    if update_ff_bn == 'UpdateFlowfilter':
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['priority']
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']

    if update_ff_bn == 'NegativeFlowfilter':
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['priority']        
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']


    if update_ff_bn == 'UpdateFlowfilterOne':
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['priority']
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']

    if update_ff_bn == 'NegativeFlowfilterOne':
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['priority']        
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']


    if update_ff_bn == 'UpdateFlowfilterOnePass':
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['priority']
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']
        action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['action_type']


    if update_ff_bn == 'UpdateFlowfilterOneDrop':
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['priority']
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']
        action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['action_type']





    url= coordinator_url + vtn_url
    url = split_blockname(blockname, url)
    url = url + '/flowfilters/' + ff_type + '/flowfilterentries/' + seqnum + '.json'
    print url

    flowfilter_entry_add = {'seqnum': seqnum, 'fl_name':fl_name, 'action_type':action_type, 'priority':priority, 'dscp':dscp}
    J_string = dict({'flowfilterentry':flowfilter_entry_add})

    count = blockname.count('|')
    if count > 0 and action_type == 'redirect':
        direction = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['direction']
        macdstaddr = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['macdstaddr']
        macsrcaddr = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['macsrcaddr']

        vnode_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['vnode_name']
        if_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['if_name']
         
        macsrcaddr = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['macsrcaddr']
        macdstaddr = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['macdstaddr']
        direction = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['direction']

        seqnum = vtn_testconfig.ReadValues(FLOWFILTERDATA,  update_ff_bn)['seqnum']
        fl_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['fl_name']
        action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA,update_ff_bn )['action_type']
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA,update_ff_bn )['priority']
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, update_ff_bn)['dscp']




        redir = {'vnode_name':vnode_name, 'if_name':if_name, 'direction':direction, 'macdstaddr':macdstaddr, 'macsrcaddr':macsrcaddr}
        
        flowfilter_entry_add = {'seqnum': seqnum,'fl_name':fl_name, 'action_type':action_type, 'priority':priority, 'dscp':dscp, 'redirectdst':redir}
        J_string = dict({'flowfilterentry':flowfilter_entry_add})
        print J_string

    r = requests.put(url, data=json.dumps(J_string), headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_UPDATE_SUCCESS and r.status_code != resp_code.RESP_UPDATE_SUCCESS_U14:
        return 1
    else:
        return 0


def delete_flowfilter_entry(blockname, ff_blockname):
    vtn_bn = blockname.split('|')[0]
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA,vtn_bn)['vtn_url']
    ff_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['type']

    seqnum = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_blockname)['seqnum']
    url= coordinator_url  + vtn_url

    url = split_blockname(blockname, url)
    url = url + '/flowfilters/' + ff_type + '/flowfilterentries/' + seqnum + '.json'
    print url

    r = requests.delete(url, headers=def_header)
    print r.status_code
    if r.status_code != resp_code.RESP_DELETE_SUCCESS and r.status_code != resp_code.RESP_DELETE_SUCCESS_U14:
        return 1
    else:
        return 0

def validate_flowfilter_entry(blockname, ff_blockname, presence='yes', position=0) :
    vtn_bn = blockname.split('|')[0]
    ff_bn = ff_blockname.split('|')[0]
    vtn_url=vtn_testconfig.ReadValues(VTNVBRDATA, vtn_bn)['vtn_url']
    ff_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['type']

    seqnum = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['seqnum']
    fl_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['fl_name']
    action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['action_type']
    nmg_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['nmg_name']
    dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['dscp']
    priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['priority']

    if ff_blockname.find('VTN') != 0:
        vnode_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['vnode_name']
        if_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['if_name']
        direction = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['direction']
        macdstaddr = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['macdstaddr']
        macsrcaddr = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['macsrcaddr']

        action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['action_type']
    ff_count = ff_blockname.count('|')
    if ff_count > 0 and  ff_blockname.find('VTN') != 0:
        ff_bn = ff_blockname.split('|')[1]
        vnode_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['vnode_name']
        if_name = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['if_name']
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['priority']
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['dscp']

        action_type = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['action_type']

    elif ff_count > 0:
        ff_bn = ff_blockname.split('|')[1]
        priority = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['priority']
        dscp = vtn_testconfig.ReadValues(FLOWFILTERDATA, ff_bn)['dscp']


    url = coordinator_url + vtn_url

    url = split_blockname(blockname, url)
    url= url + '/flowfilters/' + ff_type + '/flowfilterentries/' + 'detail.json'
    print url

    count = blockname.count('|')

    r = requests.get(url, headers=def_header)
    print r.status_code

    if presence == "no":
        if r.status_code == resp_code.RESP_NOT_FOUND:
            return 0
    if r.status_code != resp_code.RESP_GET_SUCCESS:
        return 1

    data=json.loads(r.content)
    print data

    if presence == "no":
        print data['flowfilterentries']
        if data['flowfilterentries'] == []:
            return 0
    else:
      flowfilter_content=data['flowfilterentries'][position]
      print flowfilter_content
      if flowfilter_content['seqnum'] != seqnum:
          return 1
      if flowfilter_content['fl_name'] != fl_name:
          return 1
      if flowfilter_content['action_type'] != action_type:
          print "action"
          return 1
      if flowfilter_content['priority'] != priority:
          return 1
      if flowfilter_content['dscp'] != dscp:
          return 1
      if count > 0 and action_type == 'redirect':
          print "*********Vnode present********"
          if flowfilter_content['redirectdst']['vnode_name'] != vnode_name:
             return 1
          if flowfilter_content['redirectdst']['if_name'] != if_name:
             return 1
          if flowfilter_content['redirectdst']['direction'] != direction:
             return 1
          if flowfilter_content['redirectdst']['macsrcaddr'] != macsrcaddr:
             return 1
          if flowfilter_content['redirectdst']['macdstaddr'] != macdstaddr:
             return 1

      return 0



# Main Block
if __name__ == '__main__':
    print '*****FLOWFILTER TESTS******'
else:
    print '*****VTN_VBR FLOWFILTER LOADED AS MODULE******'
