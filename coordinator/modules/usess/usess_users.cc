/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "usess_users.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "UsessUsers"

// -------------------------------------------------------------
//  Class method definitions.
// -------------------------------------------------------------
/*
 * @brief   Constructor.
 * @param   database  : database instance.
 * @return  nothing.
 * @note    
 */
UsessUsers::UsessUsers(mgmtdb::MgmtDatabase& database) : database_(database)
{
}


/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessUsers::~UsessUsers(void)
{
}


/*
 * @brief   Initialize.
 * @param   nothing.
 * @return  true  : success
 *          false : failure
 * @note    
 */
bool UsessUsers::Init(void)
{
  usess_ipc_err_e rtn = USESS_E_NG;


  L_FUNCTION_START();

  // configuration data load.
  rtn = conf_.LoadConf();
  RETURN_IF2((rtn != USESS_E_OK), false,
      "Failure configuration data load. err=%d", rtn);

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
bool UsessUsers::Fini(void)
{
  L_FUNCTION_START();
  L_FUNCTION_COMPLETE();
  return true;
}


/*
 * @brief   Get User data.
 * @param   name    : [IN]  user name.
 *          user    : [OUT] user class instance.
 * @return  USESS_E_OK             : Success
 *          USESS_E_INVALID_USER   : Invalid user name
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessUsers::GetUser(const std::string& name, UsessUser& user)
{
  usess_ipc_err_e func_rtn;

  L_FUNCTION_START();

  user.SetConf(conf_);
  func_rtn = user.Retrieve(name);
  RETURN_IF2((func_rtn != USESS_E_OK), func_rtn,
      "Failure get user data. err=%d", func_rtn);

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   Load configuration data.
 * @param   nothing.
 * @return  USESS_E_OK             : Success
 *          USESS_E_NG             : Error
 * @note    
 */
usess_ipc_err_e UsessUsers::LoadConf(void)
{
  usess_ipc_err_e func_rtn;


  L_FUNCTION_START();

  func_rtn = conf_.LoadConf();
  RETURN_IF2((func_rtn != USESS_E_OK), func_rtn,
      "Failure configuration data load. err=%d", func_rtn);

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}

}  // namespace usess
}  // namespace unc
