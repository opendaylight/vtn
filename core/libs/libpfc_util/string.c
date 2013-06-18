/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * string.c - String utilities.
 */

#include <string.h>
#include <pfc/util.h>

/*
 * size_t
 * pfc_strlcpy(char *PFC_RESTRICT dst, const char *PFC_RESTRICT src,
 *	       size_t dstsize)
 *	Copy the string specified by `src' to the buffer pointed by `dst'.
 *
 *	`dstsize' must be the number of available bytes in the buffer pointed
 *	by `dst'.
 *
 *	This function copies at most (`dstsize' - 1) bytes of characters from
 *	`src' to `dst'. Copied string in `dst' is always terminated with '\0'
 *	unless `dstsize' is zero.
 *
 * Calling/Exit State:
 *	This function returns the result of strlen(src).
 *
 * Remarks:
 *	- Specifying an address which can not be accessed, such as NULL, to
 *	  `src' or `dst' results in undefined behavior.
 *
 *	- The strings pointed by `src' and `dst' must overlap.
 */
size_t
pfc_strlcpy(char *PFC_RESTRICT dst, const char *PFC_RESTRICT src,
	    size_t dstsize)
{
	const char	*s;
	char		*dend = dst + dstsize;

	for (s = src; dst < dend; dst++, s++) {
		char	c = *s;

		*dst = c;
		if (c == '\0') {
			return (size_t)(s - src);
		}
	}

	if (PFC_EXPECT_TRUE(dstsize != 0)) {
		*(dend - 1) = '\0';
	}

	return strlen(s) + (size_t)(s - src);
}
