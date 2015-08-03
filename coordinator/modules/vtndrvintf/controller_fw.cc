/*
 * Copyright (c) 2013-2015 NEC Corporation
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
ControllerFramework::ControllerFramework(pfc::core::TaskQueue* taskq_c,
                                         uint32_t time_interval)  {
  ODC_FUNC_TRACE;
  taskq_ = taskq_c;
  time_interval_ = time_interval;
}

/**
 * @brief : constructor of ReadParams
 */
ReadParams::ReadParams(std::string ctr_name,
                       unc::driver::ControllerFramework* fw_obj)
                       :ctlr_name_(ctr_name),
                       ctr_fw_(fw_obj) {}

/**
* @brief : constructor of ReadInfo
*/
ReadConfig::ReadConfig(std::string ctr_name,
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

  controller_list.insert(std::make_pair(controller_name,
                                        controller_container));
  pfc_log_debug("%s:Controller [%s] added successfully", PFC_FUNCNAME,
                controller_name.c_str());

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
  pfc_bool_t physicalconfig_needed =
      driver_instance->is_physicalconfig_needed();

  if (physicalconfig_needed == PFC_TRUE) {
    controller_instance->physical_timed_ = pfc::core::Timer::create(
        taskq_->getId());
    post_physical_taskq(controller_name,
                        driver_instance,
                        controller_instance,
                        first_physical_task_interval);
  } else {
    pfc_log_debug("physicalconfig not needed for the controller %s",
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
  pfc_log_debug("Ping interval is %d ", ping_interval);

  if (ping_interval == 0) {
    return ret;
  } else {
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

  controller_operation util_obj(ctr_fw_, READ_FROM_CONTROLLER, ctlr_name_);
  ctr_instance = util_obj.get_controller_handle();
  drv_instance = util_obj.get_driver_handle();

  if ((NULL == ctr_instance) || (NULL == drv_instance)) {
    pfc_log_error("%s: Controller (%s)/driver instance is NULL",
                  PFC_FUNCNAME, ctlr_name_.c_str());
    return;
  }

  pfc_bool_t ping_status = drv_instance->ping_controller(ctr_instance);

  if (PFC_TRUE == ping_status) {
    pfc_log_debug("Controller [%s] is reachable.Ping status: %d",
                  ctlr_name_.c_str(), ping_status);
    if (ctr_instance->get_connection_status() == CONNECTION_DOWN) {
      //  sendnotification as up
      ctr_fw_->SendNotificationToPhysical(ctlr_name_, CONNECTION_UP);
      pfc_log_debug("Controller status is UP ctlr_name: %s",
                    ctlr_name_.c_str());
    }
    ctr_instance->set_connection_status(CONNECTION_UP);
  } else {
    pfc_log_debug("Controller [%s] is unreachable.Ping status: %d",
                  ctlr_name_.c_str(), ping_status);
    if (ctr_instance->get_connection_status() == CONNECTION_UP) {
      //  sendNotification as down
      ctr_fw_->SendNotificationToPhysical(ctlr_name_, CONNECTION_DOWN);
      pfc_log_debug("Controller status is DOWN ctlr_name:%s",
                    ctlr_name_.c_str());
    }
    ctr_fw_->SetDomainFlag(ctlr_name_, PFC_FALSE);
    ctr_fw_->SetEventFlag(ctlr_name_, PFC_FALSE);
    ctr_instance->set_connection_status(CONNECTION_DOWN);
  }
  uint32_t ping_interval = drv_instance->get_ping_interval();
  ctr_fw_->PostTimer(ctlr_name_, drv_instance, ctr_instance, ping_interval);
}

/**
 * @brief     : The time interval for every ping is set and the
 *              timer is get reset.
 * @param[in] : controller name , driver*
 * @retval    : int
 */

int ControllerFramework::post_physical_taskq(std::string& controller_name,
                                             driver* driver_instance,
                                             controller* controller_instance,
                                             uint32_t time_interval) {
  ODC_FUNC_TRACE;
  int ret = 0;
  pfc_timespec_t timeout;
  pfc_timeout_t time_out_id;

  PFC_ASSERT(driver_instance != NULL);
  pfc_log_debug("Time interval is %d ", time_interval);

  if (time_interval == 0) {
    return ret;
  } else {
    ReadConfig fun_obj(controller_name, this);
    pfc::core::timer_func_t timer_func(fun_obj);

    timeout.tv_sec = time_interval;
    timeout.tv_nsec = 0;
    PFC_ASSERT(controller_instance != NULL);
    pfc::core::Timer* timer = controller_instance->physical_timed_;
    ret = timer->post(&timeout, timer_func, &time_out_id);

    if (ret != 0) {
      pfc_log_fatal("Failed to post() for GetPhysicalConfig operation");
    }
    pfc_log_debug("Timer successfully posted for GetPhysicalConfig");
  }

  return ret;
}

/**
 * @brief     : Method to get physical configuration from controller
 *              at particular time interval
 * @param[in] : None
 * @retval    : None
 */

void ReadConfig::GetPhysicalConfig() {
  ODC_FUNC_TRACE;
  unc::driver::driver* drv_instance = NULL;
  unc::driver::controller* ctr_instance = NULL;

  controller_operation util_obj(ctr_fw_, READ_FROM_CONTROLLER, ctlr_name_);
  ctr_instance = util_obj.get_controller_handle();
  drv_instance = util_obj.get_driver_handle();
  if ((NULL == ctr_instance) || (NULL == drv_instance)) {
    pfc_log_error("%s: Controller (%s)/driver instance is NULL",
                  PFC_FUNCNAME, ctlr_name_.c_str());
    return;
  }
  if (ctr_instance->physical_port_cache == NULL) {
    pfc_log_debug("physical_port Cache for controller created");
    ctr_instance->physical_port_cache =
        unc::vtndrvcache::KeyTree::create_cache();
  }
  pfc_bool_t ping_status =
      drv_instance->get_physical_port_details(ctr_instance);

  if (PFC_TRUE == ping_status) {
    pfc_log_debug("Controller [%s] is reachable.GetPhysicalInfo status: %d",
                  ctlr_name_.c_str(), ping_status);
  } else {
    pfc_log_debug("Controller[%s] is unreachable.GetPhysicalInfo status:%d",
                  ctlr_name_.c_str(), ping_status);
  }

  ctr_fw_->post_physical_taskq(ctlr_name_, drv_instance, ctr_instance,
                               ctr_fw_->time_interval_);
}

/**
 * @brief     : Updating controler as per controller name,if controllername
 *              not found from list,it's consider as new controller and
 *              add it to list
 * @param[in] : controller_name, controller*, driver*
 * @retval    : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */

UncRespCode ControllerFramework::UpdateControllerConfiguration(
    std::string& controller_name,
    controller* controller_instance, driver* driver_instance,
    const key_ctr_t& key_ctr, const val_ctr_t& val_ctr) {
  ODC_FUNC_TRACE;
  std::map<std::string, ControllerContainer*>::iterator
      controller_list_iterator;

  PFC_VERIFY(controller_instance != NULL);
  PFC_VERIFY(driver_instance != NULL);

  ControllerContainer* controller_container(NULL);
  driver_instance->update_controller(
      key_ctr, val_ctr, controller_instance);

  controller_list_iterator = controller_list.find(controller_name);
  pfc_bool_t ping_needed = driver_instance->is_ping_needed();

  pfc_bool_t physicalconfig_needed =
      driver_instance->is_physicalconfig_needed();

  if (controller_list_iterator != controller_list.end()) {
    controller_container = controller_list_iterator->second;
    PFC_VERIFY(controller_container != NULL);

    controller_container->ctr = controller_instance;

    if (physicalconfig_needed == PFC_TRUE) {
      post_physical_taskq(controller_name,
                          driver_instance,
                          controller_container->ctr,
                          first_physical_task_interval);
    } else {
      pfc_log_debug("physicalconfig not needed for the controller %s",
                    controller_name.c_str());
    }
    if (PFC_TRUE == ping_needed) {
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

    if (PFC_TRUE == physicalconfig_needed) {
      controller_instance->physical_timed_ = pfc::core::Timer::create(
          taskq_->getId());
      post_physical_taskq(controller_name, driver_instance,
                          new_controller_container->ctr,
                          time_interval_);
    } else {
      pfc_log_debug("physicalconfig not needed for the controller %s",
                    controller_name.c_str());
    }
    if (PFC_TRUE == ping_needed) {
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
  return UNC_RC_SUCCESS;
}

/**
 * @brief     : This function removes the existing controller from list
 * @param[in] : controller_name
 * @retval    : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode
ControllerFramework::RemoveControllerConfiguration(
    std::string& controller_name,
    controller* controller_instance,
    driver* driver_instance) {
  ODC_FUNC_TRACE;
  PFC_VERIFY(controller_instance != NULL);
  PFC_VERIFY(driver_instance != NULL);

  if (controller_list.size() == 0) {
    pfc_log_error("Controller name not found .List is Empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  std::map<std::string, ControllerContainer*>::iterator
      controller_list_iterator;

  ControllerContainer* controller_container(NULL);
  controller_list_iterator = controller_list.find(controller_name);

  if (controller_list_iterator != controller_list.end()) {
    controller_container = controller_list_iterator->second;
    PFC_VERIFY(controller_container != NULL);

    delete controller_container;
    controller_list.erase(controller_list_iterator);
    pfc_log_debug("Existing controller configuration gets removed");
  } else {
    pfc_log_error("controller not found");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  return UNC_RC_SUCCESS;
}

/**
 * GetDriverByControllerName:
 * @brief     : This function gets the driver type for the appropriate
 *              controllers
 * @param[in] : controller_name, controller**, driver**
 * @retval    : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode ControllerFramework::GetDriverByControllerName(
    std::string& controller_name,
    controller** controller_instance,
    driver** driver_instance) {
  ODC_FUNC_TRACE;

  ControllerContainer* controller_container(NULL);
  std::map<std::string, ControllerContainer*>::iterator
      controller_list_iterator;
  controller_list_iterator = controller_list.find(controller_name);

  if (controller_list_iterator == controller_list.end()) {
    pfc_log_error("controller name not found");
    return  UNC_DRV_RC_ERR_GENERIC;
  }

  controller_container = controller_list_iterator->second;

  *controller_instance = controller_container->ctr;
  *driver_instance = controller_container->drv;

  if ((NULL == *controller_instance) || (NULL == *driver_instance)) {
    pfc_log_error("controller instance or driver instance is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}


/**
 * @brief     : This function stores the driver instance for the appropriate
 *              controller type in map
 * @param[in] : controller_type, driver*
 * @retval    : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode ControllerFramework::RegisterDriver(
    unc_keytype_ctrtype_t controller_type,
    driver* driver_instance)  {
  ODC_FUNC_TRACE;

  if (NULL == driver_instance) {
    pfc_log_error("RegisterDriver:Driver instance is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  controller_list_rwlock_.lock();
  driver_container.insert(std::make_pair(controller_type, driver_instance));
  controller_list_rwlock_.unlock();

  return UNC_RC_SUCCESS;
}

/**
 * @brief     : This function gets the driver instance and ctl instance
 *              for the appropriate without acquiring the lock controllers
 * @param[in] : controller_name, controller*, driver*
 * @param[out]: driver*
 * @retval    : UNC_DRV_RC_ERR_GENERIC / UNC_RC_SUCCESS
 */
UncRespCode ControllerFramework::GetControllerInstance(
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
    return  UNC_DRV_RC_ERR_GENERIC;
  }

  controller_container = controller_list_iterator->second;

  *controller_instance = controller_container->ctr;
  *driver_instance = controller_container->drv;

  PFC_VERIFY(*controller_instance != NULL);
  PFC_VERIFY(*driver_instance != NULL);

  return UNC_RC_SUCCESS;
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

void ControllerFramework::SetDomainFlag(std::string ctr_name,
                                        pfc_bool_t flag) {
  ControllerContainer* controller_container(NULL);
  std::map<std::string, ControllerContainer*>::iterator
      controller_list_iterator = controller_list.begin();
  controller_list_iterator = controller_list.find(ctr_name);

  if (controller_list_iterator == controller_list.end()) {
    pfc_log_error("Controller Name not found in the list");
  } else {
    controller_container = controller_list_iterator->second;
    PFC_VERIFY(controller_container != NULL);
    controller_container->Domain_event_ = flag;
  }
}

pfc_bool_t ControllerFramework::GetDomainFlag(std::string ctr_name) {
  ControllerContainer* controller_container(NULL);
  std::map<std::string, ControllerContainer*>::iterator
      controller_list_iterator = controller_list.begin();
  controller_list_iterator = controller_list.find(ctr_name);

  if (controller_list_iterator == controller_list.end()) {
    pfc_log_error("Controller Name not found in the list");
  } else {
    controller_container = controller_list_iterator->second;
    PFC_VERIFY(controller_container != NULL);
    return controller_container->Domain_event_;
  }
  return PFC_TRUE;
}

void ControllerFramework::SetEventFlag(std::string ctr_name,
                                        pfc_bool_t flag) {
  ControllerContainer* controller_container(NULL);
  std::map<std::string, ControllerContainer*>::iterator
      controller_list_iterator = controller_list.begin();
  controller_list_iterator = controller_list.find(ctr_name);

  if (controller_list_iterator == controller_list.end()) {
    pfc_log_error("Controller Name not found in the list");
  } else {
    controller_container = controller_list_iterator->second;
    PFC_VERIFY(controller_container != NULL);
    controller_container->Start_event_ = flag;
  }
}

pfc_bool_t ControllerFramework::GetEventFlag(std::string ctr_name) {
  ControllerContainer* controller_container(NULL);
  std::map<std::string, ControllerContainer*>::iterator
      controller_list_iterator = controller_list.begin();
  controller_list_iterator = controller_list.find(ctr_name);

  if (controller_list_iterator == controller_list.end()) {
    pfc_log_error("Controller Name not found in the list");
  } else {
    controller_container = controller_list_iterator->second;
    PFC_VERIFY(controller_container != NULL);
    return controller_container->Start_event_;
  }
  return PFC_TRUE;
}


controller_operation::controller_operation(ControllerFramework *fw_ptr,
                                           ControllerOps operation,
                                           std::string ctl_id,
                                           unc_keytype_ctrtype_t ctl_type)
                                           : ctl_fw_(fw_ptr),
                                           ctl_oper_(operation),
                                           ctr_(NULL),
                                           drv_(NULL) {
  ODC_FUNC_TRACE;

  ctl_fw_->controller_list_rwlock_.lock();
  pfc_log_debug(" Controller TYPE:%d", ctl_type);
  drv_ = ctl_fw_->GetDriverInstance(ctl_type);
  controller_status_ = PFC_FALSE;
}

controller_operation::controller_operation(ControllerFramework *fw_ptr,
                                           ControllerOps operation,
                                           std::string ctl_id)
                                           : ctl_fw_(fw_ptr),
                                           ctl_oper_(operation),
                                           ctr_(NULL),
                                           drv_(NULL),
                                           controller_status_(PFC_FALSE) {
  ODC_FUNC_TRACE;
  ctl_fw_->controller_list_rwlock_.lock();
  ctl_fw_->GetDriverByControllerName(ctl_id, &ctr_, &drv_);

  if (ctr_== NULL && drv_ == NULL) {
    set_controller_status(PFC_FALSE);
  } else {
    set_controller_status(PFC_TRUE);
    switch (ctl_oper_) {
      case CONTROLLER_DELETE:
        ctr_->access_.mark_controller_for_delete();
      case CONTROLLER_UPDATE: {
        if (ctl_oper_ ==  CONTROLLER_UPDATE)
          ctr_->access_.mark_controller_for_update();
        if (ctr_->access_.can_delete_update() == PFC_TRUE) {
          pfc_log_debug("No r/w operation.. Proced with Delete");
        } else {
          pfc_log_debug(" r/w operation Inprogress.Wait before Delete");
          ctr_->access_.delete_wait(ctl_fw_->controller_list_rwlock_);
        }
        break;
      }
      case READ_FROM_CONTROLLER: {
        if ( ctr_->access_.can_delete_controller_marked_for_delete()
            == PFC_TRUE ) {
          drv_= NULL;
          ctr_= NULL;
          break;
        } else if ( ctr_->access_.get_controller_marked_for_update()
                   == PFC_TRUE ) {
          ctr_->access_.increment_read_wait_count();
          ctr_->access_.wait_until_update(ctl_fw_->controller_list_rwlock_);
          ctr_->access_.decrement_read_wait_count();
        }
        uint32_t read_count = ctr_->access_.get_read_count();
        read_count++;
        ctr_->access_.set_read_count(read_count);
        ctl_fw_->controller_list_rwlock_.unlock();
        break;
      }
      case WRITE_TO_CONTROLLER: {
        uint32_t write_count = ctr_->access_.get_write_count();
        pfc_log_debug("write count:%d", write_count);
        if ( ctr_->access_.can_delete_controller_marked_for_delete()
            == PFC_TRUE ) {
          pfc_log_debug("can_delete_controller:TRUE");
          drv_= NULL;
          ctr_= NULL;
          break;
        } else if ((write_count != 0) ||
                   (ctr_->access_.get_controller_marked_for_update() ==
                    PFC_TRUE)) {
          ctr_->access_.increment_write_wait_count();
          ctr_->access_.wait_for_write(ctl_fw_->controller_list_rwlock_);
          pfc_log_debug("Write cond Wait");
          ctr_->access_.decrement_write_wait_count();
        }
        pfc_log_debug("WRITE in progress");
        ctr_->access_.set_write_in_progress();
        ctr_->access_.set_write_count(1);
        ctl_fw_->controller_list_rwlock_.unlock();
        break;
      }
      default:
        pfc_log_debug("Not a valid oper..");
    }
  }
}

controller_operation::~controller_operation() {
  pfc_log_debug("~ControllerFwUtil--oper_type:%d", ctl_oper_);
  if (ctr_== NULL && drv_ == NULL) {
    ctl_fw_->controller_list_rwlock_.unlock();
  } else {
    switch (ctl_oper_) {
      case CONTROLLER_UPDATE: {
        ctr_->access_.set_controller_marked_for_update(PFC_FALSE);
        ctr_->access_.update_completed();
        if (ctr_->access_.get_write_wait_count() > 0)
          ctr_->access_.release_write_wait();
      }
      case CONTROLLER_DELETE:
      case CONTROLLER_ADD:
        ctl_fw_->controller_list_rwlock_.unlock();
        break;
      case READ_FROM_CONTROLLER:
      case WRITE_TO_CONTROLLER: {
        ctl_fw_->controller_list_rwlock_.lock();
        uint32_t read_count = ctr_->access_.get_read_count();
        uint32_t write_count = ctr_->access_.get_write_count();
        if (ctl_oper_ == READ_FROM_CONTROLLER)
          read_count--;
        if (ctl_oper_ == WRITE_TO_CONTROLLER) {
          write_count--;
          pfc_log_debug("Write completed set write_in_progress_ FALSE");
          ctr_->access_.write_completed();
        }
        ctr_->access_.set_read_count(read_count);
        ctr_->access_.set_write_count(write_count);
        if ( write_count <= 0 && ctr_->access_.get_write_wait_count() > 0 )
          ctr_->access_.release_write_wait();
        if ( write_count <= 0 && ctr_->access_.get_write_wait_count() == 0
            && read_count == 0 )
          ctr_->access_.delete_allow();
        ctl_fw_->controller_list_rwlock_.unlock();
        break;
      }
      default:
        pfc_log_debug("Not a valid oper..");
    }
  }
}

}  // namespace driver
}  // namespace unc
