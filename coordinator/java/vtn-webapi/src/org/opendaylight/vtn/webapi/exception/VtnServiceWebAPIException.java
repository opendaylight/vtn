/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.exception;

import org.opendaylight.vtn.webapi.constants.ApplicationConstants;

/**
 * The Class VtnServiceException.
 * 
 * This is a common exception class which will be caught by service and thrown
 * by all subsequent layers.
 */
public class VtnServiceWebAPIException extends Exception {

	/** The Constant serialVersionUID. */
	private static final long serialVersionUID = 1L;
	/** The error code. */
	private int errorCode;

	/**
	 * Instantiates a new vtn service web api exception.
	 */
	public VtnServiceWebAPIException() {
		super();
	}

	/**
	 * Constructor for ServiceException- This constructor will pass the code and
	 * description to the Super class constructor.
	 * 
	 * @param code
	 *            String
	 */
	public VtnServiceWebAPIException(final int code) {
		this.errorCode = code;
	}

	/**
	 * Method getErrorCode.
	 * 
	 * @return int
	 */
	public int getErrorCode() {
		return errorCode;
	}

	@Override
	public String getMessage() {
		return errorCode + ApplicationConstants.COLON + super.getMessage();
	}
}
