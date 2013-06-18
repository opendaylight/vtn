/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import org.opendaylight.vtn.core.CoreSystem;

/**
 * <p>
 *   Native library loader of the PFC-Core utility classes.
 * </p>
 *
 * @since	C10
 */
final class PfcUtil
{
	/**
	 * Load native library.
	 */
	static {
		CoreSystem.loadLibrary("pfc_util_jni");
	}

	/**
	 * Ensure that the native library is loaded.
	 */
	static void load() {}

	/**
	 * Private constructor to reject instantiation.
	 */
	private PfcUtil()
	{
	}
}
