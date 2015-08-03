/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include<tc_operations.hh>
namespace unc {
namespace tc {

/**
 *@brief    TcTaskqUtil constructor,create a new task queue.
 *@param[in]  concurrency  Number of simultanious tasks.
 */

TcTaskqUtil::TcTaskqUtil(uint32_t concurrency) {
  taskq_ = pfc::core::TaskQueue::create(concurrency);
  timed_ = pfc::core::Timer::create(taskq_->getId());
}

/**
 *@brief    TcTaskqUtil destructor.
 */

TcTaskqUtil::~TcTaskqUtil() {
  if (taskq_) {
    delete taskq_;
    taskq_ = NULL;
  }
  if (timed_) {
    delete timed_;
    timed_ = NULL;
  }
}

/**
 * @brief ReadParams Constructor
 * @param[IN] session_id to be released
 * @param[IN] tclock instance
 * @param[IN] Channel names map
 */
ReadParams::ReadParams(uint32_t session_id,
                       TcLock* tclock,
                       TcChannelNameMap& unc_map)
  : session_id_(session_id),
    tclock_(tclock),
    unc_channel_map_(unc_map) {}

/**
 *@brief    Handle read time out .
 *@param[in]   none.
 */
void ReadParams::HandleReadTimeout(void) {
  TcReadOperations tc_read_oper_(tclock_,
                                 NULL,
                                 NULL,
                                 unc_channel_map_,
                                 NULL);

  tc_read_oper_.session_id_ = session_id_;
  tc_read_oper_.tc_oper_ = TC_OP_READ_RELEASE;

  pfc_log_info("Read Timeout sess_id:%d", session_id_);
  if ( tc_read_oper_.Dispatch() != TC_OPER_SUCCESS )
    pfc_log_info("taskq read release failed");
}

/*
 * @brief AuditPrams Constructor
 *
 */
AuditParams::AuditParams(std::string controller_id,
                         TcDbHandler* db_handler,
                         TcLock* tclock,
                         TcChannelNameMap& unc_map,
                         unc_keytype_ctrtype_t driver_id)
  : controller_id_(controller_id),
    audit_db_hdlr_(db_handler),
    tclock_(tclock),
    unc_channel_map_(unc_map),
    driver_id_(driver_id) { }

/**
 *@brief    Handling of audit driver.
 *@param[in]   none.
 */
void AuditParams::HandleDriverAudit(void) {
  pfc_log_info("Audit Operation from taskq");
  TcAuditOperations tc_audit_oper_(tclock_,
                                   NULL,
                                   audit_db_hdlr_,
                                   unc_channel_map_,
                                   NULL);
  tc_audit_oper_.controller_id_ = controller_id_;
  tc_audit_oper_.tc_oper_ = TC_OP_DRIVER_AUDIT;
  tc_audit_oper_.driver_id_ = driver_id_;

  TcOperStatus ret = TC_OPER_SUCCESS;

  // If the audit is cancelled, call Dispatch again
  // The dispatch will wait in GetExclustion() till
  // the audit request can be processed.

  TcAuditOperations::PushToWaitQueue(&tc_audit_oper_);

  do {
    ret = tc_audit_oper_.Dispatch();
    pfc_log_info("%s Driver audit Dispatch ret=%d",
                 __FUNCTION__, ret);
  } while (ret == TC_OPER_AUDIT_CANCELLED);

  if (ret == TC_OPER_SUCCESS) {
    pfc_log_info("%s Driver audit (ctr_id=%s) completed successfully",
                 __FUNCTION__, tc_audit_oper_.controller_id_.c_str());
  } else {
     pfc_log_error("Driver Audit from taskq failed controller_id=%s",
                  tc_audit_oper_.controller_id_.c_str());
  }

  TcAuditOperations::PopWaitQueue();
}

/**
 *@brief  Create task queue.
 *@param[in]  session_id  Session Identifier.
 *@param[in] time_out     Read time out in seconds.
 */
int
TcTaskqUtil ::PostReadTimer(uint32_t session_id,
                            uint32_t time_out,
                            TcLock* tclock,
                            TcChannelNameMap& unc_map)  {
  int ret = 0;
  pfc_timeout_t  time_out_id;
  pfc_timespec_t  timeout;
  if (time_out == 0)  {
    return 0;
  } else  {
    ReadParams  func_obj(session_id,
                         tclock,
                         unc_map);
    pfc::core::timer_func_t timer_func(func_obj);

    timeout.tv_sec = time_out;
    timeout.tv_nsec = 0;
    ret = timed_->post(&timeout, timer_func, &time_out_id);
    if (ret != 0) {
       pfc_log_fatal("failed to post() for read operation");
    }
  }
  return ret;
}

/**
 *@brief  Dispatch the audit driver request.
 *@param[in]  controller_id  name of controller.
 */
int TcTaskqUtil::DispatchAuditDriverRequest(std::string controller_id,
                                            TcDbHandler* tc_db_hdlr,
                                            TcLock* tclock,
                                            TcChannelNameMap& unc_map,
                                            unc_keytype_ctrtype_t driver_id) {
  int ret = 0;
  AuditParams  func_obj(controller_id,
                        tc_db_hdlr,
                        tclock,
                        unc_map,
                        driver_id);
  pfc::core::taskq_func_t  task_func(func_obj);
  ret = taskq_->dispatch(task_func);
  if (ret != 0) {
     pfc_log_fatal("failed to dispatch() for audit driver operation");
  }
  return ret;
}
}  // namespace tc
}  // namespace unc
