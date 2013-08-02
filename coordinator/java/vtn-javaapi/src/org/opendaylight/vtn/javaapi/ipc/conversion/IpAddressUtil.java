/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.conversion;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.UnknownHostException;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;

public class IpAddressUtil {

	private final static int ipV4Size = 4;
	private final static int ipV6Size = 8;

	private static final Logger LOG = Logger.getLogger(IpAddressUtil.class
			.getName());

	/**
	 * Convert IPV4 text format to byte array
	 * 
	 * @param ipV4Add
	 *            : IPV4 address in text format
	 * @return : IPV4 address in byte array format
	 */
	public static byte[] textToNumericFormatV4(final String ipV4Add) {
		LOG.trace("Start textToNumericFormatV4 : " + ipV4Add);
		byte[] responseArray = null;
		if (ipV4Add.split(VtnServiceConsts.DOT_REGEX).length == ipV4Size) {
			try {
				responseArray = Inet4Address.getByName(ipV4Add).getAddress();
			} catch (UnknownHostException e) {
				responseArray = null;
				LOG.error("incorrect format ipv4 address " + e.getMessage());
			}
		} else {
			LOG.error("incorrect length ipv4 address");
		}
		LOG.trace("Complete textToNumericFormatV4 : " + responseArray);
		return responseArray;
	}

	/**
	 * Convert IPV6 text format to byte array
	 * 
	 * @param ipV6Add
	 *            : IPV6 address in text format
	 * @return : IPV6 address in byte array format
	 */
	public static byte[] textToNumericFormatV6(final String ipV6Add) {
		LOG.trace("Start textToNumericFormatV6 : " + ipV6Add);
		byte[] responseArray = null;
		if (ipV6Add.split(VtnServiceConsts.COLON).length == ipV6Size) {
			try {
				responseArray = Inet6Address.getByName(ipV6Add).getAddress();
			} catch (UnknownHostException e) {
				responseArray = null;
				LOG.error("incorrect format ipv6 address " + e.getMessage());
			}
		} else {
			LOG.error("incorrect length ipv6 address");
		}
		LOG.trace("Complete textToNumericFormatV6 : " + responseArray);
		return responseArray;
	}
}