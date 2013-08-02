/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/*
 * Native library for PFC::Conf.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pfc/base.h>

#define	CLASS_PFC_CONF_INTEGER		"PFC::Conf::Integer"

/* String length enough to represent 64-bit integer. */
#define	INTEGER_STRSIZE		32

/*
 * Integer data type.
 */
typedef struct {
	const char	*ct_name;		/* name of type */
	uint8_t		ct_bits;		/* bit width */
	uint8_t		ct_unsigned;		/* unsigned value */
} conf_inttype_t;

static const conf_inttype_t	inttype_defs[] = {
	{ "BYTE", 8, 1},
	{ "INT32", 32, 0},
	{ "UINT32", 32, 1},
	{ "INT64", 64, 0},
	{ "UINT64", 64, 1},
	{ "LONG", 0, 0},
	{ "ULONG", 0, 1},
};

/* Bit width of LONG type.. */
static uint8_t	conf_long_width = 64;

/*
 * Return bit width of the specified conf_inttype_t.
 */
#define	PFC_CONF_INTTYPE_WIDTH(tp)					\
	(((tp)->ct_bits == 0) ? conf_long_width : (tp)->ct_bits)

/*
 * Instance of PFC::Conf::Integer.
 */
typedef struct {
	uint64_t		ci_value;	/* absolute value */
	const conf_inttype_t	*ci_type;	/* value type */
	uint8_t			ci_minus;	/* true if negative value */
} conf_integer_t;

#define	PFC_CONF_INTEGER_NEW(var)	Newx((var), 1, conf_integer_t)

/*
 * static int
 * conf_integer_validate(conf_integer_t *obj)
 *	Validate an integer value in the specified object.
 *	If not valid, an exception is thrown and zero is returned.
 */
static int
conf_integer_validate(conf_integer_t *obj)
{
	const conf_inttype_t	*tp = obj->ci_type;
	uint8_t	bits = PFC_CONF_INTTYPE_WIDTH(tp);

	if (obj->ci_value == 0) {
		obj->ci_minus = 0;
	}

	if (tp->ct_unsigned) {
		if (obj->ci_minus) {
			croak("%s: Value must be positive.\n", tp->ct_name);

			return 0;
		}
		if (bits != 64) {
			uint64_t	max = ((1ULL << bits) - 1);

			if (obj->ci_value > max) {
				croak("%s: Integer overflow.\n", tp->ct_name);

				return 0;
			}
		}
	}
	else if (obj->ci_minus) {
		uint64_t	max = (1ULL << (bits - 1));

		if (obj->ci_value > max) {
			croak("%s: Integer underflow.\n", tp->ct_name);

			return 0;
		}
	}
	else {
		uint64_t	max = ((1ULL << (bits - 1)) - 1);

		if (obj->ci_value > max) {
			croak("%s: Integer overflow.\n", tp->ct_name);

			return 0;
		}
	}

	return 1;
}

/*
 * static int
 * conf_integer_from_string(conf_integer_t *obj, const char *string)
 *	Convert string representation of integer into integer value.
 *	If the given string is invalid, an exception is thrown and zero is
 *	returned.
 */
static int
conf_integer_from_string(conf_integer_t *obj, const char *string)
{
	unsigned long long	ull;
	const char		*str;
	const conf_inttype_t	*tp;
	char		*p;

	tp = obj->ci_type;

	/* Determine the value is positive or not. */
	if (*string == '-') {
		obj->ci_minus = 1;
		str = string + 1;
	}
	else {
		obj->ci_minus = 0;
		str = string;
	}

	/* Ensure that no more sign character. */
	for (p = (char *)str; *p != '\0'; p++) {
		if (*p == '-') {
			croak("%s: Illegal character in a string.\n",
			      tp->ct_name);

			return 0;
		}
	}

	errno = 0;
	ull = strtoull(str, &p, 0);
	if (*p != '\0') {
		croak("%s: Illegal integer value: %s\n", tp->ct_name, string);

		return 0;
	}
	if (errno != 0) {
		croak("%s: Could not convert string to integer: %s\n",
		      tp->ct_name, string);

		return 0;
	}

	obj->ci_value = (uint64_t)ull;

	return conf_integer_validate(obj);
}

/*
 * static conf_integer_t *
 * conf_integer_get(SV *sv)
 *	Convert the given SV into PFC::Conf::Integer instance.
 *	NULL is returned if the given SV is not PFC::Conf::Integer instance.
 */
static conf_integer_t *
conf_integer_get(SV *sv)
{
	if (sv_derived_from(sv, CLASS_PFC_CONF_INTEGER)) {
		return (conf_integer_t *)(uintptr_t)(SvIV(SvRV(sv)));
	}

	return NULL;
}

/*
 * static conf_integer_t *
 * conf_integer_instance(SV *sv)
 *	Convert the given SV into PFC::Conf::Integer instance.
 *	An exception will be thrown if the given SV is not PFC::Conf::Integer
 *	instance.
 */
static conf_integer_t *
conf_integer_instance(SV *sv)
{
	conf_integer_t	*obj = conf_integer_get(sv);

	if (PFC_EXPECT_TRUE(obj != NULL)) {
		return obj;
	}

	croak("A reference to " CLASS_PFC_CONF_INTEGER " is required.");

	return NULL;
}

MODULE = PFC  PACKAGE = PFC

PROTOTYPES: DISABLE

MODULE = PFC::Conf  PACKAGE = PFC::Conf::Integer  PREFIX = conf_integer_

PROTOTYPES: DISABLE

##
## PFC::Conf::Integer::new(value, type)
##	Constructor of PFC::Conf::Integer.
##
conf_integer_t *
conf_integer_new(...)
    PREINIT:
	conf_integer_t	*me, newbuf;
    CODE:
	newbuf.ci_value = 0;
	newbuf.ci_minus = 0;
	newbuf.ci_type = &inttype_defs[0];

	if (items > 1) {
		if (items >= 3) {
			uint32_t	type;

			if (SvUOK(ST(2))) {
				type = (uint32_t)SvUV(ST(2));
			}
			else if (SvIOK(ST(2))) {
				type = (uint32_t)SvIV(ST(2));
			}
			else {
				/* Internal error */
				croak(CLASS_PFC_CONF_INTEGER
				      ": An integer type must be specified.\n");
			}

			if (type >= PFC_ARRAY_CAPACITY(inttype_defs)) {
				croak("Invalid integer type: %u\n", type);
			}
			newbuf.ci_type = &inttype_defs[type];
		}

		if (SvPOK(ST(1))) {
			const char	*str;

			str = (const char *)SvPV_nolen(ST(1));
			conf_integer_from_string(&newbuf, str);
		}
		else if (SvUOK(ST(1))) {
			newbuf.ci_value = (uint64_t)SvUV(ST(1));
			newbuf.ci_minus = 0;
			conf_integer_validate(&newbuf);
		}
		else if (SvIOK(ST(1))) {
			IV	v = SvIV(ST(1));

			if (v < 0) {
				newbuf.ci_value = (uint64_t)(-v);
				newbuf.ci_minus = 1;
			}
			else {
				newbuf.ci_value = (uint64_t)v;
				newbuf.ci_minus = 0;
			}
			conf_integer_validate(&newbuf);
		}
		else {
			/* Internal error */
			croak(CLASS_PFC_CONF_INTEGER
			      ": A string must be specified.\n");
		}

	}

	PFC_CONF_INTEGER_NEW(me);
	*me = newbuf;
	RETVAL = me;
    OUTPUT:
	RETVAL

##
## PFC::Conf::Integer::DESTROY()
##	Destructor of PFC::Conf::Integer.
##
void
conf_integer_DESTROY(me)
	conf_integer_t	*me;
  CODE:
	Safefree(me);

##
## PFC::Conf::Integer::stringify()
##	Return string representation of PFC::Conf::Integer instance.
##
SV *
conf_integer_stringify(me, ...)
	conf_integer_t	*me
    PREINIT:
	char	buf[INTEGER_STRSIZE];
	int	buflen;
    CODE:
	buflen = snprintf(buf, sizeof(buf), "%s%" PFC_PFMT_u64,
			  (me->ci_minus) ? "-" : "", me->ci_value);
	if (buflen >= sizeof(buf)) {
		/* This should not happen. */
		buflen = sizeof(buf) - 1;
		buf[buflen] = '\0';
	}
	RETVAL = newSVpvn(buf, buflen);
    OUTPUT:
	RETVAL

##
## PFC::Conf::Integer::compare(me, o, ...)
##	Compare this instance with the given object.
##
IV
conf_integer_compare(me, o, ...)
	conf_integer_t	*me
	SV		*o
    PREINIT:
	IV		ret;
	uint8_t		minus, cm;
	uint64_t	value, cv;
	conf_integer_t	*obj, objbuf;
    CODE:
	if ((obj = conf_integer_get(o)) == NULL) {
		obj = &objbuf;

		obj->ci_minus = 0;
		if (SvUOK(ST(1))) {
			obj->ci_value = (uint64_t)SvUV(ST(1));
		}
		else if (SvIOK(ST(1))) {
			IV	v = SvIV(ST(1));

			if (v < 0) {
				v *= -1;
				obj->ci_minus = 1;
			}
			obj->ci_value = (uint64_t)v;
		}
		else {
			/* Internal error */
			die(CLASS_PFC_CONF_INTEGER
			    ": compare: An integer must be specified.");
		}
	}

	minus = me->ci_minus;
	value = me->ci_value;
	cm = obj->ci_minus;
	cv = obj->ci_value;

	if (minus > cm) {
		ret = -1;
	}
	else if (minus < cm) {
		ret = 1;
	}
	else if (minus) {
		/* Both values are negative. */
		if (value < cv) {
			ret = 1;
		}
		else if (value > cv) {
			ret = -1;
		}
		else {
			ret = 0;
		}
	}
	else {
		/* Both values are positive. */
		if (value < cv) {
			ret = -1;
		}
		else if (value > cv) {
			ret = 1;
		}
		else {
			ret = 0;
		}
	}

	if (items >= 3 && SvTRUE(ST(2))) {
		/* The caller passed reverse flag. */
		ret *= -1;
	}

	RETVAL = ret;
    OUTPUT:
	RETVAL

##
## PFC::Conf::Integer::getHexValue()
##	Return a string which represents this value in hexadecimal format.
##
SV *
conf_integer_getHexValue(me, ...)
	conf_integer_t	*me
    PREINIT:
	char		buf[INTEGER_STRSIZE];
	int		buflen;
	uint64_t	value;
    CODE:
	if (me->ci_minus) {
		int64_t	v = (int64_t)me->ci_value * -1;
		value = (uint64_t)v;
	}
	else {
		value = me->ci_value;
	}
	buflen = snprintf(buf, sizeof(buf), "0x%" PFC_PFMT_x64, value);
	if (buflen >= sizeof(buf)) {
		/* This should not happen. */
		buflen = sizeof(buf) - 1;
		buf[buflen] = '\0';
	}
	RETVAL = newSVpvn(buf, buflen);
    OUTPUT:
	RETVAL

##
## PFC::Conf::Integer::getMinHexValue()
##	Return the minimum value of this integer type in hexadecimal format.
##
SV *
conf_integer_getMinHexValue(me, ...)
	conf_integer_t	*me
    PREINIT:
	char		buf[INTEGER_STRSIZE];
	int		buflen;
	const conf_inttype_t	*tp;
    CODE:
	tp = me->ci_type;
	if (tp->ct_unsigned) {
		buflen = snprintf(buf, sizeof(buf), "0x0");
	}
	else {
		uint64_t	value;
		uint8_t		bits = PFC_CONF_INTTYPE_WIDTH(tp);

		if (bits == 64) {
			value = (1ULL << 63);
		}
		else {
			value = (uint64_t)-1 & ~((1ULL << (bits - 1)) - 1);
		}

		buflen = snprintf(buf, sizeof(buf), "0x%" PFC_PFMT_x64, value);
	}
	if (buflen >= sizeof(buf)) {
		/* This should not happen. */
		buflen = sizeof(buf) - 1;
		buf[buflen] = '\0';
	}
	RETVAL = newSVpvn(buf, buflen);
    OUTPUT:
	RETVAL

##
## PFC::Conf::Integer::getMaxHexValue()
##	Return the maximum value of this integer type in hexadecimal format.
##
SV *
conf_integer_getMaxHexValue(me, ...)
	conf_integer_t	*me
    PREINIT:
	char		buf[INTEGER_STRSIZE];
	int		buflen;
	uint64_t	value;
	uint8_t		bits;
	const conf_inttype_t	*tp;
    CODE:
	tp = me->ci_type;
	bits = PFC_CONF_INTTYPE_WIDTH(tp);
	if (tp->ct_unsigned) {
		if (bits == 64) {
			value = (uint64_t)-1;
		}
		else {
			value = (1ULL << bits) - 1;
		}
	}
	else {
		value = (1ULL << (bits - 1)) - 1;
	}
	buflen = snprintf(buf, sizeof(buf), "0x%" PFC_PFMT_x64, value);
	if (buflen >= sizeof(buf)) {
		/* This should not happen. */
		buflen = sizeof(buf) - 1;
		buf[buflen] = '\0';
	}
	RETVAL = newSVpvn(buf, buflen);
    OUTPUT:
	RETVAL

MODULE = PFC::Conf  PACKAGE = PFC::Conf  PREFIX = pfc_conf_

PROTOTYPES: DISABLE

##
## PFC::Conf::LP64($lp64)
##	Configure the target system type.
##
IV
pfc_conf_LP64(...)
    PREINIT:
	IV	ret = 1;
    CODE:
	if (items >= 1) {
		if (SvTRUE(ST(0))) {
			/* The target is LP64. */
			conf_long_width = 64;
		}
		else {
			/* The target is ILP32. */
			conf_long_width = 32;
		}
	}

	if (conf_long_width != 64) {
		XSRETURN_UNDEF;
	}
	RETVAL = ret;
    OUTPUT:
	RETVAL
