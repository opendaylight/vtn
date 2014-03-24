/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.enums;

/**
 * The Enum ResourcePathEnum.This enum provided the path of resources like
 * session path, config path
 */
public enum ResourcePathEnum {

	/** The SESSIO n_ path. */
	SESSION_PATH("/sessions"),

	/** The ACQUIR e_ confi g_ path. */
	ACQUIRE_CONFIG_PATH("/configuration/configmode"),

	/** The RELEAS e_ session. */
	RELEASE_SESSION("/sessions/"),

	/** The RELEAS e_ configuration. */
	RELEASE_CONFIGURATION("/configuration/configmode/"),

	/** The ACQUIR e_ releas e_ monitorin g_ path. */
	ACQUIRE_RELEASE_MONITORING_PATH("/configuration/readlock"),

	/** The COMMI t_ configuration. */
	COMMIT_CONFIGURATION("/configuration"),

	/** The ABORT_ configuration. */
	ABORT_CONFIGURATION("/configuration/candidate");
	/** The path. */
	private String path;

	/**
	 * Instantiates a new resource path enum.
	 * 
	 * @param resourcePath
	 *            the resource path
	 */
	private ResourcePathEnum(final String resourcePath) {
		this.path = resourcePath;
	}

	/**
	 * Gets the resource path.
	 * 
	 * @return the resource path
	 */
	public String getPath() {
		return path;
	}
}
