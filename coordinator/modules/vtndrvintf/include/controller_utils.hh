/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <string>

#ifndef  __CONTROLLER_UTILS_HH__
#define  __CONTROLLER_UTILS_HH__

namespace unc {
namespace driver {

class ControllerFramework;
class controller;
class driver;

typedef enum {
  CONTROLLER_ADD = 0,
  CONTROLLER_UPDATE,
  CONTROLLER_DELETE,
  READ_FROM_CONTROLLER,
  WRITE_TO_CONTROLLER
} ControllerOps;

/**
 * @brief       - This class contains the operation count,
                  conditional variables and mutex lock
 */
class controller_exclusion {
 public:
  /**
   * @brief - constructor
   */
  controller_exclusion() {
    // Number of Read Operations on the Controller
    read_count_ = 0;

    // Only one write in the controller
    write_count_= 0;

    // Flag to denote write operation in progress
    write_in_progress_ = PFC_FALSE;

    // Number of waiters to write
    write_wait_count_= 0;

    // Number of waiters in read
    read_wait_count_= 0;

    //  Flag indicates Controller Marked for delete
    // to  fail read/write requests
    controller_marked_for_delete_ = PFC_FALSE;

    //  Flag indicates Controller Marked for update
    // to  fail read/write requests
    controller_marked_for_update_= PFC_FALSE;
  }

  /**
   * @brief     - Retrieves the read operation count
   * @param[in] - NA
   * @retval    - uint32_t (read count)
   */
  uint32_t get_read_count() {
    return  read_count_;
  }

  /**
   * @brief     - Set the read operation count
   * @param[in] - uint32_t (read count)
   * @retval    - NA
   */
  void set_read_count(uint32_t read_val) {
    read_count_ = read_val;
  }

  /**
   * @brief     - Increments the read operation count
   * @param[in] - NA
   * @retval    - NA
   */
  void  increment_read_count() {
    read_count_++;
  }

  /**
   * @brief     - Decrements the read operation count
   * @param[in] - NA
   * @retval    - NA
   */
  void  decrement_read_count() {
    read_count_--;
  }

  /**
   * @brief     - Retrieves the write operation count
   * @param[in] - NA
   * @retval    - uint32_t (write count)
   */
  uint32_t get_write_count() {
    return write_count_;
  }

  /**
   * @brief     - Set the write operation count
   * @param[in] - uint32_t (write count)
   * @retval    - NA
   */
  void set_write_count(uint32_t write_val) {
    write_count_ = write_val;
  }

  /**
   * @brief     - Checks if controller delete is allowed
   * @param[in] - NA
   * @retval    - PFC_TRUE/PFC_FALSE
   */
  pfc_bool_t  can_delete_update() {
    if ( read_count_ == 0   &&
        write_count_ == 0 ) {
      return PFC_TRUE;
    }
    return PFC_FALSE;
  }

  /**
   * @brief     - Set write in progress as true
   * @param[in] - NA
   * @retval    - NA
   */
  void set_write_in_progress() {
    write_in_progress_ = PFC_TRUE;
  }

  /**
   * @brief     - Set write in progress as false
   * @param[in] - NA
   * @retval    - NA
   */

  void write_completed() {
    write_in_progress_ = PFC_FALSE;
  }

  /**
   * @brief     - get write wait count
   * @param[in] - NA
   * @retval    - NA
   */
  uint32_t get_write_wait_count() {
    return write_wait_count_;
  }

  /**
   * @brief     - increment write wait count
   * @param[in] - NA
   * @retval    - NA
   */

  void  increment_write_wait_count() {
    write_wait_count_++;
  }

  /**
   * @brief     - decrement write wait count
   * @param[in] - NA
   * @retval    - NA
   */

  void  decrement_write_wait_count() {
    write_wait_count_--;
  }

  /**
   * @brief     - get read wait count
   * @param[in] - NA
   * @retval    - uint32_t (read count)
   */
  uint32_t get_read_wait_count() {
    return read_wait_count_;
  }

  /**
   * @brief     - increment read wait count
   * @param[in] - NA
   * @retval    - NA
   */

  void  increment_read_wait_count() {
    read_wait_count_++;
  }


  /**
   * @brief     - decrement read wait count
   * @param[in] - NA
   * @retval    - NA
   */
  void  decrement_read_wait_count() {
    read_wait_count_--;
  }

  /**
   * @brief     - mark controller for delete as true
   * @param[in] - NA
   * @retval    - NA
   */
  void mark_controller_for_delete() {
    controller_marked_for_delete_ = PFC_TRUE;
  }

  /**
   * @brief     - checks controller delete if mark controller for delete
   * @param[in] - NA
   * @retval    - PFC_TRUE/PFC_FALSE
   */
  pfc_bool_t can_delete_controller_marked_for_delete() {
    if (controller_marked_for_delete_ == PFC_TRUE) {
      return PFC_TRUE;
    }

    return PFC_FALSE;
  }

  /**
   * @brief     - mark controller for update as true
   * @param[in] - NA
   * @retval    - NA
   */
  void mark_controller_for_update() {
    controller_marked_for_update_ = PFC_TRUE;
  }

  /**
   * @brief     - get controller marked for update
   * @param[in] - NA
   * @retval    - PFC_TRUE/PFC_FALSE
   */
  pfc_bool_t get_controller_marked_for_update() {
    return controller_marked_for_update_;
  }

  /**
   * @brief     - set controller marked for update
   * @param[in] - pfc_bool_t
   * @retval    - NA
   */
  void set_controller_marked_for_update(pfc_bool_t set_val) {
    controller_marked_for_update_ = set_val;
  }

  /**
   * @brief     - wait for write
   * @param[in] - Mutex
   * @retval    - NA
   */
  void wait_for_write(pfc::core::Mutex& wait_mutex) {
    parellel_write_wait_cond_.wait(wait_mutex);
  }

  /**
   * @brief     - release write wait lock
   * @param[in] - NA
   * @retval    - NA
   */

  void  release_write_wait() {
    parellel_write_wait_cond_.broadcast();
  }

  /**
   * @brief     - wait until update finish
   * @param[in] - Mutex
   * @retval    - NA
   */
  void wait_until_update(pfc::core::Mutex& update_mutex) {
    update_in_progress_cond_.wait(update_mutex);
  }

  /**
   * @brief     - update completed and broadcast
   * @param[in] - NA
   * @retval    - NA
   */
  void update_completed() {
    update_in_progress_cond_.broadcast();
  }

  /**
   * @brief     - delete wait
   * @param[in] - Mutex
   * @retval    - NA
   */

  void  delete_wait(pfc::core::Mutex& delete_mutex) {
    delete_waiting_cond_.wait(delete_mutex);
  }

  /**
   * @brief     - delete allow
   * @param[in] - NA
   * @retval    - NA
   */
  void  delete_allow() {
    delete_waiting_cond_.broadcast();
  }

 private:
  // Number of Read Operations on the Controller
  uint32_t read_count_;
  // Only one write in the controller
  uint32_t write_count_;

  // Flag to denote write operation in progress
  pfc_bool_t  write_in_progress_;
  // Number of waiters to write
  uint32_t  write_wait_count_;
  // Number of waiters in read
  uint32_t  read_wait_count_;
  // Flag indicates Controller Marked for delete
  // to  fail read/write requests
  pfc_bool_t controller_marked_for_delete_;
  // Flag indicates Controller Marked for update
  // to  fail read/write requests
  pfc_bool_t controller_marked_for_update_;


  // Parellel writes will wait on this condition
  pfc::core::Condition parellel_write_wait_cond_;
  // read/write will wait on this condition until update
  //  relinquishes control
  pfc::core::Condition update_in_progress_cond_;
  // delete requests will wait on this condition
  //  until READ/WRITE relinquishes control
  pfc::core::Condition delete_waiting_cond_;
};

/**
 * @brief       - This class contains ControllerFramework , controller, driver.
 conditional variables and mutex lock
 */

class controller_operation {
 public:
  /**
   * @brief     - constructor
   * @param[in] - ControllerFramework*, ControllerOps, std::string,
   unc_keytype_ctrtype_t
   */
  controller_operation(ControllerFramework* fw_ptr,
                       ControllerOps operation,
                       std::string ctl_id,
                       unc_keytype_ctrtype_t ctl_type);

  /**
   * @brief     - constructor
   * @param[in] - ControllerFramework*, ControllerOps, std::string,
   unc_keytype_ctrtype_t, std::string
   */
  controller_operation(ControllerFramework* fw_ptr,
                       ControllerOps operation,
                       std::string ctl_id);
  /**
   * @brief     - destructor
   */
  ~controller_operation();

  /**
   * @brief     - get controller Status, PFC_FALSE if controller not exist in
   *              list or PFC_TRUE if controller exist in list.
   * @param[in] - NA
   * @retval    - returns PFC_FALSE/PFC_TRUE(pfc_bool_t)
   */
  pfc_bool_t get_controller_status() {
    return controller_status_;
  }

  /**
   * @brief     - set controller PFC_FALSE if controller not exist in list or
   *              set PFC_TRUE if controller exist in list
   * @param[in] - PFC_FALSE/PFC_TRUE(pfc_bool_t)
   * @retval    - void
   */
  void set_controller_status(pfc_bool_t status) {
    controller_status_ = status;
  }

  /**
   * @brief     - get controller
   * @param[in] - NA
   * @retval    - controller pointer
   */
  controller* get_controller_handle() {
    return ctr_;
  }

  /**
   * @brief     - get driver
   * @param[in] - NA
   * @retval    - driver pointer
   */
  unc::driver::driver* get_driver_handle() {
    return drv_;
  }

 private:
  ControllerFramework* ctl_fw_;
  ControllerOps ctl_oper_;
  controller* ctr_;
  driver* drv_;
  pfc_bool_t controller_status_;
};

}  //  namespace driver
}  //  namespace unc
#endif
