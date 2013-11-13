//#include <vtn_drv_module.hh>
#include <cxx/pfcxx/module.hh>

namespace pfc {
namespace core {

Module* Module::capaModule=NULL;
Module* Module::tcLib=NULL;
Module* Module::PhysicalLayer=NULL;

 Module*  Module::getModule(const char* moduleName) {
	 if (!strcmp(moduleName,"uppl"))
	 {
		 return PhysicalLayer;
	 }
   else
   {
     return NULL;
   }
 }

}
}
