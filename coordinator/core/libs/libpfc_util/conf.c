/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * conf.c - APIs to access PFC configuration file.
 */

#include <string.h>
#include <pfc/refptr.h>
#include "conf_impl.h"

/*
 * Determine whether the given array range is valid or not.
 */
#define	CONF_ARRAY_VALID_RANGE(param, start, nelems)	\
	((start) < PFC_CF_MAX_ARRAY_SIZE &&		\
	 (nelems) <= PFC_CF_MAX_ARRAY_SIZE &&		\
	 (start) + (nelems) <= (param)->cp_nelems)

/*
 * Internal prototypes.
 */
static conf_param_t	*conf_get_param(pfc_cfblk_t block,
					const char *PFC_RESTRICT name);
static pfc_cfblk_t	conf_get_block(pfc_conf_t PFC_RESTRICT conf,
				       const char *PFC_RESTRICT bname);
static pfc_cfblk_t	conf_get_map(pfc_conf_t PFC_RESTRICT conf,
				     const char *mname, const char *key);
static int		conf_get_mapkeys(pfc_conf_t PFC_RESTRICT conf,
					 const char *PFC_RESTRICT mname,
					 pfc_listm_t *PFC_RESTRICT keysp);

/*
 * pfc_cfblk_t
 * pfc_conf_get_block(pfc_conf_t PFC_RESTRICT conf,
 *		      const char *PFC_RESTRICT bname)
 *	Return parameter block handle associated with the specified block name.
 *
 *	Returned parameter block handle is associated with a bunch of
 *	parameters defined in the configuration file like this:
 *
 *	bname {
 *		int_value = 1;
 *		...
 *	}
 *
 * Calling/Exit State:
 *	Upon successful completion, a parameter block handle is returned.
 *	On error, PFC_CFBLK_INVALID is returned.
 */
pfc_cfblk_t
pfc_conf_get_block(pfc_conf_t PFC_RESTRICT conf, const char *PFC_RESTRICT bname)
{
	pfc_cfblk_t	block;

	CONF_PARSER_RDLOCK();
	block = conf_get_block(conf, bname);
	CONF_PARSER_UNLOCK();

	return block;
}

/*
 * pfc_cfblk_t
 * pfc_conf_get_map(pfc_conf_t PFC_RESTRICT conf, const char *mname,
 *		    const char *key)
 *	Return parameter block handle associated with the specified map name
 *	and key.
 *
 *	Returned parameter block handle is associated with a bunch of
 *	parameters defined in the configuration file like this:
 *
 *	mname "key" {
 *		int_value = 1;
 *		...
 *	}
 *
 * Calling/Exit State:
 *	Upon successful completion, a parameter block handle is returned.
 *	On error, PFC_CFBLK_INVALID is returned.
 */
pfc_cfblk_t
pfc_conf_get_map(pfc_conf_t PFC_RESTRICT conf, const char *mname,
		 const char *key)
{
	pfc_cfblk_t	block;

	CONF_PARSER_RDLOCK();
	block = conf_get_map(conf, mname, key);
	CONF_PARSER_UNLOCK();

	return block;
}

/*
 * int
 * pfc_conf_get_mapkeys(pfc_conf_t PFC_RESTRICT conf,
 *			const char *PFC_RESTRICT mname,
 *			pfc_listm_t *PFC_RESTRICT keysp)
 *	Create a list model instance which contains all map keys in the
 *	parameter map specified by the map name `mname'.
 *
 *	Map keys are stored to the list model instance as refptr string
 *	object.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list model instance is set to the
 *	buffer pointed by `keysp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_conf_get_mapkeys(pfc_conf_t PFC_RESTRICT conf,
		     const char *PFC_RESTRICT mname,
		     pfc_listm_t *PFC_RESTRICT keysp)
{
	int		err;

	CONF_PARSER_RDLOCK();
	err = conf_get_mapkeys(conf, mname, keysp);
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * pfc_cfblk_t
 * pfc_sysconf_get_block(const char *bname)
 *	Return parameter block handle for the PFC system configuration file,
 *	which is associated with the specified block name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a parameter block handle is returned.
 *	On error, PFC_CFBLK_INVALID is returned.
 */
pfc_cfblk_t
pfc_sysconf_get_block(const char *bname)
{
	pfc_cfblk_t	block;

	CONF_PARSER_RDLOCK();
	block = conf_get_block(pfcd_conf, bname);
	CONF_PARSER_UNLOCK();

	return block;
}

/*
 * pfc_cfblk_t
 * pfc_sysconf_get_map(const char *mname, const char *key)
 *	Return parameter block handle for the PFC system configuration file,
 *	which is associated with the specified map name and key.
 *
 * Calling/Exit State:
 *	Upon successful completion, a parameter block handle is returned.
 *	On error, PFC_CFBLK_INVALID is returned.
 */
pfc_cfblk_t
pfc_sysconf_get_map(const char *mname, const char *key)
{
	pfc_cfblk_t	block;

	CONF_PARSER_RDLOCK();
	block = conf_get_map(pfcd_conf, mname, key);
	CONF_PARSER_UNLOCK();

	return block;
}

/*
 * int
 * pfc_sysconf_get_mapkeys(const char *PFC_RESTRICT mname,
 *			   pfc_listm_t *PFC_RESTRICT keysp)
 *	Create a list model instance which contains all map keys in the
 *	parameter map defined in PFC system configuration file.
 *
 *	Map keys are stored to the list model instance as refptr string
 *	object.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list model instance is set to the
 *	buffer pointed by `keysp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_sysconf_get_mapkeys(const char *PFC_RESTRICT mname,
			pfc_listm_t *PFC_RESTRICT keysp)
{
	int		err;

	CONF_PARSER_RDLOCK();
	err = conf_get_mapkeys(pfcd_conf, mname, keysp);
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * uint8_t
 * pfc_conf_get_byte(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *		     uint8_t defvalue)
 *	Return a byte parameter value associated with the specified name in
 *	the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	A byte value associated with the specified name is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 */
uint8_t
pfc_conf_get_byte(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		  uint8_t defvalue)
{
	uint8_t		value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_BYTE(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		value = CONF_VALUE_BYTE(param);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * const char *
 * pfc_conf_get_string(pfc_cfblk_t block, const char *name,
 *		       const char *defvalue)
 *	Return a string parameter value associated with the specified name in
 *	the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	A string value associated with the specified name is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 *
 * Remarks:
 *	The returned pointer will be released when pfc_conf_close() is called.
 *	If you want to keep string after the configuration file is closed,
 *	use pfc_conf_copy_string() instead.
 */
const char *
pfc_conf_get_string(pfc_cfblk_t block, const char *name, const char *defvalue)
{
	const char	*value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_STRING(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		pfc_refptr_t	*ref = CONF_VALUE_STRING(param);

		value = pfc_refptr_string_value(ref);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * pfc_bool_t
 * pfc_conf_get_bool(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *		     pfc_bool_t defvalue)
 *	Return a boolean parameter value associated with the specified name
 *	in the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	A boolean value associated with the specified name is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 */
pfc_bool_t
pfc_conf_get_bool(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		  pfc_bool_t defvalue)
{
	pfc_bool_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_BOOL(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		value = CONF_VALUE_BOOL(param);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * int32_t
 * pfc_conf_get_int32(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *		      int32_t defvalue)
 *	Return a signed 32-bit integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	An int32 value associated with the specified name is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 */
int32_t
pfc_conf_get_int32(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		   int32_t defvalue)
{
	int32_t		value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_INT32(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		value = CONF_VALUE_INT32(param);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * uint32_t
 * pfc_conf_get_uint32(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *		       uint32_t defvalue)
 *	Return an unsigned 32-bit integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	An uint32 value associated with the specified name is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 */
uint32_t
pfc_conf_get_uint32(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		    uint32_t defvalue)
{
	uint32_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_UINT32(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		value = CONF_VALUE_UINT32(param);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * int64_t
 * pfc_conf_get_int64(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *		      int64_t defvalue)
 *	Return a signed 64-bit integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	An int64 value associated with the specified name is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 */
int64_t
pfc_conf_get_int64(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		   int64_t defvalue)
{
	int64_t		value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_INT64(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		value = CONF_VALUE_INT64(param);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * uint64_t
 * pfc_conf_get_uint64(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *		       uint64_t defvalue)
 *	Return an unsigned 64-bit integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	An uint64 value associated with the specified name is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 */
uint64_t
pfc_conf_get_uint64(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		    uint64_t defvalue)
{
	uint64_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_UINT64(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		value = CONF_VALUE_UINT64(param);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * pfc_long_t
 * pfc_conf_get_long(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *		     pfc_long_t defvalue)
 *	Return a signed long integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	A long integer value associated with the specified name is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 */
pfc_long_t
pfc_conf_get_long(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		  pfc_long_t defvalue)
{
	pfc_long_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_LONG(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		value = CONF_VALUE_LONG(param);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * pfc_ulong_t
 * pfc_conf_get_ulong(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *		      pfc_ulong_t defvalue)
 *	Return an unsigned long integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 * Calling/Exit State:
 *	An unsigned long integer  value associated with the specified name
 *	is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid.
 */
pfc_ulong_t
pfc_conf_get_ulong(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		   pfc_ulong_t defvalue)
{
	pfc_ulong_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_ULONG(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		value = CONF_VALUE_ULONG(param);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * void
 * pfc_conf_copy_string(pfc_cfblk_t block, const char *name,
 *			const char *defvalue, char *PFC_RESTRICT buffer,
 *			uint32_t bufsize)
 *	Copy a string parameter value associated with the specified name
 *	into the specified buffer.
 *
 *	The parameter specified by name must not be an array parameter.
 *
 *	If the parameter is not defined or the specified handle is invalid,
 *	a string specified to `defvalue' is copied. If `defvalue' is NULL,
 *	an empty string is copied.
 *
 *	This function guarantees that the string copied into `buf' is
 *	terminated by '\0', unless `bufsize' is zero.
 */
void
pfc_conf_copy_string(pfc_cfblk_t block, const char *name, const char *defvalue,
		     char *PFC_RESTRICT buffer, uint32_t bufsize)
{
	conf_param_t	*param;
	const char	*src;
	uint32_t	srclen;

	if (PFC_EXPECT_FALSE(bufsize == 0)) {
		return;
	}

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_STRING(param) &&
			    !CONF_VALUE_IS_ARRAY(param))) {
		pfc_refptr_t	*ref = CONF_VALUE_STRING(param);

		src = pfc_refptr_string_value(ref);
		srclen = pfc_refptr_string_length(ref);
	}
	else {
		src = defvalue;
		if (PFC_EXPECT_FALSE(src == NULL)) {
			*buffer = '\0';
			goto out;
		}
		srclen = strlen(src);
	}

	if (bufsize > srclen) {
		strcpy(buffer, src);
	}
	else {
		memcpy(buffer, src, bufsize - 1);
		*(buffer + bufsize - 1) = '\0';
	}
out:
	CONF_PARSER_UNLOCK();
}

/*
 * pfc_bool_t
 * pfc_conf_is_defined(pfc_cfblk_t block, const char *PFC_RESTRICT name)
 *	Determine whether the parameter associated with the specified name
 *	exists in the parameter block or not.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the parameter is defined in the block.
 *	Otherwise PFC_FALSE is returned, including the specified block handle
 *	is invalid.
 */
pfc_bool_t
pfc_conf_is_defined(pfc_cfblk_t block, const char *PFC_RESTRICT name)
{
	pfc_bool_t	ret;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();
	param = conf_get_param(block, name);
	ret = (PFC_EXPECT_TRUE(param != NULL)) ? PFC_TRUE : PFC_FALSE;
	CONF_PARSER_UNLOCK();

	return ret;
}

/*
 * pfc_bool_t
 * pfc_conf_is_array(pfc_cfblk_t block, const char *PFC_RESTRICT name)
 *	Determine whether the parameter associated with the specified name
 *	in the parameter block is array parameter or not.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the parameter is array parameter.
 *	Otherwise PFC_FALSE is returned, including the specified block handle
 *	is invalid.
 */
pfc_bool_t
pfc_conf_is_array(pfc_cfblk_t block, const char *PFC_RESTRICT name)
{
	pfc_bool_t	ret;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();
	param = conf_get_param(block, name);
	ret = (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_ARRAY(param)))
		? PFC_TRUE : PFC_FALSE;
	CONF_PARSER_UNLOCK();

	return ret;
}

/*
 * int
 * pfc_conf_array_size(pfc_cfblk_t block, const char *name)
 *	Return number of array elements defined by the parameter associated
 *	with the specified name.
 *
 * Calling/Exit State:
 *	If an array parameter specified by the name is defined, the number of
 *	array elements is returned.
 *
 *	If the specified parameter block handle is invalid, or the specified
 *	parameter is not defined, or it is not an array parameter, a negative
 *	value is returned.
 */
int
pfc_conf_array_size(pfc_cfblk_t block, const char *name)
{
	int	size;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();
	param = conf_get_param(block, name);
	size = (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_ARRAY(param)))
		? (int)param->cp_nelems : -1;
	CONF_PARSER_UNLOCK();

	return size;
}

/*
 * uint8_t
 * pfc_conf_array_byteat(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			 uint32_t index, uint8_t defvalue)
 *	Return a byte parameter value associated with the specified name in
 *	the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	A byte value associated with the specified name and array index is
 *	returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 */
uint8_t
pfc_conf_array_byteat(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		      uint32_t index, uint8_t defvalue)
{
	uint8_t		value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_BYTE(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		uint8_t	*array = CONF_VALUE_BYTE_ARRAY(param);

		value = *(array + index);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * const char *
 * pfc_conf_array_stringat(pfc_cfblk_t block, const char *name, uint32_t index,
 *			   const char *defvalue defvalue)
 *	Return a string parameter value associated with the specified name in
 *	the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	A string value associated with the specified name and array index is
 *	returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 *
 * Remarks:
 *	The returned pointer will be released when pfc_conf_close() is called.
 *	If you want to keep string after the configuration file is closed,
 *	use pfc_conf_array_copy_string() instead.
 */
const char *
pfc_conf_array_stringat(pfc_cfblk_t block, const char *name, uint32_t index,
			const char *defvalue)
{
	const char	*value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_STRING(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		pfc_refptr_t	**array = CONF_VALUE_STRING_ARRAY(param);
		pfc_refptr_t	*ref = *(array + index);

		value = pfc_refptr_string_value(ref);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * pfc_bool_t
 * pfc_conf_array_boolat(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			 uint32_t index, pfc_bool_t defvalue)
 *	Return a boolean parameter value associated with the specified name in
 *	the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	A boolean value associated with the specified name and array index is
 *	returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 */
pfc_bool_t
pfc_conf_array_boolat(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		      uint32_t index, pfc_bool_t defvalue)
{
	pfc_bool_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_BOOL(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		pfc_bool_t	*array = CONF_VALUE_BOOL_ARRAY(param);

		value = *(array + index);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * int32_t
 * pfc_conf_array_int32at(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			  uint32_t index, int32_t defvalue)
 *	Return a signed 32-bit integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	An int32 value associated with the specified name and array index is
 *	returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 */
int32_t
pfc_conf_array_int32at(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		       uint32_t index, int32_t defvalue)
{
	int32_t		value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_INT32(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		int32_t	*array = CONF_VALUE_INT32_ARRAY(param);

		value = *(array + index);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * uint32_t
 * pfc_conf_array_uint32at(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			   uint32_t index, uint32_t defvalue)
 *	Return an unsigned 32-bit integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	An uint32 value associated with the specified name and array index is
 *	returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 */
uint32_t
pfc_conf_array_uint32at(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			uint32_t index, uint32_t defvalue)
{
	uint32_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_UINT32(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		uint32_t	*array = CONF_VALUE_UINT32_ARRAY(param);

		value = *(array + index);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * int64_t
 * pfc_conf_array_int64at(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			  uint32_t index, int64_t defvalue)
 *	Return a signed 64-bit integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	An int64 value associated with the specified name and array index is
 *	returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 */
int64_t
pfc_conf_array_int64at(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		       uint32_t index, int64_t defvalue)
{
	int64_t		value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_INT64(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		int64_t	*array = CONF_VALUE_INT64_ARRAY(param);

		value = *(array + index);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * uint64_t
 * pfc_conf_array_uint64at(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			   uint32_t index, uint64_t defvalue)
 *	Return an unsigned 64-bit integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	An uint64 value associated with the specified name and array index is
 *	returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 */
uint64_t
pfc_conf_array_uint64at(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			uint32_t index, uint64_t defvalue)
{
	uint64_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_UINT64(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		uint64_t	*array = CONF_VALUE_UINT64_ARRAY(param);

		value = *(array + index);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * pfc_long_t
 * pfc_conf_array_longat(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			 uint32_t index, pfc_long_t defvalue)
 *	Return a signed long integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	A long integer value associated with the specified name and array
 *	index is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 */
pfc_long_t
pfc_conf_array_longat(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		      uint32_t index, pfc_long_t defvalue)
{
	pfc_long_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_LONG(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		pfc_long_t	*array = CONF_VALUE_LONG_ARRAY(param);

		value = *(array + index);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * pfc_ulong_t
 * pfc_conf_array_ulongat(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			  uint32_t index, pfc_ulong_t defvalue)
 *	Return an unsigned long integer parameter value associated with
 *	the specified name in the specified parameter block.
 *
 *	This function returns an array element at the specified index.
 *
 * Calling/Exit State:
 *	An unsigned long integer value associated with the specified name
 *	and array index is returned.
 *	A value specified to `defvalue' is returned if the parameter is not
 *	defined, or the specified handle is invalid, or the parameter is not
 *	an array, or invalid array index is specified.
 */
pfc_ulong_t
pfc_conf_array_ulongat(pfc_cfblk_t block, const char *PFC_RESTRICT name,
		       uint32_t index, pfc_ulong_t defvalue)
{
	pfc_ulong_t	value;
	conf_param_t	*param;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_ULONG(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		pfc_ulong_t	*array = CONF_VALUE_ULONG_ARRAY(param);

		value = *(array + index);
	}
	else {
		value = defvalue;
	}

	CONF_PARSER_UNLOCK();

	return value;
}

/*
 * void
 * pfc_conf_array_copy_string(pfc_cfblk_t block, const char *name,
 *			      uint32_t index, const char *defvalue,
 *			      char *PFC_RESTRICT buffer, uint32_t bufsize)
 *	Copy a string parameter value associated with the specified name
 *	into the specified buffer.
 *
 *	This function copies an array element at the specified index.
 *
 *	If the parameter is not defined, or the specified handle is invalid,
 *	or the specified array index is invalid, a string specified to
 *	`defvalue' is copied. If `defvalue' is NULL, an empty string is copied.
 *
 *	This function guarantees that the string copied into `buf' is
 *	terminated by '\0', unless `bufsize' is zero.
 */
void
pfc_conf_array_copy_string(pfc_cfblk_t block, const char *name, uint32_t index,
			   const char *defvalue, char *PFC_RESTRICT buffer,
			   uint32_t bufsize)
{
	conf_param_t	*param;
	const char	*src;
	uint32_t	srclen;

	if (PFC_EXPECT_FALSE(bufsize == 0)) {
		return;
	}

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_TRUE(param != NULL && CONF_VALUE_IS_STRING(param) &&
			    CONF_VALUE_IS_ARRAY(param) &&
			    index < param->cp_nelems)) {
		pfc_refptr_t	**array = CONF_VALUE_STRING_ARRAY(param);
		pfc_refptr_t	*ref = *(array + index);

		src = pfc_refptr_string_value(ref);
		srclen = pfc_refptr_string_length(ref);
	}
	else {
		src = defvalue;
		if (PFC_EXPECT_FALSE(src == NULL)) {
			*buffer = '\0';
			goto out;
		}
		srclen = strlen(src);
	}

	if (bufsize > srclen) {
		strcpy(buffer, src);
	}
	else {
		memcpy(buffer, src, bufsize - 1);
		*(buffer + bufsize - 1) = '\0';
	}

out:
	CONF_PARSER_UNLOCK();
}

/*
 * int
 * pfc_conf_array_byte_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			     uint32_t start, uint32_t nelems,
 *			     uint8_t *PFC_RESTRICT buffer)
 *	Copy byte array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' byte elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not a byte array.
 *	ERANGE is returned if the specified array range is invalid.
 */
int
pfc_conf_array_byte_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			  uint32_t start, uint32_t nelems,
			  uint8_t *PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	uint8_t		*array, *endp, *dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_BYTE(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_BYTE_ARRAY(param) + start;
	endp  = CONF_VALUE_BYTE_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		*dst = *array;
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * int
 * pfc_conf_array_string_range(pfc_cfblk_t block,
 *			       const char *PFC_RESTRICT name, uint32_t start,
 *			       uint32_t nelems,
 *			       const char **PFC_RESTRICT buffer)
 *	Copy string array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' string pointers.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not a string array.
 *	ERANGE is returned if the specified array range is invalid.
 *
 * Remarks:
 *	All pointers in the returned array will be released when
 *	pfc_conf_close() is called.
 */
int
pfc_conf_array_string_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			    uint32_t start, uint32_t nelems,
			    const char **PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	pfc_refptr_t	**array, **endp;
	const char	**dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_STRING(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_STRING_ARRAY(param) + start;
	endp  = CONF_VALUE_STRING_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		pfc_refptr_t	*ref = *array;

		*dst = pfc_refptr_string_value(ref);
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * int
 * pfc_conf_array_bool_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			     uint32_t start, uint32_t nelems,
 *			     pfc_bool_t *PFC_RESTRICT buffer)
 *	Copy boolean array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' pfc_bool_t elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not a boolean array.
 *	ERANGE is returned if the specified array range is invalid.
 */
int
pfc_conf_array_bool_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			  uint32_t start, uint32_t nelems,
			  pfc_bool_t *PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	pfc_bool_t	*array, *endp, *dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_BOOL(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_BOOL_ARRAY(param) + start;
	endp  = CONF_VALUE_BOOL_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		*dst = *array;
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * int
 * pfc_conf_array_int32_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			      uint32_t start, uint32_t nelems,
 *			      int32_t *PFC_RESTRICT buffer)
 *	Copy INT32 array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' int32_t elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not an INT32 array.
 *	ERANGE is returned if the specified array range is invalid.
 */
int
pfc_conf_array_int32_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			   uint32_t start, uint32_t nelems,
			   int32_t *PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	int32_t		*array, *endp, *dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_INT32(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_INT32_ARRAY(param) + start;
	endp  = CONF_VALUE_INT32_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		*dst = *array;
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * int
 * pfc_conf_array_uint32_range(pfc_cfblk_t block,
 *			       const char *PFC_RESTRICT name, uint32_t start,
 *			       uint32_t nelems, uint32_t *PFC_RESTRICT buffer)
 *	Copy UINT32 array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' uint32_t elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not an UINT32 array.
 *	ERANGE is returned if the specified array range is invalid.
 */
int
pfc_conf_array_uint32_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			    uint32_t start, uint32_t nelems,
			    uint32_t *PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	uint32_t	*array, *endp, *dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_UINT32(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_UINT32_ARRAY(param) + start;
	endp  = CONF_VALUE_UINT32_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		*dst = *array;
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * int
 * pfc_conf_array_int64_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			      uint32_t start, uint32_t nelems,
 *			      int64_t *PFC_RESTRICT buffer)
 *	Copy INT64 array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' int64_t elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not an INT64 array.
 *	ERANGE is returned if the specified array range is invalid.
 */
int
pfc_conf_array_int64_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			   uint32_t start, uint32_t nelems,
			   int64_t *PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	int64_t		*array, *endp, *dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_INT64(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_INT64_ARRAY(param) + start;
	endp  = CONF_VALUE_INT64_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		*dst = *array;
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * int
 * pfc_conf_array_uint64_range(pfc_cfblk_t block,
 *			       const char *PFC_RESTRICT name, uint32_t start,
 *			       uint32_t nelems, uint64_t *PFC_RESTRICT buffer)
 *	Copy UINT64 array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' uint64_t elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not an UINT64 array.
 *	ERANGE is returned if the specified array range is invalid.
 */
int
pfc_conf_array_uint64_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			    uint32_t start, uint32_t nelems,
			    uint64_t *PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	uint64_t	*array, *endp, *dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_UINT64(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_UINT64_ARRAY(param) + start;
	endp  = CONF_VALUE_UINT64_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		*dst = *array;
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * int
 * pfc_conf_array_long_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			     uint32_t start, uint32_t nelems,
 *			     pfc_long_t *PFC_RESTRICT buffer)
 *	Copy LONG array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' pfc_long_t elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not a LONG array.
 *	ERANGE is returned if the specified array range is invalid.
 */
int
pfc_conf_array_long_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			  uint32_t start, uint32_t nelems,
			  pfc_long_t *PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	pfc_long_t	*array, *endp, *dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_LONG(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_LONG_ARRAY(param) + start;
	endp  = CONF_VALUE_LONG_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		*dst = *array;
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * int
 * pfc_conf_array_ulong_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
 *			      uint32_t start, uint32_t nelems,
 *			      pfc_ulong_t *PFC_RESTRICT buffer)
 *	Copy ULONG array elements to the specified buffer.
 *
 *	This function `nelems' array elements from array index specified
 *	by `start'. The caller must guarantee that a storage pointed by
 *	`buffer' has enough memory to store `nelems' pfc_ulong_t elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the specified parameter is not found.
 *	EINVAL is returned if the specified parameter is not an ULONG array.
 *	ERANGE is returned if the specified array range is invalid.
 */
int
pfc_conf_array_ulong_range(pfc_cfblk_t block, const char *PFC_RESTRICT name,
			   uint32_t start, uint32_t nelems,
			   pfc_ulong_t *PFC_RESTRICT buffer)
{
	conf_param_t	*param;
	pfc_ulong_t	*array, *endp, *dst = buffer;
	int	err = 0;

	CONF_PARSER_RDLOCK();

	param = conf_get_param(block, name);
	if (PFC_EXPECT_FALSE(param == NULL)) {
		err = ENOENT;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_VALUE_IS_ULONG(param) ||
			     !CONF_VALUE_IS_ARRAY(param))) {
		err = EINVAL;
		goto out;
	}
	if (PFC_EXPECT_FALSE(!CONF_ARRAY_VALID_RANGE(param, start, nelems))) {
		err = ERANGE;
		goto out;
	}

	array = CONF_VALUE_ULONG_ARRAY(param) + start;
	endp  = CONF_VALUE_ULONG_ARRAY(param) + start + nelems;

	for (; array < endp; array++, dst++) {
		*dst = *array;
	}
	PFC_ASSERT(dst == buffer + nelems);

out:
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * static conf_param_t *
 * conf_get_param(pfc_cfblk_t block, const char *PFC_RESTRICT name)
 *	Retrieve parameter instance associated with the specified parameter
 *	name.
 *
 * Calling/Exit State:
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called with holding parser lock in reader or
 *	writer mode.
 */
static conf_param_t *
conf_get_param(pfc_cfblk_t block, const char *PFC_RESTRICT name)
{
	if (PFC_EXPECT_TRUE(block != PFC_CFBLK_INVALID && name != NULL)) {
		conf_block_t	*bp;

		bp = conf_hdlmap_get(block);
		if (PFC_EXPECT_TRUE(bp != NULL)) {
			return conf_pmap_get(&bp->cb_params, name);
		}
	}

	return NULL;
}

/*
 * static pfc_cfblk_t
 * conf_get_block(pfc_conf_t PFC_RESTRICT conf,
 *		      const char *PFC_RESTRICT bname)
 *	Return parameter block handle associated with the block name `bname'
 *	in the configuration file handle `conf'.
 *
 *	Returned parameter block handle is associated with a bunch of
 *	parameters defined in the configuration file like this:
 *
 *	bname {
 *		int_value = 1;
 *		...
 *	}
 *
 * Calling/Exit State:
 *	Upon successful completion, a parameter block handle is returned.
 *	On error, PFC_CFBLK_INVALID is returned.
 *
 * Remarks:
 *	This function must be called with holding parse lock in reader or
 *	writer mode.
 */
static pfc_cfblk_t
conf_get_block(pfc_conf_t PFC_RESTRICT conf, const char *PFC_RESTRICT bname)
{
	pfc_cfblk_t	block;
	conf_file_t	*cfp;
	conf_block_t	*bp;

	if (PFC_EXPECT_FALSE(conf == PFC_CONF_INVALID || bname == NULL)) {
		return PFC_CFBLK_INVALID;
	}

	cfp = CONF_FILE_PTR(conf);
	bp = conf_blkmap_get(&cfp->cf_blocks, bname);
	block = (PFC_EXPECT_TRUE(bp != NULL))
		? bp->cb_handle : PFC_CFBLK_INVALID;

	return block;
}

/*
 * static pfc_cfblk_t
 * conf_get_map(pfc_conf_t PFC_RESTRICT conf, const char *mname,
 *		const char *key)
 *	Return parameter block handle associated with the map name `mname'
 *	and the map key `key' in the configuration file handle `conf'.
 *
 *	Returned parameter block handle is associated with a bunch of
 *	parameters defined in the configuration file like this:
 *
 *	mname "key" {
 *		int_value = 1;
 *		...
 *	}
 *
 * Calling/Exit State:
 *	Upon successful completion, a parameter block handle is returned.
 *	On error, PFC_CFBLK_INVALID is returned.
 */
static pfc_cfblk_t
conf_get_map(pfc_conf_t PFC_RESTRICT conf, const char *mname, const char *key)
{
	pfc_cfblk_t	block;
	conf_file_t	*cfp;
	conf_blkmap_t	*blkmap;

	if (PFC_EXPECT_FALSE(conf == PFC_CONF_INVALID || mname == NULL ||
			     key == NULL)) {
		return PFC_CFBLK_INVALID;
	}

	cfp = CONF_FILE_PTR(conf);

	blkmap = conf_imap_getmap(&cfp->cf_maps, mname);
	if (PFC_EXPECT_TRUE(blkmap != NULL)) {
		conf_block_t	*bp;

		bp = conf_blkmap_get(blkmap, key);
		block = (PFC_EXPECT_TRUE(bp != NULL))
			? bp->cb_handle : PFC_CFBLK_INVALID;
	}
	else {
		block = PFC_CFBLK_INVALID;
	}

	return block;
}

/*
 * static int
 * conf_get_mapkeys(pfc_conf_t PFC_RESTRICT conf,
 *		    const char *PFC_RESTRICT mname,
 *		    pfc_listm_t *PFC_RESTRICT keysp)
 *	Create a list model instance which contains all map keys in the
 *	parameter map specified by configuration file handle `conf' and
 *	the map name `mname'.
 *
 *	Map keys are stored to the list model instance as refptr string
 *	object.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list model instance is set to the
 *	buffer pointed by `keysp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding parse lock in reader or
 *	writer mode.
 */
static int
conf_get_mapkeys(pfc_conf_t PFC_RESTRICT conf, const char *PFC_RESTRICT mname,
		 pfc_listm_t *PFC_RESTRICT keysp)
{
	conf_file_t	*cfp;
	conf_imapent_t	*imp;
	pfc_listm_t	ret, keys;
	int		err, i, size;

	if (PFC_EXPECT_FALSE(conf == PFC_CONF_INVALID || mname == NULL ||
			     keysp == NULL)) {
		return EINVAL;
	}

	cfp = CONF_FILE_PTR(conf);
	imp = conf_imap_get(&cfp->cf_maps, mname);
	if (PFC_EXPECT_FALSE(imp == NULL)) {
		return ENOENT;
	}

	keys = imp->cie_keys;
	size = pfc_listm_get_size(keys);
	PFC_ASSERT(size >= 0);

	/* Create a new string vector to store map keys. */
	err = pfc_vector_create_ref(&ret, pfc_refptr_string_ops(), size, 1);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Put all map keys to the vector. */
	for (i = 0; i < size; i++) {
		pfc_refptr_t	*rkey;

		PFC_ASSERT_INT(pfc_listm_getat(keys, i, (pfc_cptr_t *)&rkey),
			       0);
		PFC_ASSERT(conf_blkmap_get(&imp->cie_blkmap,
					   pfc_refptr_string_value(rkey))
			   != NULL);
		err = pfc_listm_push_tail(ret, rkey);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_listm_destroy(ret);

			return err;
		}
	}

	*keysp = ret;

	return 0;
}
