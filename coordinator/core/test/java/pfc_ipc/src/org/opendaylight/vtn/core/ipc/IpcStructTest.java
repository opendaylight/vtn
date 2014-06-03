/* -*- mode: java; coding: utf-8; -*- */

/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.Arrays;
import java.util.Set;
import java.util.HashMap;
import java.util.Random;
import java.net.InetAddress;
import java.nio.charset.Charset;
import java.lang.reflect.Method;
import junit.framework.Test;
import junit.framework.TestSuite;
import org.opendaylight.vtn.core.CoreSystem;

import static junit.framework.TestCase.assertEquals;
import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertNotNull;
import static junit.framework.TestCase.assertTrue;
import static junit.framework.TestCase.fail;

/**
 * <p>
 *   Unit test class for {@link IpcStruct}.
 * </p>
 */
public class IpcStructTest extends TestBase
{
	/**
	 * <p>
	 *   Determine whether the IPC structure information was successfully
	 *   loaded or not.
	 * </p>
	 */
	private final static boolean  loaded;

	/**
	 * <p>
	 *   Load IPC structure information for JUnit tests.
	 * </p>
	 */
	static {
		loaded = IpcStructTest.load();
	}

	/**
	 * <p>
	 *   Determine whether the IPC structure information was successfully
	 *   loaded or not.
	 * </p>
	 *
	 * @return  {@code true} is returned only if IPC structure information
	 *          was successfully loaded.
	 */
	public static boolean isLoaded()
	{
		return loaded;
	}

	/**
	 * Return test suite to run.
	 *
	 * @return  Test suite.
	 */
	public static Test suite()
	{
		TestSuite suite = new TestSuite("IpcStruct Test");

		// IpcStruct tests can run only if IPC structure information
		// is available.
		if (loaded) {
			Class<IpcStructTest> cl = IpcStructTest.class;
			for (Method m: cl.getMethods()) {
				String name = m.getName();
				if (!name.startsWith("test")) {
					continue;
				}

				Class<?> retType = m.getReturnType();
				if (retType.equals(void.class)) {
					suite.addTest(new IpcStructTest(name));
				}
			}
		}

		return suite;
	}

	/**
	 * Create JUnit test case for {@link IpcStruct}.
	 *
	 * @param name	The test name.
	 */
	public IpcStructTest(String name)
	{
		super(name);
	}

	/**
	 * Tear down the test environment.
	 */
	@Override
	protected void tearDown()
	{
		super.tearDown();

		// Call finalizer of IpcStruct by force.
		System.gc();
	}

	/**
	 * Test case for {@link IpcStruct#IpcStruct(String)}.
	 */
	public void testCtor()
	{
		String[] names = {
			"ut_struct_1",
			"ut_struct_2",
			"ut_struct_3",
			"ut_struct_4",
		};

		for (String name: names) {
			IpcStruct struct = new IpcStruct(name);
			assertEquals(name, struct.getName());
			assertEquals("ipc_struct " + name, struct.toString());
			assertEquals(IpcDataUnit.STRUCT, struct.getType());

			TestStruct tst = TestStruct.create(struct);

			// Contents of IPC structure must be zeroed out.
			assertTrue(tst.equals());
		}

		// null structure name.
		try {
			new IpcStruct(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "name is null.");
		}

		// Invalid structure name.
		for (int i = 5; i <= 10; i++) {
			String name = "ut_struct_" + i;

			try {
				new IpcStruct(name);
				needException();
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Unknown IPC struct: " + name);
			}
		}
	}

	/**
	 * Test case for {@link IpcStruct#getField(String, String)} and
	 * {@link IpcStruct#getFieldNames(String)}.
	 */
	public void testGetField1()
	{
		String[] stnames = {
			"ut_struct_1",
			"ut_struct_2",
			"ut_struct_3",
			"ut_struct_4",
		};

		for (String stname: stnames) {
			String[] names = IpcStruct.getFieldNames(stname);
			TestStruct tst =
				TestStruct.create(new IpcStruct(stname));
			String[] defnames = tst.getFieldNames();
			assertTrue(Arrays.equals(defnames, names));

			checkFields(stname, tst, names);
		}

		// Specify null to structure name.
		try {
			IpcStruct.getFieldNames(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "stname is null.");
		}

		try {
			IpcStruct.getField(null, "a");
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "stname is null.");
		}

		// Specify unknown structure name.
		String[] invalid = {
			"",
			"_",
			" a",
			"_invalid_struct",
			"ut_struct_10",
			"あいうえお",
		};

		for (String inv: invalid) {
			try {
				IpcStruct.getField(inv, "a");
				needException();
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Unknown IPC struct: " + inv);
			}
		}
	}

	/**
	 * Test case for {@link IpcStruct#getField(String)} and
	 * {@link IpcStruct#getFieldNames()}.
	 */
	public void testGetField2()
	{
		String[] stnames = {
			"ut_struct_1",
			"ut_struct_2",
			"ut_struct_3",
			"ut_struct_4",
		};

		for (String stname: stnames) {
			IpcStruct struct = new IpcStruct(stname);
			String[] names = struct.getFieldNames();
			TestStruct tst = TestStruct.create(struct);
			String[] defnames = tst.getFieldNames();
			assertTrue(Arrays.equals(defnames, names));

			checkFields(struct, tst, names);
		}
	}

	/**
	 * Ensure that an {@code NullPointerException} is thrown if a null data
	 * is passed to {@link IpcStruct#set(String, IpcDataUnit)} or
	 * {@link IpcStruct#set(String, int, IpcDataUnit)}.
	 */
	public void testSetNull()
	{
		IpcStruct ut1 = new IpcStruct("ut_struct_1");
		IpcStruct ut2 = new IpcStruct("ut_struct_2");

		try {
			ut1.set("ut1_int8", (IpcInt8)null);
			needException();
		}
		catch (NullPointerException e) {}

		try {
			ut2.set("ut2_int8", 0, (IpcInt8)null);
			needException();
		}
		catch (NullPointerException e) {}
	}

	/**
	 * Ensure that none of {@link IpcString}, {@link IpcBinary}, and
	 * {@link IpcNull} object can be set to structure field.
	 */
	public void testUnsupported()
	{
		IpcUnsupportedFieldTest impl = new IpcUnsupportedFieldTest();

		testAccessor(impl);
	}

	/**
	 * Test case for accessor which uses {@link IpcInt8} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcInt8()
	{
		IpcInt8FieldTest impl = new IpcInt8FieldTest();

		testAccessor(impl);

		// Pass null field name to get(String) and get(String, int).
		IpcStruct struct = new IpcStruct("ut_struct_1");
		try {
			impl.get(struct, null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "name is null.");
		}

		try {
			impl.get(struct, null, 0);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "name is null.");
		}
	}

	/**
	 * Test case for accessor which uses {@link IpcUint8} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcUint8()
	{
		testAccessor(new IpcUint8FieldTest());
	}

	/**
	 * Test case for accessor which uses {@code byte} value.
	 * <ul>
	 *   <li>{@link IpcStruct#getByte(String)}</li>
	 *   <li>{@link IpcStruct#getByte(String, int)}</li>
	 *   <li>{@link IpcStruct#setByte(String, byte)}</li>
	 *   <li>{@link IpcStruct#setByte(String, int, byte)}</li>
	 *   <li>{@link IpcStruct#set(String, byte)}</li>
	 *   <li>{@link IpcStruct#set(String, int, byte)}</li>
	 * </ul>
	 */
	public void testGetSetByte()
	{
		testAccessor(new ByteFieldTest());
		testAccessor(new ByteFieldSetTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcInt16} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcInt16()
	{
		testAccessor(new IpcInt16FieldTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcUint16} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcUint16()
	{
		testAccessor(new IpcUint16FieldTest());
	}

	/**
	 * Test case for accessor which uses {@code short} value.
	 * <ul>
	 *   <li>{@link IpcStruct#getShort(String)}</li>
	 *   <li>{@link IpcStruct#getShort(String, int)}</li>
	 *   <li>{@link IpcStruct#setShort(String, short)}</li>
	 *   <li>{@link IpcStruct#setShort(String, int, short)}</li>
	 *   <li>{@link IpcStruct#set(String, short)}</li>
	 *   <li>{@link IpcStruct#set(String, int, short)}</li>
	 * </ul>
	 */
	public void testGetSetShort()
	{
		testAccessor(new ShortFieldTest());
		testAccessor(new ShortFieldSetTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcInt32} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcInt32()
	{
		testAccessor(new IpcInt32FieldTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcUint32} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcUint32()
	{
		testAccessor(new IpcUint32FieldTest());
	}

	/**
	 * Test case for accessor which uses {@code int} value.
	 * <ul>
	 *   <li>{@link IpcStruct#getInt(String)}</li>
	 *   <li>{@link IpcStruct#getInt(String, int)}</li>
	 *   <li>{@link IpcStruct#setInt(String, int)}</li>
	 *   <li>{@link IpcStruct#setInt(String, int, int)}</li>
	 *   <li>{@link IpcStruct#set(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, int, int)}</li>
	 * </ul>
	 */
	public void testGetSetInt()
	{
		testAccessor(new IntFieldTest());
		testAccessor(new IntFieldSetTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcInt64} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcInt64()
	{
		testAccessor(new IpcInt64FieldTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcUint64} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcUint64()
	{
		testAccessor(new IpcUint64FieldTest());
	}

	/**
	 * Test case for accessor which uses {@code long} value.
	 * <ul>
	 *   <li>{@link IpcStruct#getLong(String)}</li>
	 *   <li>{@link IpcStruct#getLong(String, int)}</li>
	 *   <li>{@link IpcStruct#setLong(String, long)}</li>
	 *   <li>{@link IpcStruct#setLong(String, int, long)}</li>
	 *   <li>{@link IpcStruct#set(String, long)}</li>
	 *   <li>{@link IpcStruct#set(String, int, long)}</li>
	 * </ul>
	 */
	public void testGetSetLong()
	{
		testAccessor(new LongFieldTest());
		testAccessor(new LongFieldSetTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcFloat} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcFloat()
	{
		testAccessor(new IpcFloatFieldTest());
	}

	/**
	 * Test case for accessor which uses {@code float} value.
	 * <ul>
	 *   <li>{@link IpcStruct#getFloat(String)}</li>
	 *   <li>{@link IpcStruct#getFloat(String, int)}</li>
	 *   <li>{@link IpcStruct#setFloat(String, float)}</li>
	 *   <li>{@link IpcStruct#setFloat(String, int, float)}</li>
	 *   <li>{@link IpcStruct#set(String, float)}</li>
	 *   <li>{@link IpcStruct#set(String, int, float)}</li>
	 * </ul>
	 */
	public void testGetSetFloat()
	{
		testAccessor(new FloatFieldTest());
		testAccessor(new FloatFieldSetTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcDouble} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcDouble()
	{
		testAccessor(new IpcDoubleFieldTest());
	}

	/**
	 * Test case for accessor which uses {@code double} value.
	 * <ul>
	 *   <li>{@link IpcStruct#getDouble(String)}</li>
	 *   <li>{@link IpcStruct#getDouble(String, int)}</li>
	 *   <li>{@link IpcStruct#setDouble(String, double)}</li>
	 *   <li>{@link IpcStruct#setDouble(String, int, double)}</li>
	 *   <li>{@link IpcStruct#set(String, double)}</li>
	 *   <li>{@link IpcStruct#set(String, int, double)}</li>
	 * </ul>
	 */
	public void testGetSetDouble()
	{
		testAccessor(new DoubleFieldTest());
		testAccessor(new DoubleFieldSetTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcInet4Address} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcInet4Address()
	{
		testAccessor(new IpcInet4AddressFieldTest());
	}

	/**
	 * Test case for accessor which uses {@code Inet4Address} value.
	 * <ul>
	 *   <li>{@link IpcStruct#getInetAddress(String)}</li>
	 *   <li>{@link IpcStruct#getInetAddress(String, int)}</li>
	 *   <li>{@link IpcStruct#setInetAddress(String, InetAddress)}</li>
	 *   <li>{@link IpcStruct#setInetAddress(String, int, InetAddress)}</li>
	 *   <li>{@link IpcStruct#set(String, InetAddress)}</li>
	 *   <li>{@link IpcStruct#set(String, int, InetAddress)}</li>
	 * </ul>
	 */
	public void testGetSetInet4Address()
	{
		testAccessor(new Inet4AddressFieldTest());
		testAccessor(new Inet4AddressFieldSetTest());
	}

	/**
	 * Test case for accessor which uses {@link IpcInet6Address} object.
	 * <ul>
	 *   <li>{@link IpcStruct#get(String)}</li>
	 *   <li>{@link IpcStruct#get(String, int)}</li>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetIpcInet6Address()
	{
		testAccessor(new IpcInet6AddressFieldTest());
	}

	/**
	 * Test case for accessor which uses {@code Inet6Address} value.
	 * <ul>
	 *   <li>{@link IpcStruct#getInetAddress(String)}</li>
	 *   <li>{@link IpcStruct#getInetAddress(String, int)}</li>
	 *   <li>{@link IpcStruct#setInetAddress(String, InetAddress)}</li>
	 *   <li>{@link IpcStruct#setInetAddress(String, int, InetAddress)}</li>
	 *   <li>{@link IpcStruct#set(String, InetAddress)}</li>
	 *   <li>{@link IpcStruct#set(String, int, InetAddress)}</li>
	 * </ul>
	 */
	public void testGetSetInet6Address()
	{
		testAccessor(new Inet6AddressFieldTest());
		testAccessor(new Inet6AddressFieldSetTest());
	}

	/**
	 * Ensure an {@link IpcStruct} object associated with can be set to
	 * another IPC structure.
	 * <ul>
	 *   <li>{@link IpcStruct#set(String, IpcDataUnit)}</li>
	 *   <li>{@link IpcStruct#set(String, int, IpcDataUnit)}</li>
	 * </ul>
	 */
	public void testGetSetStruct()
	{
		String[] stnames = {
			"ut_struct_1",
			"ut_struct_2",
			"ut_struct_3",
			"ut_struct_4",
		};

		for (String stname: stnames) {
			testAccessor(new InnerStructFieldTest(stname));
		}
	}

	/**
	 * Test case for {@code byte} array accessor.
	 * <ul>
	 *   <li>{@link IpcStruct#getByteArray(String)}</li>
	 *   <li>{@link IpcStruct#setByteArray(String, byte[])}</li>
	 *   <li>{@link IpcStruct#set(String, byte[])}</li>
	 * </ul>
	 */
	public void testByteArray()
	{
		testArrayAccessor(new ByteArrayFieldTest());
		testArrayAccessor(new ByteArrayFieldSetTest());
	}

	/**
	 * Test case for {@code short} array accessor.
	 * <ul>
	 *   <li>{@link IpcStruct#getShortArray(String)}</li>
	 *   <li>{@link IpcStruct#setShortArray(String, short[])}</li>
	 *   <li>{@link IpcStruct#set(String, short[])}</li>
	 * </ul>
	 */
	public void testShortArray()
	{
		testArrayAccessor(new ShortArrayFieldTest());
		testArrayAccessor(new ShortArrayFieldSetTest());
	}

	/**
	 * Test case for {@code int} array accessor.
	 * <ul>
	 *   <li>{@link IpcStruct#getIntArray(String)}</li>
	 *   <li>{@link IpcStruct#setIntArray(String, int[])}</li>
	 *   <li>{@link IpcStruct#set(String, int[])}</li>
	 * </ul>
	 */
	public void testIntArray()
	{
		testArrayAccessor(new IntArrayFieldTest());
		testArrayAccessor(new IntArrayFieldSetTest());
	}

	/**
	 * Test case for {@code long} array accessor.
	 * <ul>
	 *   <li>{@link IpcStruct#getLongArray(String)}</li>
	 *   <li>{@link IpcStruct#setLongArray(String, long[])}</li>
	 *   <li>{@link IpcStruct#set(String, long[])}</li>
	 * </ul>
	 */
	public void testLongArray()
	{
		testArrayAccessor(new LongArrayFieldTest());
		testArrayAccessor(new LongArrayFieldSetTest());
	}

	/**
	 * Test case for {@code float} array accessor.
	 * <ul>
	 *   <li>{@link IpcStruct#getFloatArray(String)}</li>
	 *   <li>{@link IpcStruct#setFloatArray(String, float[])}</li>
	 *   <li>{@link IpcStruct#set(String, float[])}</li>
	 * </ul>
	 */
	public void testFloatArray()
	{
		testArrayAccessor(new FloatArrayFieldTest());
		testArrayAccessor(new FloatArrayFieldSetTest());
	}

	/**
	 * Test case for {@code double} array accessor.
	 * <ul>
	 *   <li>{@link IpcStruct#getDoubleArray(String)}</li>
	 *   <li>{@link IpcStruct#setDoubleArray(String, double[])}</li>
	 *   <li>{@link IpcStruct#set(String, double[])}</li>
	 * </ul>
	 */
	public void testDoubleArray()
	{
		testArrayAccessor(new DoubleArrayFieldTest());
		testArrayAccessor(new DoubleArrayFieldSetTest());
	}

	/**
	 * Test case for {@code InetAddress} array accessor.
	 * <ul>
	 *   <li>{@link IpcStruct#getInetAddressArray(String)}</li>
	 *   <li>{@link IpcStruct#setInetAddressArray(String, InetAddress[])}</li>
	 *   <li>{@link IpcStruct#set(String, InetAddress[])}</li>
	 * </ul>
	 */
	public void testInetAddressArray()
	{
		for (int type: new int[]{IpcDataUnit.IPV4,
					 IpcDataUnit.IPV6}) {
			testArrayAccessor(new InetAddressArrayFieldTest(type));
			testArrayAccessor(new InetAddressArrayFieldSetTest
					  (type));
		}
	}

	/**
	 * Test case for {@code String} array accessor.
	 * <ul>
	 *   <li>{@link IpcStruct#getString(String)}</li>
	 *   <li>{@link IpcStruct#setString(String, String)}</li>
	 *   <li>{@link IpcStruct#set(String, String)}</li>
	 * </ul>
	 */
	public void testString()
	{
		testArrayAccessor(new StringFieldTest());
		testArrayAccessor(new StringFieldSetTest());
	}

	/**
	 * Test case for {@link IpcStruct#clone()}.
	 */
	public void testClone()
	{
		IpcStruct ut1 = new IpcStruct("ut_struct_1");
		checkClone(ut1);

		// Ensure that clone() works on inner structure.
		IpcStruct ut3 = new IpcStruct("ut_struct_3");
		IpcStruct ut4 = new IpcStruct("ut_struct_4");
		TestStruct tst3 = TestStruct.create(ut3);
		TestStruct tst4 = TestStruct.create(ut4);
		tst3.randomize();
		tst4.randomize();
		assertTrue(tst3.equals());
		assertTrue(tst4.equals());

		IpcStruct[] tests = {
			ut3.getInner("ut3_struct1"),
			ut4.getInner("ut4_struct3", 0).getInner("ut3_struct1"),
			ut4.getInner("ut4_struct3", 1).getInner("ut3_struct1"),
			ut4.getInner("ut4_struct3", 2).getInner("ut3_struct1"),
		};

		for (IpcStruct struct: tests) {
			IpcStruct copy = (IpcStruct)struct.clone();
			checkClone(copy);
		}

		assertTrue(tst3.equals());
		assertTrue(tst4.equals());

		// Ensure that clone() works on inner structure.
		// data are modified in inner structure.
		for (IpcStruct struct: tests) {
			checkClone(struct);
		}

		assertFalse(tst3.equals());
		assertFalse(tst4.equals());
	}

	/**
	 * Ensure that {@link IpcStruct#get(String)} and
	 * {@link IpcStruct#get(String, int)} return a deep copy of the
	 * specified inner structure field.
	 */
	public void testGetStruct()
	{
		IpcStruct  ut3 = new IpcStruct("ut_struct_3");
		IpcStruct  ut4 = new IpcStruct("ut_struct_4");
		TestStruct.randomize(ut3);
		TestStruct.randomize(ut4);

		checkClone((IpcStruct)ut3.get("ut3_struct1"),
			   ut3.getInner("ut3_struct1"));
		IpcStructField field  = ut3.getField("ut3_struct2");
		int alen = field.getArrayLength();
		for (int i = 0; i < alen; i++) {
			checkClone((IpcStruct)ut3.get("ut3_struct2", i),
				   ut3.getInner("ut3_struct2", i));
		}

		field  = ut4.getField("ut4_struct3");
		int alen4 = field.getArrayLength();
		for (int i = 0; i < alen4; i++) {
			IpcStruct inner = ut4.getInner("ut4_struct3", i);

			checkClone((IpcStruct)ut4.get("ut4_struct3", i),
				   inner);
			for (int j = 0; j < alen; j++) {
				checkClone((IpcStruct)inner.
					   get("ut3_struct2", j),
					   inner.getInner("ut3_struct2", j));
			}
		}
	}

	/**
	 * Ensure that {@link IpcStruct#getField(String, String)}} returns
	 * correct value.
	 *
	 * @param stname	The name of IPC structure.
	 * @param tst		A {@link TestStruct} object.
	 * @param names		All field names.
	 */
	private void checkFields(String stname, TestStruct tst, String[] names)
	{
		HashMap<String, FieldDef> fmap = tst.getFieldMap();

		for (String name: names) {
			IpcStructField field =
				IpcStruct.getField(stname, name);
			FieldDef fdef = fmap.get(name);

			assertEquals(name, field.getName());
			assertEquals(stname, field.getStructureName());
			assertEquals("ipc_struct " + stname + "." + name,
				     field.toString());
			assertEquals(fdef.getType(), field.getType());
			assertEquals(fdef.isArray(), field.isArray());
			assertEquals(fdef.getArrayLength(),
				     field.getArrayLength());
			assertEquals(fdef.getNestedStructureName(),
				     field.getNestedStructureName());
		}

		// Specify null to field name.
		try {
			IpcStruct.getField(stname, null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "name is null.");
		}

		// Specify invalid field name.
		String[] invalid = {
			"",
			"_",
			" a",
			"_invalid_field",
			"あいうえお",
		};

		for (String inv: invalid) {
			try {
				IpcStruct.getField(stname, inv);
				needException();
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Unknown field name: " + inv);
			}
		}
	}

	/**
	 * Ensure that {@link IpcStruct#getField(String)} returns correct
	 * value.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param tst		A {@link TestStruct} object.
	 * @param names		All field names.
	 */
	private void checkFields(IpcStruct struct, TestStruct tst,
				 String[] names)
	{
		HashMap<String, FieldDef> fmap = tst.getFieldMap();

		for (String name: names) {
			IpcStructField field = struct.getField(name);
			FieldDef fdef = fmap.get(name);
			String nested = fdef.getNestedStructureName();
			int alen = fdef.getArrayLength();

			assertEquals(name, field.getName());
			assertEquals(struct.getName(),
				     field.getStructureName());
			assertEquals(struct.toString() + "." + name,
				     field.toString());
			assertEquals(fdef.getType(), field.getType());
			assertEquals(fdef.isArray(), field.isArray());
			assertEquals(alen, field.getArrayLength());
			assertEquals(nested, field.getNestedStructureName());

			if (nested == null) {
				continue;
			}

			// Test for inner structure.
			if (alen == 0) {
				IpcStruct inner = struct.getInner(name);

				checkInnerFields(inner, nested);
				continue;
			}

			for (int i = 0; i < alen; i++) {
				IpcStruct inner = struct.getInner(name, i);

				checkInnerFields(inner, nested);
			}
		}

		// Specify null to field name.
		try {
			struct.getField(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "name is null.");
		}

		// Specify invalid field name.
		String[] invalid = {
			"",
			"_",
			" a",
			"_invalid_field",
			"あいうえお",
		};

		for (String inv: invalid) {
			try {
				struct.getField(inv);
				needException();
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Unknown field name: " + inv);
			}
		}
	}

	/**
	 * Ensure that {@link IpcStruct#getField(String)} works for
	 * inner structure.
	 *
	 * @param inner		An {@link IpcStruct} object associated with
	 *			inner structure.
	 * @param nested	The name of {@code inner}.
	 */
	private void checkInnerFields(IpcStruct inner, String nested)
	{
		assertEquals(nested, inner.getName());

		TestStruct tst = TestStruct.create(inner);
		String[] names = inner.getFieldNames();
		String[] defnames = tst.getFieldNames();
		assertTrue(Arrays.equals(defnames, names));

		checkFields(inner, tst, names);
	}

	/**
	 * Common tests for scalar field accessor.
	 *
	 * @param impl		Field test implementation.
	 */
	private void testAccessor(TestFieldImpl impl)
	{
		String[] stnames = {
			"ut_struct_1",
			"ut_struct_2",
			// "ut_struct_3",
			// "ut_struct_4",
		};
		IpcDataUnit[] samples = impl.createSamples();

		for (String stname: stnames) {
			IpcStruct struct = new IpcStruct(stname);
			TestStruct tst = TestStruct.create(struct);

			// Fill the structure with random bytes.
			tst.randomize();

			for (IpcDataUnit data: samples) {
				tst.update(impl, struct, null, data);
			}
		}
	}

	/**
	 * Common tests for array field accessor.
	 *
	 * @param impl		Field test implementation.
	 */
	private void testArrayAccessor(TestArrayFieldImpl impl)
	{
		String[] stnames = {
			"ut_struct_1",
			"ut_struct_2",
			"ut_struct_3",
			"ut_struct_4",
		};

		for (String stname: stnames) {
			IpcStruct struct = new IpcStruct(stname);
			TestStruct tst = TestStruct.create(struct);

			// Set all bits in the IPC structure in order to
			// detect buffer overflow.
			tst.fill((byte)0xff);

			tst.arrayTest(impl, struct, null);
		}
	}

	/**
	 * Ensure that {@link IpcStruct#clone()} works correctly.
	 * Note that {@code struct} must be {@code ut_struct_1}.
	 *
	 * @param struct	An {@code IpcStruct} associated with
	 *			{@code ut_struct_1}.
	 */
	private void checkClone(IpcStruct struct)
	{
		// Create values to be set to IPC structure.
		HashMap<String,IpcDataUnit> vmap =
			new HashMap<String,IpcDataUnit>();

		vmap.put("ut1_int8", new IpcInt8((byte)1));
		vmap.put("ut1_uint8", new IpcUint8((byte)2));
		vmap.put("ut1_int16", new IpcInt16((short)3));
		vmap.put("ut1_uint16", new IpcUint16((short)4));
		vmap.put("ut1_int32", new IpcInt32(5));
		vmap.put("ut1_uint32", new IpcUint32(6));
		vmap.put("ut1_int64", new IpcInt64(7L));
		vmap.put("ut1_uint64", new IpcUint64(8L));
		vmap.put("ut1_float", new IpcFloat(2.7320508f));
		vmap.put("ut1_double", new IpcDouble(3.141592));

		InetAddress addr4 = null;
		InetAddress addr6 = null;
		try {
			addr4 = InetAddress.getByName("192.168.10.1");
			addr6 = InetAddress.getByName("::1");
		}
		catch (Exception e) {
			unexpected(e);
		}

		vmap.put("ut1_ipv4", IpcInetAddress.create(addr4));
		vmap.put("ut1_ipv6", IpcInetAddress.create(addr6));

		String[] names = struct.getFieldNames();
		for (String name: names) {
			IpcDataUnit data = vmap.get(name);
			struct.set(name, data);
			assertEquals(data, struct.get(name));
		}

		// Create a deep copy.
		IpcStruct copy = (IpcStruct)struct.clone();

		assertTrue(copy != struct);
		assertEquals("ut_struct_1", copy.getName());
		Long buf = (Long)getFieldValue(struct, "_buffer");
		Long cbuf = (Long)getFieldValue(copy, "_buffer");
		assertFalse(buf.equals(cbuf));
		Integer base = (Integer)getFieldValue(copy, "_baseOffset");
		assertEquals(0, base.intValue());
		Long info = (Long)getFieldValue(struct, "_info");
		Long cinfo = (Long)getFieldValue(copy, "_info");
		assertEquals(info, cinfo);

		for (String name: names) {
			IpcDataUnit data = vmap.get(name);
			assertEquals(data, copy.get(name));
		}

		// Update the source structure.
		HashMap<String,IpcDataUnit> nvmap =
			new HashMap<String,IpcDataUnit>();

		nvmap.put("ut1_int8", new IpcInt8((byte)10));
		nvmap.put("ut1_uint8", new IpcUint8((byte)20));
		nvmap.put("ut1_int16", new IpcInt16((short)300));
		nvmap.put("ut1_uint16", new IpcUint16((short)400));
		nvmap.put("ut1_int32", new IpcInt32(5000));
		nvmap.put("ut1_uint32", new IpcUint32(6000));
		nvmap.put("ut1_int64", new IpcInt64(70000L));
		nvmap.put("ut1_uint64", new IpcUint64(80000L));
		nvmap.put("ut1_float", new IpcFloat(1.4142f));
		nvmap.put("ut1_double", new IpcDouble(2.3620679));

		try {
			addr4 = InetAddress.getByName("10.1.123.254");
			addr6 = InetAddress.
				getByName("fe80::234:abcd:ffce:5678");
		}
		catch (Exception e) {
			unexpected(e);
		}

		nvmap.put("ut1_ipv4", IpcInetAddress.create(addr4));
		nvmap.put("ut1_ipv6", IpcInetAddress.create(addr6));
		for (String name: names) {
			IpcDataUnit data = nvmap.get(name);
			struct.set(name, data);
			assertEquals(data, struct.get(name));
		}

		// This does not affect a clone.
		for (String name: names) {
			IpcDataUnit data = vmap.get(name);
			assertEquals(data, copy.get(name));
		}
	}

	/**
	 * Ensure that {@code copy} is a deep copy of {@code struct}.
	 */
	private void checkClone(IpcStruct copy, IpcStruct struct)
	{
		assertEquals(IpcDataUnit.STRUCT, struct.getType());
		assertEquals(IpcDataUnit.STRUCT, copy.getType());
		checkEquals(struct, copy);

		Long buf = (Long)getFieldValue(struct, "_buffer");
		Long cbuf = (Long)getFieldValue(copy, "_buffer");
		assertFalse(buf.equals(cbuf));
		Integer base = (Integer)getFieldValue(copy, "_baseOffset");
		assertEquals(0, base.intValue());
		Long info = (Long)getFieldValue(struct, "_info");
		Long cinfo = (Long)getFieldValue(copy, "_info");
		assertEquals(info, cinfo);

		// Modifying `copy' must not affect `struct'.
		IpcStruct clone = (IpcStruct)struct.clone();
		TestStruct.randomize(copy);
		checkEquals(clone, struct);
	}

	/**
	 * Load IPC structure information for JUnit tests.
	 *
	 * @return  {@code true} is returned only if IPC structure information
	 *          was successfully loaded.
	 */
	private static native boolean load();
}

/**
 * Structure field definition.
 */
class FieldDef
{
	/**
	 * Field name.
	 */
	private final String  _name;

	/**
	 * Data type identifier.
	 */
	private final int  _type;

	/**
	 * Array length of this field.
	 * Zero is set for non-array field.
	 */
	private final int  _arrayLength;

	/**
	 * Structure name of this field.
	 * null is set if non-structure field.
	 */
	private final String  _nested;

	/**
	 * Construct a field definition.
	 *
	 * @param name		The field name.
	 * @param type		Data type identifier.
	 */
	FieldDef(String name, int type)
	{
		this(name, type, 0, null);
	}

	/**
	 * Construct a field definition.
	 *
	 * @param name		The field name.
	 * @param type		Data type identifier.
	 * @param alen		Array length.
	 */
	FieldDef(String name, int type, int alen)
	{
		this(name, type, alen, null);
	}

	/**
	 * Construct a new field definition.
	 *
	 * @param name		The field name.
	 * @param type		Data type identifier.
	 * @param alen		Array length.
	 * @param nested	Nested structure name.
	 */
	FieldDef(String name, int type, int alen, String nested)
	{
		_name = name;
		_type = type;
		_arrayLength = alen;
		_nested = nested;
	}

	/**
	 * Return the name of this field.
	 *
	 * @return	The field name.
	 */
	String getName()
	{
		return _name;
	}

	/**
	 * Return a data type identifier of this field.
	 *
	 * @return	A data type identifier.
	 */
	int getType()
	{
		return _type;
	}

	/**
	 * Return the array length of this field.
	 * Zero is returned if this field is non-array field.
	 *
	 * @return	The length of array field.
	 */
	int getArrayLength()
	{
		return _arrayLength;
	}

	/**
	 * Determine whether this field is an array field or not.
	 *
	 * @return	True if this field is an array field.
	 *		Otherwise false.
	 */
	boolean isArray()
	{
		return (_arrayLength != 0);
	}

	/**
	 * Return the name of nested structure.
	 * null is returned if this field is not nested structure field.
	 *
	 * @return	The name of nested structure.
	 */
	String getNestedStructureName()
	{
		return _nested;
	}
}

/**
 * Implementation of structure field test.
 */
abstract class TestFieldImpl
{
	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	abstract IpcDataUnit[] createSamples();

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	abstract int[] getValidTypes();

	/**
	 * Determine whether the specified data type can be handled by this
	 * object.
	 *
	 * @param field		Field information.
	 * @return		True if the specified type can be handled.
	 *			Otherwise false.
	 */
	boolean isMatched(IpcStructField field)
	{
		int ftype = field.getType();

		for (int t: getValidTypes()) {
			if (t == ftype) {
				return true;
			}
		}

		return false;
	}

	/**
	 * Determine whether the specified objects are identical or not.
	 *
	 * @param data1		A data to be compared.
	 * @param data2		A data to be compared.
	 * @return		True if identical. Otherwise false.
	 */
	boolean equals(IpcDataUnit data1, IpcDataUnit data2)
	{
		return data1.equals(data2);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	abstract IpcDataUnit get(IpcStruct struct, String name);

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	abstract IpcDataUnit get(IpcStruct struct, String name, int index);

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	abstract void set(IpcStruct struct, String name, IpcDataUnit data);

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	abstract void set(IpcStruct struct, String name, int index,
			  IpcDataUnit data);

	/**
	 * Update the structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	abstract void update(TestStruct tst, String key, int index,
			     IpcDataUnit data, IpcDataUnit cur);

	/**
	 * Ensure that an exception is caused by data type mismatch.
	 *
	 * @param e		An exception to be tested.
	 * @param field		Field definition.
	 * @param badtypemsg	Expected error message if the data type is
	 *			{@link IpcDataUnit#STRING} or
	 *			{@link IpcDataUnit#BINARY}.
	 */
	void checkException(Exception e, IpcStructField field,
			    String badtypemsg)
	{
		String msg;

		if (badtypemsg != null) {
			msg = badtypemsg;
		}
		else {
			int[] types = getValidTypes();
			String ftype = TestStruct.getTypeName(field.getType());

			msg = "Data type of the field \"" + field.getName() +
				"\" is ";
			if (types.length == 1) {
				msg += "not " +
					TestStruct.getTypeName(types[0]);
			}
			else {
				msg += "neither " +
					TestStruct.getTypeName(types[0]) +
					" nor " +
					TestStruct.getTypeName(types[1]);
			}
			msg += ": type=" + ftype;
		}

		TestStruct.checkException(e, IllegalArgumentException.class,
					  msg);
	}

	/**
	 * Ensure that an invalid field name is rejected.
	 *
	 * @param data		A dummy {@link IpcDataUnit} object.
	 */
	void checkInvalidField(IpcStruct struct, IpcDataUnit data)
	{
		checkInvalidField(struct, data, null, null);
	}

	/**
	 * Ensure that an invalid field name is rejected.
	 *
	 * @param data		A dummy {@link IpcDataUnit} object.
	 * @param badcls	A specified error class for unsupported type.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	void checkInvalidField(IpcStruct struct, IpcDataUnit data,
			       Class<?> badcls, String badtypemsg)
	{
		// Specify null field name.
		Class<?> acls = null;
		Class<?> cls = null;
		String   msg = null;
		String   amsg = null;

		acls = NullPointerException.class;
		cls = (badcls == null) ? acls : badcls;
		amsg = "name is null.";
		msg = (badtypemsg == null) ? amsg : badtypemsg;

		try {
			get(struct, null);
			TestStruct.needException();
		}
		catch (Exception e) {
			TestStruct.checkException(e, acls, amsg);
		}

		try {
			get(struct, null, 0);
			TestStruct.needException();
		}
		catch (Exception e) {
			TestStruct.checkException(e, acls, amsg);
		}

		try {
			set(struct, null, data);
			TestStruct.needException();
		}
		catch (Exception e) {
			TestStruct.checkException(e, cls, msg);
		}

		try {
			set(struct, null, 0, data);
			TestStruct.needException();
		}
		catch (Exception e) {
			TestStruct.checkException(e, cls, msg);
		}

		// Specify invalid field name.
		String[] invalid = {
			"",
			"_",
			" a",
			"_invalid_field",
			"ut10_int200",
			"あいうえお",
		};

		acls = IllegalArgumentException.class;
		cls = (badcls == null) ? acls : badcls;
		for (String inv: invalid) {
			amsg =  "Unknown field name: " + inv;
			msg = (badtypemsg == null) ? amsg : badtypemsg;

			try {
				get(struct, inv);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, acls, amsg);
			}

			try {
				get(struct, inv, 0);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, acls, amsg);
			}

			try {
				set(struct, inv, data);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, cls, msg);
			}

			try {
				set(struct, inv, 0, data);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, cls, msg);
			}
		}
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	void checkGetUnmatchedField(IpcStruct struct, String name,
				    IpcStructField field, String badtypemsg) {}

	/**
	 * Ensure that an unmatched type and array field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index         The array index.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	void checkGetUnmatchedField(IpcStruct struct, String name, int index,
				    IpcStructField field, String badtypemsg) {}
}

/**
 * Base class of test implementation which uses {@link IpcDataUnit}.
 */
abstract class IpcDataUnitFieldTest extends TestFieldImpl
{
	/**
	 * Data type identifier.
	 */
	private final int  _type;

	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	IpcDataUnitFieldTest(int type)
	{
		_type = type;
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{_type};
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name)
	{
		return struct.get(name);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name, int index)
	{
		return struct.get(name, index);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		struct.set(name, data);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		struct.set(name, index, data);
	}
}

/**
 * Implementation of {@link IpcInt8} test.
 */
class IpcInt8FieldTest extends IpcDataUnitFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcInt8FieldTest()
	{
		super(IpcDataUnit.INT8);
	}

	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	IpcInt8FieldTest(int type)
	{
		super(type);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcInt8((byte)1),
			new IpcInt8((byte)0xff),
		};
	}

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		assertEquals(tst.getByte(key, index),
			     ((IpcNumber)cur).byteValue());
		tst.setByte(key, index, ((IpcNumber)data).byteValue());
	}
}

/**
 * Implementation of {@link IpcUint8} test.
 */
class IpcUint8FieldTest extends IpcInt8FieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcUint8FieldTest()
	{
		super(IpcDataUnit.UINT8);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcUint8((byte)1),
			new IpcUint8((byte)0xff),
		};
	}
}

/**
 * Implementation of {@code byte} accessor tests.
 */
class ByteFieldTest extends IpcInt8FieldTest
{
	/**
	 * Construct a new test object.
	 */
	ByteFieldTest()
	{
		super();
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT8, IpcDataUnit.UINT8};
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name)
	{
		byte b = struct.getByte(name);

		return new IpcInt8(b);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name, int index)
	{
		byte b = struct.getByte(name, index);

		return new IpcInt8(b);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		byte b = ((IpcNumber)data).byteValue();

		struct.setByte(name, b);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		byte b = ((IpcNumber)data).byteValue();

		struct.setByte(name, index, b);
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name, int index,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name, index);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, byte)} and
 * {@link IpcStruct#set(String, int, byte)} test.
 */
class ByteFieldSetTest extends ByteFieldTest
{
	/**
	 * Construct a new test object.
	 */
	ByteFieldSetTest()
	{
		super();
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		byte b = ((IpcNumber)data).byteValue();

		struct.set(name, b);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		byte b = ((IpcNumber)data).byteValue();

		struct.set(name, index, b);
	}
}

/**
 * Implementation of {@link IpcInt16} test.
 */
class IpcInt16FieldTest extends IpcDataUnitFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcInt16FieldTest()
	{
		super(IpcDataUnit.INT16);
	}

	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	IpcInt16FieldTest(int type)
	{
		super(type);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcInt16((short)1),
			new IpcInt16((short)0xffff),
		};
	}

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		assertEquals(tst.getShort(key, index),
			     ((IpcNumber)cur).shortValue());
		tst.setShort(key, index, ((IpcNumber)data).shortValue());
	}
}

/**
 * Implementation of {@link IpcUint16} test.
 */
class IpcUint16FieldTest extends IpcInt16FieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcUint16FieldTest()
	{
		super(IpcDataUnit.UINT16);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcUint16((short)1),
			new IpcUint16((short)0xffff),
		};
	}
}

/**
 * Implementation of {@code short} accessor tests.
 */
class ShortFieldTest extends IpcInt16FieldTest
{
	/**
	 * Construct a new test object.
	 */
	ShortFieldTest()
	{
		super();
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT16, IpcDataUnit.UINT16};
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name)
	{
		short v = struct.getShort(name);

		return new IpcInt16(v);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name, int index)
	{
		short v = struct.getShort(name, index);

		return new IpcInt16(v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		short v = ((IpcNumber)data).shortValue();

		struct.setShort(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		short v = ((IpcNumber)data).shortValue();

		struct.setShort(name, index, v);
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name, int index,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name, index);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, short)} and
 * {@link IpcStruct#set(String, int, short)} test.
 */
class ShortFieldSetTest extends ShortFieldTest
{
	/**
	 * Construct a new test object.
	 */
	ShortFieldSetTest()
	{
		super();
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		short v = ((IpcNumber)data).shortValue();

		struct.set(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		short v = ((IpcNumber)data).shortValue();

		struct.set(name, index, v);
	}
}

/**
 * Implementation of {@link IpcInt32} test.
 */
class IpcInt32FieldTest extends IpcDataUnitFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcInt32FieldTest()
	{
		super(IpcDataUnit.INT32);
	}

	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	IpcInt32FieldTest(int type)
	{
		super(type);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcInt32(1),
			new IpcInt32(0xffffffff),
		};
	}

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		assertEquals(tst.getInt(key, index),
			     ((IpcNumber)cur).intValue());
		tst.setInt(key, index, ((IpcNumber)data).intValue());
	}
}

/**
 * Implementation of {@link IpcUint32} test.
 */
class IpcUint32FieldTest extends IpcInt32FieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcUint32FieldTest()
	{
		super(IpcDataUnit.UINT32);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcUint32(1),
			new IpcUint32(0xffffffff),
		};
	}
}

/**
 * Implementation of {@code int} accessor tests.
 */
class IntFieldTest extends IpcInt32FieldTest
{
	/**
	 * Construct a new test object.
	 */
	IntFieldTest()
	{
		super();
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT32, IpcDataUnit.UINT32};
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name)
	{
		int v = struct.getInt(name);

		return new IpcInt32(v);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name, int index)
	{
		int v = struct.getInt(name, index);

		return new IpcInt32(v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		int v = ((IpcNumber)data).intValue();

		struct.setInt(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		int v = ((IpcNumber)data).intValue();

		struct.setInt(name, index, v);
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name, int index,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name, index);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, int)} and
 * {@link IpcStruct#set(String, int, int)} test.
 */
class IntFieldSetTest extends IntFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IntFieldSetTest()
	{
		super();
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		int v = ((IpcNumber)data).intValue();

		struct.set(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		int v = ((IpcNumber)data).intValue();

		struct.set(name, index, v);
	}
}

/**
 * Implementation of {@link IpcInt64} test.
 */
class IpcInt64FieldTest extends IpcDataUnitFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcInt64FieldTest()
	{
		super(IpcDataUnit.INT64);
	}

	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	IpcInt64FieldTest(int type)
	{
		super(type);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcInt64(1L),
			new IpcInt64(-1L),
		};
	}

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		assertEquals(tst.getLong(key, index),
			     ((IpcNumber)cur).longValue());
		tst.setLong(key, index, ((IpcNumber)data).longValue());
	}
}

/**
 * Implementation of {@link IpcUint64} test.
 */
class IpcUint64FieldTest extends IpcInt64FieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcUint64FieldTest()
	{
		super(IpcDataUnit.UINT64);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcUint64(1L),
			new IpcUint64(-1L),
		};
	}
}

/**
 * Implementation of {@code long} accessor tests.
 */
class LongFieldTest extends IpcInt64FieldTest
{
	/**
	 * Construct a new test object.
	 */
	LongFieldTest()
	{
		super();
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT64, IpcDataUnit.UINT64};
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name)
	{
		long v = struct.getLong(name);

		return new IpcInt64(v);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name, int index)
	{
		long v = struct.getLong(name, index);

		return new IpcInt64(v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		long v = ((IpcNumber)data).longValue();

		struct.setLong(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		long v = ((IpcNumber)data).longValue();

		struct.setLong(name, index, v);
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name, int index,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name, index);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, long)} and
 * {@link IpcStruct#set(String, int, long)} test.
 */
class LongFieldSetTest extends LongFieldTest
{
	/**
	 * Construct a new test object.
	 */
	LongFieldSetTest()
	{
		super();
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		long v = ((IpcNumber)data).longValue();

		struct.set(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		long v = ((IpcNumber)data).longValue();

		struct.set(name, index, v);
	}
}

/**
 * Implementation of {@link IpcFloat} test.
 */
class IpcFloatFieldTest extends IpcDataUnitFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcFloatFieldTest()
	{
		super(IpcDataUnit.FLOAT);
	}

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		assertEquals(tst.getFloat(key, index),
			     ((IpcNumber)cur).floatValue());
		tst.setFloat(key, index, ((IpcNumber)data).floatValue());
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcFloat(Float.NaN),
			new IpcFloat(3.1415f),
		};
	}
}

/**
 * Implementation of {@code float} accessor tests.
 */
class FloatFieldTest extends IpcFloatFieldTest
{
	/**
	 * Construct a new test object.
	 */
	FloatFieldTest()
	{
		super();
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name)
	{
		float v = struct.getFloat(name);

		return new IpcFloat(v);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name, int index)
	{
		float v = struct.getFloat(name, index);

		return new IpcFloat(v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		float v = ((IpcNumber)data).floatValue();

		struct.setFloat(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		float v = ((IpcNumber)data).floatValue();

		struct.setFloat(name, index, v);
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name, int index,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name, index);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, float)} and
 * {@link IpcStruct#set(String, int, float)} test.
 */
class FloatFieldSetTest extends FloatFieldTest
{
	/**
	 * Construct a new test object.
	 */
	FloatFieldSetTest()
	{
		super();
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		float v = ((IpcNumber)data).floatValue();

		struct.set(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		float v = ((IpcNumber)data).floatValue();

		struct.set(name, index, v);
	}
}

/**
 * Implementation of {@link IpcDouble} test.
 */
class IpcDoubleFieldTest extends IpcDataUnitFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcDoubleFieldTest()
	{
		super(IpcDataUnit.DOUBLE);
	}

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		assertEquals(tst.getDouble(key, index),
			     ((IpcNumber)cur).doubleValue());
		tst.setDouble(key, index, ((IpcNumber)data).doubleValue());
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{
			new IpcDouble(Double.NaN),
			new IpcDouble(2.71828182845904523536),
		};
	}
}

/**
 * Implementation of {@code double} accessor tests.
 */
class DoubleFieldTest extends IpcDoubleFieldTest
{
	/**
	 * Construct a new test object.
	 */
	DoubleFieldTest()
	{
		super();
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name)
	{
		double v = struct.getDouble(name);

		return new IpcDouble(v);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	IpcDataUnit get(IpcStruct struct, String name, int index)
	{
		double v = struct.getDouble(name, index);

		return new IpcDouble(v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		double v = ((IpcNumber)data).doubleValue();

		struct.setDouble(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		double v = ((IpcNumber)data).doubleValue();

		struct.setDouble(name, index, v);
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name, int index,
				    IpcStructField field, String badtypemsg)
	{
		try {
			get(struct, name, index);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, badtypemsg);
		}
	}
}


/**
 * Implementation of {@link IpcStruct#set(String, double)} and
 * {@link IpcStruct#set(String, int, double)} test.
 */
class DoubleFieldSetTest extends DoubleFieldTest
{
	/**
	 * Construct a new test object.
	 */
	DoubleFieldSetTest()
	{
		super();
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		double v = ((IpcNumber)data).doubleValue();

		struct.set(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		double v = ((IpcNumber)data).doubleValue();

		struct.set(name, index, v);
	}
}

/**
 * Base class of test implementation which handles {@link IpcInetAddress}.
 */
abstract class IpcInetAddressFieldTest extends IpcDataUnitFieldTest
{
	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	IpcInetAddressFieldTest(int type)
	{
		super(type);
	}

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		assertEquals(tst.getInetAddress(key, index),
			     ((IpcInetAddress)cur).getValue());
		InetAddress iaddr = ((IpcInetAddress)data).getValue();
		tst.setInetAddress(key, index, iaddr);
	}

	/**
	 * Create {@link IpcInet4Address} sample data.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	IpcDataUnit[] createSamplesInet4()
	{
		IpcDataUnit[] ret = new IpcDataUnit[2];

		for (int i = 0; i < ret.length; i++) {
			byte[] raw = {
				(byte)(10 + i),
				(byte)(50 + i * 2),
				(byte)(100 + i * 3),
				(byte)(150 + i * 4),
			};

			ret[i] = IpcInetAddress.create(raw);
		};

		return ret;
	}

	/**
	 * Create {@link IpcInet6Address} sample data.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	IpcDataUnit[] createSamplesInet6()
	{
		IpcDataUnit[] ret = new IpcDataUnit[2];

		for (int i = 0; i < ret.length; i++) {
			byte[] raw = {
				(byte)(1 + i * 16),
				(byte)(10 + i * 15),
				(byte)(20 + i * 14),
				(byte)(30 + i * 13),
				(byte)(40 + i * 12),
				(byte)(50 + i * 11),
				(byte)(60 + i * 10),
				(byte)(70 + i * 9),
				(byte)(80 + i * 8),
				(byte)(90 + i * 7),
				(byte)(100 + i * 6),
				(byte)(110 + i * 5),
				(byte)(120 + i * 4),
				(byte)(130 + i * 3),
				(byte)(140 + i * 2),
				(byte)(150 + i),
			};

			ret[i] = IpcInetAddress.create(raw);
		};

		return ret;
	}
}

/**
 * Implementation of {@link IpcInet4Address} test.
 */
class IpcInet4AddressFieldTest extends IpcInetAddressFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcInet4AddressFieldTest()
	{
		super(IpcDataUnit.IPV4);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	final IpcDataUnit[] createSamples()
	{
		return createSamplesInet4();
	}
}

/**
 * Implementation of {@link IpcInet6Address} test.
 */
class IpcInet6AddressFieldTest extends IpcInetAddressFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcInet6AddressFieldTest()
	{
		super(IpcDataUnit.IPV6);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	final IpcDataUnit[] createSamples()
	{
		return createSamplesInet6();
	}
}

/**
 * Base class of {@code InetAddress} accessor test implementation.
 */
abstract class InetAddressFieldTest extends IpcInetAddressFieldTest
{
	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	InetAddressFieldTest(int type)
	{
		super(type);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	final IpcDataUnit get(IpcStruct struct, String name)
	{
		InetAddress v = struct.getInetAddress(name);

		return IpcInetAddress.create(v);
	}

	/**
	 * Return a value of the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @return		An {@code IpcDataUnit} object which contains
	 *			the value.
	 */
	@Override
	final IpcDataUnit get(IpcStruct struct, String name, int index)
	{
		InetAddress v = struct.getInetAddress(name, index);

		return IpcInetAddress.create(v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		InetAddress v = ((IpcInetAddress)data).getValue();

		struct.setInetAddress(name, v);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		InetAddress v = ((IpcInetAddress)data).getValue();

		struct.setInetAddress(name, index, v);
	}

	/**
	 * Set a value to the specified field using
	 * {@link IpcStruct#set(String, InetAddress)} or
	 * {@link IpcStruct#set(String, int, InetAddress)}.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	protected void setAddress(IpcStruct struct, String name,
				  IpcDataUnit data)
	{
		InetAddress v = ((IpcInetAddress)data).getValue();

		struct.set(name, v);
	}

	/**
	 * Set a value to the specified field using
	 * {@link IpcStruct#set(String, InetAddress)} or
	 * {@link IpcStruct#set(String, int, InetAddress)}.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	protected void setAddress(IpcStruct struct, String name, int index,
				  IpcDataUnit data)
	{
		InetAddress v = ((IpcInetAddress)data).getValue();

		struct.set(name, index, v);
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name,
				    IpcStructField field, String badtypemsg)
	{
		int ftype = field.getType();
		if ((ftype == IpcDataUnit.IPV4) || (ftype == IpcDataUnit.IPV6)) {
			return;
		}

		String amsg = "Data type of the field \"" + name + "\" is " +
				"neither IPV4 nor IPV6: type=" +
				TestStruct.getTypeName(ftype);
		String msg = (badtypemsg == null) ? amsg : badtypemsg;
			
		try {
			get(struct, name);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, msg);
		}
	}

	/**
	 * Ensure that an unmatched type field is rejected.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param field		Field information.
	 * @param badtypemsg	A specified error message for unsupported type.
	 */
	@Override
	void checkGetUnmatchedField(IpcStruct struct, String name, int index,
				    IpcStructField field, String badtypemsg)
	{
		int ftype = field.getType();
		if ((ftype == IpcDataUnit.IPV4) || (ftype == IpcDataUnit.IPV6)) {
			return;
		}

		String amsg = "Data type of the field \"" + name + "\" is " +
				"neither IPV4 nor IPV6: type=" +
				TestStruct.getTypeName(ftype);
		String msg = (badtypemsg == null) ? amsg : badtypemsg;

		try {
			get(struct, name, index);
			TestStruct.needException();
		}
		catch (Exception e) {
			checkException(e, field, msg);
		}
	}
}

/**
 * Implementation of {@code Inet4Address} accessor tests.
 */
class Inet4AddressFieldTest extends InetAddressFieldTest
{
	/**
	 * Construct a new test object.
	 */
	Inet4AddressFieldTest()
	{
		super(IpcDataUnit.IPV4);
	}


	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	final IpcDataUnit[] createSamples()
	{
		return createSamplesInet4();
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, InetAddress)} and
 * {@link IpcStruct#set(String, int, InetAddress)} test which handles
 * {@code Inet4Address} value.
 */
class Inet4AddressFieldSetTest extends Inet4AddressFieldTest
{
	/**
	 * Construct a new test object.
	 */
	Inet4AddressFieldSetTest()
	{
		super();
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		setAddress(struct, name, data);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		setAddress(struct, name, index, data);
	}
}

/**
 * Implementation of {@code Inet6Address} accessor tests.
 */
class Inet6AddressFieldTest extends InetAddressFieldTest
{
	/**
	 * Construct a new test object.
	 */
	Inet6AddressFieldTest()
	{
		super(IpcDataUnit.IPV6);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	final IpcDataUnit[] createSamples()
	{
		return createSamplesInet6();
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, InetAddress)} and
 * {@link IpcStruct#set(String, int, InetAddress)} test which handles
 * {@code Inet6Address} value.
 */
class Inet6AddressFieldSetTest extends Inet6AddressFieldTest
{
	/**
	 * Construct a new test object.
	 */
	Inet6AddressFieldSetTest()
	{
		super();
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, IpcDataUnit data)
	{
		setAddress(struct, name, data);
	}

	/**
	 * Set a value to the specified field.
	 *
	 * @param struct	An {@code IpcStruct} object.
	 * @param name		The field name.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int index, IpcDataUnit data)
	{
		setAddress(struct, name, index, data);
	}
}

/**
 * Base class of {@link IpcStruct} accessor test Implementation.
 */
class InnerStructFieldTest extends IpcDataUnitFieldTest
{
	/**
	 * The name of the structure.
	 */
	protected final String  _structureName;

	/**
	 * Construct a new test object.
	 *
	 * @param stname	The name of the structure.
	 */
	InnerStructFieldTest(String stname)
	{
		super(IpcDataUnit.STRUCT);

		_structureName = stname;
	}

	/**
	 * Determine whether the specified data type can be handled by this
	 * object.
	 *
	 * @param field		Field information.
	 * @return		True if the specified type can be handled.
	 *			Otherwise false.
	 */
	boolean isMatched(IpcStructField field)
	{
		int ftype = field.getType();

		if (ftype != IpcDataUnit.STRUCT) {
			return false;
		}

		String nested = field.getNestedStructureName();
		assertNotNull(nested);

		return nested.equals(_structureName);
	}

	/**
	 * Determine whether the specified objects are identical or not.
	 *
	 * @param data1		A data to be compared.
	 * @param data2		A data to be compared.
	 * @return		True if identical. Otherwise false.
	 */
	boolean equals(IpcDataUnit data1, IpcDataUnit data2)
	{
		assertEquals(IpcDataUnit.STRUCT, data1.getType());
		assertEquals(IpcDataUnit.STRUCT, data2.getType());

		IpcStruct st1 = (IpcStruct)data1;
		IpcStruct st2 = (IpcStruct)data2;

		Long info1 = (Long)TestBase.getFieldValue(st1, "_info");
		Long info2 = (Long)TestBase.getFieldValue(st2, "_info");
		if (!info1.equals(info2)) {
			// Structure type differs.
			return false;
		}

		Long buf1 = (Long)TestBase.getFieldValue(st1, "_buffer");
		Long buf2 = (Long)TestBase.getFieldValue(st2, "_buffer");
		if (buf1.equals(buf2)) {
			// They share the same buffer.
			return true;
		}

		Integer base1 = (Integer)TestBase.
			getFieldValue(st1, "_baseOffset");
		Integer base2 = (Integer)TestBase.
			getFieldValue(st2, "_baseOffset");

		return equalsStruct(buf1.longValue(), base1.intValue(),
				    buf2.longValue(), base2.intValue(),
				    info1.longValue());
	}

	/**
	 * Determine whether the specified data is identical or not.
	 *
	 * @param buf1		Buffer for IPC structure 1.
	 * @param base1		Base offset for IPC structure 1.
	 * @param buf2		Buffer for IPC structure 2.
	 * @param base2		Base offset for IPC structure 2.
	 * @param info		Information about IPC structure.
	 * @return		True if identical. Otherwise false.
	 */
	private native boolean equalsStruct(long buf1, int base1, long buf2,
					    int base2, long info);

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		tst.setStruct(key, index, (IpcStruct)data);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	final IpcDataUnit[] createSamples()
	{
		IpcDataUnit[] ret = new IpcDataUnit[2];

		for (int i = 0; i < ret.length; i++) {
			IpcStruct struct = new IpcStruct(_structureName);
			ret[i] = struct;

			TestStruct.randomize(struct);
		}

		return ret;
	}

	/**
	 * Ensure that an exception is caused by data type mismatch.
	 *
	 * @param e		An exception to be tested.
	 * @param field		Field definition.
	 * @param badtypemsg	Expected error message if the data type is
	 *			{@link IpcDataUnit#STRING} or
	 *			{@link IpcDataUnit#BINARY}.
	 */
	@Override
	void checkException(Exception e, IpcStructField field,
			    String badtypemsg)
	{
		String msg;

		if (badtypemsg != null) {
			msg = badtypemsg;
		}
		else {
			int ftype = field.getType();

			if (ftype != IpcDataUnit.STRUCT) {
				msg = "Data type of the field \"" +
					field.getName() +
					"\" is not STRUCT: type=" +
					TestStruct.getTypeName(ftype);
			}
			else {
				msg = "Data type of the field \"" +
					field.getName() + "\" is \"" +
					field.getNestedStructureName() +
					"\", but \"" + _structureName +
					"\" is specified.";
			}
		}

		TestStruct.checkException(e, IllegalArgumentException.class,
					  msg);
	}
}

/**
 * Implementation of {@link IpcString}, {@link IpcBinary} and {@link IpcNull}
 * test.
 */
class IpcUnsupportedFieldTest extends IpcDataUnitFieldTest
{
	/**
	 * Construct a new test object.
	 */
	IpcUnsupportedFieldTest()
	{
		super(IpcDataUnit.STRING);
	}

	/**
	 * Create sample data to be set to an {@link IpcStruct} object.
	 *
	 * @return	An {@link IpcDataUnit} array which contains sample
	 *		data.
	 */
	@Override
	IpcDataUnit[] createSamples()
	{
		return new IpcDataUnit[]{new IpcString(), new IpcBinary(),
					 new IpcNull()};
	}

	/**
	 * Update the test structure field.
	 *
	 * @param tst		A {@link TestStruct} object.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param index		An array index, -1 must be specified for
	 *			scalar field.
	 * @param data		A value to be set.
	 * @param cur		Current value at the specified field.
	 */
	@Override
	final void update(TestStruct tst, String key, int index,
			  IpcDataUnit data, IpcDataUnit cur)
	{
		fail("This method should never ever be called.");
	}

	/**
	 * Ensure that an invalid field name is rejected.
	 *
	 * @param data		A dummy {@link IpcDataUnit} object.
	 */
	@Override
	void checkInvalidField(IpcStruct struct, IpcDataUnit data)
	{
		// decide exception class and bad messages for data type.
		Class<?> cls = null;
		String badtypemsg = null;

		int type = data.getType();
		if (type == IpcDataUnit.STRING) {
			cls = IllegalArgumentException.class;
			badtypemsg = "IpcString is not supported.";
		}
		else if (type == IpcDataUnit.BINARY) {
			cls = IllegalArgumentException.class;
			badtypemsg = "IpcBinary is not supported.";
		}
		else if (type == IpcDataUnit.NULL) {
			cls = IllegalArgumentException.class;
			badtypemsg = "IpcNull is not supported.";
		}

		checkInvalidField(struct, data, cls, badtypemsg);
	}
}

/**
 * Implementation of array structure field test.
 */
abstract class TestArrayFieldImpl
{
	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	abstract void get(IpcStruct struct, String name);

	/**
	 * Set an array to the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	abstract void set(IpcStruct struct, String name, int length);

	/**
	 * Ensure that the array setter checks the length of the specified
	 * array strictly.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param length	The capacity of the array field specified by
	 *			{@code name}.
	 */
	void checkLength(IpcStruct struct, String name, TestStruct tst,
			 int length)
	{
		for (int i = 0; i < length; i++) {
			String msg = "Array length(" + i +
				") does not match the field " + name +
				"[" + length + "].";
			Class<?> cls = IllegalArgumentException.class;

			try {
				set(struct, name, i);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, cls, msg);
			}

			assertTrue(tst.equals());
		}

		for (int i = length + 1; i <= length + 10; i++) {
			String msg = "Array length(" + i +
				") does not match the field " + name +
				"[" + length + "].";
			Class<?> cls = IllegalArgumentException.class;

			try {
				set(struct, name, i);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, cls, msg);
			}

			assertTrue(tst.equals());
		}
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The capacity of the array field specified by
	 *			{@code name}.
	 */
	abstract void check(IpcStruct struct, String name, TestStruct tst,
			    String key, int length);

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	abstract int[] getValidTypes();

	/**
	 * Return a string message which indicates null is specified as
	 * an array to be set.
	 *
	 * @return	A message set to {@code NullPointerException}.
	 */
	String getNullValueMessage()
	{
		return "value is null.";
	}

	/**
	 * Determine whether the specified data type can be handled by this
	 * object.
	 *
	 * @param field		Field information.
	 * @return		True if the specified type can be handled.
	 *			Otherwise false.
	 */
	boolean isMatched(IpcStructField field)
	{
		int ftype = field.getType();

		for (int t: getValidTypes()) {
			if (t == ftype) {
				return true;
			}
		}

		return false;
	}

	/**
	 * Determine whether the classified data type can be handled by this
	 * object.
	 *
	 * @param field		Field information.
	 * @return		True if the classified type can be handled.
	 *			Otherwise false.
	 */
	boolean isClassifiedMatched(IpcStructField field)
	{
		return isMatched(field);
	}

	/**
	 * Ensure that an exception is caused by data type mismatch.
	 *
	 * @param e		An exception to be tested.
	 * @param field		Field definition.
	 */
	void checkException(Exception e, IpcStructField field)
	{
		String msg;
		int[] types = getValidTypes();
		String ftype = TestStruct.getTypeName(field.getType());

		msg = "Data type of the field \"" + field.getName() +
			"\" is ";
		if (types.length == 1) {
			msg += "not " +
				TestStruct.getTypeName(types[0]);
		}
		else {
			msg += "neither " +
				TestStruct.getTypeName(types[0]) +
				" nor " +
				TestStruct.getTypeName(types[1]);
		}
		msg += ": type=" + ftype;

		TestStruct.checkException(e, IllegalArgumentException.class,
					  msg);
	}

	/**
	 * Throw an error which indicates an unexpected exception is caught.
	 *
	 * @param t	A throwable.
	 */
	protected static void unexpected(Throwable t)
	{
		throw new AssertionError("Unexpected exception: " + t, t);
	}
}

/**
 * Implementation of {@code byte} array accessor test.
 */
class ByteArrayFieldTest extends TestArrayFieldImpl
{
	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	@Override
	void get(IpcStruct struct, String name)
	{
		struct.getByteArray(name);
	}

	/**
	 * Set a {@code byte} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	void set(IpcStruct struct, String name, byte[] array)
	{
		struct.setByteArray(name, array);
	}

	/**
	 * Set an array to the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int length)
	{
		byte[] array = (length < 0) ? null : new byte[length];

		set(struct, name, array);
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 */
	@Override
	void check(IpcStruct struct, String name, TestStruct tst, String key,
		   int length)
	{
		// Verify current value.
		byte[] cur = struct.getByteArray(name);
		for (int i = 0; i < length; i++) {
			assertEquals(tst.getByte(key, i), cur[i]);
		}

		// Update the structure, and verify it.
		byte[] array = new byte[length];
		for (int i = 0; i < length; i++) {
			array[i] = (byte)(i * 2 + 1);
		}

		set(struct, name, array);

		for (int i = 0; i < length; i++) {
			assertFalse(tst.equals());
			tst.setByte(key, i, array[i]);
		}

		cur = struct.getByteArray(name);
		assertTrue(Arrays.equals(array, cur));
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT8, IpcDataUnit.UINT8};
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, byte[])} test.
 */
class ByteArrayFieldSetTest extends ByteArrayFieldTest
{
	/**
	 * Set a {@code byte} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, byte[] array)
	{
		struct.set(name, array);
	}
}

/**
 * Implementation of {@code short} array accessor test.
 */
class ShortArrayFieldTest extends TestArrayFieldImpl
{
	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	@Override
	void get(IpcStruct struct, String name)
	{
		struct.getShortArray(name);
	}

	/**
	 * Set a {@code short} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	void set(IpcStruct struct, String name, short[] array)
	{
		struct.setShortArray(name, array);
	}

	/**
	 * Set an array to the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int length)
	{
		short[] array = (length < 0) ? null : new short[length];

		set(struct, name, array);
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 */
	@Override
	void check(IpcStruct struct, String name, TestStruct tst, String key,
		   int length)
	{
		// Verify current value.
		short[] cur = struct.getShortArray(name);
		for (int i = 0; i < length; i++) {
			assertEquals(tst.getShort(key, i), cur[i]);
		}

		// Update the structure, and verify it.
		short[] array = new short[length];
		for (int i = 0; i < length; i++) {
			array[i] = (short)(i * 0x13 + 1);
		}

		set(struct, name, array);

		for (int i = 0; i < length; i++) {
			assertFalse(tst.equals());
			tst.setShort(key, i, array[i]);
		}

		cur = struct.getShortArray(name);
		assertTrue(Arrays.equals(array, cur));
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT16, IpcDataUnit.UINT16};
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, short[])} test.
 */
class ShortArrayFieldSetTest extends ShortArrayFieldTest
{
	/**
	 * Set a {@code short} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, short[] array)
	{
		struct.set(name, array);
	}
}

/**
 * Implementation of {@code int} array accessor test.
 */
class IntArrayFieldTest extends TestArrayFieldImpl
{
	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	@Override
	void get(IpcStruct struct, String name)
	{
		struct.getIntArray(name);
	}

	/**
	 * Set an {@code int} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	void set(IpcStruct struct, String name, int[] array)
	{
		struct.setIntArray(name, array);
	}

	/**
	 * Set an array to the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int length)
	{
		int[] array = (length < 0) ? null : new int[length];

		set(struct, name, array);
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 */
	@Override
	void check(IpcStruct struct, String name, TestStruct tst, String key,
		   int length)
	{
		// Verify current value.
		int[] cur = struct.getIntArray(name);
		for (int i = 0; i < length; i++) {
			assertEquals(tst.getInt(key, i), cur[i]);
		}

		// Update the structure, and verify it.
		int[] array = new int[length];
		for (int i = 0; i < length; i++) {
			array[i] = (i * 0x1234 + 1);
		}

		set(struct, name, array);

		for (int i = 0; i < length; i++) {
			assertFalse(tst.equals());
			tst.setInt(key, i, array[i]);
		}

		cur = struct.getIntArray(name);
		assertTrue(Arrays.equals(array, cur));
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT32, IpcDataUnit.UINT32};
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, int[])} test.
 */
class IntArrayFieldSetTest extends IntArrayFieldTest
{
	/**
	 * Set an {@code int} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int[] array)
	{
		struct.set(name, array);
	}
}

/**
 * Implementation of {@code long} array accessor test.
 */
class LongArrayFieldTest extends TestArrayFieldImpl
{
	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	@Override
	void get(IpcStruct struct, String name)
	{
		struct.getLongArray(name);
	}

	/**
	 * Set a {@code long} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	void set(IpcStruct struct, String name, long[] array)
	{
		struct.setLongArray(name, array);
	}

	/**
	 * Set an array to the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int length)
	{
		long[] array = (length < 0) ? null : new long[length];

		set(struct, name, array);
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 */
	@Override
	void check(IpcStruct struct, String name, TestStruct tst, String key,
		   int length)
	{
		// Verify current value.
		long[] cur = struct.getLongArray(name);
		for (int i = 0; i < length; i++) {
			assertEquals(tst.getLong(key, i), cur[i]);
		}

		// Update the structure, and verify it.
		long[] array = new long[length];
		for (int i = 0; i < length; i++) {
			array[i] = (long)i * 0x123456L + 1L;
		}

		set(struct, name, array);

		for (int i = 0; i < length; i++) {
			assertFalse(tst.equals());
			tst.setLong(key, i, array[i]);
		}

		cur = struct.getLongArray(name);
		assertTrue(Arrays.equals(array, cur));
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT64, IpcDataUnit.UINT64};
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, long[])} test.
 */
class LongArrayFieldSetTest extends LongArrayFieldTest
{
	/**
	 * Set a {@code long} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, long[] array)
	{
		struct.set(name, array);
	}
}

/**
 * Implementation of {@code float} array accessor test.
 */
class FloatArrayFieldTest extends TestArrayFieldImpl
{
	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	@Override
	void get(IpcStruct struct, String name)
	{
		struct.getFloatArray(name);
	}

	/**
	 * Set a {@code float} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	void set(IpcStruct struct, String name, float[] array)
	{
		struct.setFloatArray(name, array);
	}

	/**
	 * Set an array to the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int length)
	{
		float[] array = (length < 0) ? null : new float[length];

		set(struct, name, array);
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 */
	@Override
	void check(IpcStruct struct, String name, TestStruct tst, String key,
		   int length)
	{
		// Verify current value.
		float[] cur = struct.getFloatArray(name);
		for (int i = 0; i < length; i++) {
			assertEquals(tst.getFloat(key, i), cur[i]);
		}

		// Update the structure, and verify it.
		float[] array = new float[length];
		for (int i = 0; i < length; i++) {
			array[i] = (float)i * 1.41421356f + 3.14f;
		}

		set(struct, name, array);

		for (int i = 0; i < length; i++) {
			assertFalse(tst.equals());
			tst.setFloat(key, i, array[i]);
		}

		cur = struct.getFloatArray(name);
		assertTrue(Arrays.equals(array, cur));
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.FLOAT};
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, float[])} test.
 */
class FloatArrayFieldSetTest extends FloatArrayFieldTest
{
	/**
	 * Set a {@code float} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, float[] array)
	{
		struct.set(name, array);
	}
}

/**
 * Implementation of {@code double} array accessor test.
 */
class DoubleArrayFieldTest extends TestArrayFieldImpl
{
	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	@Override
	void get(IpcStruct struct, String name)
	{
		struct.getDoubleArray(name);
	}

	/**
	 * Set a {@code double} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	void set(IpcStruct struct, String name, double[] array)
	{
		struct.setDoubleArray(name, array);
	}

	/**
	 * Set an array to the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int length)
	{
		double[] array = (length < 0) ? null : new double[length];

		set(struct, name, array);
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 */
	@Override
	void check(IpcStruct struct, String name, TestStruct tst, String key,
		   int length)
	{
		// Verify current value.
		double[] cur = struct.getDoubleArray(name);
		for (int i = 0; i < length; i++) {
			assertEquals(tst.getDouble(key, i), cur[i]);
		}

		// Update the structure, and verify it.
		double[] array = new double[length];
		for (int i = 0; i < length; i++) {
			array[i] = (double)i * 3.141592 + 2.23620679;
		}

		set(struct, name, array);

		for (int i = 0; i < length; i++) {
			assertFalse(tst.equals());
			tst.setDouble(key, i, array[i]);
		}

		cur = struct.getDoubleArray(name);
		assertTrue(Arrays.equals(array, cur));
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.DOUBLE};
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, double[])} test.
 */
class DoubleArrayFieldSetTest extends DoubleArrayFieldTest
{
	/**
	 * Set a {@code double} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, double[] array)
	{
		struct.set(name, array);
	}
}

/**
 * Implementation of {@code InetAddress} array accessor test.
 */
class InetAddressArrayFieldTest extends TestArrayFieldImpl
{
	/**
	 * Required data type.
	 */
	private final int  _type;

	/**
	 * Random number generator.
	 */
	private final Random  _rand = new Random();

	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	InetAddressArrayFieldTest(int type)
	{
		_type = type;
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{_type};
	}

	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	@Override
	void get(IpcStruct struct, String name)
	{
		struct.getInetAddressArray(name);
	}

	/**
	 * Set an {@code InetAddress} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	void set(IpcStruct struct, String name, InetAddress[] array)
	{
		struct.setInetAddressArray(name, array);
	}

	/**
	 * Set an array to the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int length)
	{
		InetAddress[] array = (length < 0) ? null
			: createAddress(length);

		set(struct, name, array);
	}

	/**
	 * Ensure that an exception is caused by data type mismatch.
	 *
	 * @param e		An exception to be tested.
	 * @param field		Field definition.
	 */
	@Override
	void checkException(Exception e, IpcStructField field)
	{
		int ftype = field.getType();
		assertTrue(ftype != _type);

		String msg;
		if (ftype == IpcDataUnit.IPV4) {
			msg = "Unexpected address length at 0: length=16";
		}
		else if (ftype == IpcDataUnit.IPV6) {
			msg = "Unexpected address length at 0: length=4";
		}
		else {
			msg = "Data type of the field \"" + field.getName() +
				"\" is neither IPV4 nor IPV6: type=" +
				TestStruct.getTypeName(ftype);
		}

		TestStruct.checkException(e, IllegalArgumentException.class,
					  msg);
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 */
	@Override
	void check(IpcStruct struct, String name, TestStruct tst, String key,
		   int length)
	{
		// Verify current value.
		InetAddress[] cur = struct.getInetAddressArray(name);
		for (int i = 0; i < length; i++) {
			assertEquals(tst.getInetAddress(key, i), cur[i]);
		}

		InetAddress[] array = createAddress(length);

		// Embed a null to array elements.
		for (int i = 0; i < length; i++) {
			InetAddress[] invarray = array.clone();

			invarray[i] = null;
			try {
				set(struct, name, invarray);
				TestStruct.needException();
			}
			catch (NullPointerException e) {}

			assertTrue(tst.equals());
		}

		// Embed an invalid address into ths array.
		InetAddress invalid = createInvalidAddress();
		int invlen = invalid.getAddress().length;
		for (int i = 0; i < length; i++) {
			InetAddress[] invarray = array.clone();
			String msg = "Unexpected address length at " + i +
				": length=" + invlen;
			Class<?> cls = IllegalArgumentException.class;

			invarray[i] = invalid;
			try {
				set(struct, name, invarray);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, cls, msg);
			}

			assertTrue(tst.equals());
		}

		// Update the structure, and verify it.
		set(struct, name, array);

		for (int i = 0; i < length; i++) {
			assertFalse(tst.equals());
			tst.setInetAddress(key, i, array[i]);
		}

		cur = struct.getInetAddressArray(name);
		assertTrue(Arrays.equals(array, cur));
	}

	/**
	 * Return a string message which indicates null is specified as
	 * an array to be set.
	 *
	 * @return	A message set to {@code NullPointerException}.
	 */
	@Override
	String getNullValueMessage()
	{
		return null;
	}

	/**
	 * Determine whether the classified data type can be handled by this
	 * object.
	 *
	 * @param field		Field information.
	 * @return		True if the classified type can be handled.
	 *			Otherwise false.
	 */
	@Override
	boolean isClassifiedMatched(IpcStructField field)
	{
		int[] ftypes = {IpcDataUnit.IPV4,
				IpcDataUnit.IPV6};
		int ftype = field.getType();

		for (int t: ftypes) {
			if (t == ftype) {
				return true;
			}
		}

		return false;
	}


	/**
	 * Create an array of {@code InetAddress}.
	 *
	 * @param length	An array length.
	 */
	private InetAddress[] createAddress(int length)
	{
		InetAddress[] array = new InetAddress[length];
		int addrlen = (_type == IpcDataUnit.IPV4) ? 4 : 16;
		byte[] raw = new byte[addrlen];

		for (int i = 0; i < length; i++) {
			_rand.nextBytes(raw);
			try {
				array[i] = InetAddress.getByAddress(raw);
			}
			catch (Exception e) {
				unexpected(e);
			}
		}

		return array;
	}

	/**
	 * Create an invalid {@code InetAddress} object.
	 *
	 * @return	An invalid {@code InetAddress} object.
	 */
	private InetAddress createInvalidAddress()
	{
		String addr = (_type == IpcDataUnit.IPV4)
			? "::1" : "192.168.10.1";

		InetAddress iaddr = null;
		try {
			iaddr = InetAddress.getByName(addr);
		}
		catch (Exception e) {
			unexpected(e);
		}

		return iaddr;
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, InetAddress[])} test.
 */
class InetAddressArrayFieldSetTest extends InetAddressArrayFieldTest
{
	/**
	 * Construct a new test object.
	 *
	 * @param type		A data type identifier.
	 */
	InetAddressArrayFieldSetTest(int type)
	{
		super(type);
	}

	/**
	 * Set an {@code InetAddress} array.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param array		An array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, InetAddress[] array)
	{
		struct.set(name, array);
	}
}

/**
 * Implementation of {@code String} value accessor test.
 */
class StringFieldTest extends TestArrayFieldImpl
{
	/**
	 * UTF-8 character set.
	 */
	private final Charset  _charset;

	/**
	 * Random number generator.
	 */
	private final Random  _rand = new Random();

	/**
	 * The first character of byte sequence returned by
	 * {@link #createBytes(int)}.
	 */
	private short  _firstChar = 0x20;

	/**
	 * The first Unicode character returned by
	 * {@link #createUTF8Bytes(int)}.
	 */
	private char  _firstUnicodeChar = 0x3000;

	/**
	 * Construct a new test object.
	 */
	StringFieldTest()
	{
		try {
			_charset = Charset.forName("utf-8");
		}
		catch (Exception e) {
			throw new IllegalStateException
				("Unable to determine UTF-8 charset: " + e, e);
		}
	}

	/**
	 * Return an array which contains all valid data types.
	 *
	 * @return	An {@code int} array which contains all valid data
	 *		types.
	 */
	@Override
	int[] getValidTypes()
	{
		return new int[]{IpcDataUnit.INT8, IpcDataUnit.UINT8};
	}

	/**
	 * Get all elements in the specified array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 */
	@Override
	void get(IpcStruct struct, String name)
	{
		struct.getString(name);
	}

	/**
	 * Put a {@code String} into {@code byte} array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param str		A string to be set.
	 */
	void set(IpcStruct struct, String name, String str)
	{
		struct.setString(name, str);
	}

	/**
	 * Put a {@code String} into {@code byte} array field.
	 * This method is expected to throw an exception.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param length	The length of array to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, int length)
	{
		String str = (length < 0) ? null : createString(length);

		set(struct, name, str);
	}

	/**
	 * Ensure that the string setter checks the length of the specified
	 * string strictly.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param length	The capacity of the array field specified by
	 *			{@code name}.
	 */
	@Override
	void checkLength(IpcStruct struct, String name, TestStruct tst,
			 int length)
	{
		Class<?> cls = ArrayIndexOutOfBoundsException.class;

		// Try to set too long string.
		for (int len = length + 1; len <= length + 10; len++) {
			String msg = "Too long string: nbytes=" + len +
				", field=" + name + "[" + length + "]";

			try {
				set(struct, name, len);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, cls, msg);
			}

			assertTrue(tst.equals());
		}

		// Try to set too long string which contains UTF-8 characters.
		byte[] utf8 = "あいうえお".getBytes(_charset);

		for (int len = length + 1; len <= length + 10; len++) {
			byte[] b = new byte[len];

			for (int i = 0; i < len; i++) {
				b[i] = 'a';
			}
			if (len >= utf8.length) {
				System.arraycopy(utf8, 0, b, 0, utf8.length);
			}

			String str = new String(b, _charset);
			String msg = "Too long string: nbytes=" + len +
				", field=" + name + "[" + length + "]";

			try {
				set(struct, name, str);
				TestStruct.needException();
			}
			catch (Exception e) {
				TestStruct.checkException(e, cls, msg);
			}

			assertTrue(tst.equals());
		}
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 */
	@Override
	void check(IpcStruct struct, String name, TestStruct tst, String key,
		   int length)
	{
		// Verify current value.
		byte[] cur = struct.getByteArray(name);
		for (int i = 0; i < length; i++) {
			assertEquals(tst.getByte(key, i), cur[i]);
		}

		// Set an US-ASCII string.
		for (int len = 0; len <= length; len++) {
			checkString(struct, name, tst, key, length,
				    createBytes(len), cur);
		}

		// Set an UTF-8 string.
		for (int len = 0; len <= length; len++) {
			checkString(struct, name, tst, key, length,
				    createUTF8Bytes(len), cur);
		}
	}

	/**
	 * Ensure that the array field accessor works against the specified
	 * array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param tst		An {@link TestStruct} associated with
	 *			{@code struct}.
	 * @param key		The key which indicates value in {@code tst}.
	 * @param length	The length of the specified array field.
	 * @param bytes		A {@code byte} array to be set as string.
	 * @param current	A {@code byte} array which contains current
	 *			field value. Note that this method updates
	 *			this array.
	 */
	private void checkString(IpcStruct struct, String name, TestStruct tst,
				 String key, int length, byte[] bytes,
				 byte[] current)
	{
		String str = new String(bytes, _charset);

		// Update the structure, and verify it.
		set(struct, name, str);

		int len = bytes.length;
		for (int i = 0; i < len; i++) {
			assertFalse(tst.equals());
			tst.setByte(key, i, bytes[i]);
		}
		if (len < length) {
			// A string terminator must be copied.
			if (current[len] != (byte)0) {
				assertFalse(tst.equals());
				tst.setByte(key, len, (byte)0);
			}
		}

		assertTrue(tst.equals());
		assertEquals(str, struct.getString(name));

		System.arraycopy(bytes, 0, current, 0, len);
		if (bytes.length < length) {
			current[len] = 0;
		}

		byte[] cur = struct.getByteArray(name);
		assertTrue(Arrays.equals(current, cur));
	}

	/**
	 * Create an {@code byte} array which contains random US-ASCII
	 * characters.
	 *
	 * @param length	The number of bytes in a string to be created.
	 * @return		An {@code byte} array.
	 */
	private byte[] createBytes(int length)
	{
		byte[] b = new byte[length];

		if (length > 0) {
			b[0] = (byte)_firstChar;
			_firstChar++;
			if (_firstChar == 0x7f) {
				_firstChar = 0x20;
			}

			for (int i = 1; i < length; i++) {
				int c = _rand.nextInt(0x5f);
				b[i] = (byte)(c + 0x20);
			}
		}

		return b;
	}

	/**
	 * Create an {@code byte} array which contains random characters.
	 * This function embeds at least one UTF-8 character if possible.
	 *
	 * @param length	The number of bytes in a string to be created.
	 * @return		An {@code byte} array.
	 */
	private byte[] createUTF8Bytes(int length)
	{
		byte[] b = createBytes(length);

		final int uclen = 3;
		if (length < uclen) {
			// Too small.
			return b;
		}

		final int min = 0x3000;
		final int max = 0x30fe;
		final int range = max - min + 1;

		int clen = length / uclen;
		char[] c = new char[clen];
		for (int i = 0; i < clen; i++) {
			c[i] = (char)(_rand.nextInt(range) + min);
		}
		c[0] = _firstUnicodeChar;
		_firstUnicodeChar++;
		if (_firstUnicodeChar > max) {
			_firstUnicodeChar = min;
		}

		String ustr = new String(c);
		byte[] ub = ustr.getBytes(_charset);
		System.arraycopy(ub, 0, b, 0, ub.length);

		return b;
	}

	/**
	 * Create an US-ASCII string which contains random characters.
	 *
	 * @param length	The number of bytes in a string to be created.
	 * @return		An US-ASCII string.
	 */
	private String createString(int length)
	{
		return new String(createBytes(length), _charset);
	}
}

/**
 * Implementation of {@link IpcStruct#set(String, String)} test.
 */
class StringFieldSetTest extends StringFieldTest
{
	/**
	 * Put a {@code String} into {@code byte} array field.
	 *
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param str		A string to be set.
	 */
	@Override
	void set(IpcStruct struct, String name, String str)
	{
		struct.set(name, str);
	}
}

/**
 * Base class for test structures.
 */
abstract class TestStruct
{
	/**
	 * Data type identifier.
	 */
	private static String[]  _types = {
		"INT8",
		"UINT8",
		"INT16",
		"UINT16",
		"INT32",
		"UINT32",
		"INT64",
		"UINT64",
		"FLOAT",
		"DOUBLE",
		"IPV4",
		"IPV6",
		"STRING",
		"BINARY",
		"NULL",
	};

	/**
	 * The type of get ({@link IpcStruct#get(String)} and 
	 * {@link IpcStruct#get(String, int)}). Test runs for deep coppy 
	 * if ture, otherwise for shallow copy.
	 */
	protected final boolean _getTestType = true;


	/**
	 * The threshold of randomizing in udpate test.
	 */
	protected final int _retry_max = 10;

	/**
	 * An {@link IpcStruct} object to be tested.
	 */
	protected final IpcStruct  _struct;

	/**
	 * Buffer for structure.
	 */
	protected long  _buffer;

	/**
	 * Structure information.
	 */
	protected long  _info;

	/**
	 * Convert a data type identifier into string.
	 *
	 * @param type	Data type.
	 * @return	A string representation of {@code type}.
	 */
	static String getTypeName(int type)
	{
		if (type >= 0 && type < _types.length) {
			return _types[type];
		}

		if (type == IpcDataUnit.STRUCT) {
			return "STRUCT";
		}

		return "unknown:" + type;
	}

	/**
	 * Create a test structure object.
	 *
	 * @param struct	An {@link IpcStruct} object.
	 * @return		A test structure object.
	 */
	static TestStruct create(IpcStruct struct)
	{
		String name = struct.getName();

		if (name.equals("ut_struct_1")) {
			return new TestStruct1(struct);
		}
		if (name.equals("ut_struct_2")) {
			return new TestStruct2(struct);
		}
		if (name.equals("ut_struct_3")) {
			return new TestStruct3(struct);
		}
		if (name.equals("ut_struct_4")) {
			return new TestStruct4(struct);
		}

		throw new IllegalArgumentException("Unexpected name: " + name);
	}

	/**
	 * Fill the IPC structure with random bytes.
	 *
	 * @param struct	An {@link IpcStruct} object.
	 */
	static void randomize(IpcStruct struct)
	{
		Long buf = (Long)TestBase.getFieldValue(struct, "_buffer");
		Integer base = (Integer)TestBase.
			getFieldValue(struct, "_baseOffset");
		Long info = (Long)TestBase.getFieldValue(struct, "_info");

		randomize(buf.longValue(), base.intValue(), info.longValue());
	}

	/**
	 * Fill the IPC structure with the specified byte.
	 *
	 * @param struct	An {@link IpcStruct} object.
	 * @param b		A {@code byte} value to be set.
	 */
	static void fill(IpcStruct struct, byte b)
	{
		Long buf = (Long)TestBase.getFieldValue(struct, "_buffer");
		Integer base = (Integer)TestBase.
			getFieldValue(struct, "_baseOffset");
		Long info = (Long)TestBase.getFieldValue(struct, "_info");

		fill(b, buf.longValue(), base.intValue(), info.longValue());
	}

	/**
	 * Construct a new test structure.
	 *
	 * @param struct	An {@link IpcStruct} object to be tested.
	 */
	TestStruct(IpcStruct struct)
	{
		_struct = struct;
	}

	/**
	 * Fill the IPC structure associated with this structure with random
	 * bytes, and copy it into this test structure.
	 */
	void randomize()
	{
		Long buf = (Long)TestBase.getFieldValue(_struct, "_buffer");
		Integer base = (Integer)TestBase.
			getFieldValue(_struct, "_baseOffset");

		randomize(_struct);
		sync(buf.longValue(), base.intValue(), _buffer, _info);
	}

	/**
	 * Fill the IPC structure associated with this structure with the
	 * specified byte, and copy it into this test structure.
	 *
	 * @param b		A {@code byte} value to be set.
	 */
	void fill(byte b)
	{
		Long buf = (Long)TestBase.getFieldValue(_struct, "_buffer");
		Integer base = (Integer)TestBase.
			getFieldValue(_struct, "_baseOffset");

		fill(_struct, b);
		sync(buf.longValue(), base.intValue(), _buffer, _info);
	}

	/**
	 * Get a {@code byte} value at the specified field.
	 *
	 * @param name		The field name.
	 * @return		A value at the specified location.
	 */
	byte getByte(String name)
	{
		return getByteAt(name, -1, _buffer, _info);
	}

	/**
	 * Get a {@code byte} value at the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @return		A value at the specified location.
	 */
	byte getByte(String name, int index)
	{
		return getByteAt(name, index, _buffer, _info);
	}

	/**
	 * Get a {@code short} value at the specified field.
	 *
	 * @param name		The field name.
	 * @return		A value at the specified location.
	 */
	short getShort(String name)
	{
		return getShortAt(name, -1, _buffer, _info);
	}

	/**
	 * Get a {@code short} value at the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @return		A value at the specified location.
	 */
	short getShort(String name, int index)
	{
		return getShortAt(name, index, _buffer, _info);
	}

	/**
	 * Get an {@code int} value at the specified field.
	 *
	 * @param name		The field name.
	 * @return		A value at the specified location.
	 */
	int getInt(String name)
	{
		return getIntAt(name, -1, _buffer, _info);
	}

	/**
	 * Get an {@code int} value at the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @return		A value at the specified location.
	 */
	int getInt(String name, int index)
	{
		return getIntAt(name, index, _buffer, _info);
	}

	/**
	 * Get a {@code long} value at the specified field.
	 *
	 * @param name		The field name.
	 * @return		A value at the specified location.
	 */
	long getLong(String name)
	{
		return getLongAt(name, -1, _buffer, _info);
	}

	/**
	 * Get a {@code long} value at the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @return		A value at the specified location.
	 */
	long getLong(String name, int index)
	{
		return getLongAt(name, index, _buffer, _info);
	}

	/**
	 * Get a {@code float} value at the specified field.
	 *
	 * @param name		The field name.
	 * @return		A value at the specified location.
	 */
	float getFloat(String name)
	{
		return getFloatAt(name, -1, _buffer, _info);
	}

	/**
	 * Get a {@code float} value at the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @return		A value at the specified location.
	 */
	float getFloat(String name, int index)
	{
		return getFloatAt(name, index, _buffer, _info);
	}

	/**
	 * Get a {@code double} value at the specified field.
	 *
	 * @param name		The field name.
	 * @return		A value at the specified location.
	 */
	double getDouble(String name)
	{
		return getDoubleAt(name, -1, _buffer, _info);
	}

	/**
	 * Get a {@code double} value at the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @return		A value at the specified location.
	 */
	double getDouble(String name, int index)
	{
		return getDoubleAt(name, index, _buffer, _info);
	}

	/**
	 * Get an {@code InetAddress} object at the specified field.
	 *
	 * @param name		The field name.
	 * @return		A value at the specified location.
	 */
	InetAddress getInetAddress(String name)
	{
		byte[] raw = getInetAddressAt(name, -1, _buffer, _info);

		try {
			return InetAddress.getByAddress(raw);
		}
		catch (Exception e) {
			throw new IllegalStateException
				("Unable to create InetAddress: " + e, e);
		}
	}

	/**
	 * Get an {@code InetAddress} object at the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @return		A value at the specified location.
	 */
	InetAddress getInetAddress(String name, int index)
	{
		byte[] raw = getInetAddressAt(name, index, _buffer, _info);
		try {
			return InetAddress.getByAddress(raw);
		}
		catch (Exception e) {
			throw new IllegalStateException
				("Unable to create InetAddress: " + e, e);
		}
	}

	/**
	 * Set a {@code byte} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param value		A value to be set.
	 */
	void setByte(String name, byte value)
	{
		setByteAt(name, -1, value, _buffer, _info);
	}

	/**
	 * Set a {@code byte} value to the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 */
	void setByte(String name, int index, byte value)
	{
		setByteAt(name, index, value, _buffer, _info);
	}

	/**
	 * Set a {@code short} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param value		A value to be set.
	 */
	void setShort(String name, short value)
	{
		setShortAt(name, -1, value, _buffer, _info);
	}

	/**
	 * Set a {@code short} value to the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 */
	void setShort(String name, int index, short value)
	{
		setShortAt(name, index, value, _buffer, _info);
	}

	/**
	 * Set an {@code int} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param value		A value to be set.
	 */
	void setInt(String name, int value)
	{
		setIntAt(name, -1, value, _buffer, _info);
	}

	/**
	 * Set an {@code int} value to the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 */
	void setInt(String name, int index, int value)
	{
		setIntAt(name, index, value, _buffer, _info);
	}

	/**
	 * Set a {@code long} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param value		A value to be set.
	 */
	void setLong(String name, long value)
	{
		setLongAt(name, -1, value, _buffer, _info);
	}

	/**
	 * Set a {@code long} value to the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 */
	void setLong(String name, int index, long value)
	{
		setLongAt(name, index, value, _buffer, _info);
	}

	/**
	 * Set a {@code float} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param value		A value to be set.
	 */
	void setFloat(String name, float value)
	{
		setFloatAt(name, -1, value, _buffer, _info);
	}

	/**
	 * Set a {@code float} value to the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 */
	void setFloat(String name, int index, float value)
	{
		setFloatAt(name, index, value, _buffer, _info);
	}

	/**
	 * Set a {@code double} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param value		A value to be set.
	 */
	void setDouble(String name, double value)
	{
		setDoubleAt(name, -1, value, _buffer, _info);
	}

	/**
	 * Set a {@code double} value to the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 */
	void setDouble(String name, int index, double value)
	{
		setDoubleAt(name, index, value, _buffer, _info);
	}

	/**
	 * Set an {@code InetAddress} object to the specified field.
	 *
	 * @param name		The field name.
	 * @param value		A value to be set.
	 */
	void setInetAddress(String name, InetAddress value)
	{
		setInetAddressAt(name, -1, value.getAddress(), _buffer, _info);
	}

	/**
	 * Set an {@code InetAddress} object to the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 */
	void setInetAddress(String name, int index, InetAddress value)
	{
		setInetAddressAt(name, index, value.getAddress(), _buffer,
				 _info);
	}

	/**
	 * Set an {@link IpcStruct} object to the specified field.
	 *
	 * @param name		The field name.
	 * @param value		A value to be set.
	 */
	void setStruct(String name, IpcStruct value)
	{
		setStruct(name, -1, value);
	}

	/**
	 * Set an {@link IpcStruct} object to the specified array field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 */
	void setStruct(String name, int index, IpcStruct value)
	{
		Long buf = (Long)TestBase.getFieldValue(value, "_buffer");
		Integer base = (Integer)TestBase.
			getFieldValue(value, "_baseOffset");
		Long info = (Long)TestBase.getFieldValue(value, "_info");

		setStructAt(name, index, buf.longValue(), base.intValue(),
			    info.longValue(), _buffer, _info);
	}

	/**
	 * Ensure that an {@link IpcStruct} object associated with this
	 * object is initialized properly.
	 */
	void checkStruct()
	{
		Long buf = (Long)TestBase.getFieldValue(_struct, "_buffer");
		Integer base = (Integer)TestBase.
			getFieldValue(_struct, "_baseOffset");
		Long info = (Long)TestBase.getFieldValue(_struct, "_info");

		checkObject(buf.longValue(), base.intValue(),
			    info.longValue(), _info);
	}

	/**
	 * Determine contents of an {@link IpcStruct} associated with this
	 * object equals this  test structure.
	 *
	 * @return		True if equals. False otherwise.
	 */
	boolean equals()
	{
		Long buf = (Long)TestBase.getFieldValue(_struct, "_buffer");
		Integer base = (Integer)TestBase.
			getFieldValue(_struct, "_baseOffset");

		return equals(buf.longValue(), base.intValue(), _buffer,
			      _info);
	}

	/**
	 * Return a {@code String} array which contains all field names.
	 *
	 * @return	A {@code String} array.
	 */
	String[] getFieldNames()
	{
		Set<String> nmset = getFieldMap().keySet();

		String[] ret = nmset.toArray(new String[0]);
		Arrays.sort(ret);

		return ret;
	}

	/**
	 * Set the specified {@link IpcDataUnit} object to an {@link IpcStruct}
	 * object, and verify contents of IPC structure.
	 *
	 * @param impl		Field test implementation.
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param prefix	Prefix of field name.
	 * @param data		An {@link IpcDataUnit} object to be set.
	 */
	void update(TestFieldImpl impl, IpcStruct struct, String prefix,
		    IpcDataUnit data)
	{
		for (String name: struct.getFieldNames()) {
			update(impl, struct, name, prefix, data);
		}

		impl.checkInvalidField(struct, data);
	}

	/**
	 * Ensure that array field accessor works correctly, which get or set
	 * whole elements in an array field.
	 *
	 * @param impl		Field test implementation.
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param prefix	Prefix of field name.
	 */
	void arrayTest(TestArrayFieldImpl impl, IpcStruct struct,
		       String prefix)
	{
		for (String name: struct.getFieldNames()) {
			arrayTest(impl, struct, name, prefix);
		}

		// Specify null field name.
		try {
			impl.set(struct, null, 1);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "name is null.");
		}

		try {
			impl.get(struct, null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "name is null.");
		}

		// Specify invalid field name.
		String[] invalid = {
			"",
			"_",
			" a",
			"_invalid_field",
			"ut10_int200",
			"あいうえお",
		};

		Class<?> cls = IllegalArgumentException.class;
		for (String inv: invalid) {
			String msg = "Unknown field name: " + inv;

			try {
				impl.set(struct, inv, 1);
				needException();
			}
			catch (Exception e) {
				checkException(e, cls, msg);
			}

			try {
				impl.get(struct, inv);
				needException();
			}
			catch (Exception e) {
				checkException(e, cls, msg);
			}
		}
	}

	/**
	 * Return a {@code HashMap} which contains field definitions.
	 *
	 * @return	{@code HashMap} which contains field definitions.
	 */
	abstract HashMap<String, FieldDef> getFieldMap();

	/**
	 * Finalize this object.
	 *
	 * @throws Throwable
	 *	An error occurs while finalization.
	 */
	@Override
	protected void finalize() throws Throwable
	{
		finalize(_buffer);
	}

	/**
	 * Throw an error which indicates an exception is required.
	 */
	static void needException()
	{
		fail("An execption must be thrown.");
	}

	/**
	 * Ensure that required exception was thrown.
	 *
	 * @param e	An exception to be tested.
	 * @param cls	Required exception class.
	 * @param msg	Required message in an exception.
	 */
	static void checkException(Exception e, Class<?> cls, String msg)
	{
		assertEquals(cls, e.getClass());
		if (msg != null) {
			assertEquals(msg, e.getMessage());
		}
	}

	/**
	 * Set the specified {@link IpcDataUnit} object to an {@link IpcStruct}
	 * object, and verify contents of IPC structure.
	 *
	 * @param impl		Field test implementation.
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param prefix	Prefix of field name.
	 * @param data		An {@link IpcDataUnit} object to be set.
	 */
	private void update(TestFieldImpl impl, IpcStruct struct, String name,
			    String prefix, IpcDataUnit data)
	{
		IpcStructField field = struct.getField(name);
		int alen = field.getArrayLength();
		int type = data.getType();
		String key = (prefix == null) ? name : prefix + "." + name;

		assertTrue(equals());

		String badtypemsg;
		if (type == IpcDataUnit.STRING) {
			badtypemsg = "IpcString is not supported.";
		}
		else if (type == IpcDataUnit.BINARY) {
			badtypemsg = "IpcBinary is not supported.";
		}
		else if (type == IpcDataUnit.NULL) {
			badtypemsg = "IpcNull is not supported.";
		}
		else {
			badtypemsg = null;
		}

		if (alen == 0) {
			// An exception must be thrown if an array index is
			// specified.
			Class<?> cls = IllegalArgumentException.class;
			String amsg = "\"" + name +
				"\" is not an array field.";
			String msg = (badtypemsg == null)
				? amsg : badtypemsg;
			for (int i = 0; i < 10; i++) {
				try {
					impl.set(struct, name, i, data);
					needException();
				}
				catch (Exception e) {
					checkException(e, cls, msg);
				}

				try {
					impl.get(struct, name, i);
					needException();
				}
				catch (Exception e) {
					checkException(e, cls, amsg);
				}
			}
		}
		else {
			// Omit array index.
			String amsg = "Array index for \"" + name +
				"\" is not specified.";

			try {
				impl.set(struct, name, data);
				needException();
			}
			catch (Exception e) {
				String msg = (badtypemsg == null)
					? amsg : badtypemsg;
				checkException
					(e, IllegalArgumentException.class,
					 msg);
			}

			try {
				impl.get(struct, name);
				needException();
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 amsg);
			}

			// Specify illegal array index.
			for (int i = -10; i < 0; i++) {
				String msg = "Negative array index: " + i;
				Class<?> cls = ArrayIndexOutOfBoundsException.
					class;

				try {
					impl.set(struct, name, i, data);
					needException();
				}
				catch (Exception e) {
					checkException(e, cls, msg);
				}

				try {
					impl.get(struct, name, i);
					needException();
				}
				catch (Exception e) {
					checkException(e, cls, msg);
				}

				assertTrue(equals());
			}

			for (int i = alen; i <= alen + 10; i++) {
				String msg;
				Class<?> cls;

				amsg = "Illegal array index for \"" +
					name + "\": " + i + " >= " +
					alen;
				if (badtypemsg == null) {
					msg = amsg;
					cls = ArrayIndexOutOfBoundsException.
						class;
				}
				else {
					msg = badtypemsg;
					cls = IllegalArgumentException.class;
				}

				try {
					impl.set(struct, name, i, data);
					needException();
				}
				catch (Exception e) {
					checkException(e, cls, msg);
				}

				cls = ArrayIndexOutOfBoundsException.class;
				try {
					impl.get(struct, name, i);
					needException();
				}
				catch (Exception e) {
					checkException(e, cls, amsg);
				}

				assertTrue(equals());
			}
		}

		assertTrue(equals());
		if (!impl.isMatched(field)) {
			// Data type mismatch.
			if (alen == 0) {
				try {
					impl.set(struct, name, data);
					needException();
				}
				catch (Exception e) {
					impl.checkException(e, field,
							    badtypemsg);
				}
				impl.checkGetUnmatchedField(struct, name,
							    field, badtypemsg);
			}
			else {
				for (int i = 0; i < alen; i++) {
					try {
						impl.set(struct, name, i,
							 data);
					}
					catch (Exception e) {
						impl.checkException
							(e, field,
							 badtypemsg);
					}					
					impl.checkGetUnmatchedField(struct, name,
								    i, field,
								    badtypemsg);
				}
			}

			assertTrue(equals());

			if (field.getType() == IpcDataUnit.STRUCT) {
				// Check for inner structure.
				if (prefix == null) {
					prefix = name;
				}
				else {
					prefix = prefix + "." + name;
				}

				if (alen == 0) {
					update(impl, struct.getInner(name),
					       prefix, data);
				}
				else {
					for (int i = 0; i < alen; i++) {
						String f = prefix + "[" + i +
							"]";
						update(impl,
						       struct.getInner(name, i),
						       f, data);
					}
				}
			}

			return;
		}

		assertTrue(type != IpcDataUnit.STRING &&
			   type != IpcDataUnit.BINARY &&
			   type != IpcDataUnit.NULL);

		if (alen == 0) {
			updateField(impl, struct, name, key, data);
		}
		else {
			for (int i = 0; i < alen; i++) {
				String f = key + "[" + i + "]";
				updateField(impl, struct, name, key, i, data);
			}
		}
	}

	/**
	 * Update test data.
	 *
	 * @param impl		Field test implementation.
	 * @param struct	The target {@link IpcStruct}.
	 * @param name		The field name.
	 * @param key		The key string associated with data.
	 * @param data		A value to be set.
	 */
	private void updateField(TestFieldImpl impl, IpcStruct struct,
				 String name, String key, IpcDataUnit data)
	{
		IpcDataUnit old = impl.get(struct, name);
		int retry;

		/* check randomize */
		for (retry = 0 ; data.equals(old) && retry < _retry_max ;
		     retry++) {
			randomize();
			old = impl.get(struct, name);
		}
		if (retry == _retry_max) {
			fail ("Please retry test by different randomizing seed.");
		}

		// Update the field in the test structure.
		impl.update(this, key, -1, data, old);
		assertFalse(equals());

		// Update IpcStruct.
		impl.set(struct, name, data);
		IpcDataUnit cur = impl.get(struct, name);

		assertTrue(equals());
		if ((_getTestType) || (data.getType() != IpcDataUnit.STRUCT)){
			assertFalse(impl.equals(old, cur));
			assertTrue(impl.equals(data, cur));
		}
	}

	/**
	 * Update test data.
	 *
	 * @param impl		Field test implementation.
	 * @param struct	The target {@link IpcStruct}.
	 * @param name		The field name.
	 * @param key		The key string associated with data.
	 * @param index		An array index.
	 * @param data		A value to be set.
	 */
	private void updateField(TestFieldImpl impl, IpcStruct struct,
				 String name, String key, int index,
				 IpcDataUnit data)
	{
		IpcDataUnit old = impl.get(struct, name, index);
		int retry;

		/* check randomize */
		for (retry = 0 ; data.equals(old) && retry < _retry_max ;
		     retry++) {
			randomize();
			old = impl.get(struct, name, index);
		}
		if (retry == _retry_max) {
			fail ("Please retry test by different randomizing seed.");
		}

		// Update the field in the test structure.
		impl.update(this, key, index, data, old);
		assertFalse(equals());

		// Update IpcStruct.
		impl.set(struct, name, index, data);
		IpcDataUnit cur = impl.get(struct, name, index);

		assertTrue(equals());
		if (data.getType() != IpcDataUnit.STRUCT) {
			assertFalse(impl.equals(old, cur));
			assertTrue(impl.equals(data, cur));
		}
	}

	/**
	 * Ensure that array field accessor works correctly, which get or set
	 * whole elements in an array field.
	 *
	 * @param impl		Field test implementation.
	 * @param struct	An {@link IpcStruct} to be tested.
	 * @param name		The field name.
	 * @param prefix	Prefix of field name.
	 */
	private void arrayTest(TestArrayFieldImpl impl, IpcStruct struct,
			       String name, String prefix)
	{
		IpcStructField field = struct.getField(name);
		int alen = field.getArrayLength();
		String key = (prefix == null) ? name : prefix + "." + name;

		assertTrue(equals());

		if (alen == 0) {
			// Array field accessor must fail.
			Class<?> cls = IllegalArgumentException.class;
			String msg = "\"" + name +
				"\" is not an array field.";
			for (int i = 0; i < 10; i++) {
				try {
					impl.set(struct, name, i);
					needException();
				}
				catch (Exception e) {
					checkException(e, cls, msg);
				}
			}

			try {
				impl.get(struct, name);
				needException();
			}
			catch (Exception e) {
				checkException(e, cls, msg);
			}

			assertTrue(equals());
		}

		if (!impl.isMatched(field)) {
			// Data type mismatch.
			if (alen != 0) {
				try {
					impl.set(struct, name, alen);
					TestStruct.needException();
				}
				catch (Exception e) {
					impl.checkException(e, field);
				}

				if (!impl.isClassifiedMatched(field)){
					try {
						impl.get(struct, name);
						TestStruct.needException();
					}
					catch (Exception e) {
						impl.checkException(e, field);
					}
				}
			}

			assertTrue(equals());

			if (field.getType() == IpcDataUnit.STRUCT) {
				// Check for inner structure.
				if (prefix == null) {
					prefix = name;
				}
				else {
					prefix = prefix + "." + name;
				}

				if (alen == 0) {
					arrayTest(impl, struct.getInner(name),
						  prefix);
				}
				else {
					for (int i = 0; i < alen; i++) {
						String f = prefix + "[" + i +
							"]";
						IpcStruct inner = struct.
							getInner(name, i);
						arrayTest(impl, inner, f);
					}
				}
			}

			return;
		}

		if (alen != 0) {
			// Specify null array.
			try {
				impl.set(struct, name, -1);
				needException();
			}
			catch (Exception e) {
				checkException(e, NullPointerException.class,
					       impl.getNullValueMessage());
			}

			impl.checkLength(struct, name, this, alen);

			assertTrue(equals());
			impl.check(struct, name, this, key, alen);
			assertTrue(equals());
		}
	}

	/**
	 * Fill the IPC structure with random bytes.
	 *
	 * @param buffer	A buffer of IPC structure.
	 * @param base		Base offset of IPC structure.
	 * @param info		IPC structure information.
	 */
	private static native void randomize(long buffer, int base, long info);

	/**
	 * Fill the IPC structure with the specified byte.
	 *
	 * @param b		A {@code byte} value to be set.
	 * @param buffer	A buffer of IPC structure.
	 * @param base		Base offset of IPC structure.
	 * @param info		IPC structure information.
	 */
	private static native void fill(byte b, long buffer, int base,
					long info);

	/**
	 * Ensure that an {@link IpcStruct} object is initialized properly.
	 *
	 * @param buffer	A buffer of IPC structure.
	 * @param base		Base offset of IPC structure.
	 * @param info		IPC structure information.
	 * @param tinfo		Information about test structure.
	 */
	private native void checkObject(long buffer, int base, long info,
					long tinfo);

	/**
	 * Copy contents of the IPC structure into this structure.
	 *
	 * @param buffer	A buffer of IPC structure.
	 * @param base		Base offset of IPC structure.
	 * @param tbuffer	A buffer of test structure.
	 * @param tinfo		Information about test structure.
	 */
	private native void sync(long buffer, int base, long tbuffer,
				 long tinfo);

	/**
	 * Determine contents of the specified {@link IpcStruct} equals this
	 * test structure.
	 *
	 * @param buffer	A buffer of IPC structure.
	 * @param base		Base offset of IPC structure.
	 * @param tbuffer	A buffer of test structure.
	 * @param tinfo		Information about test structure.
	 * @return		True if equals. False otherwise.
	 */
	private native boolean equals(long buffer, int base, long tbuffer,
				      long tinfo);

	/**
	 * Get a {@code byte} value at the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 * @return		A value at the specified location.
	 */
	private native byte getByteAt(String name, int index,
				      long buffer, long info);

	/**
	 * Get a {@code short} value at the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 * @return		A value at the specified location.
	 */
	private native short getShortAt(String name, int index,
					long buffer, long info);

	/**
	 * Get an {@code int} value at the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 * @return		A value at the specified location.
	 */
	private native int getIntAt(String name, int index,
				    long buffer, long info);

	/**
	 * Get a {@code long} value at the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 * @return		A value at the specified location.
	 */
	private native long getLongAt(String name, int index,
				      long buffer, long info);

	/**
	 * Get a {@code float} value at the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 * @return		A value at the specified location.
	 */
	private native float getFloatAt(String name, int index,
					long buffer, long info);

	/**
	 * Get a {@code double} value at the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 * @return		A value at the specified location.
	 */
	private native double getDoubleAt(String name, int index,
					  long buffer, long info);

	/**
	 * Get a raw IP address value at the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 * @return		A value at the specified location.
	 */
	private native byte[] getInetAddressAt(String name, int index,
					       long buffer, long info);

	/**
	 * Set a {@code byte} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 */
	private native void setByteAt(String name, int index, byte value,
				      long buffer, long info);

	/**
	 * Set a {@code short} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 */
	private native void setShortAt(String name, int index, short value,
				       long buffer, long info);

	/**
	 * Set an {@code int} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 */
	private native void setIntAt(String name, int index, int value,
				     long buffer, long info);

	/**
	 * Set a {@code long} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 */
	private native void setLongAt(String name, int index, long value,
				      long buffer, long info);

	/**
	 * Set a {@code float} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 */
	private native void setFloatAt(String name, int index, float value,
				       long buffer, long info);

	/**
	 * Set a {@code double} value to the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 */
	private native void setDoubleAt(String name, int index, double value,
					long buffer, long info);

	/**
	 * Set a raw IP address value to the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param value		A value to be set.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 */
	private native void setInetAddressAt(String name, int index,
					     byte[] value, long buffer,
					     long info);

	/**
	 * Copy IPC structure  to the specified field.
	 *
	 * @param name		The field name.
	 * @param index		An array index.
	 * @param stbuf		Buffer for IPC structure.
	 * @param stbase	Base offset of IPC structure buffer.
	 * @param stinfo	Information about IPC structure.
	 * @param buffer	Buffer for structure.
	 * @param info		Structure information.
	 */
	private native void setStructAt(String name, int index, long stbuf,
					int stbase, long stinfo, long buffer,
					long info);

	/**
	 * Finalize this object.
	 *
	 * @param buffer	Buffer for structure.
	 */
	private native void finalize(long buffer);
}

/**
 * An instance of {@code TestStruct1} represents an IPC structure named
 * {@code ut_struct_1}.
 */
class TestStruct1 extends TestStruct
{
	/**
	 * Field definition map.
	 */
	private final static HashMap<String, FieldDef> _fields =
		new HashMap<String, FieldDef>();

	/**
	 * Initialize field definition.
	 */
	static {
		FieldDef[] defs = {
			new FieldDef("ut1_int8", IpcDataUnit.INT8),
			new FieldDef("ut1_uint8", IpcDataUnit.UINT8),
			new FieldDef("ut1_int16", IpcDataUnit.INT16),
			new FieldDef("ut1_uint16",IpcDataUnit.UINT16),
			new FieldDef("ut1_int32", IpcDataUnit.INT32),
			new FieldDef("ut1_uint32", IpcDataUnit.UINT32),
			new FieldDef("ut1_int64", IpcDataUnit.INT64),
			new FieldDef("ut1_uint64", IpcDataUnit.UINT64),
			new FieldDef("ut1_float", IpcDataUnit.FLOAT),
			new FieldDef("ut1_double", IpcDataUnit.DOUBLE),
			new FieldDef("ut1_ipv4", IpcDataUnit.IPV4),
			new FieldDef("ut1_ipv6", IpcDataUnit.IPV6),
		};

		for (FieldDef fdef: defs) {
			_fields.put(fdef.getName(), fdef);
		}
	}

	/**
	 * Construct an object.
	 *
	 * @param struct	An {@link IpcStruct} object.
	 */
	TestStruct1(IpcStruct struct)
	{
		super(struct);
		_buffer = createBuffer();
		_info = getInfo();
		checkStruct();
	}

	/**
	 * Return a {@code HashMap} which contains field definitions.
	 *
	 * @return	{@code HashMap} which contains field definitions.
	 */
	HashMap<String, FieldDef> getFieldMap()
	{
		return TestStruct1._fields;
	}

	/**
	 * Create a new buffer for {@code ut_struct_1}.
	 *
	 * @return	A new buffer for {@code ut_struct_1}.
	 */
	private native long createBuffer();

	/**
	 * Return an IPC structure information about {@code ut_struct_1}.
	 *
	 * @return	Information about {@code ut_struct_1}.
	 */
	private native long getInfo();
}


/**
 * An instance of {@code TestStruct2} represents an IPC structure named
 * {@code ut_struct_2}.
 */
class TestStruct2 extends TestStruct
{
	/**
	 * Field definition map.
	 */
	private final static HashMap<String, FieldDef> _fields =
		new HashMap<String, FieldDef>();

	/**
	 * Initialize field definition.
	 */
	static {
		FieldDef[] defs = {
			new FieldDef("ut2_int8", IpcDataUnit.INT8, 31),
			new FieldDef("ut2_uint8", IpcDataUnit.UINT8, 25),
			new FieldDef("ut2_int16", IpcDataUnit.INT16, 29),
			new FieldDef("ut2_uint16", IpcDataUnit.UINT16, 23),
			new FieldDef("ut2_int32", IpcDataUnit.INT32, 21),
			new FieldDef("ut2_uint32", IpcDataUnit.UINT32, 9),
			new FieldDef("ut2_int64", IpcDataUnit.INT64, 27),
			new FieldDef("ut2_uint64", IpcDataUnit.UINT64, 19),
			new FieldDef("ut2_float", IpcDataUnit.FLOAT, 17),
			new FieldDef("ut2_double", IpcDataUnit.DOUBLE, 15),
			new FieldDef("ut2_ipv4", IpcDataUnit.IPV4, 13),
			new FieldDef("ut2_ipv6", IpcDataUnit.IPV6, 11),
		};

		for (FieldDef fdef: defs) {
			_fields.put(fdef.getName(), fdef);
		}
	}

	/**
	 * Construct an object.
	 *
	 * @param struct	An {@link IpcStruct} object.
	 */
	TestStruct2(IpcStruct struct)
	{
		super(struct);
		_buffer = createBuffer();
		_info = getInfo();
		checkStruct();
	}

	/**
	 * Return a {@code HashMap} which contains field definitions.
	 *
	 * @return	{@code HashMap} which contains field definitions.
	 */
	HashMap<String, FieldDef> getFieldMap()
	{
		return TestStruct2._fields;
	}

	/**
	 * Create a new buffer for {@code ut_struct_2}.
	 *
	 * @return	A new buffer for {@code ut_struct_2}.
	 */
	private native long createBuffer();

	/**
	 * Return an IPC structure information about {@code ut_struct_2}.
	 *
	 * @return	Information about {@code ut_struct_2}.
	 */
	private native long getInfo();
}

/**
 * An instance of {@code TestStruct3} represents an IPC structure named
 * {@code ut_struct_3}.
 */
class TestStruct3 extends TestStruct
{
	/**
	 * Field definition map.
	 */
	private final static HashMap<String, FieldDef> _fields =
		new HashMap<String, FieldDef>();

	/**
	 * Initialize field definition.
	 */
	static {
		FieldDef[] defs = {
			new FieldDef("ut3_struct1", IpcDataUnit.STRUCT, 0,
				     "ut_struct_1"),
			new FieldDef("ut3_struct2", IpcDataUnit.STRUCT, 4,
				     "ut_struct_2"),
		};

		for (FieldDef fdef: defs) {
			_fields.put(fdef.getName(), fdef);
		}
	}

	/**
	 * Construct an object.
	 *
	 * @param struct	An {@link IpcStruct} object.
	 */
	TestStruct3(IpcStruct struct)
	{
		super(struct);
		_buffer = createBuffer();
		_info = getInfo();
		checkStruct();
	}

	/**
	 * Return a {@code HashMap} which contains field definitions.
	 *
	 * @return	{@code HashMap} which contains field definitions.
	 */
	HashMap<String, FieldDef> getFieldMap()
	{
		return TestStruct3._fields;
	}

	/**
	 * Create a new buffer for {@code ut_struct_3}.
	 *
	 * @return	A new buffer for {@code ut_struct_3}.
	 */
	private native long createBuffer();

	/**
	 * Return an IPC structure information about {@code ut_struct_3}.
	 *
	 * @return	Information about {@code ut_struct_3}.
	 */
	private native long getInfo();
}

/**
 * An instance of {@code TestStruct4} represents an IPC structure named
 * {@code ut_struct_4}.
 */
class TestStruct4 extends TestStruct
{
	/**
	 * Field definition map.
	 */
	private final static HashMap<String, FieldDef> _fields =
		new HashMap<String, FieldDef>();

	/**
	 * Initialize field definition.
	 */
	static {
		FieldDef[] defs = {
			new FieldDef("ut4_int8", IpcDataUnit.INT8),
			new FieldDef("ut4_struct3", IpcDataUnit.STRUCT, 3,
				     "ut_struct_3"),
			new FieldDef("ut4_int64", IpcDataUnit.INT64),
		};

		for (FieldDef fdef: defs) {
			_fields.put(fdef.getName(), fdef);
		}
	}

	/**
	 * Construct an object.
	 *
	 * @param struct	An {@link IpcStruct} object.
	 */
	TestStruct4(IpcStruct struct)
	{
		super(struct);
		_buffer = createBuffer();
		_info = getInfo();
		checkStruct();
	}

	/**
	 * Return a {@code HashMap} which contains field definitions.
	 *
	 * @return	{@code HashMap} which contains field definitions.
	 */
	HashMap<String, FieldDef> getFieldMap()
	{
		return TestStruct4._fields;
	}

	/**
	 * Create a new buffer for {@code ut_struct_4}.
	 *
	 * @return	A new buffer for {@code ut_struct_4}.
	 */
	private native long createBuffer();

	/**
	 * Return an IPC structure information about {@code ut_struct_4}.
	 *
	 * @return	Information about {@code ut_struct_4}.
	 */
	private native long getInfo();
}
