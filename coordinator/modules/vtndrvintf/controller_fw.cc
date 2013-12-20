/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <controller_fw.hh>
#include <pfcxx/ipc_server.hh>

const std::string ctr_version = "1.0.0.0";

namespace unc {
namespace driver {
    /**
    * @brief : constructor
    */
    ControllerFramework::ControllerFramework(pfc::core::TaskQueue* taskq_c)  {
      ODC_FUNC_TRACE;
      taskq_ = taskq_c;
    }

    /**
     * @brief : constructor of ReadParams
     */
    ReadParams::ReadParams(std::string ctr_name,
                           unc::driver::ControllerFramework* fw_obj)
                           :ctlr_name_(ctr_name),
                           ctr_fw_(fw_obj) {}

    /**
     * @brief : destructor
     */
    ControllerFramework::~ControllerFramework() {
      ODC_FUNC_TRACE;
      if (!controller_list.empty()) {
      for (std::map<std::string, ControllerContainer*>::iterator it =
       controller_list.begin(); it != controller_list.end(); ++it) {
       ControllerContainer* DeleteObject = it->second;
       delete DeleteObject;
       }
      controller_list.clear();
      }
    }

    /**
    * @brief     : Adding new controller with controller information in
    *              a map  and also checks  that ping is needed for the
    *              particular controller or not
    * @param[in] : controller name , controller*, driver*
    * @retval    : None
    **/
    void ControllerFramework::AddController(
                  std::string& controller_name,
                  controller* controller_instance, driver* driver_instance) {
      ODC_FUNC_TRACE;

      PFC_VERIFY(driver_instance != NULL);
      PFC_VERIFY(controller_instance != NULL);

      ControllerContainer *controller_container = new ControllerContainer();
      controller_container->ctr = controller_instance;
      controller_container->drv = driver_instance;

      controller_list_rwlock_.lock();
      controller_list.insert(std::make_pair(controller_name,
                                            controller_container));
      pfc_log_debug("%s:Controller [%s] added successfully", PFC_FUNCNAME,
                    controller_name.c_str());
      controller_list_rwlock_.unlock();

      pfc_bool_t ping_needed = driver_instance->is_ping_needed();

      if (ping_needed == PFC_TRUE) {
        controller_instance->timed_ = pfc::core::Timer::create(
                                                  taskq_->getId());
        PostTimer(controller_name, driver_instance, controller_instance,
                  first_ping_interval);
      } else {
        pfc_log_debug("Ping not needed for the controller %s",
                     controller_name.c_str());
      }
     }

    /**
    * @brief     : The time interval for every ping is set and the
    *              timer is get reset.
    * @param[in] : controller name , driver*
    * @retval    : int
    */

    int ControllerFramework::PostTimer(std::string& controller_name,
                                       driver* driver_instance,
                                       controller* controller_instance,
                                       uint32_t ping_interval) {
      ODC_FUNC_TRACE;
      int ret = 0;
      pfc_timespec_t timeout;
      pfc_timeout_t time_out_id;

      PFC_VERIFY(driver_instance != NULL);

      if (ping_interval == 0) {
        pfc_log_debug("Ping interval is 0");
        return ret;
      } else {
        pfc_log_debug("Ping interval is %d ", ping_interval);
        ReadParams fun_obj(controller_name, this);
        pfc::core::timer_func_t timer_func(fun_obj);

        timeout.tv_sec = ping_interval;
        timeout.tv_nsec = 0;
        PFC_VERIFY(controller_instance != NULL);
        pfc::core::Timer* timer = controller_instance->timed_;
        ret = timer->post(&timeout, timer_func, &time_out_id);

        if (ret != 0) {
          pfc_log_fatal("Failed to post() for ping operation");
        }
        pfc_log_debug("Timer successfully posted");
      }

      return ret;
    }

    /**
    * @brief     : Ping the controller and update the connection status
    *              based on the response
    * @param[in] : None
    * @retval    : None
    */

    void ReadParams::PingController() {
      ODC_FUNC_TRACE;
      unc::driver::driver* drv_instance = NULL;
      unc::driver::controller* ctr_instance = NULL;

      ctr_fw_->GetDriverByControllerName(ctlr_name_, &ctr_instance,
                                        &drv_instance);
      if ((ctr_instance == NULL) || (drv_instance == NULL)) {
        pfc_log_error("%s: Controller (%s)/driver instance is NULL",
                      PFC_FUNCNAME, ctlr_name_.c_str());
        return;
      }

      pfc_bool_t ping_status = drv_instance->ping_controller(ctr_instance);

      if (ping_status == PFC_TRUE) {
        pfc_log_debug("Controller [%s] is reachable.Ping status: %d",
                      ctlr_name_.c_str(), ping_status);
        if (ctr_instance->get_connection_status() == CONNECTION_DOWN) {
          pfc_log_debug("Controller status is UP ctlr_name: %s",
                     ctlr_name_.c_str());
          //  sendnotification as up
          ctr_fw_->SendNotificationToPhysical(ctlr_name_, CONNECTION_UP);
        }
        ctr_instance->set_connection_status(CONNECTION_UP);
      } else {
        pfc_log_debug("Controller [%s] is unreachable.Ping status: %d",
                      ctlr_name_.c_str(), ping_status);
        if (ctr_instance->get_connection_status() == CONNECTION_UP) {
          pfc_log_debug("Controller status is DOWN ctlr_name:%s",
                     ctlr_name_.c_str());
          //  sendNotification as down
          ctr_fw_->SendNotificationToPhysical(ctlr_name_, CONNECTION_DOWN);
        }
        ctr_instance->set_connection_status(CONNECTION_DOWN);
      }
      uint32_t ping_interval = drv_instance->get_ping_interval();
      ctr_fw_->PostTimer(ctlr_name_, drv_instance, ctr_instance, ping_interval);
      ctr_fw_->controller_list_rwlock_.lock();
      ctr_instance->cond_var = PFC_FALSE;
      pfc_log_debug("%s Cond var is changed to %d in cond signal",
                    PFC_FUNCNAME, ctr_instance->cond_var);
      ctr_instance->rw_cond.broadcast();
      ctr_fw_->controller_list_rwlock_.unlock();
    }


    /**
    * @brief     : Updating controler as per controller name,if controllername
    *              not found from list,it's consider as new controller and
    *              add it to list
    * @param[in] : controller_name, controller*, driver*
    * @retval    : DRVAPI_RESPONSE_FAILURE / DRVAPI_RESPONSE_SUCCESS
    */

    drv_resp_code_t ControllerFramework::UpdateControllerConfiguration(
                    std::string& controller_name,
                    controller* controller_instance, driver* driver_instance,
                    const key_ctr_t& key_ctr, const val_ctr_t& val_ctr) {
      ODC_FUNC_TRACE;
      std::map<std::string, ControllerContainer*>::iterator
           controller_list_iterator;

      PFC_VERIFY(controller_instance != NULL);
      PFC_VERIFY(driver_instance != NULL);

      ControllerContainer* controller_container(NULL);
      controller_list_rwlock_.lock();
      // Waits on condition variable to be changed to PFC_FALSE
      if (controller_instance->cond_var == PFC_TRUE) {
        pfc_log_debug("%s Cond var is %d in cond wait", PFC_FUNCNAME,
                      controller_instance->cond_var);
        controller_instance->rw_cond.wait(controller_list_rwlock_);
      }
      driver_instance->update_controller(
                                   key_ctr, val_ctr, controller_instance);

      controller_list_iterator = controller_list.find(controller_name);
      pfc_bool_t ping_needed = driver_instance->is_ping_needed();

      if (controller_list_iterator != controller_list.end()) {
        controller_container = controller_list_iterator->second;
        PFC_VERIFY(controller_container != NULL);

        controller_container->ctr = controller_instance;

        pfc_log_debug("Controller updated successfully");
        if (ping_needed == PFC_TRUE) {
          PostTimer(controller_name, driver_instance, controller_container->ctr,
                    first_ping_interval);
        } else {
          pfc_log_debug("%s Ping not needed for the controller %s",
                       PFC_FUNCNAME, controller_name.c_str());
        }
      } else {
        ControllerContainer* new_controller_container = new
                                               ControllerContainer();

        pfc_log_trace("%s: Consider as a new controller to be add..",
                      PFC_FUNCNAME);
        new_controller_container->ctr = controller_instance;
        new_controller_container->drv = driver_instance;

        controller_list.insert(std::make_pair(controller_name,
                                              new_controller_container));
        pfc_log_debug("Controller added successfully");
        if (ping_needed == PFC_TRUE) {
          new_controller_container->ctr->timed_ = pfc::core::Timer::create(
                                                  taskq_->getId());
          PostTimer(controller_name, driver_instance,
                    new_controller_container->ctr,
                    first_ping_interval);
        } else {
          pfc_log_debug("%s Ping not needed for the controller %s",
                       PFC_FUNCNAME, controller_name.c_str());
        }
      }
      controller_list_rwlock_.unlock();
      return DRVAPI_RESPONSE_SUCCESS;
    }

    /**
    * @brief     : This function removes the existing controller from list
    * @param[in] : controller_name
    * @retval    : DRVAPI_RESPONSE_FAILURE / DRVAPI_RESPONSE_SUCCESS
    */
    drv_resp_code_t
        ControllerFramework::RemoveControllerConfiguration(
                              std::string& controller_name,
                              controller* controller_instance,
                              driver* driver_instance) {
      ODC_FUNC_TRACE;
      PFC_VERIFY(controller_instance != NULL);
      PFC_VERIFY(driver_instance != NULL);

      if (controller_list.size() == 0) {
        pfc_log_error("Controller name not found .List is Empty");
        return DRVAPI_RESPONSE_FAILURE;
      }

      std::map<std::string, ControllerContainer*>::iterator
           controller_list_iterator;

      ControllerContainer* controller_container(NULL);
      controller_list_rwlock_.lock();
      if (controller_instance->cond_var == PFC_TRUE) {
        pfc_log_debug("%s Cond var is %d in cond wait", PFC_FUNCNAME,
                      controller_instance->cond_var);
        controller_instance->rw_cond.wait(controller_list_rwlock_);
      }
      controller_list_iterator = controller_list.find(controller_name);

      if (controller_list_iterator != controller_list.end()) {
        controller_container = controller_list_iterator->second;
        PFC_VERIFY(controller_container != NULL);

        delete controller_container;
        controller_list.erase(controller_list_iterator);
        controller_list_rwlock_.unlock();
        pfc_log_debug("Existing controller configuration gets removed");
      } else {
        pfc_log_error("controller not found");
        controller_list_rwlock_.unlock();
        return DRVAPI_RESPONSE_FAILURE;
      }

      return DRVAPI_RESPONSE_SUCCESS;
    }

    /**
    * GetDriverByControllerName:
    * @brief     : This function gets the driver type for the appropriate
    *              controllers
    * @param[in] : controller_name, controller**, driver**
    * @retval    : DRVAPI_RESPONSE_FAILURE / DRVAPI_RESPONSE_SUCCESS
    */
    drv_resp_code_t ControllerFramework::GetDriverByControllerName(
                                    std::string& controller_name,
                                    controller** controller_instance,
                                    driver** driver_instance) {
      ODC_FUNC_TRACE;

      ControllerContainer* controller_container(NULL);
      std::map<std::string, ControllerContainer*>::iterator
          controller_list_iterator;
      controller_list_rwlock_.lock();
      controller_list_iterator = controller_list.find(controller_name);

      if (controller_list_iterator == controller_list.end()) {
        pfc_log_error("controller name not found");
        controller_list_rwlock_.unlock();
        return  DRVAPI_RESPONSE_FAILURE;
      }

      controller_container = controller_list_iterator->second;

      *controller_instance = controller_container->ctr;
      *driver_instance = controller_container->drv;

      if (*controller_instance == NULL || *driver_instance == NULL) {
        pfc_log_error("controller instance or driver instance is NULL");
        controller_list_rwlock_.unlock();
        return DRVAPI_RESPONSE_FAILURE;
      }
      (*controller_instance)->cond_var = PFC_TRUE;
      pfc_log_debug("%s Cond var is changed to %d in lock", PFC_FUNCNAME,
                      (*controller_instance)->cond_var);
      controller_list_rwlock_.unlock();
      return DRVAPI_RESPONSE_SUCCESS;
    }


    /**
    * @brief     : This function stores the driver instance for the appropriate
    *              controller type in map
    * @param[in] : controller_type, driver*
    * @retval    : DRVAPI_RESPONSE_FAILURE / DRVAPI_RESPONSE_SUCCESS
    */
    drv_resp_code_t ControllerFramework::RegisterDriver(
                              unc_keytype_ctrtype_t controller_type,
                              driver* driver_instance)  {
      ODC_FUNC_TRACE;

      if (driver_instance == NULL) {
        pfc_log_error("RegisterDriver:Driver instance is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }

      controller_list_rwlock_.lock();
      driver_container.insert(std::make_pair(controller_type, driver_instance));
      controller_list_rwlock_.unlock();

      pfc_log_debug("driver instance populated");
      return DRVAPI_RESPONSE_SUCCESS;
    }

    /**
    * @brief     : This function gets the driver instance and ctl instance
    *              for the appropriate without acquiring the lock controllers
    * @param[in] : controller_name, controller*, driver*
    * @param[out]: driver*
    * @retval    : DRVAPI_RESPONSE_FAILURE / DRVAPI_RESPONSE_SUCCESS
    */
    drv_resp_code_t ControllerFramework::GetControllerInstance(
                                    std::string& controller_name,
                                    controller** controller_instance,
                                    driver** driver_instance) {
      ODC_FUNC_TRACE;

      ControllerContainer* controller_container(NULL);
      std::map<std::string, ControllerContainer*>::iterator
          controller_list_iterator = controller_list.begin();
      controller_list_iterator = controller_list.find(controller_name);

      if (controller_list_iterator == controller_list.end()) {
        pfc_log_error("Controller Name not found in the list");
        return  DRVAPI_RESPONSE_FAILURE;
      }

      controller_container = controller_list_iterator->second;

      *controller_instance = controller_container->ctr;
      *driver_instance = controller_container->drv;

      PFC_VERIFY(*controller_instance != NULL);
      PFC_VERIFY(*driver_instance != NULL);

      return DRVAPI_RESPONSE_SUCCESS;
    }

    /**
     * @brief     : This function returns the driver instance for the appropriate
     *              controller type
     * @param[in] : controller_type
     * @retval    : driver*
     */
    driver* ControllerFramework::GetDriverInstance(
        unc_keytype_ctrtype_t controller_type) {
      ODC_FUNC_TRACE;
      if (driver_container.size() == 0) {
        pfc_log_error("Driver list is empty");
        return NULL;
      }
      std::map<unc_keytype_ctrtype_t, driver*>::iterator
          iter = driver_container.begin();
      iter = driver_container.find(controller_type);
      if (iter == driver_container.end()) {
        pfc_log_error("controller type not registered");
      return NULL;
      }
      return iter->second;
    }

    /**
     * @brief     : This function sends the status change notification to UPPL
     * @param[in] : controllername, Status
     * @retval    : None
     */
void ControllerFramework::SendNotificationToPhysical(
                                          std::string ctr_name,
                                          ConnectionStatus type) {
  ODC_FUNC_TRACE;

  key_ctr_t key_ctr;
  val_ctr_st_t val_ctr_new;
  val_ctr_st_t val_ctr_old;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr_new, 0, sizeof(val_ctr_st_t));
  memset(&val_ctr_old, 0, sizeof(val_ctr_st_t));
  val_ctr_new.valid[1] = UNC_VF_VALID;
  val_ctr_new.valid[2] = UNC_VF_VALID;

  int err = 0;

  memcpy(key_ctr.controller_name, ctr_name.c_str(), ctr_name.length() + 1);
  memcpy(val_ctr_new.actual_version,
         ctr_version.c_str(), ctr_version.length() + 1);
  if (type == CONNECTION_DOWN) {
    val_ctr_new.oper_status = CONTROLLER_OPER_DOWN;
    val_ctr_new.valid[kIdxOperStatus] = UNC_VF_VALID;
    val_ctr_new.valid[kIdxActualVersion] = UNC_VF_INVALID;
    val_ctr_old.oper_status = CONTROLLER_OPER_UP;
    val_ctr_old.valid[kIdxOperStatus] = UNC_VF_VALID;
  } else if (type == CONNECTION_UP) {
    val_ctr_new.oper_status = CONTROLLER_OPER_UP;
    val_ctr_new.valid[kIdxOperStatus] = UNC_VF_VALID;
    val_ctr_new.valid[kIdxActualVersion] = UNC_VF_VALID;
    val_ctr_old.oper_status = CONTROLLER_OPER_DOWN;
    val_ctr_old.valid[kIdxOperStatus] = UNC_VF_VALID;
  }
  pfc::core::ipc::ServerEvent phys_ctr_event(
                 (uint32_t) UNC_CTLR_STATE_EVENTS, err);

  phys_ctr_event.addOutput(ctr_name);
  phys_ctr_event.addOutput(domainID);
  phys_ctr_event.addOutput((uint32_t) UNC_OP_UPDATE);
  phys_ctr_event.addOutput((uint32_t) UNC_DT_STATE);
  phys_ctr_event.addOutput((uint32_t) UNC_KT_CONTROLLER);
  phys_ctr_event.addOutput(key_ctr);
  phys_ctr_event.addOutput(val_ctr_new);
  phys_ctr_event.addOutput(val_ctr_old);
  phys_ctr_event.post();

  pfc_log_debug("%s: Controller STATE CHANGE event posted.."
    "{ old_state = (%u), new_state = (%u) }", PFC_FUNCNAME,
                val_ctr_old.oper_status, val_ctr_new.oper_status);
}
}  // namespace driver
}  // namespace unc
