/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.enums;

/**
 * The Enum ContentTypeEnum.This enum is used for the content type which we are
 * getting in the request
 */
public enum ContentTypeEnum {

	/** The APPLICATIO n_ xml. */
	APPLICATION_XML("application/xml"),
	APPLICATION_XML_SCVMM("text/xml"),
	/** The APPLICATIO n_ json. */
	APPLICATION_JSON("application/json");


	/** The content type. */
	private String contentType;

	/**
	 * Instantiates a new content type enum.
	 *
	 * @param type
	 *            the type
	 */
	private ContentTypeEnum(final String type) {
		this.contentType = type;
	}

	/**
	 * Gets the content type.
	 *
	 * @return the content type
	 */
	public String getContentType() {
		return contentType;
	}
}
