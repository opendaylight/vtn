/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation.physical;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VersionResourceValidator validates request Json object for UNC
 * Version API.
 */
public class VersionResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VersionResourceValidator.class.getName());

	public VersionResourceValidator(final AbstractResource resource) {

	}

	/**
	 * Validate request Json for UNC Version API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.info("No validation required for Show Version");
	}
}
