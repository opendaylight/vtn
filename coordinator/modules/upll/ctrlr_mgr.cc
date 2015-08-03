/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <stdlib.h>
#include <stdint.h>
#include <list>
#include <sstream>

#include "cxx/pfcxx/synch.hh"
#include "uncxx/upll_log.hh"
#include "config_mgr.hh"
#include "ctrlr_mgr.hh"

pfc::core::Mutex ctrlr_mutex_;

namespace unc {
namespace upll {
namespace config_momgr {

CtrlrMgr *CtrlrMgr::singleton_instance_;

/**
 * @brief Add new controller
 *
 * @param ctrlr[in]      Controller instance
 * @param datatype[in]   Datatype
 *
 * @retval  UPLL_RC_SUCCESS, if controller added
 * @retval  UPLL_RC_ERR_INSTANCE_EXISTS, if entry already exists
 * @retval  UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::Add(const Ctrlr &ctrlr,
                        const upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  if (datatype != UPLL_DT_CANDIDATE && datatype != UPLL_DT_RUNNING) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  Ctrlr *pctrlr = new Ctrlr(ctrlr, datatype);

  /* Adding the entry to list */
  if (ctrlrs_.empty()) {
    UPLL_LOG_INFO("Controller List is empty. Inserting first ctrlr");
    ctrlrs_.push_back(pctrlr);
  } else {
    bool inserted = false;
    for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
         it != ctrlrs_.end(); ++it) {
      if ((*it)->name_.compare(pctrlr->name_) == 0 &&
          (*it)->datatype_ == pctrlr->datatype_) {
        UPLL_LOG_DEBUG("Ctrlr(%s) Already exists in datatype(%d)",
                       pctrlr->name_.c_str(), datatype);
        delete pctrlr;
        return UPLL_RC_ERR_INSTANCE_EXISTS;
      } else if ((*it)->name_.compare(pctrlr->name_) < 0) {
        continue;
      } else if ((*it)->name_.compare(pctrlr->name_) > 0) {
        ctrlrs_.insert(it, pctrlr);
        inserted = true;
        break;
      }
    }
    if (!inserted) {
      // Inserting ctrlr entry to the last
      ctrlrs_.push_back(pctrlr);
    }
  }

  UPLL_LOG_INFO("Added new controller(%s, %d, %s, %d, %d, %d) to datatype(%d)",
                pctrlr->name_.c_str(),
                pctrlr->type_, pctrlr->version_.c_str(),
                pctrlr->audit_done_, pctrlr->config_done_,
                pctrlr->invalid_config_,
                datatype);
  return UPLL_RC_SUCCESS;
}

/**
 * @brief Delete controller in both candidate and running
 *
 * @param ctrlr_name[in]      Controller name
 *
 * @retval  UPLL_RC_SUCCESS, if controller deleted
 * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE, if entry not found
 * @retval  UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::Delete(const std::string &ctrlr_name,
                           const upll_keytype_datatype_t datatype) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  if (datatype != UPLL_DT_CANDIDATE && datatype != UPLL_DT_RUNNING) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  /* Deleting in the map */
  bool deleted = false;
  bool invalid_config = false;
  Ctrlr *ctrlr = NULL;
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(ctrlr_name) == 0 &&
      (*it)->datatype_ == datatype) {
      ctrlr = *it;
      invalid_config = ctrlr->invalid_config_;
      ctrlrs_.erase(it);
      delete ctrlr;
      deleted = true;
      break;
    }
    continue;
  }
  if (deleted) {
    UPLL_LOG_INFO("Deleted controller(%s) from datatype(%d)",
                  ctrlr_name.c_str(), datatype);
    if (invalid_config && datatype ==  UPLL_DT_RUNNING) {
      // clear invalid config alarm
      UpllConfigMgr::GetUpllConfigMgr()->SendInvalidConfigAlarm(ctrlr_name,
                                                                false);
    }
  } else {
    UPLL_LOG_ERROR("controller(%s) not found in datatype(%d)",
                   ctrlr_name.c_str(), datatype);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return UPLL_RC_SUCCESS;
}

/**
 * @brief Update controller version
 *
 * @param ctrlr_name[in]      Controller name
 * @param ctrlr_version[in]   Controller version
 *
 * @retval  UPLL_RC_SUCCESS, if controller updated
 * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE, if entry not found
 * @retval  UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::UpdateVersion(const std::string &ctrlr_name,
                                  const upll_keytype_datatype_t datatype,
                                  const std::string &ctrlr_version) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  if (datatype != UPLL_DT_CANDIDATE && datatype != UPLL_DT_RUNNING) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  /* Updating the entry in map */
  bool updated = false;
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(ctrlr_name) == 0 &&
        (*it)->datatype_ == datatype) {
      (*it)->version_ = ctrlr_version;
      updated = true;
      break;
    }
    continue;
  }
  if (!updated) {
    UPLL_LOG_ERROR("controller(%s) not found in datatype(%d)",
                   ctrlr_name.c_str(), datatype);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  UPLL_LOG_INFO("Updated controller(%s) with version(%s) in datatype(%d)",
                ctrlr_name.c_str(), ctrlr_version.c_str(), datatype);
  return UPLL_RC_SUCCESS;
}

/**
 * @brief Update audit_done flag in Running
 *
 * @param ctrlr_name[in]      Controller name
 * @param audit_done[in]      Audit Done flag
 *
 * @retval  UPLL_RC_SUCCESS, if controller updated
 * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE, if entry not found
 * @retval  UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::UpdateAuditDone(const std::string &ctrlr_name,
                                    const bool audit_done) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  /* Updating the entry in map */
  bool updated = false;
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(ctrlr_name) == 0 &&
        (*it)->datatype_ == UPLL_DT_RUNNING) {
      (*it)->audit_done_ = audit_done;
      updated = true;
      break;
    }
    continue;
  }
  if (!updated) {
    UPLL_LOG_ERROR("controller(%s) not found",
                   ctrlr_name.c_str());
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  UPLL_LOG_INFO("Updated controller(%s) with audit_done(%d)",
                ctrlr_name.c_str(), audit_done);
  return UPLL_RC_SUCCESS;
}

/**
 * @brief Update config_done flag in Running
 *
 * @param ctrlr_name[in]      Controller name
 * @param config_done[in]     Config Done flag
 *
 * @retval  UPLL_RC_SUCCESS, if controller updated
 * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE, if entry not found
 * @retval  UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::UpdateConfigDone(const std::string &ctrlr_name,
                                     const bool config_done) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  /* Updating the entry in map */
  bool updated = false;
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(ctrlr_name) == 0 &&
        (*it)->datatype_ == UPLL_DT_RUNNING) {
      (*it)->config_done_ = config_done;
      updated = true;
      break;
    }
    continue;
  }
  if (!updated) {
    UPLL_LOG_ERROR("controller(%s) not found",
                   ctrlr_name.c_str());
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  UPLL_LOG_INFO("Updated controller(%s) with config_done(%d)",
                ctrlr_name.c_str(), config_done);
  return UPLL_RC_SUCCESS;
}

/**
 * @brief Update invalid_config flag in Running
 *
 * @param ctrlr_name[in]      Controller name
 * @param invalid_config[in]  Invalid Config flag
 *
 * @retval  UPLL_RC_SUCCESS, if controller updated
 * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE, if entry not found
 * @retval  UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::UpdateInvalidConfig(const std::string &ctrlr_name,
                                        const bool invalid_config) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  /* Updating the entry in list */
  bool updated = false;
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(ctrlr_name) == 0 &&
        (*it)->datatype_ == UPLL_DT_RUNNING) {
      (*it)->invalid_config_ = invalid_config;
      updated = true;
      break;
    }
    continue;
  }
  if (!updated) {
    UPLL_LOG_ERROR("controller(%s) not found",
                   ctrlr_name.c_str());
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  UPLL_LOG_INFO("Updated controller(%s) with invalid_config(%d)",
                ctrlr_name.c_str(), invalid_config);
  return UPLL_RC_SUCCESS;
}

/**
 * @brief Get controller type from Running
 *
 * @param ctrlr_name[in]      Controller name
 * @param ctrlr_type[out]     Controller type, if found
 *
 * @return  true if controller found, false otherwise
 */
bool CtrlrMgr::GetCtrlrType(const char *ctrlr_name,
                            const upll_keytype_datatype_t datatype,
                            unc_keytype_ctrtype_t *ctrlr_type) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  UPLL_FUNC_TRACE;
  upll_keytype_datatype_t mapped_dt;
  mapped_dt = MapDataType(datatype);
  if (ctrlr_name == NULL || ctrlr_type == NULL) {
    UPLL_LOG_DEBUG("Null argument name:%p type:%p", ctrlr_name, ctrlr_type);
    return false;
  }
  std::string name(ctrlr_name);
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(name) == 0 &&
        (*it)->datatype_ == mapped_dt) {
      *ctrlr_type = (*it)->type_;
      UPLL_LOG_TRACE("Ctrlr name(%s), type(%d) in datatype(%d)", ctrlr_name,
                     *ctrlr_type, datatype);
      return true;
    }
  }
  UPLL_LOG_DEBUG("Ctrlr name(%s), not found in datatype(%d)", ctrlr_name,
                 datatype);
  return false;
}

/**
 * @brief Get controller type and its version from Running
 *
 * @param ctrlr_name[in]      Controller name
 * @param ctrlr_type[out]     Controller type
 * @param ctrlr_version[out]  Controller version, if found
 *
 * @return  true if controller found, false otherwise
 */
bool CtrlrMgr::GetCtrlrTypeAndVersion(const char *ctrlr_name,
                                      const upll_keytype_datatype_t datatype,
                                      unc_keytype_ctrtype_t *ctrlr_type,
                                      std::string *ctrlr_version) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  UPLL_FUNC_TRACE;
  upll_keytype_datatype_t mapped_dt;
  mapped_dt = MapDataType(datatype);
  if (ctrlr_name == NULL || ctrlr_type == NULL || ctrlr_version == NULL) {
    UPLL_LOG_DEBUG("Null argument name:%p type:%p version:%p",
                   ctrlr_name, ctrlr_type, ctrlr_version);
    return false;
  }
  std::string name(ctrlr_name);
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(name) == 0 &&
        (*it)->datatype_ == mapped_dt) {
      *ctrlr_type = (*it)->type_;
      *ctrlr_version = (*it)->version_;
      UPLL_LOG_TRACE("Ctrlr name(%s), type(%d), version(%s) in datatype(%d)",
                     ctrlr_name, *ctrlr_type, ctrlr_version->c_str(), datatype);
      return true;
    }
  }
  UPLL_LOG_DEBUG("Ctrlr name(%s), not found in datatype(%d)", ctrlr_name,
                 datatype);
  return false;
}

/**
 * @brief Get configuration state for the controller from Running
 *
 * @param ctrlr_name[in]  Controller Name
 *
 * @return
 */
upll_rc_t CtrlrMgr::IsConfigInvalid(const char *ctrlr_name,
                                    bool *config_invalid) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  UPLL_FUNC_TRACE;
  if (ctrlr_name == NULL || config_invalid == NULL) {
    UPLL_LOG_DEBUG("Null argument ctrlr_name/invalid");
    return UPLL_RC_ERR_GENERIC;
  }
  std::string name(ctrlr_name);
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(name) == 0 &&
        (*it)->datatype_ == UPLL_DT_RUNNING) {
      *config_invalid = (*it)->invalid_config_;
      UPLL_LOG_TRACE("Ctrlr name(%s), invalid_config(%d) in Running",
                     ctrlr_name, *config_invalid);
      return UPLL_RC_SUCCESS;
    }
  }
  UPLL_LOG_DEBUG("Ctrlr name(%s), not found in Running", ctrlr_name);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

/**
 * @brief Get configuration is done for the controller from Running
 *
 * @param ctrlr_name[in]  Controller Name
 *
 * @return
 *
 */
upll_rc_t CtrlrMgr::IsConfigDone(const char *ctrlr_name, bool *config_done) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  UPLL_FUNC_TRACE;
  if (ctrlr_name == NULL || config_done == NULL) {
    UPLL_LOG_DEBUG("Null argument ctrlr_name/config_done");
    return UPLL_RC_ERR_GENERIC;
  }
  std::string name(ctrlr_name);
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(name) == 0 &&
        (*it)->datatype_ == UPLL_DT_RUNNING) {
      *config_done = (*it)->config_done_;
      UPLL_LOG_TRACE("Ctrlr name(%s), config_done(%d) in Running",
                     ctrlr_name, *config_done);
      return UPLL_RC_SUCCESS;
    }
  }
  UPLL_LOG_DEBUG("Ctrlr name(%s), not found in Running", ctrlr_name);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

/**
 * @brief Get audit is done for the controller from Running
 *
 * @param ctrlr_name[in]  Controller Name
 *
 * @return  Returns false only  if controller is found and its audit
 *          is not done
 */
upll_rc_t CtrlrMgr::IsAuditDone(const char *ctrlr_name, bool *audit_done) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  UPLL_FUNC_TRACE;
  if (ctrlr_name == NULL) {
    UPLL_LOG_DEBUG("Null argument ctrlr_name/audit_done");
    return UPLL_RC_ERR_GENERIC;
  }
  std::string name(ctrlr_name);
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(name) == 0 &&
        (*it)->datatype_ == UPLL_DT_RUNNING) {
      *audit_done = (*it)->audit_done_;
      UPLL_LOG_TRACE("Ctrlr name(%s), audit_done(%d) in Running",
                     ctrlr_name, *audit_done);
      return UPLL_RC_SUCCESS;
    }
  }
  UPLL_LOG_DEBUG("Ctrlr name(%s), not found in Running", ctrlr_name);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

/**
 * @brief Get Name of the first controller in the list
 *
 * @param[in]  datatype      Datatype in which to be read
 * @param[out] first_name    Name of the first controller
 *
 * @return     UPLL_RC_SUCCES, when name is read from the list
 *             UPLL_RC_ERR_NO_SUCH_INSTANCE when there are no entries
 *             UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::GetFirstCtrlrName(
    const upll_keytype_datatype_t datatype, std::string *first_name) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  upll_keytype_datatype_t mapped_dt;
  mapped_dt = MapDataType(datatype);
  if (first_name == NULL) {
    UPLL_LOG_DEBUG("Null argument first_name");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ctrlrs_.empty()) {
    UPLL_LOG_DEBUG("Controller List is Empty");
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->datatype_ == mapped_dt) {
      UPLL_LOG_TRACE("First Ctrlr name is (%s) in datatype(%d)",
                     (*it)->name_.c_str(), datatype);
      *first_name = (*it)->name_;
      return UPLL_RC_SUCCESS;
    } else {
      continue;
    }
  }
  UPLL_LOG_TRACE("No Ctrlr name in datatype(%d)", datatype);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

/**
 * @brief Get Name of the next controller in the list
 *
 * @param[in]  name          Name of the input controller
 * @param[in]  datatype      Datatype in which to be read
 * @param[out] next_name     Name of the next controller
 *
 * @return     UPLL_RC_SUCCES, when name is read from the list
 *             UPLL_RC_ERR_NO_SUCH_INSTANCE when there is no matching entry
 *             UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::GetNextCtrlrName(
    const std::string in_name, const upll_keytype_datatype_t datatype,
    std::string *next_name) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  UPLL_LOG_TRACE("Input Ctrlr (%s) next Ctrlr (%s) ",
                     in_name.c_str(), (*next_name).c_str());
  upll_keytype_datatype_t mapped_dt;
  mapped_dt = MapDataType(datatype);
  if (next_name == NULL) {
    UPLL_LOG_DEBUG("Null argument next_name");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ctrlrs_.empty()) {
    UPLL_LOG_DEBUG("Controller List is Empty");
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->datatype_ != mapped_dt)
      continue;
    if ((*it)->name_.compare(in_name) > 0) {
      UPLL_LOG_DEBUG("ctlr in list (%s) and input ctrlr (%s) are same",
       (*it)->name_.c_str(), in_name.c_str());
      *next_name = (*it)->name_;
      UPLL_LOG_DEBUG("next_name Ctrlr (%s)", (*next_name).c_str());
      return UPLL_RC_SUCCESS;
    }
  }
  UPLL_LOG_DEBUG("No Ctrlr is next to (%s) in datatype(%d)", in_name.c_str(),
                 datatype);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

/**
 * @brief Get controller type from Running
 *
 * @param in_dt[in]      Input Datatype
 *
 * @return  Corresponding output datatype
 */
upll_keytype_datatype_t CtrlrMgr::MapDataType(
    const upll_keytype_datatype_t in_dt) {
  switch (in_dt) {
    case UPLL_DT_STATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_AUDIT:
      return UPLL_DT_RUNNING;
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_IMPORT:
      return UPLL_DT_CANDIDATE;
    case UPLL_DT_STARTUP:
      return UPLL_DT_STARTUP;
    default:
      return UPLL_DT_INVALID;
  }
}

/**
 * @brief Clears controllers in the list
 *
 * @param  none
 *
 * @return none
 */
void CtrlrMgr::CleanUp() {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  UPLL_FUNC_TRACE;
  Ctrlr *pctrlr;
  // Delete each element from the list
  while (!ctrlrs_.empty()) {
    pctrlr = ctrlrs_.back();
    ctrlrs_.pop_back();
    delete pctrlr;
  }
}

/**
 * @brief Prints details of all controllers in the list
 *
 * @param  none
 *
 * @return none
 */
void CtrlrMgr::PrintCtrlrList() {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  std::stringstream ss;
  if (ctrlrs_.empty()) {
    UPLL_LOG_DEBUG("Controller List is Empty");
    return;
  }
  ss << "\n***********************************************************"
     << "name, type, version, audit_done, config_done, invalid_config,"
     << " datatype\n"
     << "\n***********************************************************";
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    ss << (*it)->name_.c_str() << ", " << (*it)->type_ << ", "
       << (*it)->version_.c_str() << ", " << (*it)->audit_done_ << ", "
       << (*it)->config_done_ << ", " << (*it)->invalid_config_ << ", "
       << (*it)->datatype_ << "\n";
  }
  UPLL_LOG_DEBUG("\n%s", ss.str().c_str());
  ss << "\n***********************************************************";
  return;
}

/**
 * @brief Check controller audit type
 *
 * @param ctrlr[in]       Controller instance
 * @param datatype[in]    Datatype
 * @param audit_type[out] Audit type
 *
 * @retval  UPLL_RC_SUCCESS, if controller added
 * @retval  UPLL_RC_ERR_INSTANCE_EXISTS, if entry already exists
 * @retval  UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::GetAuditType(const std::string &ctrlr_name,
                        const upll_keytype_datatype_t datatype,
                        bool *audit_type) {
  UPLL_FUNC_TRACE;
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  if (datatype != UPLL_DT_CANDIDATE && datatype != UPLL_DT_RUNNING
      && UPLL_DT_IMPORT != datatype) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  bool get_audit_type_ = false;
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(ctrlr_name) == 0 &&
        (*it)->datatype_ == datatype) {
      *audit_type = (*it)->audit_type_;
      get_audit_type_ = true;
      break;
    }
    continue;
  }
  if (!get_audit_type_) {
    UPLL_LOG_ERROR("controller(%s) not found in datatype(%d)",
                   ctrlr_name.c_str(), datatype);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  UPLL_LOG_INFO("Updated controller(%s) with audit type(%d) in datatype(%d)",
                ctrlr_name.c_str(), *audit_type, datatype);
  return UPLL_RC_SUCCESS;
}

/**
 * @brief Update controller's audit type
 *
 * @param ctrlr[in]       Controller instance
 * @param datatype[in]    Datatype
 * @param audit_type[in] Audit type
 *
 * @retval  UPLL_RC_SUCCESS, if controller added
 * @retval  UPLL_RC_ERR_INSTANCE_EXISTS, if entry already exists
 * @retval  UPLL_RC_ERR_GENERIC, otherwise
 */
upll_rc_t CtrlrMgr::UpdateAuditType(const std::string &ctrlr_name,
                                  const upll_keytype_datatype_t datatype,
                                  bool audit_type) {
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  if (datatype != UPLL_DT_CANDIDATE && datatype != UPLL_DT_RUNNING) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  /* Updating the entry in map */
  bool updated = false;
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(ctrlr_name) == 0 &&
        (*it)->datatype_ == datatype) {
      (*it)->audit_type_ = audit_type;
      updated = true;
      break;
    }
    continue;
  }
  if (!updated) {
    UPLL_LOG_ERROR("controller(%s) not found in datatype(%d)",
                   ctrlr_name.c_str(), datatype);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  UPLL_LOG_INFO("Updated controller(%s) with audittype(%d) in datatype(%d)",
                ctrlr_name.c_str(), audit_type, datatype);
  return UPLL_RC_SUCCESS;
}

/**
 * @brief Get list of controllers which have invalidConfig
 *
 * @param invalidConfigCtr[in/out]  Controller Name list
 */
void CtrlrMgr::GetInvalidConfigList(std::list<std::string>
                                     &invalidConfigCtr) {
  UPLL_FUNC_TRACE;
  pfc::core::ScopedMutex lock(ctrlr_mutex_);
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->datatype_ == UPLL_DT_RUNNING && (*it)->invalid_config_) {
      invalidConfigCtr.push_back((*it)->name_);
      UPLL_LOG_TRACE("Ctrlr name(%s), invalid_config(%d) in Running",
                     (*it)->name_.c_str(), (*it)->invalid_config_);
    }
  }
}

bool CtrlrMgr::GetPathFaultDomains(const char *ctr_na,
                                   set<string>* domain_names) {
  UPLL_FUNC_TRACE;
  if (ctr_na == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p",
                   ctr_na);
    return false;
  }
  std::string ctrlr_id(ctr_na);

  std::map<std::string, std::map<std::string, uint32_t> >::iterator it =
                                 path_fault_map_.find(ctrlr_id);
  if (it != path_fault_map_.end()) {
    std::map<std::string, uint32_t>::iterator
                  start_it =  path_fault_map_[ctrlr_id].begin(),
                  dom_it  =  path_fault_map_[ctrlr_id].end();
    for (; start_it != dom_it; ++start_it) {
       domain_names->insert(start_it->first);
    }
  } else {
    return false;
  }
  return true;
}

bool CtrlrMgr::UpdatePathFault(const char *ctr_na,
                               const char *domain_na, bool assert_alarm) {
  UPLL_FUNC_TRACE;
  bool update_operstatus(false);

  if (ctr_na == NULL || domain_na == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p domain_name %p",
                   ctr_na, domain_na);
    return false;
  }
  std::string ctr_name(ctr_na);
  std::string domain_name(domain_na);

  path_fault_lock_.wrlock();
  // path fault asserted
  if (assert_alarm) {
    if (path_fault_map_.end() == path_fault_map_.find(ctr_name)) {
      // path fault occurred first time on a ctr domain
      // initialize fault count to 1
      UPLL_LOG_INFO("first path fault occurred on ctrlr %s:%s",
                    ctr_name.c_str(), domain_name.c_str() );
      const uint32_t fault_count(1);
      // create map for domain  name with fault count
      std::map<std::string, uint32_t> domain_map;
      domain_map[domain_name] = fault_count;
      // update path fault map
      path_fault_map_[ctr_name] = domain_map;
      update_operstatus = true;
    } else if (path_fault_map_[ctr_name].end() ==
        path_fault_map_[ctr_name].find(domain_name)) {
      // path fault  occurred on a new domain
      // initialize fault count to 1
      UPLL_LOG_INFO("first path fault occurred on domain %s:%s",
                    ctr_name.c_str(), domain_name.c_str() );
      const uint32_t fault_count(1);
      path_fault_map_[ctr_name][domain_name] = fault_count;
      update_operstatus = true;
    } else {
      // path fault already occurred on this controller domain
      ++path_fault_map_[ctr_name][domain_name];
      UPLL_LOG_INFO("path fault already occurred for %s:%s fault count:%d",
                     ctr_name.c_str(), domain_name.c_str(),
                     path_fault_map_[ctr_name][domain_name]);
    }
  } else {
    if (path_fault_map_.end() != path_fault_map_.find(ctr_name) &&
        path_fault_map_[ctr_name].end() !=
                path_fault_map_[ctr_name].find(domain_name)) {
      // path fault recovered on this controller domain
      --path_fault_map_[ctr_name][domain_name];
      UPLL_LOG_INFO("path fault recovery for %s:%s fault count:%d",
                    ctr_name.c_str(), domain_name.c_str(),
                    path_fault_map_[ctr_name][domain_name]);
      if (path_fault_map_[ctr_name][domain_name] == 0) {
        path_fault_map_[ctr_name].erase(domain_name);
        if (path_fault_map_[ctr_name].empty()) {
          path_fault_map_.erase(ctr_name);
        }
        update_operstatus = true;
      }
    }
  }
  path_fault_lock_.unlock();
  return update_operstatus;
}

/**
 * @brief : This function returns true if path fault occurred
 */
bool CtrlrMgr::IsPathFaultOccured(const char *ctr_na, const char *domain_na) {
  UPLL_FUNC_TRACE;
  bool bOccurence(false);

  if (ctr_na == NULL || domain_na == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p domain_name %p",
                   ctr_na, domain_na);
    return false;
  }
  std::string ctr_name(ctr_na);
  std::string domain_name(domain_na);

  path_fault_lock_.rdlock();
  if (!path_fault_map_.empty()) {
    if (ctr_name == "*") {
      bOccurence = true;
    } else if (path_fault_map_.end() != path_fault_map_.find(ctr_name)) {
      if (domain_name == "*") {
        bOccurence = true;
      } else if (path_fault_map_[ctr_name].end() !=
                 path_fault_map_[ctr_name].find(domain_name)) {
        bOccurence = true;
        UPLL_LOG_TRACE("path fault on %s:%s fault count:%d",
                       ctr_name.c_str(), domain_name.c_str(),
                       path_fault_map_[ctr_name][domain_name]);
      }
    }
  }
  path_fault_lock_.unlock();

  UPLL_LOG_TRACE("path fault for %s:%s asserted:%d",
                 ctr_name.c_str(), domain_name.c_str(), bOccurence);
  return bOccurence;
}

/**
 * @brief : This function clears path fault count
 * To clear for all entries pass ctr_name and domain_name as "*"
 * To clear for a controller, pass ctr_name and * as domain name
 * To clear for a controller domain, pass valid ctr_name and domain_name
 */
void CtrlrMgr::ClearPathfault(const char *ctr_na, const char *domain_na) {
  UPLL_FUNC_TRACE;
  if (ctr_na == NULL || domain_na == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p domain_name %p",
                   ctr_na, domain_na);
    return;
  }
  std::string ctr_name(ctr_na);
  std::string domain_name(domain_na);

  path_fault_lock_.wrlock();
  if (ctr_name == "*") {
    path_fault_map_.clear();
  } else {
    if (path_fault_map_.end() != path_fault_map_.find(ctr_name)) {
      if (domain_name == "*") {
        path_fault_map_.erase(ctr_name);
      } else if (path_fault_map_[ctr_name].end() !=
          path_fault_map_[ctr_name].find(domain_name)) {
        path_fault_map_[ctr_name].erase(domain_name);
        if (path_fault_map_[ctr_name].empty()) {
          path_fault_map_.erase(ctr_name);
        }
      }
    }
  }
  path_fault_lock_.unlock();
}

bool CtrlrMgr::IsCtrDisconnected(const char *ctr_name) {
  UPLL_FUNC_TRACE;
  bool bDisconnect(false);

  if (ctr_name == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p", ctr_name);
    return false;
  }
  std::string ctr(ctr_name);

  ctr_discon_lock_.rdlock();
  if (ctr_discon_map_.end() != ctr_discon_map_.find(ctr)) {
    bDisconnect = true;
  }
  ctr_discon_lock_.unlock();
  return bDisconnect;
}

bool CtrlrMgr::GetLogicalPortSt(const char *ctr_na, const char *logical_port,
                                uint8_t &state) {
  UPLL_FUNC_TRACE;
  bool bFound(false);

  if (ctr_na == NULL || logical_port == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p logical_port %p",
                   ctr_na, logical_port);
    return false;
  }

  std::string ctr_name(ctr_na);
  std::string logical_port_id(logical_port);

  ctr_discon_lock_.rdlock();
  if (ctr_discon_map_.end() != ctr_discon_map_.find(ctr_name) &&
      ctr_discon_map_[ctr_name].end() !=
          ctr_discon_map_[ctr_name].find(logical_port_id)) {
    state = ctr_discon_map_[ctr_name][logical_port_id];
    bFound = true;
  }
  ctr_discon_lock_.unlock();

  return bFound;
}

void CtrlrMgr::AddCtrToDisconnectList(const char *ctr_name) {
  UPLL_FUNC_TRACE;

  if (ctr_name == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p", ctr_name);
    return;
  }
  std::string ctr(ctr_name);

  ctr_discon_lock_.wrlock();
  // add new logical port map to ctr
  std::map<std::string, uint8_t> logical_port_map;
  ctr_discon_map_[ctr] = logical_port_map;
  ctr_discon_lock_.unlock();
}

void CtrlrMgr::AddLogicalPort(const char *ctr_na, const char *logical_port,
                              uint8_t state) {
  UPLL_FUNC_TRACE;

  if (ctr_na == NULL || logical_port == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p logical_port %p",
                   ctr_na, logical_port);
    return;
  }
  std::string ctr_name(ctr_na);
  std::string logical_port_id(logical_port);

  ctr_discon_lock_.wrlock();
  if (ctr_discon_map_.end() != ctr_discon_map_.find(ctr_name)) {
    ctr_discon_map_[ctr_name][logical_port_id] = state;
  }
  ctr_discon_lock_.unlock();
}

void CtrlrMgr::RemoveCtrFromDisconnectList(const char *ctr_name) {
  UPLL_FUNC_TRACE;

  if (ctr_name == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name %p ", ctr_name);
    return;
  }
  std::string ctr(ctr_name);

  ctr_discon_lock_.wrlock();
  if (ctr_discon_map_.end() != ctr_discon_map_.find(ctr_name)) {
    ctr_discon_map_.erase(ctr);
  }
  ctr_discon_lock_.unlock();
}

void CtrlrMgr::AddtoUnknownCtrlrList(const char *ctr_name) {
  UPLL_FUNC_TRACE;
  if (ctr_name == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name");
    return;
  }
  std::string ctr(ctr_name);
  discon_ctrlr_lock_.wrlock();
  discon_ctrlr_list_.insert(ctr);
  discon_ctrlr_lock_.unlock();
}

void CtrlrMgr::DeleteFromUnknownCtrlrList(const char *ctr_name) {
  UPLL_FUNC_TRACE;
  if (ctr_name == NULL) {
    UPLL_LOG_ERROR("Invalid argument ctr_name");
    return;
  }
  discon_ctrlr_lock_.wrlock();
  if (strcmp(ctr_name, "*") == 0) {
    if (!discon_ctrlr_list_.empty()) {
      discon_ctrlr_list_.clear();
    }
  }
  if (discon_ctrlr_list_.find(ctr_name) != discon_ctrlr_list_.end()) {
    discon_ctrlr_list_.erase(ctr_name);
  }
  discon_ctrlr_lock_.unlock();
}
void CtrlrMgr::AddCtrToIgnoreList(const char *ctr_name) {
  UPLL_FUNC_TRACE;
  if (ctr_name == NULL) {
    UPLL_LOG_DEBUG("ctrlr_name is empty")
    return;
  }
  std::string ctr(ctr_name);
  path_fault_lock_.wrlock();
  ctrlr_ignore_list_.insert(ctr);
  path_fault_lock_.unlock();
}

void CtrlrMgr::RemoveCtrlrFromIgnoreList(const char *ctr_name) {
  UPLL_FUNC_TRACE;
  if (ctr_name == NULL) {
    UPLL_LOG_DEBUG("ctrlr_name is empty")
    return;
  }
  path_fault_lock_.wrlock();
  if (strcmp(ctr_name, "*") == 0) {
    if (!ctrlr_ignore_list_.empty()) {
      ctrlr_ignore_list_.clear();
    }
  } else {
    ctrlr_ignore_list_.erase(ctr_name);
  }
  path_fault_lock_.unlock();
}

bool CtrlrMgr::IsCtrlrInIgnoreList(const char *ctr_name) {
  UPLL_FUNC_TRACE;
  bool bFound(false);
  if (ctr_name == NULL) {
    UPLL_LOG_DEBUG("ctrlr_name is empty")
    return bFound;
  }
  path_fault_lock_.rdlock();
  if (ctrlr_ignore_list_.find(ctr_name) != ctrlr_ignore_list_.end()) {
    bFound = true;
  }
  path_fault_lock_.unlock();
  return bFound;
}

/**
 * @brief : This function adds(alarm asserted) or clears(alarm cleared)
 *          the entries from vtn_exhaust_map_
 */
bool CtrlrMgr::UpdateVtnExhaustionMap(const char *vtn_na, const char *ctr_na,
                                  const char *domain_na, bool assert_alarm) {
  UPLL_FUNC_TRACE;
  bool update_operstatus(false);

  if (ctr_na == NULL || domain_na == NULL || vtn_na == NULL) {
    UPLL_LOG_ERROR("Invalid argument vtn_na %p, ctr_name %p domain_name %p",
                   vtn_na, ctr_na, domain_na);
    return false;
  }
  std::string vtn_name(vtn_na);
  std::string ctrlr_name(ctr_na);
  std::string domain_name(domain_na);

  vtn_exhaust_lock_.wrlock();
  // path fault aerted
  if (assert_alarm) {
    if (vtn_exhaust_map_.end() == vtn_exhaust_map_.find(vtn_name)) {
      // vtn exhaustion occured for the first time in for a vtn
      UPLL_LOG_INFO("vtn exhaustion  occurred on ctrlr %s:%s:%s",
                    vtn_name.c_str(), ctrlr_name.c_str(), domain_name.c_str());
      // create map for ctrlr_domain
      std::set<std::string> domains;
      domains.insert(domain_name);
      std::map<std::string, std::set<std::string> > ctrlr_domains;
      ctrlr_domains.insert(std::make_pair(ctrlr_name, domains));
      // update vtn_exhaust_map_
      vtn_exhaust_map_[vtn_name] = ctrlr_domains;
      update_operstatus = true;
    } else if (vtn_exhaust_map_[vtn_name].end() ==
        vtn_exhaust_map_[vtn_name].find(ctrlr_name)) {
      // vtn exhaustion occured for the first time in a controller
      UPLL_LOG_INFO("first vtn exhaustion  occurred on controller %s:%s:%s",
                    vtn_name.c_str(), ctrlr_name.c_str(), domain_name.c_str());
      std::set<std::string> domains;
      domains.insert(domain_name);
      vtn_exhaust_map_[vtn_name][ctrlr_name] = domains;
      update_operstatus = true;
    } else {
      // path fault already occurred on this controller domain
      vtn_exhaust_map_[vtn_name][ctrlr_name].insert(domain_name);
      UPLL_LOG_INFO("vtn exhaustion  occurred on controller-domain%s:%s:%s",
                     vtn_name.c_str(), ctrlr_name.c_str(), domain_name.c_str());
    }
  } else {
     if ((vtn_exhaust_map_.end() != vtn_exhaust_map_.find(vtn_name)) &&
         (vtn_exhaust_map_[vtn_name].end() !=
              vtn_exhaust_map_[vtn_name].find(ctrlr_name)) &&
         (vtn_exhaust_map_[vtn_name][ctrlr_name].end() !=
              vtn_exhaust_map_[vtn_name][ctrlr_name].find(domain_name))) {
      // vtn exhaustion recovered on this controller domain
      vtn_exhaust_map_[vtn_name][ctrlr_name].erase(domain_name);
      UPLL_LOG_INFO("vtn exhaustion recovery for %s:%s:%s",
                    vtn_name.c_str(), ctrlr_name.c_str(), domain_name.c_str());
      if (vtn_exhaust_map_[vtn_name][ctrlr_name].empty()) {
        vtn_exhaust_map_[vtn_name][ctrlr_name].clear();
        if (vtn_exhaust_map_[vtn_name].empty()) {
          vtn_exhaust_map_[vtn_name].clear();
          if (vtn_exhaust_map_.empty()) {
            vtn_exhaust_map_.clear();
          }
        }
      }
      update_operstatus = true;
    }
  }
  vtn_exhaust_lock_.unlock();
  return update_operstatus;
}

/**
 * @brief : This function returns true if vtn exhaustion has occurred
 */
bool CtrlrMgr::HasVtnExhaustionOccured(const char *vtn_na,
                     const char *ctr_na, const char *domain_na) {
  UPLL_FUNC_TRACE;
  bool bOccurence(false);

  if (vtn_na == NULL || ctr_na == NULL || domain_na == NULL) {
    UPLL_LOG_ERROR("Invalid argument vtn_name %p, ctr_name %p domain_name %p",
                   vtn_na, ctr_na, domain_na);
    return false;
  }
  std::string vtn_name(vtn_na);
  std::string ctrlr_name(ctr_na);
  std::string domain_name(domain_na);

  vtn_exhaust_lock_.rdlock();
  if ((!vtn_exhaust_map_.empty()) &&
      (vtn_exhaust_map_.end() != vtn_exhaust_map_.find(vtn_name))) {
    if (ctrlr_name == "*" && domain_name == "*") {
      bOccurence = true;
    } else {
      if ((vtn_exhaust_map_[vtn_name].end() !=
        vtn_exhaust_map_[vtn_name].find(ctrlr_name)) &&
        (vtn_exhaust_map_[vtn_name][ctrlr_name].end() !=
        vtn_exhaust_map_[vtn_name][ctrlr_name].find(domain_name))) {
        bOccurence = true;
      }
    }
  }
  vtn_exhaust_lock_.unlock();

  UPLL_LOG_TRACE("Vtn Exhaustion for %s:%s:%s asserted:%d",
                 vtn_name.c_str(), ctrlr_name.c_str(),
                 domain_name.c_str(), bOccurence);
  return bOccurence;
}

/**
 * @brief : This function clears vtn_exhaust_map_
 */
void CtrlrMgr::ClearVtnExhaustionMap() {
  UPLL_FUNC_TRACE;
  vtn_exhaust_lock_.wrlock();
  vtn_exhaust_map_.clear();
  vtn_exhaust_lock_.unlock();
}
}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
