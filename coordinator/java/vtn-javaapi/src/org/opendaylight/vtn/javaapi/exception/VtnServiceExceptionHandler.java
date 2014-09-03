/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.exception;

import org.opendaylight.vtn.core.util.Logger;

/**
 * The Class VtnServiceExceptionHandler. Centralized exception handler in
 * JavaAPI
 */
public class VtnServiceExceptionHandler {

	private static final Logger LOG = Logger
			.getLogger(VtnServiceExceptionHandler.class.getName());

	/**
	 * Handle the exception thrown by the JavaAPI
	 * 
	 * @param errorContext
	 *            the error context
	 * @param errorCode
	 *            the error code
	 * @param errorText
	 *            the error text
	 * @param exception
	 *            the throwable exception
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public final void raise(final String errorContext, final String errorCode,
			final String errorText, final Exception exception)
			throws VtnServiceException {
		/*
		 * If exception is type of VtnServiceException then enrich the message,
		 * otherwise re-throw exception after wrapping
		 */
		if (exception instanceof VtnServiceException) {
			((VtnServiceException) exception).addInfo(errorContext, errorCode,
					errorText);
		} else {
			throw new VtnServiceException(errorContext, errorCode, errorText,
					exception);
		}
	}

	/**
	 * LOG the exception thrown by the JavaAPI
	 * 
	 * @param errorMsg
	 * @param errorCode
	 * @param string
	 * 
	 * @param exception
	 *            the throwable exception
	 */

	/**
	 * Handle the exception and log exception message
	 * 
	 * @param errorContext
	 *            the error context
	 * @param errorCode
	 *            the error code
	 * @param errorText
	 *            the error text
	 * @param exception
	 *            the throwable exception
	 */
	public final void handle(final String errorContext, final String errorCode,
			final String errorText, final Exception exception) {
		/*
		 * raise the exception for final handling and log the exception after
		 * that
		 */
		try {
			raise(errorContext, errorCode, errorText, exception);
		} catch (final VtnServiceException e) {
			// If exception is throw by raise method, i.e. for exceptions except
			// VtnServiceException
			LOG.error(e, "Error Occurred : \n" + e.toString());
			return;
		}
		LOG.error(exception, "Error Occurred : \n" + exception.toString());
	}

}
