/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.exception;

import org.opendaylight.vtn.webapi.utils.VtnServiceCommonUtil;

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
	private String errorCode;
    
    /** The error description. */
    private String errorDescription;

    /**
     * Instantiates a new vtn service web api exception.
     */
    public VtnServiceWebAPIException() {
		super();
	}
    
    /**
     * Constructor for ServiceException- This constructor will pass the  code and description to the Super class constructor.
     *
     * @param code String
     * @param message the message
     */
    public VtnServiceWebAPIException(final String code, final String message) {
    	super(VtnServiceCommonUtil.logErrorDetails(code));
		this.errorDescription = message;
		this.errorCode = code;
    }

  
    /**
     * Method getErrorCode.
     * 
    
     * @return String */
    public String getErrorCode() {
    	return errorCode;
    }

    /**
     * Method setErrorCode.
     * 
     * @param errorCode
     *            String
     */
    public void setErrorCode(final String errorCode) {
	this.errorCode = errorCode;
    }

    /**
     * Method getErrorDescription.
     * 
    
     * @return String */
    public String getErrorDescription() {
	return errorDescription;
    }

    /**
     * Method setErrorDescription.
     * 
     * @param errorDescription
     *            String
     */
    public void setErrorDescription(final String errorDescription) {
    	this.errorDescription = errorDescription;
    }
}
