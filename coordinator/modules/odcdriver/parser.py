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

if __name__ == '__main__':
      print 'Cannot Run this module as it holds only methods'
else:
      print "Parser Module Loaded"


