/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef CTRLR_CAPABILITY_HH_
#define CTRLR_CAPABILITY_HH_

#include <string>
#include <map>

namespace unc {
namespace capa {

class KtCapability;
class KtAttrCapability;

enum CapaFileAttrOperation {
  kCapaConfCreate    = 0,
  kCapaConfUpdate    = 1,
  kCapaConfRead      = 2,
  kCapaConfStateRead = 3
};

/**
 * @brief Capability class.
 */
class CtrlrCapability {
 public:
  /**
   * @ brief CtrlrCapability Constructor
   */
  CtrlrCapability();

  /**
   * @brief CtrlrCapability destructor
   */ 
  ~CtrlrCapability();

  /**
   * @brief  Load Capability information from configuation file into memory
   *
   * @param[in] confp Config file pointer
   * @param[in] version controller version
   *
   * @retval PFC_TRUE  Loaded Successful  
   * @retval PFC_FALSE Failure occured
   */ 
  bool LoadCtrlrCapability(const pfc_conf_t confp,
                           const std::string &version);

  /**
   * @brief  Return instance count of specified key type.
   * 
   * @param[in]  keytype  Key type.
   * @param[out] instance_count  Instance count for specified keytype.
   * 
   *
   * @retval PFC_TRUE   Successful 
   * @retval PFC_FALSE  keytype is not found
   */
  bool  GetCapability(unc_key_type_t keytype,
                      uint32_t &instance_count);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  keytype    Key type.
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetCreateCapability(unc_key_type_t keytype, uint32_t *instance_count,
                           uint32_t *num_attrs, const uint8_t  **attrs);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  keytype    Key type.
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetUpdateCapability(unc_key_type_t keytype, uint32_t *num_attrs,
                           const uint8_t  **attrs);


  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   *
   * @param[in]  keytype    Key type.
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetStateCapability(unc_key_type_t keytype, uint32_t *num_attrs,
                          const uint8_t  **attrs);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  keytype    Key type.
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetReadCapability(unc_key_type_t keytype, uint32_t *num_attrs,
                         const uint8_t  **attrs);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  keytype    Key type.
   * @param[in]  attrtype   Type of attribute(like kValAttrType or kKeyAttrType)
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * @param[out] create     Array of CONF_CREATE 
   * @param[out] update     Array of CONF_UPDATE
   * @param[out] read       Array of CONF_READ
   * @param[out] state      Array of STATE_READ
   * 
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype or attrtype is  not found
   */ 
  pfc_bool_t GetCapability(unc_key_type_t keytype, uint32_t attrtype,
                           uint32_t num_attrs, uint8_t  *attrs,
                           uint8_t  *create, uint8_t  *update,
                           uint8_t  *read, uint8_t  *state);

 private:
  /**
   * @brief  Load Capability information from configuation file into memory
   * 
   * @param[in] ktname  kt name
   * @param[in] version controller version
   * 
   * @retval PFC_TRUE  Loaded Successful
   * @retval PFC_FALSE Failure occured
   */
  pfc_bool_t LoadCapability(std::string ktname, const std::string &version);

  /**
   * @brief Return instance count of specified key type.
   *
   * @param[in] ktname  kt name
   * @param[in] version controller version
   *
   * @retval instance count
   */
  uint32_t LoadInstanceCount(const pfc_conf_t confp,
                             std::string ktname,
                             std::string version);
  /**
   * @brief  A map which stores pairs of keytype_t and KtCapability.
   */ 
  std::map<uint32_t, KtCapability*> kt_cap_map_;
};  // class CtrlrCapability

/**
 * brief KtCapability class.
 */ 
class KtCapability {
 public:
  /**
   * @ brief KtCapability Constructor
   */
  KtCapability();

  /**
   * @brief KtCapability destructor
   */ 
  ~KtCapability();

  /**
   * @brief set instance count
   *
   * @param[in] count  instance count
   *
   * @retval NONE
   */
  inline void set_instance_count(uint32_t count) {
    instance_count_ = count;
  }

  /**
   * @brief get instance count
   *
   *  @param[in] NONE
   *
   *  @retval intance count
   */
  inline uint32_t get_instance_count(void) {
    return instance_count_;
  }

  /**
   * @brief  Load Capability information from configuation file into memory
   * 
   * @param[in] confblk ktname  kt name
   * @param[in] version controller version
   * 
   * @retval PFC_TRUE  Loaded Successful
   * @retval PFC_FALSE Failure occured
   */
  pfc_bool_t LoadKtCapability(const pfc_conf_t confp,
                              uint32_t kt_map_index,
                              std::string version);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified attrtype.
   *
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetKtCreateCapability(uint32_t *instance_count,
                             uint32_t *num_attrs,
                             const uint8_t  **attrs);
  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified attrtype.
   *
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetKtUpdateCapability(uint32_t *num_attrs,
                             const uint8_t  **attrs);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified attrtype.
   *
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetKtReadCapability(uint32_t *num_attrs,
                           const uint8_t  **attrs);

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified attrtype.
   *
   * @param[out] num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetKtStateCapability(uint32_t *num_attrs,
                            const uint8_t  **attrs);


  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified attrtype.
   * 
   * @param[in]  attrtype   Type of attribute(like kValAttrType or kKeyAttrType)
   * @param[in]  num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * @param[out] create     Array of CONF_CREATE
   * @param[out] update     Array of CONF_UPDATE
   * @param[out] read       Array of CONF_READ
   * @param[out] state      Array of STATE_READ
   * 
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype or attrtype is  not found
   */
  pfc_bool_t GetKtCapability(uint32_t attrtype,
                             uint32_t num_attrs, uint8_t  *attrs,
                             uint8_t  *create, uint8_t  *update,
                             uint8_t  *read, uint8_t  *state);

 private:
  bool LoadAttrCapability(const pfc_conf_t confp,
                          std::string ktname,
                          std::string attrname,
                          uint32_t attr_index,
                          const std::string version);

  /*
   * brief Stores maximum instance count
   */
  uint32_t instance_count_;

  KtAttrCapability *attr_cap_;
};  // class KtCapability


/**
 * brief KtAttrCapability class.
 */
class KtAttrCapability {
 public:
  /**
   * @brief KtAttrCapability Constructor
   */
  KtAttrCapability();

  /**
   * @brief KtAttrCapability Destructor
   */
  ~KtAttrCapability();

  /**
   *  @brief  Create attribute
   *  
   *  @param[in] num_attrs maximum attribute
   *  
   * @retval PFC_TRUE  Memory allocation
   * @retval PFC_FALSE Memory allocation failed
   */
  bool Init(uint32_t num_attrs);

  /**
   * @brief  set attribute SUPPORTED information
   * 
   * @param[in] attr_index  Index of attribute
   * @param[in] operation operation of attribute(CONF_CREATE,
   *            CONF_UPDATE, CONF_READ, STATE_READ)
   * 
   * @retval PFC_TRUE  Success in set
   * @retval PFC_FALSE attribute position is greater
   */
  bool SetAttrCap(uint32_t attr_index, uint8_t operation[4]);

  /**
   * @brief  Get attribute Capability like SUPPORTED or NOT_SUPPORTED information
   *
   * @param[in]  num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype or attrtype is  not found
   */

  bool GetCreateCapability(uint32_t *num_attrs, const uint8_t  **create);

  /**
   * @brief  Get attribute Capability like SUPPORTED or NOT_SUPPORTED information
   *
   * @param[in]  num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype or attrtype is  not found
   */

  bool GetUpdateCapability(uint32_t *num_attrs, const uint8_t **attrs);


  /**
   * @brief  Get attribute Capability like SUPPORTED or NOT_SUPPORTED information
   *
   * @param[in]  num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */

  bool GetReadCapability(uint32_t *num_attrs, const uint8_t **attrs);

  /**
   * @brief  Get attribute Capability like SUPPORTED or NOT_SUPPORTED information
   *
   * @param[in]  num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   *
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */
  bool GetStateCapability(uint32_t *num_attrs, const uint8_t  **attrs);

  /**
   * @brief  Get attribute Capability like SUPPORTED or NOT_SUPPORTED information
   * 
   * @param[in]  num_attrs  Number of attributes for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * @param[out] create     Array of CONF_CREATE
   * @param[out] update     Array of CONF_UPDATE
   * @param[out] read       Array of CONF_READ
   * @param[out] state      Array of STATE_READ
   * 
   * @retval PFC_TRUE   Successful
   * @retval PFC_FALSE  keytype not found
   */

  bool GetCapability(uint32_t num_attrs, uint8_t  *attrs,
                     uint8_t  *create, uint8_t  *update,
                     uint8_t  *read, uint8_t  *state);

 private:
  /**
   * @brief stores the attribute count
   */
  uint32_t num_attrs_;

  /**
   * @brief stores attribute capability
   */
  // Is it required? uint8_t *attr_cap_;

  /**
   * @brief stores create operation capability
   */
  uint8_t *create_cap_;

  /**
   * @brief  stores update operation capability
   */
  uint8_t *update_cap_;

  /**
   * @brief  stores read operation capability
   */
  uint8_t *read_cap_;

  /**
   * @brief  stores state operation capability
   */
  uint8_t *state_cap_;
};  // class KtAttrCapability

} /* namespace capa */
} /* namespace unc */

#endif  // CTRLR_CAPABILITY_HH_
