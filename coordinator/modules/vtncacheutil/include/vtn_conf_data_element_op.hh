/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VTN_CONF_DATA_ELEMENT_OP_HH__
#define __VTN_CONF_DATA_ELEMENT_OP_HH__

#include "vtn_conf_utility.hh"
#include "confignode.hh"
#include<string>


namespace unc {
  namespace vtndrvcache {
    template<typename key, typename value, typename cache_oper>
      class CacheElementUtil: public  ConfigNode {
        private:
          key* key_;
          value* value_;
          cache_oper operation;

        public:
         /**
         ** Constructor to set the key struct, value structure & operation
         **/

          CacheElementUtil(key* key_ty, value* value_ty, cache_oper opet):
                         operation(opet) {

            key_ = new key();
            PFC_ASSERT(key_ != NULL);
            value_ = new value();
            PFC_ASSERT(value_ != NULL);

            memcpy(key_, key_ty, sizeof(key));
            memcpy(value_, value_ty, sizeof(value));
          }

          /**
           ** Destructor to free the key struct, value structure
           **/
          ~CacheElementUtil() {
              delete key_;
              delete value_;
          }


          /**
          ** This method returns the Keytype given the key struct
          ** @param [out] - key_Type
          **/
          unc_key_type_t  get_type() {
            pfc_log_debug("In function %s..", PFC_FUNCNAME);
            return ConfUtil::get_key_type(*key_);
          }

          /**
          ** This method returns the search Key given the key struct
          ** @param [out] - search key - string
          **/
          std::string  get_key() {
            pfc_log_debug("In %s function ", PFC_FUNCNAME);
            return ConfUtil::get_search_key(*key_);
          }

          /**
          ** This method returns the parent Key given the key struct
          ** @param [out] - parent key - string
          **/
          std::string  get_parent_key() {
            pfc_log_debug("In %s function ", PFC_FUNCNAME);
            return ConfUtil::get_parent_key(*key_);
          }

          /**
          ** This method returns the key struct
          ** @param [out] - key*
          **/
          key* getkey() {
            pfc_log_debug("In %s function ", PFC_FUNCNAME);
            return key_;
          }

          /**
          ** This method returns the value struct
          ** @param [out] - key*
          **/
          value* getval() {
            pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
            return value_;
          }

          /**
          ** This method returns the operation
          ** @param [out] - cache_oper 
          **/
          cache_oper get_operation() {
            pfc_log_debug("Entering function %s..", PFC_FUNCNAME);
            return operation;
         }
      };
  }
}



#endif
