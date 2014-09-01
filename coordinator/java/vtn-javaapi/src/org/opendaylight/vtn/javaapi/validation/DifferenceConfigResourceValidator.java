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

public class DifferenceConfigResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(DifferenceConfigResourceValidator.class.getName());

	public DifferenceConfigResourceValidator(final AbstractResource resource) {

	}

	/**
	 * Validate request Json for UNC Difference Config API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.info("Validation not required for Diffrence Config API");
	}
}
