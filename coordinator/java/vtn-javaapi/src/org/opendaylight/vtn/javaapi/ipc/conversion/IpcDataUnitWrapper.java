/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.conversion;

import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcInet4Address;
import org.opendaylight.vtn.core.ipc.IpcInet6Address;
import org.opendaylight.vtn.core.ipc.IpcInetAddress;
import org.opendaylight.vtn.core.ipc.IpcInt32;
import org.opendaylight.vtn.core.ipc.IpcString;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.ipc.IpcUint16;
import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.ipc.IpcUint64;
import org.opendaylight.vtn.core.ipc.IpcUint8;
import org.opendaylight.vtn.core.util.UnsignedInteger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;

public final class IpcDataUnitWrapper {

	private IpcDataUnitWrapper() {
	}

	/**
	 * Set json string value to IpcUint8 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint8 setIpcUint8Value(final String jsonValue) {
		return new IpcUint8(Integer.parseInt(jsonValue));
	}

	/**
	 * Set json string value to IpcUint8 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint8 setIpcUint8Value(final String jsonValue,
			final IpcStruct struct, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
			return new IpcUint8(VtnServiceConsts.DEFAULT_NUMBER);
		}
		return new IpcUint8(Integer.parseInt(jsonValue));
	}

	/**
	 * Set json integer value to IpcUint8 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint8 setIpcUint8Value(final int jsonValue) {
		return new IpcUint8(jsonValue);
	}

	/**
	 * Set json string value to IpcUint8 array type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static String setIpcUint8ArrayValue(final String jsonValue) {
		return jsonValue;
	}

	/**
	 * Set json string value to IpcUint8 array type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static String setIpcUint8ArrayValue(final String jsonValue,
			final IpcStruct struct, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
		}
		return jsonValue;
	}

	/**
	 * Set json string value to IpcUint16 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint16 setIpcUint16Value(final String jsonValue) {
		return new IpcUint16(Integer.parseInt(jsonValue));
	}

	/**
	 * Set json string value to IpcUint16 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint16 setIpcUint16Value(final String jsonValue,
			final IpcStruct struct, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
			return new IpcUint16(VtnServiceConsts.DEFAULT_NUMBER);
		}
		return new IpcUint16(Integer.parseInt(jsonValue));
	}

	/**
	 * Set json integer value to IpcUint16 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint16 setIpcUint16Value(final int jsonValue) {
		return new IpcUint16(jsonValue);
	}

	/**
	 * Set json hex string value to IpcUint16 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint16 setIpcUint16HexaValue(final String jsonValue) {
		return new IpcUint16(jsonValue);
	}

	/**
	 * Set json hex string value to IpcUint16 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint16 setIpcUint16HexaValue(final String jsonValue,
			final IpcStruct struct, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
			return new IpcUint16(VtnServiceConsts.DEFAULT_NUMBER);
		}
		return new IpcUint16(jsonValue);
	}

	/**
	 * Set json string value to IpcUint32 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint32 setIpcUint32Value(final String jsonValue) {
		return new IpcUint32(Long.parseLong(jsonValue));
	}

	/**
	 * Set json string value to IpcUint32 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint32 setIpcUint32Value(final String jsonValue,
			final IpcStruct struct, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
			return new IpcUint32(VtnServiceConsts.DEFAULT_NUMBER);
		}
		return new IpcUint32(Long.parseLong(jsonValue));
	}

	/**
	 * Set int string value to IpcInt32 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcInt32 setIpcInt32Value(final int jsonValue) {
		return new IpcInt32(jsonValue);
	}

	/**
	 * Set int string value to IpcUint32 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint32 setIpcUint32Value(final long jsonValue) {
		return new IpcUint32(jsonValue);
	}

	/**
	 * Set json string value to IpcUint64 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint64 setIpcUint64Value(final String jsonValue) {
		return new IpcUint64(jsonValue);
	}

	/**
	 * Set json string value to IpcUint64 type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcUint64 setIpcUint64Value(final String jsonValue,
			final IpcStruct struct, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
			return new IpcUint64(VtnServiceConsts.DEFAULT_NUMBER);
		}
		return new IpcUint64(jsonValue);
	}

	/**
	 * Set json ipaddress value to IpcInet4Address type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcInet4Address
			setIpcInet4AddressValue(final String jsonValue) {
		return (IpcInet4Address) IpcInetAddress.create(IpAddressUtil
				.textToNumericFormatV4(jsonValue));
	}

	/**
	 * Set json ipaddress value to IpcInet4Address type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcInet4Address setIpcInet4AddressValue(
			final String jsonValue, final IpcStruct struct, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
			return (IpcInet4Address) IpcInetAddress.create(IpAddressUtil
					.textToNumericFormatV4(VtnServiceConsts.DEFAULT_IP));
		}
		return (IpcInet4Address) IpcInetAddress.create(IpAddressUtil
				.textToNumericFormatV4(jsonValue));
	}

	/**
	 * Set json ipaddress value to IpcInet6Address type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcInet6Address
			setIpcInet6AddressValue(final String jsonValue) {
		return (IpcInet6Address) IpcInetAddress.create(IpAddressUtil
				.textToNumericFormatV6(jsonValue));
	}

	/**
	 * Set json ipaddress value to IpcInet6Address type
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcInet6Address setIpcInet6AddressValue(
			final String jsonValue, final IpcStruct struct, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
			return (IpcInet6Address) IpcInetAddress.create(IpAddressUtil
					.textToNumericFormatV6(VtnServiceConsts.DEFAULT_IPV6));
		}
		return (IpcInet6Address) IpcInetAddress.create(IpAddressUtil
				.textToNumericFormatV6(jsonValue));
	}

	/**
	 * Create IpcStruct with name as Json string value
	 * 
	 * @param jsonValue
	 * @return
	 */
	public static IpcStruct setIpcStructValue(final String jsonValue) {
		return new IpcStruct(jsonValue);
	}

	/**
	 * Get string value of IpcDataUnit
	 * 
	 * @param ipcUnit
	 * @return
	 */
	public static String getIpcDataUnitValue(final IpcDataUnit ipcUnit) {
		String stringValue = VtnServiceConsts.EMPTY_STRING;
		if (ipcUnit != null) {
			if (ipcUnit.getType() == IpcDataUnit.UINT32) {
				stringValue = ((IpcUint32) ipcUnit).toString();
			} else if (ipcUnit.getType() == IpcDataUnit.IPV4) {
				stringValue = ((IpcInet4Address) ipcUnit).toString();
			} else if (ipcUnit.getType() == IpcDataUnit.IPV6) {
				stringValue = ((IpcInet6Address) ipcUnit).toString();
			} else if (ipcUnit.getType() == IpcDataUnit.UINT8) {
				stringValue = ((IpcUint8) ipcUnit).toString();
			} else if (ipcUnit.getType() == IpcDataUnit.STRING) {
				stringValue = ((IpcString) ipcUnit).getValue();
			}
		}
		return stringValue;
	}

	/**
	 * Get string value of IpcUint8
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructUint8Value(final IpcStruct struct,
			final String parameterName) {
		return struct.get(parameterName).toString();
	}

	/**
	 * Get string value of IpcUint8 array
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructUint8ArrayValue(final IpcStruct struct,
			final String parameterName) {
		return struct.getString(parameterName);
	}

	/**
	 * Get string value of IpcUint16
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructUint16Value(final IpcStruct struct,
			final String parameterName) {
		return struct.get(parameterName).toString();
	}

	/**
	 * Get string value of IpcUint16
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructUint16HexaValue(final IpcStruct struct,
			final String parameterName) {
		final int intValue = ((IpcUint16) struct.get(parameterName)).intValue();
		final String hexString = UnsignedInteger.toHexString(intValue);
		return "0x" + hexString;
	}

	/**
	 * Get string value of IpcUint32
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructUint32Value(final IpcStruct struct,
			final String parameterName) {
		return struct.get(parameterName).toString();
	}

	/**
	 * Get string value of IpcInet4Address
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructIpv4Value(final IpcStruct struct,
			final String parameterName) {
		return struct.get(parameterName).toString();
	}

	/**
	 * Get string value of IpcInet6Address
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructIpv6Value(final IpcStruct struct,
			final String parameterName) {
		return struct.get(parameterName).toString();
	}

	/**
	 * Get string value of IpcUint64
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructUint64Value(final IpcStruct struct,
			final String parameterName) {
		return struct.get(parameterName).toString();
	}

	/**
	 * Get string value of IpcUint64
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructInt64Value(final IpcStruct struct,
			final String parameterName) {
		return struct.get(parameterName).toString();
	}

	/**
	 * Get string value of IpcStruct
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static IpcStruct getInnerIpcStruct(final IpcStruct struct,
			final String parameterName) {
		return struct.getInner(parameterName);
	}

	/**
	 * Get string value of hexadecimal IpcUint64
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getIpcStructUint64HexaValue(final IpcStruct struct,
			final String parameterName) {
		final long longValue = ((IpcUint64) struct.get(parameterName))
				.longValue();
		final String hexString = UnsignedInteger.toHexString(longValue);
		return "0x" + hexString;
	}

	/**
	 * Set the text representation format mac address to byte format mac address
	 * of IpcStruct
	 * 
	 * @param struct
	 * @param parameterName
	 * @param jsonValue
	 */
	public static void
			setMacAddress(final IpcStruct struct, final String parameterName,
					final String jsonValue, final int index) {
		if (jsonValue == null || jsonValue.isEmpty()) {
			setNoValueFlag(struct, index);
			return;
		}
		final String macAddress = jsonValue.replaceAll(
				VtnServiceConsts.DOT_REGEX, VtnServiceConsts.EMPTY_STRING);
		if (macAddress.length() != UncIndexEnum.TWELVE.ordinal()) {
			return;
		} else {
			int structIndex = UncIndexEnum.ZERO.ordinal();
			int counter = UncIndexEnum.ZERO.ordinal();
			while (structIndex != UncIndexEnum.SIX.ordinal()) {
				struct.set(
						parameterName,
						structIndex,
						new IpcUint8(Integer.valueOf(
								macAddress.substring(counter, counter
										+ UncIndexEnum.TWO.ordinal()),
								UncIndexEnum.SIXTEEN.ordinal())));
				structIndex++;
				counter = counter + UncIndexEnum.TWO.ordinal();
			}
		}
	}

	/**
	 * Get the value of mac address type of fields from IpcStruct Convert the
	 * byte value into text representation format
	 * 
	 * @param struct
	 * @param parameterName
	 * @return
	 */
	public static String getMacAddress(final IpcStruct struct,
			final String parameterName) {
		String jsonString = VtnServiceConsts.EMPTY_STRING;
		int structIndex = UncIndexEnum.ZERO.ordinal();
		while (structIndex != UncIndexEnum.SIX.ordinal()) {
			if (structIndex != UncIndexEnum.ZERO.ordinal()
					&& structIndex % UncIndexEnum.TWO.ordinal() == UncIndexEnum.ZERO
							.ordinal()) {
				jsonString = jsonString + VtnServiceConsts.DOT;
			}
			final int value = struct.getByte(parameterName, structIndex);
			if (value < UncIndexEnum.ZERO.ordinal()) {
				jsonString = jsonString
						+ Integer.toHexString(
								struct.getByte(parameterName, structIndex++))
								.substring(UncIndexEnum.SIX.ordinal(),
										UncIndexEnum.EIGHT.ordinal());
			} else if (value >= UncIndexEnum.ZERO.ordinal()
					&& value < UncIndexEnum.SIXTEEN.ordinal()) {
				jsonString = jsonString
						+ VtnServiceConsts.ZERO
						+ Integer.toHexString(struct.getByte(parameterName,
								structIndex++));
			} else {
				jsonString = jsonString
						+ Integer.toHexString(struct.getByte(parameterName,
								structIndex++));
			}
		}
		return jsonString;
	}

	/**
	 * Set UNC_VF_VALID_NO_VALUE in case of receiving empty string for parameter
	 * 
	 * @param struct
	 * @param index
	 */
	private static void setNoValueFlag(final IpcStruct struct, final int index) {
		struct.set(
				VtnServiceIpcConsts.VALID,
				index,
				IpcDataUnitWrapper
						.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
								.ordinal()));
	}

}
