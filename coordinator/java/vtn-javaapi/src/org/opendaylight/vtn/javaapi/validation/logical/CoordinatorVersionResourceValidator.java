/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.validation.logical;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;
import org.opendaylight.vtn.javaapi.validation.physical.VersionResourceValidator;

/**
 * The Class CoordinatorVersionResourceValidator validates request Json object
 * for Show Coordinator Version API.
 */
public class CoordinatorVersionResourceValidator extends VtnServiceValidator {
	/**
	 * Logger for debugging purpose.
	 */
	private static final Logger LOG = Logger
			.getLogger(VersionResourceValidator.class.getName());

	/**
	 * validator object for common validations.
	 * 
	 * @param resource
	 *            ,Abstract class object
	 */
	public CoordinatorVersionResourceValidator(final AbstractResource resource) {

	}

	/**
	 * Validate request Json for Show Coordinator Version PI.
	 * 
	 * @param requestBody
	 *            , for request.
	 * @param method
	 *            , for method.
	 * @throws VtnServiceException
	 *             , in case of failure
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.info("No validation required for Show Coordinator Version");
	}
}
