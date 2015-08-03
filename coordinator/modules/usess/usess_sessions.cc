/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "usess_sessions.hh"

#define CLASS_NAME "UsessSessions"

namespace unc {
namespace usess {

/*
 * @brief   Constructor.
 * @param   attr  : module attribute.
 * @return  nothing.
 * @note    
 */
UsessSessions::UsessSessions(void)
{
  table_.clear();
  for (int loop = 0; loop < ID_NUM; ++loop) {
    allocated_sess_id_[loop] = USESS_ID_INVALID;
  }

}

/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessSessions::~UsessSessions(void)
{
}

/*
 * @brief   Initialize.
 * @param   nothing.
 * @return  true  : success
 *          false : failure
 * @note    
 */
bool UsessSessions::Init(void)
{
  usess_ipc_err_e rtn = USESS_E_NG;

  L_FUNCTION_START();

  // configuration data load.
  rtn = conf_.LoadConf();
  RETURN_IF2((rtn != USESS_E_OK), false,
      "Failure configuration data load. err=%d", rtn);

  table_.clear();
  for (int loop = 0; loop < ID_NUM; ++loop) {
    allocated_sess_id_[loop] = USESS_ID_INVALID;
  }

  L_FUNCTION_COMPLETE();
  return true;
}

/*
 * @brief   Finalization.
 * @param   nothing.
 * @return  true  : success
 *          false : failure
 * @note    
 */
bool UsessSessions::Fini(void)
{
  usess_ipc_err_e func_rtn = USESS_E_NG;


  L_FUNCTION_START();

  // deleted of all session data.
  func_rtn = Del();
  if (func_rtn != USESS_E_NG) {
    L_DEBUG("Failure session delete. err=%d", func_rtn);
  }

  table_.clear();

  L_FUNCTION_COMPLETE();
  return true;
}


/*
 * @brief   Add the session data.
 * @param   add_sess  : [IN]  add session.
 *          user      : [IN]  user data.
 *          sess_id   : [OUT] new session ID.
 * @return  USESS_E_OK             : Success
 *          USESS_E_USESS_OVER     : Orver Session count.
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::Add(const usess_ipc_req_sess_add_t& add_sess,
                     UsessUser user, usess_ipc_sess_id_t& sess_id)
{
  usess_ipc_res_sess_info_t sess_data;
  usess_type_e sess_type;
  int rtn = -1;


  L_FUNCTION_START();

  // check parameter. session type.
  sess_type = static_cast<usess_type_e>(add_sess.sess_type);
  RETURN_IF2((IsSessType(sess_type) != true), USESS_E_INVALID_SESSTYPE,
      "Invalid session type. type = %d", add_sess.sess_type);

  // check parameter. user type.
  RETURN_IF2((IsUserType(user.type) != true), USESS_E_INVALID_USER,
      "Invalid user type. type = %d", user.type);

  // check number of sessions that are registered.
  RETURN_IF2((CheckSessTypeCount(sess_type) != true), USESS_E_SESS_OVER,
      "Session count over. session type = %d", sess_type);

  // ---------------------------------------------
  // add session.
  // ---------------------------------------------
  // Editing session information.
  rtn = pfc_clock_get_realtime((pfc_timespec_t*)&sess_data.login_time);
  RETURN_IF2((rtn != 0), USESS_E_NG, "failed get login_time. err=%d (%s)",
      rtn, strerror(rtn));

  sess_data.sess = GetNewSessionId(sess_type);
  RETURN_IF2((sess_data.sess.id == USESS_ID_INVALID), USESS_E_SESS_OVER,
    "Session count over. session type = %d", sess_type);
  sess_data.sess_type = add_sess.sess_type;
  sess_data.sess_mode = USESS_MODE_OPER;
  sess_data.user_type = user.type;
  sess_data.config_status = CONFIG_STATUS_NONE;
  memcpy(sess_data.login_name, add_sess.login_name,
         sizeof(sess_data.login_name));
  memcpy(sess_data.sess_uname, add_sess.sess_uname,
         sizeof(sess_data.sess_uname));
  sess_data.ipaddr = add_sess.ipaddr;
  memcpy(sess_data.info, add_sess.info, sizeof(sess_data.info));
  sess_data.config_mode = TC_CONFIG_INVALID;
  sess_data.vtn_name[0] = '\0';

  // create session class.
  table_.insert(usess_session_table_t::value_type(
          sess_data.sess.id, UsessSession(conf_, sess_data)));

  // setting session ID.
  sess_id = sess_data.sess;

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Delete the session data.
 * @param   target : [IN]  delete session ID.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NO_SUCH_SESSID : Not found delete session.
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::Del(const usess_ipc_sess_id_t& target)
{
  tc::TcApiRet tc_rtn = tc::TC_API_COMMON_FAILURE;
  tc::TcModule *tc_instance = NULL;     // TC module instance.

  L_FUNCTION_START();

  // chack session.
  RETURN_IF2((table_.count(target.id) != 1), USESS_E_NO_SUCH_SESSID,
      "Invalid delete session ID = %d", target.id);

  // Get TC module instance.
  tc_instance = (tc::TcModule *)pfc::core::Module::getInstance("tc");
  RETURN_IF2((tc_instance == NULL), USESS_E_NG,
      "%s", "Failure TC module getinstance.");

  // release configuration mode session.
  tc_rtn = tc_instance->TcReleaseSession(target.id);
  WARN_IF((tc_rtn != tc::TC_API_COMMON_SUCCESS && tc_rtn != tc::TC_INVALID_PARAM),
    "Without notification to TC. id=%d err=%d", target.id, tc_rtn);

  // session delete.
  table_.erase(target.id);

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Delete the session data.
 * @param   target : [IN]  delete session ID.
 * @return  USESS_E_OK               : Success
 *          USESS_E_INVALID_SESSTYPE : Invalid session type.
 *          USESS_E_NG               : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::Del(const usess_type_e target_sess_type)
{
  usess_session_table_t::iterator it;
  tc::TcApiRet tc_rtn = tc::TC_API_COMMON_FAILURE;
  tc::TcModule *tc_instance = NULL;     // TC module instance.


  L_FUNCTION_START();

  // chack session type.
  RETURN_IF2((IsSessType(target_sess_type) != true), USESS_E_INVALID_SESSTYPE,
      "Invalid delete session type = %d", target_sess_type);

  // Get TC module instance.
  tc_instance = (tc::TcModule *)pfc::core::Module::getInstance("tc");
  RETURN_IF2((tc_instance == NULL), USESS_E_NG,
      "%s", "Failure TC module getinstance.");

  // delete target search.
  for (it = table_.begin(); it != table_.end();) {
    if (it->second.sess().sess_type != target_sess_type) {
      ++it;
      continue;
    }

    // TC notification. release configuration mode session.
    tc_rtn = tc_instance->TcReleaseSession(it->second.sess().sess.id);
    WARN_IF((tc_rtn != tc::TC_API_COMMON_SUCCESS && tc_rtn != tc::TC_INVALID_PARAM),
      "Without notification to TC. id=%d err=%d",
      it->second.sess().sess.id, tc_rtn);

    // session delete.
    // attention! Because the iterator becomes invalid when you erase,
    //            do a post-increment.
    table_.erase(it++);
  }

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Delete the session data.
 * @param   nothing.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::Del(void)
{
  usess_session_table_t::iterator it;
  tc::TcApiRet tc_rtn = tc::TC_API_COMMON_FAILURE;
  tc::TcModule *tc_instance = NULL;     // TC module instance.


  L_FUNCTION_START();


  // Get TC module instance.
  tc_instance = (tc::TcModule *)pfc::core::Module::getInstance("tc");
  RETURN_IF2((tc_instance == NULL), USESS_E_NG,
      "%s", "Failure TC module getinstance.");

  // TC notification.
  for (it = table_.begin(); it != table_.end(); ++it) {
    // TC notification. release configuration mode session.
    tc_rtn = tc_instance->TcReleaseSession(it->second.sess().sess.id);
    WARN_IF((tc_rtn != tc::TC_API_COMMON_SUCCESS && tc_rtn != tc::TC_INVALID_PARAM),
      "Without notification to TC. id=%d err=%d",
      it->second.sess().sess.id, tc_rtn);
  }

  // session all delete.
  table_.clear();

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Get session.
 * @param   id    : [IN] target session id.
 *          sess  : [OUT]session data.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NO_SUCH_SESSID : Not found session id.
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::GetSession(
      const usess_ipc_sess_id_t& target, UsessSession** sess)
{
  usess_session_table_t::iterator it;


  L_FUNCTION_START();

  // chack session.
  it = table_.find(target.id);
  RETURN_IF2((it == table_.end()), USESS_E_NO_SUCH_SESSID,
              "Failed session id. id=%d", target.id);

  *sess = &it->second;

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Get session count.
 * @param   nothing.
 * @return  session count.
 * @note    
 */
uint32_t UsessSessions::GetCount(void)
{
  L_FUNCTION_START();
  L_FUNCTION_COMPLETE();
  return table_.size();
}


/*
 * @brief   Get session detail list.
 * @param   target    : [IN] target session id.
 *          info_list : [OUT]get session data.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NO_SUCH_SESSID : Not found session id.
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::GetList(const usess_ipc_sess_id_t& target,
                                       usess_session_list_v& info_list)
{
  usess_session_table_t::iterator it;
  tc::TcApiRet tc_rtn = tc::TC_API_COMMON_FAILURE;
  uint32_t         config_id = 0;
  TcConfigMode config_mode = TC_CONFIG_GLOBAL;
  std::string      vtn_name = "";
  tc::TcModule *tc_instance = NULL;     // TC module instance.


  L_FUNCTION_START();

  // chack session.
  it = table_.find(target.id);
  RETURN_IF2((it == table_.end()), USESS_E_NO_SUCH_SESSID,
      "Invalid session ID = %d", target.id);

  // list clear.
  info_list.clear();

  // list set.
  info_list.push_back(it->second.sess());
  info_list[0].config_mode = TC_CONFIG_INVALID;
  info_list[0].vtn_name[0] = '\0';

  // Get TC module instance.
  tc_instance = (tc::TcModule *)pfc::core::Module::getInstance("tc");
  RETURN_IF2((tc_instance == NULL), USESS_E_NG,
      "%s", "Failure TC module getinstance.");

  // set configration status.
  tc_rtn = tc_instance->TcGetConfigSession(target.id,
                                           config_id,
                                           config_mode,
                                           vtn_name);

  RETURN_IF2((tc_rtn != tc::TC_API_COMMON_SUCCESS && 
             tc_rtn != tc::TC_NO_CONFIG_SESSION &&
             tc_rtn != tc::TC_INVALID_UNC_STATE), USESS_E_NG,
            "Get configuration session to TC. id=%d err=%d",
            target.id, tc_rtn);

  if (tc_rtn == tc::TC_API_COMMON_SUCCESS) {
    info_list[0].config_mode = (int32_t)config_mode;

    if (config_mode == TC_CONFIG_VTN) {
      strncpy((char *)&(info_list[0].vtn_name[0]),
              (char *)vtn_name.c_str(), sizeof(info_list[0].vtn_name));
      info_list[0].vtn_name[sizeof(info_list[0].vtn_name) - 1] = '\0';
    }

    info_list[0].config_status =
      ((info_list[0].config_mode == TC_CONFIG_GLOBAL) ?
       CONFIG_STATUS_TCLOCK :
       CONFIG_STATUS_TCLOCK_PART);
  }

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Get session list.
 * @param   info_list : [OUT]get session list.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::GetList(usess_session_list_v& info_list)
{
  usess_session_table_t::iterator it;
  tc::TcApiRet tc_rtn = tc::TC_API_COMMON_FAILURE;
  uint32_t         config_id = 0;
  TcConfigMode config_mode = TC_CONFIG_GLOBAL;
  std::string      vtn_name = "";
  tc::TcModule *tc_instance = NULL;     // TC module instance.


  L_FUNCTION_START();

  // Get TC module instance.
  tc_instance = (tc::TcModule *)pfc::core::Module::getInstance("tc");
  RETURN_IF2((tc_instance == NULL), USESS_E_NG,
      "%s", "Failure TC module getinstance.");

  // list clear.
  info_list.clear();

  // list set.
  for (it = table_.begin(); it != table_.end(); ++it) {
    info_list.push_back(it->second.sess());
    info_list[info_list.size() - 1].config_mode = TC_CONFIG_INVALID;
    info_list[info_list.size() - 1].vtn_name[0] = '\0';

    // set configration status.
    tc_rtn = tc_instance->TcGetConfigSession(it->second.sess().sess.id,
                                             config_id,
                                             config_mode,
                                             vtn_name);

    WARN_IF((tc_rtn != tc::TC_API_COMMON_SUCCESS && 
             tc_rtn != tc::TC_NO_CONFIG_SESSION &&
             tc_rtn != tc::TC_INVALID_UNC_STATE),
            "Get configuration session to TC. id=%d err=%d",
            it->second.sess().sess.id, tc_rtn);

    if (tc_rtn == tc::TC_API_COMMON_SUCCESS) {
      info_list[info_list.size() - 1].config_mode = (int32_t)config_mode;

      if (config_mode == TC_CONFIG_VTN) {
        strncpy((char *)&(info_list[info_list.size() - 1].vtn_name[0]),
                (char *)vtn_name.c_str(), sizeof(info_list[0].vtn_name));
        info_list[info_list.size() - 1].vtn_name[sizeof(info_list[0].vtn_name) - 1] = '\0';
      }

      info_list[info_list.size() - 1].config_status =
        ((info_list[info_list.size() - 1].config_mode == TC_CONFIG_GLOBAL) ?
         CONFIG_STATUS_TCLOCK :
         CONFIG_STATUS_TCLOCK_PART);
    }
  }

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Sesssion privilege.
 * @param   info_list : [OUT]get session list.
 * @param   mode      :[IN] authenticate mode.
 *          current   :[IN] current session ID.
 *          target    :[IN] target session ID.
 * @return  USESS_E_OK                : Success
 *          USESS_E_INVALID_SESSID    : Invalid crrent session ID.
 *          USESS_E_NO_SUCH_SESSID    : Not found target session.
 *          USESS_E_INVALID_PRIVILEGE : Invalid privilege.
 *          USESS_E_NG                : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::Privilege(const session_privilege_e mode,
    const usess_ipc_sess_id_t& current, const usess_ipc_sess_id_t& target)
{
  usess_session_table_t::iterator current_it;
  usess_ipc_err_e err_code = USESS_E_NG;


  L_FUNCTION_START();

  // Not fixed session ID.
  // check current session for not fixed session ID.
  if (current.id < conf_.data().local[ID_FIXED].range.start ||
      current.id > conf_.data().local[ID_FIXED].range.end) {

    // get current session.
    current_it = table_.find(current.id);
    RETURN_IF2((current_it == table_.end()), USESS_E_INVALID_SESSID,
        "Invalid current session ID. ID=%u", current.id);
  }

  // check target session.
  if (current.id != target.id) {
    RETURN_IF2((table_.count(target.id) != 1), USESS_E_NO_SUCH_SESSID,
        "Invalid target session ID. ID=%u", target.id);
  }

  // No authentication required for fixed session ID.
  if (current.id >= conf_.data().local[ID_FIXED].range.start &&
      current.id <= conf_.data().local[ID_FIXED].range.end) {
    L_INFO("%s", "Privilege fixed session ID.");
    L_FUNCTION_COMPLETE();
    return USESS_E_OK;
  }

  // Privilege check.
  err_code = USESS_E_INVALID_PRIVILEGE;

  switch(mode) {
  case kPrivilegeSessDel:               // delete session.
    if (current_it->second.sess().sess_mode == USESS_MODE_OPER) {
      if (current.id == target.id) {
        err_code = USESS_E_OK;
      }
    } else if (current_it->second.sess().sess_mode == USESS_MODE_ENABLE) {
      err_code = USESS_E_OK;
    }
    break;

  case kPrivilegeSessCount:            // Get session count.
    err_code = USESS_E_OK;
    break;

  case kPrivilegeSessList:             // Get session list.
    err_code = USESS_E_OK;
    break;

  case kPrivilegeSessDetail:          // Get session detail.
    if (current_it->second.sess().sess_mode == USESS_MODE_OPER) {
      if (current.id == target.id) {
        err_code = USESS_E_OK;
      }
    } else if (current_it->second.sess().sess_mode == USESS_MODE_ENABLE) {
      err_code = USESS_E_OK;
    }
    break;

  default:
    L_ERROR("Invalid privilege mode. mode=%d", mode);
    err_code = USESS_E_NG;
    break;
  }

  if (err_code == USESS_E_OK) L_FUNCTION_COMPLETE();
  return err_code;
}


/*
 * @brief   Load configuration data.
 * @param   nothing.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessSessions::LoadConf(void)
{
  usess_ipc_err_e func_rtn = USESS_E_NG;

  L_FUNCTION_START();

  func_rtn = conf_.LoadConf();
  RETURN_IF2((func_rtn != USESS_E_OK), func_rtn,
      "Failure configuration data load. err=%d", func_rtn);

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}

/*
 * @brief   Numbering of new session ID.
 * @param   sess_type : [IN] session type.
 * @return  new numbering session id.
 * @note    Do not check the maximum number of sessions.
 */
const usess_ipc_sess_id_t UsessSessions::GetNewSessionId(
                          usess_type_e sess_type)
{
  usess_ipc_sess_id_t sess = {0};
  usess_conf_session_parameter_t info;
  uint32_t* allocated_id = NULL;
  uint32_t id_search_start = USESS_ID_INVALID;


  sess.id = USESS_ID_INVALID;

  L_FUNCTION_START();

  // get range of session ID.
  info = conf_.data().local[CONF_LOCAL_ID(sess_type)];
  allocated_id = &allocated_sess_id_[CONF_LOCAL_ID(sess_type)];

  // search for a free session.
  if ((*allocated_id != USESS_ID_INVALID) && (*allocated_id < info.range.end)) {
    id_search_start = (*allocated_id) + 1;
  } else {
    id_search_start = info.range.start;
  }

  // search of since allocated number.
  for (uint32_t loop = id_search_start; loop <= info.range.end; ++loop) {
    if (table_.count(loop) == 0) {
      sess.id = loop;
      break;
    }
  }

  // If did not find, search from the beginning.
  if (sess.id == USESS_ID_INVALID) {
    for (uint32_t loop = info.range.start; loop < id_search_start; ++loop) {
      if (table_.count(loop) == 0) {
        sess.id = loop;
        break;
      }
    }
  }

  // update of allocated session id.
  if (sess.id != USESS_ID_INVALID) {
    *allocated_id = sess.id;
  }

  L_FUNCTION_COMPLETE();
  return sess;
}


/*
 * @brief   check session count.
 * @param   sess_type : [IN]  session type.
 * @return  true  : success.
 *          false : over session count.
 * @note    
 */
bool UsessSessions::CheckSessTypeCount(usess_type_e sess_type)
{
  usess_session_table_t::iterator it;
  usess_session_table_t::iterator lower_it;
  usess_session_table_t::iterator upper_it;
  usess_conf_session_parameter_t info;
  uint32_t sess_count = 0;


  // -------------------------------------------
  // check global connect session count limit.
  // -------------------------------------------
  if (conf_.data().global.limited == true) {
    if (conf_.data().global.max_session <= GetCount()) {
      return false;
    }
  }

  // -------------------------------------------
  // check local connect session count limit.
  // -------------------------------------------
  // get range of session ID.
  info = conf_.data().local[CONF_LOCAL_ID(sess_type)];
  if (info.connect.limited == true) {

    // count number of sessions
    lower_it = table_.lower_bound(info.range.start);
    upper_it = table_.upper_bound(info.range.end);
    for (it = lower_it; it != upper_it; ++it) {
      ++sess_count;
    }

    if (info.connect.max_session <= sess_count) {
      return false;
    }
  }

  return true;
}


/*
 * @brief   Range check of sesion type.
 * @param   type  : [IN] session type.
 * @return  true  : check ok.
 *          false : check ng.
 * @note    
 */
bool UsessSessions::IsSessType(usess_type_e type) const
{
  return ((type == USESS_TYPE_UNKNOWN) ||(type == USESS_TYPE_CLI) ||
      (type == USESS_TYPE_CLI_DAEMON) || (type == USESS_TYPE_WEB_API) ||
      (type == USESS_TYPE_WEB_UI));
}

/*
 * @brief   Range check of user type.
 * @param   type  : [IN] user type.
 * @return  true  : valid range.
 *          false : invalid range.
 * @note    
 */
bool UsessSessions::IsUserType(user_type_e type) const
{
  return ((type == USER_TYPE_UNKNOWN) || (type == USER_TYPE_OPER) ||
          (type == USER_TYPE_ADMIN));
}

}  // namespace usess
}  // namespace unc
