/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * uint.c - Utilities to handle unsigned integer.
 */

#include <stdlib.h>
#include <string.h>
#include <org_opendaylight_vtn_core_util_UnsignedInteger.h>
#include <pfc/strtoint.h>
#include "pfc_util_jni.h"

/*
 * Internal prototypes.
 */
static const char	*uint_hexstring(JNIEnv *env, const char *str);
static void		divzero(JNIEnv *env);

/*
 * JNIEXPORT jfloat JNICALL
 * Java_org_opendaylight_vtn_core_util_UnsignedInteger_floatValue(
 *	JNIEnv *env, jclass cls, jlong value)
 *
 *	Convert the specified long value to float.
 *	The specified value is treated as unsigned integer.
 *
 * Calling/Exit State:
 *	A converted value is returned.
 */
JNIEXPORT jfloat JNICALL
Java_org_opendaylight_vtn_core_util_UnsignedInteger_floatValue(
	JNIEnv *env, jclass cls, jlong value)
{
	uint64_t	u64 = (uint64_t)value;

	return (float)u64;
}

/*
 * JNIEXPORT jdouble JNICALL
 * Java_org_opendaylight_vtn_core_util_UnsignedInteger_doubleValue(
 *	JNIEnv *env, jclass cls, jlong value)
 *
 *	Convert the specified long value to double.
 *	The specified value is treated as unsigned integer.
 *
 * Calling/Exit State:
 *	A converted value is returned.
 */
JNIEXPORT jdouble JNICALL
Java_org_opendaylight_vtn_core_util_UnsignedInteger_doubleValue(
	JNIEnv *env, jclass cls, jlong value)
{
	uint64_t	u64 = (uint64_t)value;

	return (double)u64;
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_util_UnsignedInteger_parseInt(
 *	JNIEnv *env, jclass cls, jstring str)
 *
 *	Parse the specified string as unsigned 32-bit integer.
 *
 * Calling/Exit State:
 *	Upon successful completion, parsed value is returned.
 *	Otherwise an exception is thrown and zero is returned.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_util_UnsignedInteger_parseInt(
	JNIEnv *env, jclass cls, jstring str)
{
	const char	*string, *s;
	uint32_t	value;

	if (str == NULL) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "str is null.");

		return 0;
	}

	string = pjni_string_get(env, str);
	if (PFC_EXPECT_FALSE(string == NULL)) {
		return 0;
	}

	s = uint_hexstring(env, string);
	if (PFC_EXPECT_TRUE(s != NULL)) {
		int	err;

		err = pfc_strtou32(s, &value);
		if (PFC_EXPECT_FALSE(err != 0)) {
			const char	*msg = (err == ERANGE)
				? "Out of range" : "Invalid string";

			pjni_throw(env, PJNI_CLASS(NumberFormatException),
				   "%s: %s", msg, string);
			value = 0;
		}

		if (s != string) {
			free((void *)s);
		}
	}

	pjni_string_release(env, str, string);

	return (jint)value;
}

/*
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_util_UnsignedInteger_parseLong(
 *	JNIEnv *env, jclass class, jstring str)
 *
 *	Parse the specified string as unsigned 64-bit integer.
 *
 * Calling/Exit State:
 *	Upon successful completion, parsed value is returned.
 *	Otherwise an exception is thrown and zero is returned.
 */
JNIEXPORT jlong JNICALL
Java_org_opendaylight_vtn_core_util_UnsignedInteger_parseLong(
	JNIEnv *env, jclass class, jstring str)
{
	const char	*string, *s;
	uint64_t	value;

	if (str == NULL) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "str is null.");

		return 0;
	}

	string = pjni_string_get(env, str);
	if (PFC_EXPECT_FALSE(string == NULL)) {
		return 0;
	}

	s = uint_hexstring(env, string);
	if (PFC_EXPECT_TRUE(s != NULL)) {
		int	err;

		err = pfc_strtou64(s, &value);
		if (PFC_EXPECT_FALSE(err != 0)) {
			const char	*msg = (err == ERANGE)
				? "Out of range" : "Invalid string";

			pjni_throw(env, PJNI_CLASS(NumberFormatException),
				   "%s: %s", msg, string);
			value = 0;
		}

		if (s != string) {
			free((void *)s);
		}
	}

	pjni_string_release(env, str, string);

	return (jlong)value;
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_util_UnsignedInteger_toString(
 *	JNIEnv *env, jclass cls, jlong value)
 *
 *	Return a new decimal string which represents the specified long value.
 *	The value is treated as unsigned integer.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to string object
 *	is returned. Otherwise an exception is thrown and NULL is returned.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_util_UnsignedInteger_toString(
	JNIEnv *env, jclass cls, jlong value)
{
	char	buf[32];

	snprintf(buf, sizeof(buf), "%" PFC_PFMT_u64, (uint64_t)value);

	return pjni_newstring(env, buf);
}

/*
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_util_UnsignedInteger_divide(
 *	JNIEnv *env, jclass cls, jlong dividend, jlong divisor)
 *
 *	Divide the specified unsigned 64-bit integer value.
 *
 * Calling/Exit State:
 *	Upon successful completion, the quotient of `dividend' divided by
 *	`divisor' is returned.
 *	If `divisor' is zero, an ArithmeticException is thrown and zero is
 *	returned.
 */
JNIEXPORT jlong JNICALL
Java_org_opendaylight_vtn_core_util_UnsignedInteger_divide(
	JNIEnv *env, jclass cls, jlong dividend, jlong divisor)
{
	uint64_t	dvd = (uint64_t)dividend;
	uint64_t	dvs = (uint64_t)divisor;
	uint64_t	quot;

	if (PFC_EXPECT_FALSE(dvs == 0)) {
		divzero(env);
		quot = 0;
	}
	else {
		quot = dvd / dvs;
	}

	return (jlong)quot;
}

/*
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_util_UnsignedInteger_modulo(
 *	JNIEnv *env, jclass cls, jlong dividend, jlong divisor)
 *
 *	Determine the remainder of unsigned 64-bit integer division.
 *
 * Calling/Exit State:
 *	Upon successful completion, the remainder of `dividend' divided by
 *	`divisor' is returned.
 *	If `divisor' is zero, an ArithmeticException is thrown and zero is
 *	returned.
 */
JNIEXPORT jlong JNICALL
Java_org_opendaylight_vtn_core_util_UnsignedInteger_modulo(
	JNIEnv *env, jclass cls, jlong dividend, jlong divisor)
{
	uint64_t	dvd = (uint64_t)dividend;
	uint64_t	dvs = (uint64_t)divisor;
	uint64_t	mod;

	if (PFC_EXPECT_FALSE(dvs == 0)) {
		divzero(env);
		mod = 0;
	}
	else {
		mod = dvd % dvs;
	}

	return (jlong)mod;
}

/*
 * static const char *
 * uint_hexstring(JNIEnv *env, const char *str)
 *	Convert hexadecimal string from Java style into C language style.
 *
 *	Java defines '#' as hexadecimal string prefix. So it must be
 *	converted into "0x".
 *
 * Calling/Exit State:
 *	If the specified string does not contains '#' prefix, the specified
 *	string is returned.
 *
 *	If it contains, a pointer to a converted string is returned.
 *	It must be released by the caller.
 *	If the buffer for a converted string can not be allocated,
 *	an OutOfMemoryError is thrown and NULL is returned.
 */
static const char *
uint_hexstring(JNIEnv *env, const char *str)
{
	char		c = *str;
	char		*newstr;
	const char	*body;
	size_t		len;

	if (c == '#') {
		body = str + 1;
	}
	else if (c == '+' && *(str + 1) == '#') {
		body = str + 2;
	}
	else {
		return str;
	}

	/* Allocate a new string. */
	len = strlen(body) + 3;
	newstr = (char *)malloc(len);
	if (PFC_EXPECT_FALSE(newstr == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to allocate buffer for string.");

		return NULL;
	}

	PFC_ASSERT_INT(snprintf(newstr, len, "0x%s", body), (int)(len - 1));

	return newstr;
}

/*
 * static void
 * divzero(JNIEnv *env)
 *	Thrown an ArithmeticException which indicates division by zero.
 */
static void
divzero(JNIEnv *env)
{
	pjni_throw(env, PJNI_CLASS(ArithmeticException), "/ by zero");
}
