/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
#include <uncxx/odc_log.hh>
#include <unc/uppl_common.h>
#include <unc/unc_events.h>
#include <functional>
#include <iterator>
#include <map>
#include <string>

#define domainID "domainname"

namespace unc {
namespace driver {

//  Ping Interval of a controller when a controller is added/updated(secs)
const uint32_t first_ping_interval = 10;

// physical task time interval for controller when a controller is
// added/updated(secs)

const uint32_t first_physical_task_interval = 15;

/**
 * @brief       - This class contains the controller instance ,driver instance,
 *                mutex lock
 * @DataMembers - controller*, driver*, rw_mutex
 */
class ControllerContainer {
 public:
  ControllerContainer():ctr(NULL), drv(NULL),
  Domain_event_(PFC_FALSE), Start_event_(PFC_FALSE) {}
  controller* ctr;
  driver* drv;
  pfc_bool_t Domain_event_;
  pfc_bool_t Start_event_;
  ~ControllerContainer() {
    pfc_log_trace("Entering ControllerContainer destructor");
    if (NULL != ctr) {
       delete ctr;
       ctr = NULL;
    }
    pfc_log_trace("Exiting ControllerContainer destructor");
  }
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
  explicit ControllerFramework(pfc::core::TaskQueue*, uint32_t time_interval);

  /**
   * @brief   - destructor
   */
  ~ControllerFramework();

  /**
   * @brief     - Retrieves the driver type for the respective controllers
   * @param[in] - controller name ,controller**, driver**
   * @retval    - VTN_DRV_RET_FAILURE/VTN_DRV_RET_SUCCESS
   */
  UncRespCode GetDriverByControllerName(std::string& controller_name,
                                          controller**, driver**);

  /**
   * @brief       - This function  sets the time interval for every ping and the
   *                timer is get reset.
   * @param[in]   - controller name, driver*, controller*,ping_interval
   * @retval      - int
   */
  int PostTimer(std::string& controller_name, driver*, controller*,
                uint32_t ping_interval);

  /**
   * @brief       - This function  sets the time interval to get controller
   *                physical configuration and the
   *                timer is get reset.
   * @param[in]   - controller name, driver*, controller*,ping_interval
   * @retval      - int
   */
  int post_physical_taskq(std::string& controller_name, driver*, controller*,
                uint32_t time_interval);

  /**
   * @brief      - This function adds the respective controller information
   *               in the controller list
   * @param[in]  - controller name, controller*, driver*
   * @retval     - None
   */
  void AddController(std::string& controller_name,
                     controller* , driver*);
  /**
   * @brief        - This function updates the respective controller information
   *                 in the controller list
   * @param[in]    - controller name, controller*, driver*
   * @retval       - VTN_DRV_RET_FAILURE/ VTN_DRV_RET_SUCCESS
   */
  UncRespCode UpdateControllerConfiguration(std::string& controller_name,
                                               controller*, driver*,
                                               const key_ctr_t& key_ctr,
                                               const val_ctr_t& val_ctr);
  /**
   * @brief         - This function removes the respective controller information
   *                 in the controller list
   * @param[in]    - controller name, controller*, driver*
   * @retval       - VTN_DRV_RET_FAILURE/ VTN_DRV_RET_SUCCESS
   */
  UncRespCode RemoveControllerConfiguration(std::string& controller_name,
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
  UncRespCode RegisterDriver(unc_keytype_ctrtype_t controller_type,
                                 driver*);


  /**
   * @brief      - This function is to get the driver instance and controller
   *               instance for the respective controllers
   * @param[in]  - controller name
   * @param[out] - controller**, driver**
   * @retval     - VTN_DRV_RET_FAILURE/ VTN_DRV_RET_SUCCESS
   */
  UncRespCode GetControllerInstance(std::string& controller_name,
                                      controller**, driver**);

  /**
   * @brief      - This function is to send controller status notifications
   *               to UPPL
   * @param[in]  - controller name,type
   * @retval     - None
   */
  void SendNotificationToPhysical(std::string ctr_name, ConnectionStatus type);
  /**
   * @brief      - This function is to Get the Domain event flag
   * @param[in]  - controller name
   * @retval     - PFC_TRUE/PFC_FALSE
   */
  pfc_bool_t GetDomainFlag(std::string ctr_name);
  /**
   * @brief      - This function is to Set the Domain event flag
   * @param[in]  - controller name
   * @param[in]  - flag to set domain
   * @retval     - None
   */
  void SetDomainFlag(std::string ctr_name, pfc_bool_t flag);
  /**
   * @brief      - This function is to Get the event start flag
   * @param[in]  - controller name
   * @retval     - PFC_TRUE/PFC_FALSE
   */
  pfc_bool_t GetEventFlag(std::string ctr_name);
  /**
   * @brief      - This function is to Set the event start flag
   * @param[in]  - controller name
   * @param[in]  - flag to set start event
   * @retval     - None
   */
  void SetEventFlag(std::string ctr_name, pfc_bool_t flag);

  pfc::core::Mutex controller_list_rwlock_;
  uint32_t time_interval_;  //  in seconds

 private:
  std::map<std::string, ControllerContainer*>  controller_list;
  std::map<unc_keytype_ctrtype_t, driver*>  driver_container;
};

/**
 * Class           - This class is invokes the ping for the given timeout
 * DataMembers     - ctlr_name_, ctr_fw_
 * MemberFunctions - PingController()
 */

class ReadParams : public std::unary_function < void, void> {
 public:
  std::string ctlr_name_;

  unc::driver::ControllerFramework* ctr_fw_;

  /**
   * @brief - parameterised constructor to initialize controller name and
   *          ControllerFramework object
   */
  ReadParams(std::string, unc::driver::ControllerFramework*);

  /**
   * @brief  - This fuction calls the method PingController()
   * @retval - None
   */
  void operator() () {
    PingController();
  }

  /**
   * @brief  - Ping the controller and update the connection status
   *           based on the response
   * @retval - None
   */
  void PingController();
};

/**
 * Class           - This class is get the physical configuration from the
 *                   controller for the given timeout
 * DataMembers     - ctlr_name_, ctr_fw_
 * MemberFunctions - GetPhysicalInfo()
 */

class ReadConfig : public std::unary_function < void, void> {
 public:
  std::string ctlr_name_;

  unc::driver::ControllerFramework* ctr_fw_;

  /**
   * @brief - parameterised constructor to initialize controller name and
   *          ControllerFramework object
   */
  ReadConfig(std::string, unc::driver::ControllerFramework*);

  /**
   * @brief  - This fuction calls the method GetPhysicalInfo()
   * @retval - None
   */
  void operator() () {
    GetPhysicalConfig();
  }

  /**
   * @brief  - Function to get the physical configuration from
   *           the controller
   * @retval - None
   */
  void GetPhysicalConfig();
};
}  // namespace driver
}  // namespace unc
#endif
