/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Random;

/**
 * <p>
 *   Unit test class for {@link IpcBinary}.
 * </p>
 */
public class IpcBinaryTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcBinary}.
	 *
	 * @param name	The test name.
	 */
	public IpcBinaryTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcBinary#IpcBinary(byte[])}.
	 */
	public void testCtor()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 0x1000; loop++) {
			int nelems = rand.nextInt(1024) + 1;
			byte[] bin = new byte[nelems];
			rand.nextBytes(bin);
			IpcBinary obj = new IpcBinary(bin);
			checkValue(obj, bin);
		}

		IpcBinary obj = new IpcBinary();
		checkValue(obj, null);

		obj = new IpcBinary(null);
		checkValue(obj, null);

		byte[] bin = new byte[0];
		obj = new IpcBinary(bin);
		checkValue(obj, bin);
	}

	/**
	 * Test case for {@link IpcBinary#equals(Object)} and
	 * {@link IpcBinary#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcBinary> set = new HashSet<IpcBinary>();
		Random rand = new Random();

		IpcBinary nil = new IpcBinary();
		assertTrue(nil.equals(nil));
		assertFalse(nil.equals(null));
		assertFalse(nil.equals(new Object()));
		assertTrue(set.add(nil));
		assertFalse(set.add(nil));

		IpcBinary empty = new IpcBinary(new byte[0]);
		assertTrue(empty.equals(empty));
		assertFalse(empty.equals(null));
		assertFalse(empty.equals(new Object()));
		assertTrue(set.add(empty));
		assertFalse(set.add(empty));

		for (int nelems = 1; nelems <= 1024; nelems++) {
			byte[] bin = new byte[nelems];
			rand.nextBytes(bin);
			IpcBinary obj = new IpcBinary(bin);

			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));
			assertFalse(obj.equals(nil));
			assertFalse(obj.equals(empty));
			assertFalse(nil.equals(obj));
			assertFalse(empty.equals(obj));
			assertTrue(set.add(obj));
			assertFalse(set.add(obj));

			byte[] b = bin.clone();
			IpcBinary obj1 = new IpcBinary(b);
			assertTrue(obj.equals(obj1));
			assertFalse(set.add(obj1));

			for (int i = 0; i < 32 && i < b.length; i++) {
				b = bin.clone();
				b[i]++;
				obj1 = new IpcBinary(b);
				assertFalse(obj.equals(obj1));
				assertTrue(set.add(obj1));
			}
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcBinary obj, byte[] value)
	{
		byte[] v = obj.getValue();

		if (value != null) {
			// getValue() must return a clone.
			assertNotSame(value, v);
			assertNotSame(v, obj.getValue());
		}

		assertTrue(Arrays.equals(value, v));
		assertEquals(Arrays.toString(value), obj.toString());
		assertEquals(IpcDataUnit.BINARY, obj.getType());
	}
}
