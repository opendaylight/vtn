/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _CONTROLLER_FRAMEWORK_HH_
#define _CONTROLLER_FRAMEWORK_HH_

#include <vtndrvintf_defs.h>
#include <driver/driver_interface.hh>
#include <driver/controller_interface.hh>
#include <map>
#include <iterator>
#include <string>
#include <pfcxx/synch.hh>


namespace unc {
namespace driver {

/**
 * @brief       : This class contains the controller instance ,driver instance,
 *                mutex lock
 * @DataMembers : controller*, driver*, rw_mutex
 */
class ControllerContainer {
 public:
  ControllerContainer():ctr(NULL), drv(NULL) {}
  controller* ctr;
  driver* drv;
  pfc::core::Mutex  rw_mutex;  // Global Mutex lock
};

/**
 * Class - ControllerFramework1
 * Base class
 * DataMembers : controller_list, driver_container, controller_list_rwlock
 * MemberFunctions : GetControllerConnectionByName,AddAndConfigureController
 * UpdateControllerConfiguration,RemoveControllerConfiguration.
 * GetDriverInstance, RegisterDriver, GetDriverByControllerName
 **/
class ControllerFramework  {
 public:
  /**
    *@brief: constructor
    **/

  ControllerFramework();
  /**
    *@brief: destructor
    **/
  ~ControllerFramework();

  /**
   *@brief: This function is to get the driver type
   for the respective controllers
   **/

  VtnDrvRetEnum GetDriverByControllerName(std::string& controller_name,
                                     controller**, driver**);

  /**
   *@brief: This function adds the respective controller information
   in the controller list
   **/

  void AddController(std::string& controller_name,
                     controller* , driver*);

  /**
   *@brief: This function updates the respective controller information
   in the controller list
   */

  void UpdateControllerConfiguration(std::string& controller_name,
                                    controller*, driver*);
  /**
   *@brief:  This function removes the respective controller information
   in the controller list
   **/
  VtnDrvRetEnum RemoveControllerConfiguration(std::string& controller_name);

  /**
   *@Brief: This function is to get driver typw
  **/

  driver* GetDriverInstance(unc_keytype_ctrtype_t controller_type);

  /**
  *@Brief: This function is to register controller type and Driver*
  **/

  void RegisterDriver(unc_keytype_ctrtype_t controller_type, driver*);

  private:
    std::map<std::string, ControllerContainer*>  controller_list;
    std::map<unc_keytype_ctrtype_t, driver*>  driver_container;
    pfc::core::ReadWriteLock controller_list_rwlock_;

};
}  //driver
}  //unc
#endif
