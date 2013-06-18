/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;

/**
 * The Interface VtnServiceResource.
 */
public interface VtnServiceResource {

	/**
	 * Delete.
	 * 
	 * @return the int
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	int delete() throws VtnServiceException;

	/**
	 * Delete.
	 * 
	 * @param queryString
	 *            the query string
	 * @return the int
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	int delete(JsonObject queryString) throws VtnServiceException;

	/**
	 * Gets the.
	 * 
	 * @return the int
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	int get() throws VtnServiceException;

	/**
	 * Gets the.
	 * 
	 * @param queryString
	 *            the query string
	 * @return the int
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	int get(JsonObject queryString) throws VtnServiceException;

	/**
	 * Post.
	 * 
	 * @param requestBody
	 *            the request body
	 * @return the int
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	int post(JsonObject requestBody) throws VtnServiceException;

	/**
	 * Put.
	 * 
	 * @param requestBody
	 *            the request body
	 * @return the int
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	int put(JsonObject requestBody) throws VtnServiceException;
}
