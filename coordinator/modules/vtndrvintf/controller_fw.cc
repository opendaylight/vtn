/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <controller_fw.hh>

namespace unc {
namespace driver {


    /**
    * @brief : constructor
    */
    ControllerFramework::ControllerFramework(pfc::core::TaskQueue* taskq_c)  {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      taskq_ = taskq_c;
      pfc_log_debug("taskq created");
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
    }

    /**
     * @brief : destructor
     */
    ControllerFramework::~ControllerFramework() {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
    }

    /**
     * @brief : constructor of ReadParams
     */
    ReadParams::ReadParams(std::string ctr_name,
                           unc::driver::ControllerFramework* fw_obj)
                           :ctlr_name_(ctr_name),
                           ctr_fw_(fw_obj) {}

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
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

      PFC_ASSERT(driver_instance != NULL);
      PFC_ASSERT(controller_instance != NULL);

      ControllerContainer *controller_container = new ControllerContainer();
      controller_container->ctr = controller_instance;
      controller_container->drv = driver_instance;

      controller_list_rwlock_.wrlock();
      controller_list.insert(std::make_pair(controller_name,
                                            controller_container));
      pfc_log_debug("controller added successfully");
      controller_list_rwlock_.unlock();

      pfc_bool_t ping_needed = driver_instance->is_ping_needed();

      if (ping_needed == PFC_TRUE) {
        controller_instance->timed_ = pfc::core::Timer::create(
                                                  taskq_->getId());
        PostTimer(controller_name, driver_instance, controller_instance);
      } else {
        pfc_log_debug("Ping not needed for the controller %s",
                     controller_name.c_str());
      }
     pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
     }

    /**
    * @brief     : The time interval for every ping is set and the
    *              timer is get reset.
    * @param[in] : controller name , driver*
    * @retval    : int
    */

    int ControllerFramework::PostTimer(std::string& controller_name,
                                       driver* driver_instance,
                                       controller* controller_instance) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      int ret = 0;
      pfc_timespec_t timeout;
      pfc_timeout_t time_out_id;

      uint32_t time_out = driver_instance->get_ping_interval();

      if (time_out == 0) {
        pfc_log_debug("Ping interval is 0");
        return ret;
      } else {
        ReadParams  fun_obj(controller_name, this);
        pfc::core::timer_func_t timer_func(fun_obj);

        timeout.tv_sec = time_out;
        timeout.tv_nsec = 0;
        pfc::core::Timer* timer = controller_instance->timed_;
        ret = timer->post(&timeout, timer_func, &time_out_id);

        if (ret != 0) {
          pfc_log_fatal("Failed to post() for ping operation");
        }
        pfc_log_debug("Timer successfully posted");
      }

      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return ret;
    }

    /**
    * @brief     : Ping the controller and update the connection status
    *              based on the response
    * @param[in] : None
    * @retval    : VTN_DRV_RET_FAILURE / VTN_DRV_RET_SUCCESS
    */

    VtnDrvRetEnum ReadParams::PingController() {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      unc::driver::driver* drv_instance = NULL;
      unc::driver::controller* ctr_instance = NULL;

      ctr_fw_->GetDriverByControllerName(ctlr_name_, &ctr_instance,
                                        &drv_instance);

      if (drv_instance == NULL || ctr_instance == NULL) {
        pfc_log_error("Driver instance not obtained");
        return VTN_DRV_RET_FAILURE;
      }

      pfc_bool_t ping_status = drv_instance->ping_controller(ctr_instance);

      if (ping_status == PFC_TRUE) {
        ctr_instance->set_connection_status(CONNECTION_UP);
        pfc_log_debug("Controller staus is UP ctlr_name: %s",
                     ctlr_name_.c_str());
      } else {
        ctr_instance->set_connection_status(CONNECTION_DOWN);
        pfc_log_debug("Controller staus is DOWN ctlr_name:%s",
                     ctlr_name_.c_str());
      }

      ctr_fw_->PostTimer(ctlr_name_, drv_instance, ctr_instance);
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return VTN_DRV_RET_SUCCESS;
    }


    /**
    * @brief     : Updating controler as per controller name,if controllername
    *              not found from list,it's consider as new controller and
    *              add it to list
    * @param[in] : controller_name, controller*, driver*
    * @retval    : VTN_DRV_RET_FAILURE / VTN_DRV_RET_SUCCESS
    */

    VtnDrvRetEnum ControllerFramework::UpdateControllerConfiguration(
                    std::string& controller_name,
                    controller* controller_instance, driver* driver_instance) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      std::map<std::string, ControllerContainer*>::iterator
           controller_list_iterator = controller_list.begin();

      PFC_ASSERT(controller_instance != NULL);
      PFC_ASSERT(driver_instance != NULL);

      ControllerContainer* controller_container(NULL);
      controller_list_iterator = controller_list.find(controller_name);

      pfc::core::ScopedMutex m(controller_list_iterator->second->rw_mutex);

      pfc_bool_t ping_needed = driver_instance->is_ping_needed();

      if (ping_needed == PFC_TRUE) {
        controller_instance->timed_ = pfc::core::Timer::create(
                                                  taskq_->getId());
        PostTimer(controller_name, driver_instance, controller_instance);
      } else {
        pfc_log_debug("%s Ping not needed for the controller %s",
                     PFC_FUNCNAME, controller_name.c_str());
      }

      if (controller_list_iterator != controller_list.end()) {
        controller_container = controller_list_iterator->second;
        PFC_ASSERT(controller_container != NULL);

        delete controller_container->ctr;

        controller_container->ctr = controller_instance;

        PFC_ASSERT(controller_container->ctr != NULL);

        pfc_log_debug("Controller updated successfully");
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
      }

      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return VTN_DRV_RET_SUCCESS;
    }

    /**
    * @brief     : This function removes the existing controller from list
    * @param[in] : controller_name
    * @retval    : VTN_DRV_RET_FAILURE / VTN_DRV_RET_SUCCESS
    */
    VtnDrvRetEnum
        ControllerFramework::RemoveControllerConfiguration(
                              std::string& controller_name,
                              controller* controller_instance,
                              driver* driver_instance) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      PFC_ASSERT(controller_instance != NULL);
      PFC_ASSERT(driver_instance != NULL);

      std::map<std::string, ControllerContainer*>::iterator
           controller_list_iterator = controller_list.begin();


      if (controller_list.size() == 0) {
        pfc_log_error("Controller name not found .List is Empty");
        return VTN_DRV_RET_FAILURE;
      }

      ControllerContainer* controller_container(NULL);
      controller_list_iterator = controller_list.find(controller_name);

      if (controller_list_iterator != controller_list.end()) {
        controller_container = controller_list_iterator->second;
        PFC_ASSERT(controller_container != NULL);

        controller_container->rw_mutex.lock();

        if (controller_instance->timed_) {
          delete controller_instance->timed_;
          controller_instance->timed_ = NULL;
        }

        driver_instance->delete_controller(controller_instance);
        controller_container->rw_mutex.unlock();

        controller_list_rwlock_.wrlock();
        delete controller_container;
        controller_list.erase(controller_list_iterator);
        controller_list_rwlock_.unlock();
        pfc_log_debug("Existing controller configuration gets removed");
      } else {
        pfc_log_error("controller not found");
        return VTN_DRV_RET_FAILURE;
      }

      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return VTN_DRV_RET_SUCCESS;
    }

    /**
    * GetDriverByControllerName:
    * @brief     : This function gets the driver type for the appropriate
    *              controllers
    * @param[in] : controller_name, controller**, driver**
    * @retval    : VTN_DRV_RET_FAILURE / VTN_DRV_RET_SUCCESS
    */
    VtnDrvRetEnum ControllerFramework::GetDriverByControllerName(
                                    std::string& controller_name,
                                    controller** controller_instance,
                                    driver** driver_instance) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

      ControllerContainer* controller_container(NULL);
      std::map<std::string, ControllerContainer*>::iterator
          controller_list_iterator = controller_list.begin();
      controller_list_iterator = controller_list.find(controller_name);

      if (controller_list_iterator == controller_list.end()) {
        pfc_log_error("controller name not found");
        return  VTN_DRV_RET_FAILURE;
      }

      pfc::core::ScopedMutex m(controller_list_iterator->second->rw_mutex);

      controller_container = controller_list_iterator->second;

      *controller_instance = controller_container->ctr;
      *driver_instance = controller_container->drv;

      if (*controller_instance == NULL || *driver_instance == NULL) {
        pfc_log_error("controller instance or driver instance is NULL");
        return VTN_DRV_RET_FAILURE;
      }

      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return VTN_DRV_RET_SUCCESS;
    }


    /**
    * @brief     : This function stores the driver instance for the appropriate
    *              controller type in map
    * @param[in] : controller_type, driver*
    * @retval    : VTN_DRV_RET_FAILURE / VTN_DRV_RET_SUCCESS
    */
    VtnDrvRetEnum ControllerFramework::RegisterDriver(
                              unc_keytype_ctrtype_t controller_type,
                              driver* driver_instance)  {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

      if (driver_instance == NULL) {
        pfc_log_error("RegisterDriver:Driver instance is NULL");
        return VTN_DRV_RET_FAILURE;
      }

      controller_list_rwlock_.wrlock();
      driver_container.insert(std::make_pair(controller_type, driver_instance));
      controller_list_rwlock_.unlock();

      pfc_log_debug("driver instance populated");
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return VTN_DRV_RET_SUCCESS;
    }

    /**
    * @brief     : This function gets the driver instance and ctl instance
    *              for the appropriate without acquiring the lock controllers
    * @param[in] : controller_name, controller*, driver*
    * @param[out]: driver*
    * @retval    : VTN_DRV_RET_FAILURE / VTN_DRV_RET_SUCCESS
    */
    VtnDrvRetEnum ControllerFramework::GetControllerInstance(
                                    std::string& controller_name,
                                    controller** controller_instance,
                                    driver** driver_instance) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

      ControllerContainer* controller_container(NULL);
      std::map<std::string, ControllerContainer*>::iterator
          controller_list_iterator = controller_list.begin();
      controller_list_iterator = controller_list.find(controller_name);

      if (controller_list_iterator == controller_list.end()) {
        pfc_log_error("Controller Name not found in the list");
        return  VTN_DRV_RET_FAILURE;
      }

      controller_container = controller_list_iterator->second;

      *controller_instance = controller_container->ctr;
      *driver_instance = controller_container->drv;

      PFC_ASSERT(*controller_instance != NULL);
      PFC_ASSERT(*driver_instance != NULL);

      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return VTN_DRV_RET_SUCCESS;
    }

    /**
     * @brief     : This function returns the driver instance for the appropriate
     *              controller type
     * @param[in] : controller_type
     * @retval    : driver*
     */
    driver* ControllerFramework::GetDriverInstance(
        unc_keytype_ctrtype_t controller_type) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

      if (driver_container.size() == 0) {
        pfc_log_error("Driver list is empty");
        return NULL;
      }

      return driver_container.find(controller_type)->second;
    }
}  // namespace driver
}  // namespace unc
