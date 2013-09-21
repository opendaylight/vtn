#include "cxx/pfcxx/module.hh"

namespace pfc {
namespace core {

Module* Module::capaModule=NULL;
Module* Module::tcLib=NULL;


 Module*  Module::getModule(const char* moduleName) {
	 if (!strcmp(moduleName,"capa"))
	 {
		 return capaModule;
	 }
	 else if(!strcmp(moduleName,"tclib"))
	 {
		 return tcLib;
	 }
	 else
	 {
		 return NULL;
	 }

 }

}
}
