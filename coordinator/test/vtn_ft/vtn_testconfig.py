#!/usr/bin/python

#
# Copyright (c) 2013-2016 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import ConfigParser, sys, time

def ReadValues(filename,section):
    dict1 = {}
    Config=ConfigParser.ConfigParser()
    Config.read(filename)
    options = Config.options(section)
    for option in options:
        try:
            dict1[option] = Config.get(section, option)
            if dict1[option] == -1:
                DebugPrint("skip: %s" % option)
        except:
            print("exception on %s!" % option)
            dict1[option] = None
    return dict1

def InProgress(delay):
    for i in range(delay):
        time.sleep(1)
        sys.stdout.write("\rWaiting..%ds.." %i)
        sys.stdout.flush()



coordinator_url = "http://127.0.0.1:8083/vtn-webapi"
coordinator_headers = {'content-type':'application/json', 'username' : 'admin' , 'password' : 'adminpass'}
controller_headers = {'Accept':'application/json', 'content-type': 'application/json'}
controller_url_part='/restconf'
VTNVBRDATA = "vtn_vbr.data"
VBRIFDATA = "vtn_vbr_vbrif.data"
VTNVTERMDATA = "vtn_vterm.data"
VTERMIFDATA = "vtn_vterm_vtermif.data"
VLANMAPDATA = "vtn_vbr_vlanmap.data"
CONTROLLERDATA = "controller.data"
FLOWLISTDATA = "flowlist.data"
FLOWLISTENTRYDATA = "flowlistentry.data"
FLOWFILTERDATA = "flowfilter.data"
MININETDATA = "mininet_test.data"
SWITCHDATA = "switch_test.data"
PORTDATA = "port_test.data"

if __name__ == '__main__':
  print 'Cannot Run this module as it holds only methods'
else:
  print "VTN Test Module Loaded"


