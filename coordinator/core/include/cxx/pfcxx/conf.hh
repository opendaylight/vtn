/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCXX_CONF_HH
#define	_PFCXX_CONF_HH

/*
 * PFC configuration file interface for C++ language.
 */

#include <string>
#include <boost/noncopyable.hpp>
#include <pfc/conf.h>

namespace pfc {
namespace core {

class ConfBlock;

/*
 * The ConfHandle instance represents a configuration file handle.
 */
class ConfHandle
    : boost::noncopyable
{
    friend class ConfBlock;

public:
    /*
     * Create a configuration file handle.
     */
    ConfHandle(const char *path, const pfc_cfdef_t *defs)
        : _conf(PFC_CONF_INVALID),
          _error(pfc_conf_open(&_conf, path, defs)) {}

    ConfHandle(const std::string &path, const pfc_cfdef_t *defs)
        : _conf(PFC_CONF_INVALID),
          _error(pfc_conf_open(&_conf, path.c_str(), defs)) {}

    /*
     * Destructor.
     */
    virtual ~ConfHandle(void)
    {
        pfc_conf_close(_conf);
    }

    /*
     * Reload configuration file.
     */
    inline int
    reload(void)
    {
        return pfc_conf_reload(_conf);
    }

    /*
     * Create a list model instance which contains all map keys in the
     * parameter map specified by `mname'.
     */
    inline int
    getMapKeys(const char *mname, pfc_listm_t &keys)
    {
        return pfc_conf_get_mapkeys(_conf, mname, &keys);
    }

    inline int
    getMapKeys(const std::string &mname, pfc_listm_t &keys)
    {
        return pfc_conf_get_mapkeys(_conf, mname.c_str(), &keys);
    }

    /*
     * Get error number returned by pfc_conf_open().
     * If this method returns non-zero value, it means that this configuration
     * file handle is invalid.
     */
    inline int
    getError(void) const
    {
        return _error;
    }

private:
    /* Configuration file handle. */
    pfc_conf_t	_conf;

    /* Error number returned by pfc_conf_open(). */
    int	_error;
};

/*
 * The ConfBlock instance represents a bunch of parameters defined in
 * the parameter block or map.
 */
class ConfBlock
{
public:
    /*
     * Create a block handle instance associated with a parameter block
     * specified by the block name. This constructor is used to retrieve
     * parameter block in the PFC system configuration file.
     */
    ConfBlock(const char *bname)
        : _block(pfc_conf_get_block(pfc_sysconf_open(), bname)) {}

    ConfBlock(const std::string &bname)
        : _block(pfc_conf_get_block(pfc_sysconf_open(), bname.c_str())) {}

    /*
     * Create a block handle instance associated with a parameter map block
     * specified by the map name and the map key. This constructor is used to
     * retrieve parameter map in the PFC system configuration file.
     */
    ConfBlock(const char *mname, const char *key)
        : _block(pfc_conf_get_map(pfc_sysconf_open(), mname, key)) {}

    ConfBlock(const std::string &mname, const std::string &key)
        : _block(pfc_conf_get_map(pfc_sysconf_open(), mname.c_str(),
                                  key.c_str())) {}

    /*
     * Create a block handle instance associated with a parameter block
     * specified by the configuration file handle and the block name.
     */
    ConfBlock(ConfHandle &cf, const char *bname)
        : _block(pfc_conf_get_block(cf._conf, bname)) {}

    ConfBlock(ConfHandle &cf, const std::string &bname)
        : _block(pfc_conf_get_block(cf._conf, bname.c_str())) {}

    /*
     * Create a block handle instance associated with a parameter map block
     * specified by the configuration file handle, the map name, and the
     * map key.
     */
    ConfBlock(ConfHandle &cf, const char *mname, const char *key)
        : _block(pfc_conf_get_map(cf._conf, mname, key)) {}

    ConfBlock(ConfHandle &cf, const std::string &mname, const std::string &key)
        : _block(pfc_conf_get_map(cf._conf, mname.c_str(), key.c_str())) {}

    /*
     * Destructor.
     */
    virtual ~ConfBlock(void) {}

    /*
     * Return a byte parameter value.
     */
    inline uint8_t
    getByte(const char *name, uint8_t defvalue)
    {
        return pfc_conf_get_byte(_block, name, defvalue);
    }

    inline uint8_t
    getByte(const std::string &name, uint8_t defvalue)
    {
        return pfc_conf_get_byte(_block, name.c_str(), defvalue);
    }

    /*
     * Return a C-styled string value.
     */
    inline const char *
    getString(const char *name, const char *defvalue)
    {
        return pfc_conf_get_string(_block, name, defvalue);
    }

    inline const char *
    getString(const std::string &name, const char *defvalue)
    {
        return pfc_conf_get_string(_block, name.c_str(), defvalue);
    }

    /*
     * Return a boolean value.
     */
    inline pfc_bool_t
    getBool(const char *name, pfc_bool_t defvalue)
    {
        return pfc_conf_get_bool(_block, name, defvalue);
    }

    inline pfc_bool_t
    getBool(const std::string &name, pfc_bool_t defvalue)
    {
        return pfc_conf_get_bool(_block, name.c_str(), defvalue);
    }

    /*
     * Return a signed 32-bit integer.
     */
    inline int32_t
    getInt32(const char *name, int32_t defvalue)
    {
        return pfc_conf_get_int32(_block, name, defvalue);
    }

    inline int32_t
    getInt32(const std::string &name, int32_t defvalue)
    {
        return pfc_conf_get_int32(_block, name.c_str(), defvalue);
    }

    /*
     * Return an unsigned 32-bit integer.
     */
    inline uint32_t
    getUint32(const char *name, uint32_t defvalue)
    {
        return pfc_conf_get_uint32(_block, name, defvalue);
    }

    inline uint32_t
    getUint32(const std::string &name, uint32_t defvalue)
    {
        return pfc_conf_get_uint32(_block, name.c_str(), defvalue);
    }

    /*
     * Return a signed 64-bit integer.
     */
    inline int64_t
    getInt64(const char *name, int64_t defvalue)
    {
        return pfc_conf_get_int64(_block, name, defvalue);
    }

    inline int64_t
    getInt64(const std::string &name, int64_t defvalue)
    {
        return pfc_conf_get_int64(_block, name.c_str(), defvalue);
    }

    /*
     * Return an unsigned 64-bit integer.
     */
    inline uint64_t
    getUint64(const char *name, uint64_t defvalue)
    {
        return pfc_conf_get_uint64(_block, name, defvalue);
    }

    inline uint64_t
    getUint64(const std::string &name, uint64_t defvalue)
    {
        return pfc_conf_get_uint64(_block, name.c_str(), defvalue);
    }

    /*
     * Return a signed long integer.
     */
    inline pfc_long_t
    getLong(const char *name, pfc_long_t defvalue)
    {
        return pfc_conf_get_long(_block, name, defvalue);
    }

    inline pfc_long_t
    getLong(const std::string &name, pfc_long_t defvalue)
    {
        return pfc_conf_get_long(_block, name.c_str(), defvalue);
    }

    /*
     * Return an unsigned long integer.
     */
    inline pfc_ulong_t
    getUlong(const char *name, pfc_ulong_t defvalue)
    {
        return pfc_conf_get_ulong(_block, name, defvalue);
    }

    inline pfc_ulong_t
    getUlong(const std::string &name, pfc_ulong_t defvalue)
    {
        return pfc_conf_get_ulong(_block, name.c_str(), defvalue);
    }

    /*
     * Determine whether the parameter specified by the name is defined in
     * the configuration file or not.
     */
    inline pfc_bool_t
    isDefined(const char *name)
    {
        return pfc_conf_is_defined(_block, name);
    }

    inline pfc_bool_t
    isDefined(const std::string &name)
    {
        return pfc_conf_is_defined(_block, name.c_str());
    }

    /*
     * Return PFC_TRUE if the type of parameter specified by the name is array.
     */
    inline pfc_bool_t
    isArray(const char *name)
    {
        return pfc_conf_is_array(_block, name);
    }

    inline pfc_bool_t
    isArray(const std::string &name)
    {
        return pfc_conf_is_array(_block, name.c_str());
    }

    /*
     * Return number of array elements defined by the specified parameter.
     */
    inline int
    arraySize(const char *name)
    {
        return pfc_conf_array_size(_block, name);
    }

    inline int
    arraySize(const std::string &name)
    {
        return pfc_conf_array_size(_block, name.c_str());
    }

    /*
     * Returns a block handle in this instance.
     * If this method returns PFC_CFBLK_INVALID, it means that this block
     * handle is invalid.
     */
    inline pfc_cfblk_t
    getBlock(void) const
    {
        return _block;
    }

    /*
     * Return an array elements specified by the parameter name and array
     * index. Negative value is returned if the specified parameter is not
     * an array.
     */
    inline uint8_t
    getByteAt(const char *name, uint32_t index, uint8_t defvalue)
    {
        return pfc_conf_array_byteat(_block, name, index, defvalue);
    }

    inline uint8_t
    getByteAt(const std::string &name, uint32_t index, uint8_t defvalue)
    {
        return pfc_conf_array_byteat(_block, name.c_str(), index, defvalue);
    }

    inline const char *
    getStringAt(const char *name, uint32_t index, const char *defvalue)
    {
        return pfc_conf_array_stringat(_block, name, index, defvalue);
    }

    inline const char *
    getStringAt(const std::string &name, uint32_t index, const char *defvalue)
    {
        return pfc_conf_array_stringat(_block, name.c_str(), index, defvalue);
    }

    inline pfc_bool_t
    getBoolAt(const char *name, uint32_t index, pfc_bool_t defvalue)
    {
        return pfc_conf_array_boolat(_block, name, index, defvalue);
    }

    inline pfc_bool_t
    getBoolAt(const std::string &name, uint32_t index, pfc_bool_t defvalue)
    {
        return pfc_conf_array_boolat(_block, name.c_str(), index, defvalue);
    }

    inline int32_t
    getInt32At(const char *name, uint32_t index, int32_t defvalue)
    {
        return pfc_conf_array_int32at(_block, name, index, defvalue);
    }

    inline int32_t
    getInt32At(const std::string &name, uint32_t index, int32_t defvalue)
    {
        return pfc_conf_array_int32at(_block, name.c_str(), index, defvalue);
    }

    inline uint32_t
    getUint32At(const char *name, uint32_t index, uint32_t defvalue)
    {
        return pfc_conf_array_uint32at(_block, name, index, defvalue);
    }

    inline uint32_t
    getUint32At(const std::string &name, uint32_t index, uint32_t defvalue)
    {
        return pfc_conf_array_uint32at(_block, name.c_str(), index, defvalue);
    }

    inline int64_t
    getInt64At(const char *name, uint32_t index, int64_t defvalue)
    {
        return pfc_conf_array_int64at(_block, name, index, defvalue);
    }

    inline int64_t
    getInt64At(const std::string &name, uint32_t index, int64_t defvalue)
    {
        return pfc_conf_array_int64at(_block, name.c_str(), index, defvalue);
    }

    inline uint64_t
    getUint64At(const char *name, uint32_t index, uint64_t defvalue)
    {
        return pfc_conf_array_uint64at(_block, name, index, defvalue);
    }

    inline uint64_t
    getUint64At(const std::string &name, uint32_t index, uint64_t defvalue)
    {
        return pfc_conf_array_uint64at(_block, name.c_str(), index, defvalue);
    }

    inline pfc_long_t
    getLongAt(const char *name, uint32_t index, pfc_long_t defvalue)
    {
        return pfc_conf_array_longat(_block, name, index, defvalue);
    }

    inline pfc_long_t
    getLongAt(const std::string &name, uint32_t index, pfc_long_t defvalue)
    {
        return pfc_conf_array_longat(_block, name.c_str(), index, defvalue);
    }

    inline pfc_ulong_t
    getUlongAt(const char *name, uint32_t index, pfc_ulong_t defvalue)
    {
        return pfc_conf_array_ulongat(_block, name, index, defvalue);
    }

    inline pfc_ulong_t
    getUlongAt(const std::string &name, uint32_t index, pfc_ulong_t defvalue)
    {
        return pfc_conf_array_ulongat(_block, name.c_str(), index, defvalue);
    }

    /*
     * Copy array elements specified by the name and array index range.
     */
    inline int
    getByteRange(const char *name, uint32_t start, uint32_t nelems,
                 uint8_t *buffer)
    {
        return pfc_conf_array_byte_range(_block, name, start, nelems, buffer);
    }

    inline int
    getByteRange(const std::string &name, uint32_t start, uint32_t nelems,
                 uint8_t *buffer)
    {
        return pfc_conf_array_byte_range(_block, name.c_str(), start, nelems,
                                         buffer);
    }

    inline int
    getStringRange(const char *name, uint32_t start, uint32_t nelems,
                   const char **buffer)
    {
        return pfc_conf_array_string_range(_block, name, start, nelems, buffer);
    }

    inline int
    getStringRange(const std::string &name, uint32_t start, uint32_t nelems,
                   const char **buffer)
    {
        return pfc_conf_array_string_range(_block, name.c_str(), start, nelems,
                                           buffer);
    }

    inline int
    getBoolRange(const char *name, uint32_t start, uint32_t nelems,
                 pfc_bool_t *buffer)
    {
        return pfc_conf_array_bool_range(_block, name, start, nelems, buffer);
    }

    inline int
    getBoolRange(const std::string &name, uint32_t start, uint32_t nelems,
                 pfc_bool_t *buffer)
    {
        return pfc_conf_array_bool_range(_block, name.c_str(), start, nelems,
                                         buffer);
    }

    inline int
    getInt32Range(const char *name, uint32_t start, uint32_t nelems,
                  int32_t *buffer)
    {
        return pfc_conf_array_int32_range(_block, name, start, nelems, buffer);
    }

    inline int
    getInt32Range(const std::string &name, uint32_t start, uint32_t nelems,
                  int32_t *buffer)
    {
        return pfc_conf_array_int32_range(_block, name.c_str(), start, nelems,
                                          buffer);
    }

    inline int
    getUint32Range(const char *name, uint32_t start, uint32_t nelems,
                   uint32_t *buffer)
    {
        return pfc_conf_array_uint32_range(_block, name, start, nelems, buffer);
    }

    inline int
    getUint32Range(const std::string &name, uint32_t start, uint32_t nelems,
                   uint32_t *buffer)
    {
        return pfc_conf_array_uint32_range(_block, name.c_str(), start, nelems,
                                           buffer);
    }

    inline int
    getInt64Range(const char *name, uint32_t start, uint32_t nelems,
                  int64_t *buffer)
    {
        return pfc_conf_array_int64_range(_block, name, start, nelems, buffer);
    }

    inline int
    getInt64Range(const std::string &name, uint32_t start, uint32_t nelems,
                  int64_t *buffer)
    {
        return pfc_conf_array_int64_range(_block, name.c_str(), start, nelems,
                                          buffer);
    }

    inline int
    getUint64Range(const char *name, uint32_t start, uint32_t nelems,
                   uint64_t *buffer)
    {
        return pfc_conf_array_uint64_range(_block, name, start, nelems, buffer);
    }

    inline int
    getUint64Range(const std::string &name, uint32_t start, uint32_t nelems,
                   uint64_t *buffer)
    {
        return pfc_conf_array_uint64_range(_block, name.c_str(), start, nelems,
                                           buffer);
    }

    inline int
    getLongRange(const char *name, uint32_t start, uint32_t nelems,
                 pfc_long_t *buffer)
    {
        return pfc_conf_array_long_range(_block, name, start, nelems, buffer);
    }

    inline int
    getLongRange(const std::string &name, uint32_t start, uint32_t nelems,
                 pfc_long_t *buffer)
    {
        return pfc_conf_array_long_range(_block, name.c_str(), start, nelems,
                                         buffer);
    }

    inline int
    getUlongRange(const char *name, uint32_t start, uint32_t nelems,
                  pfc_ulong_t *buffer)
    {
        return pfc_conf_array_ulong_range(_block, name, start, nelems, buffer);
    }

    inline int
    getUlongRange(const std::string &name, uint32_t start, uint32_t nelems,
                  pfc_ulong_t *buffer)
    {
        return pfc_conf_array_ulong_range(_block, name.c_str(), start, nelems,
                                          buffer);
    }

protected:
    /*
     * Create a block handle instance associated with a parameter block
     * specified by the block handle.
     */
    ConfBlock(pfc_cfblk_t block) : _block(block) {}

private:
    /* Parameter block handle. */
    pfc_cfblk_t	_block;
};

}	// core
}	// pfc

#endif	/* !_PFCXX_CONF_HH */
