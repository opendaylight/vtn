/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.exception;

/**
 * The Class VtnsInitFailException.
 */
public class VtnServiceInitFailException extends RuntimeException {

	/**
	 *
	 */
	private static final long serialVersionUID = -9114264797870890041L;

	/**
	 * Instantiates a new vtns init fail exception.
	 * 
	 * @param cause
	 *            the cause
	 */
	public VtnServiceInitFailException(final Throwable cause) {
		super(cause);
	}

	/**
	 * 
	 * @param message
	 * @param cause
	 */
	public VtnServiceInitFailException(final String message,
			final Throwable cause) {
		super(message, cause);
	}
}
