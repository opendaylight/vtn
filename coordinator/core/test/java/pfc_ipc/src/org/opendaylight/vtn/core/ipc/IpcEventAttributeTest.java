/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.Random;

/**
 * <p>
 *   Unit test class for {@link IpcEventAttribute}.
 * </p>
 */
public class IpcEventAttributeTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcEventAttribute}.
	 *
	 * @param name	The test name.
	 */
	public IpcEventAttributeTest(String name)
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

		// Call finalizer of IpcEventAttribute by force.
		System.gc();
	}

	/**
	 * Ensure that an {@link IpcEventAttribute#IpcEventAttribute()}
	 * initializes all parameters with default values.
	 */
	public void testCtor()
	{
		IpcEventAttribute attr = new IpcEventAttribute();

		assertNull(attr.getHostSet());
		assertEquals(IpcEventAttribute.DEFAULT_PRIORITY,
			     attr.getPriority());
		assertFalse(attr.isLogEnabled());

		String[] services = {
			null,
			"",
			"service1",
			"service2",
		};
		for (String service: services) {
			IpcEventMask mask = attr.getTarget(service);
			assertTrue(mask.isFilled());
		}
	}

	/**
	 * Test case for {@link IpcEventAttribute#getHostSet()} and
	 * {@link IpcEventAttribute#setHostSet(String)}.
	 */
	public void testHostSet()
	{
		String[] names = {
			"",
			"set1",
			"set2",
			"set3",
		};

		IpcEventAttribute attr = new IpcEventAttribute();
		Class<?> cls = IpcNoSuchHostSetException.class;

		for (String name: names) {
			try {
				attr.setHostSet(name);
				needException();
			}
			catch (Exception e) {
				checkException(e, cls, null);
			}

			assertNull(attr.getHostSet());
		}

		// Invoke hostSetTest() on a child process.
		runOnChild("hostSetTest");
	}

	/**
	 * Test case for {@link IpcEventAttribute#getTarget(String)},
	 * {@link IpcEventAttribute#addTarget(String)},
	 * {@link IpcEventAttribute#addTarget(String, IpcEventMask)},
	 * and {@link IpcEventAttribute#resetTarget()}.
	 */
	public void testTarget()
	{
		String[] names = {
			"serv1",
			"serv2",
			"serv3",
			"serv4",
			"serv5",
		};

		IpcEventAttribute attr = new IpcEventAttribute();
		IpcEventMask mask1 = new IpcEventMask(3);
		String service1 = "service1";
		attr.addTarget(service1, mask1);
		assertEquals(mask1, attr.getTarget(service1));

		for (String name: names) {
			IpcEventMask m = attr.getTarget(name);
			assertTrue(m.isEmpty());
		}

		// Merge event types to "service1".
		for (int i = 0; i < 64; i++) {
			IpcEventMask m = new IpcEventMask(i);

			mask1.add(i);
			attr.addTarget(service1, m);
			assertEquals(mask1, attr.getTarget(service1));
		}

		// Add target for "service2".
		String service2 = "service2";
		IpcEventMask mask2 = new IpcEventMask(0);
		mask2.add(15);
		mask2.add(33);
		attr.addTarget(service2, mask2);
		assertEquals(mask1, attr.getTarget(service1));
		assertEquals(mask2, attr.getTarget(service2));

		// Add all event types for "service3".
		String service3 = "service3";
		attr.addTarget(service3);
		IpcEventMask mask3 = attr.getTarget(service3);
		assertTrue(mask3.isFilled());
		
		// Add target for channel state change event.
		IpcEventMask maskState = new IpcEventMask(1);
		maskState.add(2);
		attr.addTarget(null, maskState);
		assertEquals(mask1, attr.getTarget(service1));
		assertEquals(mask2, attr.getTarget(service2));
		assertEquals(mask3, attr.getTarget(service3));
		assertEquals(maskState, attr.getTarget(null));

		for (String name: names) {
			IpcEventMask m = attr.getTarget(name);
			assertTrue(m.isEmpty());
		}

		// Reset target.
		attr.resetTarget();
		for (String name: names) {
			IpcEventMask m = attr.getTarget(name);
			assertTrue(m.isFilled());
		}

		// Reset target in init state.
		IpcEventAttribute attr2 = new IpcEventAttribute();
		attr2.resetTarget();
		for (String name: names) {
			IpcEventMask m = attr2.getTarget(name);
			assertTrue(m.isFilled());
		}

		String[] validNames = {
			null,
			service1,
			service2,
			service3,
		};
		for (String name: validNames) {
			IpcEventMask m = attr.getTarget(name);
			assertTrue(m.isFilled());
		}

		// Event mask is null.
		try {
			attr.addTarget(service1, null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class);
		}

		// Empty event mask.
		mask1.clear();
		try {
			attr.addTarget(service1, mask1);
			needException();
		}
		catch (Exception e) {
			checkException(e, IllegalArgumentException.class);
		}

		// Invalid IPC service name.
		mask1.add(1);
		try {
			attr.addTarget("invalid service name", mask1);
			needException();
		}
		catch (Exception e) {
			checkException(e, IllegalArgumentException.class);
		}

		String[] invalidNames = {
			"serv!", "serv\"", "serv#", "serv$", "serv%",
			"serv&", "serv'", "serv(", "serv)", "serv-",
			"serv~", "serv=", "serv|", "serv\\", "serv@",
			"serv`", "serv[", "serv]", "serv{", "serv}",
			"serv<", "serv>", "serv,", "serv/", "serv?"
		};
		Class<?> cls = IllegalArgumentException.class;
		for (String name : invalidNames) {
			try {
				attr.addTarget(name, mask1);
				needException();
			}
			catch (Exception e) {
				checkException(e, cls);
			}
		}

		try {
			attr.addTarget("", mask1);
			needException();
		}
		catch (Exception e) {
			checkException(e, IllegalArgumentException.class);
		}

		try {
			attr.addTarget("1service", mask1);
			needException();
		}
		catch (Exception e) {
			checkException(e, IllegalArgumentException.class);
		}

		// 31 characters.
		attr.addTarget("abcdefghijklmnopqrstuvwxyzabcde", mask1);

		try {
			// 32 characters.
			attr.addTarget("abcdefghijklmnopqrstuvwxyzabcdef",
				       mask1);
			needException();
		}
		catch (Exception e) {
			checkException(e, IllegalArgumentException.class);
		}
	}

	/**
	 * Test case for {@link IpcEventAttribute#getPriority()} and
	 * {@link IpcEventAttribute#setPriority(int)}.
	 */
	public void testPriority()
	{
		Random rand = new Random();

		int prev = IpcEventAttribute.DEFAULT_PRIORITY;
		IpcEventAttribute attr = new IpcEventAttribute();
		for (int loop = 0; loop < 10000; loop++) {
			int pri = rand.nextInt();

			assertEquals(prev, attr.getPriority());
			attr.setPriority(pri);
			assertEquals(pri, attr.getPriority());
			prev = pri;
		}
	}

	/**
	 * Test case for {@link IpcEventAttribute#isLogEnabled()} and
	 * {@link IpcEventAttribute#setLogEnabled(boolean)}.
	 */
	public void testLogEnabled()
	{
		IpcEventAttribute attr = new IpcEventAttribute();
		assertFalse(attr.isLogEnabled());

		boolean[] bools = {
			false,
			true,
			true,
			false,
			false,
		};

		for (boolean b: bools) {
			attr.setLogEnabled(b);
			assertEquals(b, attr.isLogEnabled());
		}
	}

	/**
	 * Test case for {@link IpcEventAttribute#getHostSet()} and
	 * {@link IpcEventAttribute#setHostSet(String)}.
	 *
	 * This method is run on a child process.
	 */
	static void hostSetTest()
	{
		IpcEventSystem esys = IpcEventSystem.getInstance();

		try {
			esys.initialize();
		}
		catch (Exception e) {
			unexpected(e);
		}

		IpcEventAttribute attr = new IpcEventAttribute();
		String[] names = {
			"a",
			"host",
			"host_set",
			"host_123_set",
			"too_long_ipc_host_set_name_1234",
		};

		// Create host set, and set it to attributes.
		String prevName = null;
		for (String name: names) {
			try {
				IpcHostSet hset = new IpcHostSet(name);
				hset.create();

				assertEquals(prevName, attr.getHostSet());
				attr.setHostSet(name);
				assertEquals(name, attr.getHostSet());
				prevName = name;
			}
			catch (Exception e) {
				unexpected(e);
			}
		}

		// Reset host set.
		try {
			assertEquals(prevName, attr.getHostSet());
			attr.setHostSet(null);
			assertEquals(null, attr.getHostSet());
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Set invalid host set name.
		for (int i = 0; i <= 10; i++) {
			String name = "hset_" + i;
			try {
				attr.setHostSet(name);
				needException("name=" + name);
			}
			catch (Exception e) {
				assertEquals(IpcNoSuchHostSetException.class,
					     e.getClass());
			}
		}

		// Delete host set.
		for (String name: names) {
			try {
				assertTrue(IpcHostSet.exists(name));
				IpcHostSet.destroy(name);
				assertFalse(IpcHostSet.exists(name));
			}
			catch (Exception e) {
				unexpected(e);
			}

			try {
				attr.setHostSet(name);
				needException("name=" + name);
			}
			catch (Exception e) {
				assertEquals(IpcNoSuchHostSetException.class,
					     e.getClass());
			}
		}

		try {
			esys.disable();
		}
		catch (Exception e) {
			unexpected(e);
		}
	}
}
