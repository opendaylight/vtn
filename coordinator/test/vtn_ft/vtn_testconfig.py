#! /usr/bin/python

import ConfigParser

def ReadValues(filename,section):
    dict1 = {}
    Config=ConfigParser.ConfigParser()
    Config.read(filename)
#    print Config.sections()
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



coordinator_url = "http://127.0.0.1:8080/vtn-webapi"
coordinator_headers = {'content-type':'application/json', 'username' : 'admin' , 'password' : 'adminpass'}
controller_headers = {'Accept':'application/json', 'content-type': 'application/json'}
controller_url_part='/controller/nb/v2/vtn/default'
VTNVBRDATA = "vtn_vbr.data"
VBRIFDATA = "vtn_vbr_vbrif.data"
VLANMAPDATA = "vtn_vbr_vlanmap.data"
CONTROLLERDATA = "controller.data"


if __name__ == '__main__':
  print 'Cannot Run this module as it holds only methods'
else:
  print "VTN Test Module Loaded"


