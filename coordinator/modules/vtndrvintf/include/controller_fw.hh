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
#include <pfcxx/timer.hh>
#include <pfcxx/task_queue.hh>
#include <pfc/taskq.h>
#include <pfc/thread.h>
#include <pfcxx/synch.hh>
#include <unc/uppl_common.h>
#include <unc/unc_events.h>
#include <functional>
#include <iterator>
#include <map>
#include <string>

#define domainID "domainname"

namespace unc {
namespace driver {

/**
 * @brief       - This class contains the controller instance ,driver instance,
 *                mutex lock
 * @DataMembers - controller*, driver*, rw_mutex
 */
class ControllerContainer {
 public:
  ControllerContainer():ctr(NULL), drv(NULL) {}
  controller* ctr;
  driver* drv;
  pfc::core::Mutex  rw_mutex;
};


/**
 * Class                - ControllerFramework
 * DataMembers          - controller_list, driver_container, controller_list_rwlock
 * MemberFunctions      - GetControllerConnectionByName,AddAndConfigureController
 *                        UpdateControllerConfiguration,RemoveControllerConfiguration.
 *                        GetDriverInstance, RegisterDriver, GetDriverByControllerName
 */
class ControllerFramework  {
 public:
  pfc::core::TaskQueue* taskq_;

  /**
   * @brief - constructor
   */
  explicit ControllerFramework(pfc::core::TaskQueue*);

  /**
   * @brief   - destructor
   */
  ~ControllerFramework();

  /**
   * @brief     - Retrieves the driver type for the respective controllers
   * @param[in] - controller name ,controller**, driver**
   * @retval    - VTN_DRV_RET_FAILURE/VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum GetDriverByControllerName(std::string& controller_name,
                                          controller**, driver**);

  /**
   * @brief      - This function adds the respective controller information
   *               in the controller list
   * @param[in]  - controller name, controller*, driver*
   * @retval     - None
   */
  void AddController(std::string& controller_name,
                     controller* , driver*);

  /**
   * @brief        - This function  sets the time interval for every ping and the
   *                timer is get reset.
   * @param[in]   - controller name, driver*, controller*
   * @retval      - int
   */
  int PostTimer(std::string& controller_name, driver*, controller*);

  /**
   * @brief         - This function updates the respective controller information
   *                 in the controller list
   * @param[in]     - controller name, controller*, driver*
   * @retval       - VTN_DRV_RET_FAILURE/ VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum UpdateControllerConfiguration(std::string& controller_name,
                                              controller*, driver*);
  /**
   * @brief         - This function removes the respective controller information
   *                 in the controller list
   * @param[in]    - controller name, controller*, driver*
   * @retval       - VTN_DRV_RET_FAILURE/ VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum RemoveControllerConfiguration(std::string& controller_name,
                                              controller*, driver*);

  /**
   * @brief      - This function is to get driver type
   * @param[in]  - controller type
   * @retval     - driver*
   */
  driver* GetDriverInstance(unc_keytype_ctrtype_t controller_type);

  /**
   * @brief       - This function is to stores the controller type and Driver*
   * @param[in]   - controller type, driver*
   * @retval      - VTN_DRV_RET_FAILURE/ VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum RegisterDriver(unc_keytype_ctrtype_t controller_type, driver*);


  /**
   * @brief      - This function is to get the driver instance and controller
   *               instance for the respective controllers
   * @param[in]  - controller name, controller**, driver**
   * @retval     - VTN_DRV_RET_FAILURE/ VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum GetControllerInstance(std::string& controller_name,
                                      controller**, driver**);

  /**
   * @brief      - This function is to send controller status notifications
   *               to UPPL
   * @param[in]  - controller name,type
   * @retval     - None
   */
  void SendNotificationToPhysical(std::string ctr_name, ConnectionStatus type);

 private:
  std::map<std::string, ControllerContainer*>  controller_list;
  std::map<unc_keytype_ctrtype_t, driver*>  driver_container;
  pfc::core::ReadWriteLock controller_list_rwlock_;
};

/**
 * Class           - ReadParams
 * DataMembers     - ctlr_name_, ctr_fw_
 * MemberFunctions - PingController()
 */

class ReadParams : public std::unary_function < void, void> {
 public:
  std::string ctlr_name_;

  unc::driver::ControllerFramework* ctr_fw_;

  /**
   * @brief - constructor
   */
  ReadParams(std::string, unc::driver::ControllerFramework*);

  /**
   * @brief  - This fuction calls the method PingController()
   * @retval - None
   */
  void operator() ()  {
    PingController();
  }

  /**
   * @brief  - Ping the controller and update the connection status
   *           based on the response
   * @retval - VTN_DRV_RET_FAILURE/ VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum PingController();
};
}  // namespace driver
}  // namespace unc
#endif
