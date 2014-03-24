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
 * The Enum SessionEnum.This enum have all the required parameters that will be
 * passed to lower layer for creating session.
 */
public enum SessionEnum {

	/** The USERNAME. */
	USERNAME("username"),

	/** The PASSWORD. */
	PASSWORD("password"),

	/** The IPADDRESS. */
	IPADDRESS("ipaddr");

	/** The session element. */
	private String sessionElement;

	/**
	 * Instantiates a new session enum.
	 * 
	 * @param element
	 *            the element
	 */
	private SessionEnum(final String element) {
		this.sessionElement = element;
	}

	/**
	 * Gets the session element.
	 * 
	 * @return the session element
	 */
	public String getSessionElement() {
		return sessionElement;
	}
}
