/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef _CONTROLLER_FRAMEWORK_HH_
#define _CONTROLLER_FRAMEWORK_HH_

#include <pfcxx/module.hh>
#include <pfcxx/synch.hh>
#include <map>
#include <iterator>
#include <string>
#include "driver/driver_interface.hh"


namespace unc {
namespace driver {

/**
 * @brief       : This class store sthe controller* ,driver* , mutex lock
 */
class ControllerContainer {
 public:
  ControllerContainer();
  controller* ctr;
  driver* drv;
  pfc::core::Mutex  rw_mutex;  // Global Mutex lock
};

/**
 * Class - ControllerFramework1
 * Base class
 * DataMembers : connection_list
 * MemberFunctions : GetControllerConnectionByName,AddAndConfigureController
 * UpdateControllerConfiguration,RemoveControllerConfiguration
 * AcquireCtrlrConnectionLock,ReleaseCtrlrConnectionLock
 */
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
  uint32_t GetDriverByControllerName(std::string &controller_name,
                                     controller*, driver*);

  /**
   *@brief: This function adds the respective controller information
   in the controller list
   **/

  void AddController(std::string &controller_name,
                     controller* , driver*);

  /**
   *@brief: This function updates the respective controller information
   in the controller list
   */

  void UpdateControllerConfiguration(std::string &controller_name,
                                    controller*, driver*);
  /**
   *@brief:  This function removes the respective controller information
   in the controller list
   **/
  void RemoveControllerConfiguration(std::string &controller_name);

  private:
    std::map<std::string, ControllerContainer*>  controller_list;
    pfc::core::ReadWriteLock controller_list_rwlock_;
};
}  // namespace driver
}  // namespace unc
#endif
