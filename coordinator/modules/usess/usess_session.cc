/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "usess_session.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "UsessSession"

/*
 * @brief   Constructor.
 * @param   conf  : [IN] configuration data.
 * @return  nothing.
 * @note    
 */
UsessSession::UsessSession(const UsessConfSession& conf) : conf_(conf)
{
}

UsessSession::UsessSession(const UsessConfSession& conf,
        const usess_ipc_res_sess_info_t& sess) : conf_(conf)
{
  sess_ = sess;
}


/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessSession::~UsessSession(void)
{
}

/*
 * @brief   Modify session mode.
 * @param   sess_mode   : [IN] session mode.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessSession::TransitMode(const usess_mode_e sess_mode)
{
  tc::TcModule *tc_instance = NULL;


  L_FUNCTION_START();

  // check session mode of session data.
  RETURN_IF2((sess_.sess_mode != USESS_MODE_OPER &&
                sess_.sess_mode != USESS_MODE_ENABLE), USESS_E_NG,
              "Invalid session mode. mode = %d", sess_mode);

  // TC notification
  if (sess_mode == USESS_MODE_OPER) {

    // Get TC module instance.
    tc_instance = (tc::TcModule *)pfc::core::Module::getInstance("tc");
    RETURN_IF2((tc_instance == NULL), USESS_E_NG,
        "%s", "Failure TC module getinstance.");

    // release configuration mode session.
    tc_instance->TcReleaseSession(sess_.sess.id);
  }

  // modify session mode.
  sess_.sess_mode = sess_mode;

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Get session data.
 * @param   nothing.
 * @return  session data.
 * @note    
 */
const usess_ipc_res_sess_info_t& UsessSession::sess(void)
{
  return sess_;
}

}  // namespace usess
}  // namespace unc

