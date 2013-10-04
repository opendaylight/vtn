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
    * @brief : default constructor
    */
    ControllerFramework::ControllerFramework()  {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
    }

    /**
     * @brief : default destructor
     */
    ControllerFramework::~ControllerFramework() {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
    }

    /**
    * AddController:
    * @brief     : Adding new controller with controller information in
    * a map
    * @param[in] : controller name , controller*, driver*
    * @retval    : None
    */
    void  ControllerFramework::AddController(
                  std::string& controller_name,
                  controller* controller_instance, driver* driver_instance) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

      ControllerContainer *controller_container = new ControllerContainer();
      controller_container->ctr = controller_instance;
      controller_container->drv = driver_instance;

      controller_list_rwlock_.wrlock();
      controller_list.insert(std::make_pair(controller_name,
                                            controller_container));
      pfc_log_info("controller added successfully");
      controller_list_rwlock_.unlock();
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
    }

    /**
    * UpdateControllerConfiguration:
    * @brief     : Updating controler as per controller name,if controllername
    * not found from list,it's consider as new controller and add it to
    * list
    * @param[in] : controller_name, controller*, driver*
    * @retval    : None
    */

    void ControllerFramework::UpdateControllerConfiguration(
                    std::string& controller_name,
                    controller* controller_instance, driver* driver_instance) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      //PFC_ASSERT(controller_connection != NULL);
      std::map<std::string, ControllerContainer*>::iterator
           controller_list_iterator = controller_list.begin();
      controller_list_rwlock_.wrlock();
      ControllerContainer* controller_container(NULL);
      controller_list_iterator = controller_list.find(controller_name);

      if (controller_list_iterator != controller_list.end()) {
        controller_container = controller_list_iterator->second;
        PFC_ASSERT(controller_container != NULL);

        delete controller_container->ctr;
        delete controller_container->drv;

        controller_list.erase(controller_list_iterator);

        controller_container->ctr = controller_instance;
        controller_container->drv = driver_instance;

        controller_list.insert(std::make_pair(controller_name,
                                              controller_container));
        pfc_log_info("Controller updated successfully");
      } else {
        ControllerContainer* new_controller_container = new ControllerContainer();
        pfc_log_trace("%s: Consider as a new controller to be add..",
                      PFC_FUNCNAME);
        new_controller_container->ctr = controller_instance;
        new_controller_container->drv = driver_instance;

        controller_list.insert(std::make_pair(controller_name,
                                              new_controller_container));
        pfc_log_info("Controller added successfully");
      }

      controller_list_rwlock_.unlock();
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
    }

    /**
    * RemoveControllerConfiguration:
    * @brief     : This function removes the existing controller from list
    * @param[in] : controller_name
    * @retval    : None
    */
    VtnDrvRetEnum
        ControllerFramework::RemoveControllerConfiguration(
                              std::string& controller_name) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      std::map<std::string, ControllerContainer*>::iterator
           controller_list_iterator = controller_list.begin();

      if (controller_list.size() == 0) {
        pfc_log_info("Controller name not found .List is Empty");
        return VTN_DRV_RET_FAILURE;
      }

      controller_list_rwlock_.wrlock();
      ControllerContainer* controller_container(NULL);
      controller_list_iterator = controller_list.find(controller_name);

      if (controller_list_iterator != controller_list.end()) {
        controller_container = controller_list_iterator->second;
        PFC_ASSERT(controller_container != NULL);

        delete controller_container->ctr;
        delete controller_container->drv;

        controller_list.erase(controller_list_iterator);
        pfc_log_info("Existing controller configuration gets removed");
      } else {
        pfc_log_debug("controller name not found");
        return VTN_DRV_RET_FAILURE;
      }

      controller_list_rwlock_.unlock();
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return VTN_DRV_RET_SUCCESS;
    }

    /**
    * GetControllerConnectionByName:
    * @brief     : This function gets the driver type for the appropriate
    * controllers
    * @param[in] : controller_name, controller*, driver*
    * @retval    : driver*
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
        pfc_log_info("controller name not found");
        return  VTN_DRV_RET_FAILURE;
      }

      pfc::core::ScopedMutex m(controller_list_iterator->second->rw_mutex);

      controller_container = controller_list_iterator->second;

      *controller_instance = controller_container->ctr; 
      *driver_instance = controller_container->drv;

      PFC_ASSERT(*controller_instance != NULL );
      PFC_ASSERT(*driver_instance != NULL );

      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
      return VTN_DRV_RET_SUCCESS;
    }

    void ControllerFramework::RegisterDriver(
                              unc_keytype_ctrtype_t controller_type,
                              driver* driver_instance)  {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
      PFC_ASSERT(driver_instance != NULL);

      controller_list_rwlock_.wrlock();
      driver_container.insert(std::make_pair(controller_type, driver_instance));
      controller_list_rwlock_.unlock();

      pfc_log_info("driver instance populated");
      pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
    }


    driver* ControllerFramework::GetDriverInstance(
        unc_keytype_ctrtype_t controller_type) {
      pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

      if (driver_container.size() == 0) {
        pfc_log_error("Driver list is empty");
        return NULL;
      }

      return driver_container.find(controller_type)->second;
    }


} //driver
} //unc
