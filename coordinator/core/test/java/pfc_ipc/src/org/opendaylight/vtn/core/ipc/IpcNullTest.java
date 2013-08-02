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

/**
 * <p>
 *   Unit test class for {@link IpcNull}.
 * </p>
 */
public class IpcNullTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcNull}.
	 *
	 * @param name	The test name.
	 */
	public IpcNullTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcNull#IpcNull()}.
	 */
	public void testCtor()
	{
		for (int loop = 0; loop < 0x1000; loop++) {
			IpcNull obj = new IpcNull();
			assertEquals(IpcDataUnit.NULL, obj.getType());
			assertEquals(IpcDataUnit.NULL, obj.hashCode());
			assertEquals("null", obj.toString());
		}
	}

	/**
	 * Test case for {@link IpcNull#equals(Object)} and
	 * {@link IpcNull#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<Object> set = new HashSet<Object>();

		IpcNull obj = new IpcNull();
		assertTrue(obj.equals(new IpcNull()));
		assertFalse(obj.equals(new Object()));
		assertFalse(obj.equals(null));
		assertTrue(set.add(obj));
		assertFalse(set.add(obj));
		assertEquals(1, set.size());

		int nelems;
		for (nelems = 1; nelems <= 1024; nelems++) {
			IpcNull o = new IpcNull();
			assertFalse(set.add(o));
			assertEquals(nelems, set.size());

			assertTrue(set.add(new Integer(nelems)));
			assertEquals(nelems + 1, set.size());
		}

		obj = new IpcNull();
		assertTrue(set.contains(obj));
		assertEquals(nelems, set.size());
		assertTrue(set.remove(obj));
		assertFalse(set.remove(obj));
		assertEquals(nelems - 1, set.size());
	}
}
