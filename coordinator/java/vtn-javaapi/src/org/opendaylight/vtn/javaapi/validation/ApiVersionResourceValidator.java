/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * The Class ApiVersionResourceValidator validates request for API Version API
 */
public class ApiVersionResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(ApiVersionResourceValidator.class.getName());

	/** The instance of AbstractResource. */
	private final AbstractResource resource;

	/**
	 * Instantiates a new ApiVersion resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public ApiVersionResourceValidator(final AbstractResource resource) {
		this.resource = resource;
		LOG.info(this.resource.toString());
	}

	/**
	 * Validate request for Show API Version
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.info("No validation required for Get Api Version");
	}
}
