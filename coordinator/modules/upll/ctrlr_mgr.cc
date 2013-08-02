/*
 * Copyright (c) 2012-2013 NEC Corporation
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
#include "upll/upll_log.hh"
// #include "config_mgr.hh"
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
  // UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();
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
        UPLL_LOG_ERROR("Ctrlr(%s) Already exists", pctrlr->name_.c_str());
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
  // UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();
  pfc::core::ScopedMutex lock(ctrlr_mutex_);

  if (datatype != UPLL_DT_CANDIDATE && datatype != UPLL_DT_RUNNING) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  /* Deleting in the map */
  bool deleted = false;
  for (std::list<Ctrlr*>::iterator it = ctrlrs_.begin();
       it != ctrlrs_.end(); ++it) {
    if ((*it)->name_.compare(ctrlr_name) == 0 &&
      (*it)->datatype_ == datatype) {
      ctrlrs_.erase(it);
      deleted = true;
      break;
    }
    continue;
  }
  if (deleted) {
    UPLL_LOG_INFO("Deleted controller(%s) from datatype(%d)",
                  ctrlr_name.c_str(), datatype);
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
  // UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();
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
  // UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();
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
  // UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();
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
  // UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();
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
    if ((*it)->name_.compare(in_name) < 0) {
      continue;
    } else if ((*it)->datatype_ != mapped_dt) {
      continue;
    } else {
      UPLL_LOG_TRACE("Ctrlr (%s) is next to (%s) in datatype(%d)",
                     (*it)->name_.c_str(), in_name.c_str(), datatype);
      *next_name = (*it)->name_;
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

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
