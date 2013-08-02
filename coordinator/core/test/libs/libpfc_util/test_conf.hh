/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_CONF_H
#define _TEST_CONF_H

/*
 * Test Parameter
 */
#define	TEST_CONF_MAX_ARRAY_SIZE	3
#define	TEST_CONF_MIN_INDEX		0
#define	TEST_CONF_MAX_INDEX		(TEST_CONF_MAX_ARRAY_SIZE - 1)
#define	TEST_CONF_ARRAY_RANGE_ELEM	(TEST_CONF_MAX_ARRAY_SIZE - 1)
#define	TEST_CONF_UNDEF_PARAM		"UNDEFINED_PARAMETER"
#define	TEST_CONF_INVALID_INDEX		100
#define	TEST_CONF_HDL_CNT		2	/* block + map */
#define	TEST_CONF_DUMMY_DATA		0xf
#define	GET_STRING_TEST_COUNT		4
#define	ARY_STRING_TEST_COUNT		5
#define	CPY_STRING_TEST_COUNT		5
#define	CPY_STRING_2_TEST_COUNT		(CPY_STRING_TEST_COUNT + 3)
#define	ARY_CPY_STR_TEST_COUNT		7
#define	ARY_CPY_STR_2_TEST_COUNT	(ARY_CPY_STR_TEST_COUNT + 3)
#define	ARY_RNG_TEST_COUNT		6
#define	ARY_RNG_STR_BUF_SIZE		50

/*
 * Test Data Struct
 *    pfc_conf_array_xxx_range()
 */
typedef struct {
	/* Parameter */
	const char *param;
	/* Expected return value */
	int ext_ret;
	/* Start index */
	uint32_t st_idx;
	/* Element count */
	uint32_t el_cnt;
	/* Buffer Size */
	uint32_t bufsz;
} tc_ary_rng_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	uint8_t ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_byt_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	const char *ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_str_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	pfc_bool_t ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_bol_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	int32_t ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_i32_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	uint32_t ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_ui32_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	int64_t ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_i64_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	uint64_t ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_ui64_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	pfc_long_t ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_lng_t;

typedef struct {
	tc_ary_rng_t ary_rng_cmn;
	/* Expected value */
	pfc_ulong_t ext_ary[TEST_CONF_ARRAY_RANGE_ELEM];
} tc_ary_rng_ulng_t;

typedef struct {
	/* Parameter */
	const char *param;
	/* Expected value */
	const char *expect;
} test_case_get_str_t;

typedef struct {
	/* Parameter */
	const char *param;
	/* Expected value */
	const char *expect;
	/* Array index */
	uint32_t index;
} test_case_ary_str_t;

typedef struct {
	/* Parameter */
	const char *param;
	/* Expected value */
	const char *expect;
	/* Buffer Size */
	uint32_t bufsz;
} test_case_cpy_str_t;

typedef struct {
	/* Parameter */
	const char *param;
	/* Expected value */
	const char *expect;
	/* Array index */
	uint32_t index;
	/* Buffer Size */
	uint32_t bufsz;
} tc_ary_cpy_str_t;

#endif  /* !_TEST_CONF_H */
