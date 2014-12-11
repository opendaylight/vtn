#!/usr/bin/python

#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import ConfigParser

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

REST_SPEC_FILE='odl_vtn.rest,odl_vbr.rest,odl_vterm.rest'


if __name__ == '__main__':
  print 'Cannot Run this module as it holds only methods'
else:
  print "Parser Module Loaded"


