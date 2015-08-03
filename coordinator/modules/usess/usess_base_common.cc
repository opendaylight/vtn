/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <unistd.h>
#include <regex.h>
#include <crypt.h>
#include <pfcxx/synch.hh>
#include "usess_base_common.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "UsessBaseCommon"

pfc::core::ReadWriteLock usess_crypt_lock;
struct crypt_data usess_crypt_data;

// -------------------------------------------------------------
//  Class method definitions.
// -------------------------------------------------------------
/*
 * @brief   Constructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessBaseCommon::UsessBaseCommon(void)
{
}

/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessBaseCommon::~UsessBaseCommon(void)
{
}

/*
 * @brief   Hash conversion.
 * @param   str       : [IN] target string.
 *          hash_key  : [IN] hash key.
 *          hash_type : [IN] hash type.
 *          hash_str  : [OUT]hash string.
 * @return  nothing.
 * @note    
 */
void UsessBaseCommon::Hash(const char* str, const std::string& hash_key,
      const hash_type_e hash_type, std::string& hash_str) const
{
  char salt[128];

  // parameter check.
  if (hash_key.empty() ||
      (hash_type != HASH_TYPE_MD5 && hash_type != HASH_TYPE_SHA256 &&
       hash_type != HASH_TYPE_SHA512)) {
    hash_str.erase();
    return;
  }

  // create hash salt.
  snprintf(salt, sizeof(salt), "$%d$%s", hash_type, hash_key.c_str());

  Hash(str, salt, hash_str);
  return;
}


/*
 * @brief   Hash conversion.
 * @param   str       : [IN] target string.
 *          hash_key  : [IN] hash key.
 *          hash_type : [IN] hash type.
 *          hash_str  : [OUT]hash string.
 * @return  nothing.
 * @note    
 */
void UsessBaseCommon::Hash(const char* str, const pfc_timespec_t& hash_key,
    const hash_type_e hash_type, std::string& hash_str) const
{
  char salt[128];

  // parameter check.
  if (hash_type != HASH_TYPE_MD5 && hash_type != HASH_TYPE_SHA256 &&
      hash_type != HASH_TYPE_SHA512) {
    hash_str.erase();
    return;
  }

  // create hash salt.
  snprintf(salt, sizeof(salt), "$%d$%014ld", hash_type, hash_key.tv_sec);

  Hash(str, salt, hash_str);
  return;
}


/*
 * @brief   Hash conversion.
 * @param   str       : [IN] target string.
 *          salt      : [IN] hash salt.
 *          hash_str  : [OUT]hash string.
 * @return  nothing.
 * @note    
 */
void UsessBaseCommon::Hash(const char* str,
      const std::string& salt, std::string& hash_str) const
{
  // parameter check.
  if (salt.empty()) {
    hash_str.erase();
    return;
  }

  usess_crypt_lock.wrlock();
  hash_str = crypt_r(str, salt.c_str(), &usess_crypt_data);
  usess_crypt_lock.unlock();

  return;
}


/*
 * @brief   Hash comparison.
 * @param   str       : [IN] target string.
 *          hash_str  : [IN] comparison hash string.
 *          salt      : [IN] hash salt.
 * @return  nothing.
 * @note    
 */
bool UsessBaseCommon::CheckDigest(const char* str,
    const std::string& hash_str, const std::string& salt) const
{
  std::string cmp_str;


  // parameter check.
  if (hash_str.empty() || salt.empty()) {
    return false;
  }

  Hash(str, salt, cmp_str);
  return (cmp_str.compare(hash_str) == 0);
}


/*
 * @brief   Check regular expression.
 * @param   check_str   : [IN] target string.
 *          regular_str : [IN] regular pattern string.
 * @return  true  : match.
 *          false : no match.
 * @note    
 */
bool UsessBaseCommon::CheckRegular(const char* check_str,
      const std::string& regular_str) const
{
  regex_t preg;
  size_t nmatch = strlen(check_str) + 1;
  regmatch_t pmatch[nmatch];

  int rtn = -1;
  char err_buff[128];

  L_FUNCTION_START();

  memset(pmatch, 0x00, sizeof(pmatch));

  // parameter check.
  if (strlen(check_str) == 0) return true;
  if (regular_str.empty()) return false;

  // regulation compile.
  rtn = regcomp(&preg, regular_str.c_str(), (REG_EXTENDED | REG_NEWLINE));
  regerror(rtn, &preg, err_buff, sizeof(err_buff));
  GOTO_IF2((rtn != 0), err_end, "regcomp %d(%s)", rtn, err_buff);

  // regulation exec.
  rtn = regexec(&preg, check_str, nmatch, pmatch, 0);
  regerror(rtn, &preg, err_buff, sizeof(err_buff));
  GOTO_IF2((rtn != 0), err_end, "regexec %d(%s)", rtn, err_buff);

  // Check the number of matched characters.
  GOTO_IF2((pmatch[0].rm_so != 0 || pmatch[0].rm_eo != (int)strlen(check_str)),
              err_end, "%s", "no match.");

  regfree(&preg);
  L_FUNCTION_COMPLETE();
  return true;

err_end:
  regfree(&preg);
  return false;

}

}  // namespace usess
}  // namespace unc
