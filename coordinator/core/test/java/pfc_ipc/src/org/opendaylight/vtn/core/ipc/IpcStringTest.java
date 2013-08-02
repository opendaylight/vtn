/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.HashSet;
import java.util.Random;

/**
 * <p>
 *   Unit test class for {@link IpcString}.
 * </p>
 */
public class IpcStringTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcString}.
	 *
	 * @param name	The test name.
	 */
	public IpcStringTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcString#IpcString(String)}.
	 */
	public void testCtor()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 0x100000; loop++) {
			int i = rand.nextInt();
			String str = "test: " + i;
			IpcString obj = new IpcString(str);
			checkValue(obj, str);
		}

		IpcString obj = new IpcString();
		checkValue(obj, null);

		obj = new IpcString(null);
		checkValue(obj, null);

		obj = new IpcString("");
		checkValue(obj, "");
	}

	/**
	 * Test case for {@link IpcString#compareTo(IpcString)}.
	 */
	public void testCompareTo()
	{
		Random rand = new Random();

		IpcString objnull = new IpcString();

		for (int loop = 0; loop < 0x100000; loop++) {
			int i = rand.nextInt();
			int j = rand.nextInt();
			String stri = "test: " + i;
			String strj = "test: " + j;
			IpcString obji = new IpcString(stri);
			IpcString objj = new IpcString(strj);

			assertTrue(obji.compareTo(objnull) > 0);
			assertTrue(objnull.compareTo(obji) < 0);
			assertTrue(objj.compareTo(objnull) > 0);
			assertTrue(objnull.compareTo(objj) < 0);

			int required = stri.compareTo(strj);
			int result = obji.compareTo(objj);
			if (required == 0) {
				assertEquals(0, result);
			}
			else if (required < 0) {
				assertTrue(result < 0);
			}
			else {
				assertTrue(result > 0);
			}
		}

		for (int loop = 0; loop < 0x100000; loop++) {
			int i = rand.nextInt();
			String stri = "test: " + i;
			IpcString obji = new IpcString(stri);
			IpcString objj = new IpcString(stri);

			assertTrue(obji != objj);
			assertEquals(0, obji.compareTo(obji));
			assertEquals(0, obji.compareTo(objj));
			assertEquals(0, objj.compareTo(obji));
			assertEquals(0, objj.compareTo(objj));
		}
	}

	/**
	 * Test case for {@link IpcString#equals(Object)} and
	 * {@link IpcString#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcString> set = new HashSet<IpcString>();

		IpcString nil = new IpcString();
		assertTrue(nil.equals(nil));
		assertFalse(nil.equals(null));
		assertFalse(nil.equals(new Object()));
		assertTrue(set.add(nil));
		assertFalse(set.add(nil));
		assertEquals(1, set.size());

		IpcString empty = new IpcString("");
		assertTrue(empty.equals(empty));
		assertFalse(empty.equals(null));
		assertFalse(empty.equals(new Object()));
		assertTrue(set.add(empty));
		assertFalse(set.add(empty));
		assertEquals(2, set.size());

		for (int i = 0; i <= 0x10000; i++) {
			String str = "test: " + i;
			IpcString obj = new IpcString(str);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));
			assertFalse(obj.equals(nil));
			assertFalse(nil.equals(obj));
			assertFalse(obj.equals(empty));
			assertFalse(empty.equals(obj));

			assertTrue(set.add(obj));
			assertFalse(set.add(obj));
			assertEquals(i + 3, set.size());
		}

		for (int i = 0; i <= 0x10000; i++) {
			String str = "test: " + i;
			IpcString obj = new IpcString(str);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcString obj, String value)
	{
		assertEquals(value, obj.getValue());

		if (value == null) {
			assertEquals("null", obj.toString());
		}
		else {
			assertEquals(value, obj.toString());
		}

		assertEquals(IpcDataUnit.STRING, obj.getType());
	}
}
