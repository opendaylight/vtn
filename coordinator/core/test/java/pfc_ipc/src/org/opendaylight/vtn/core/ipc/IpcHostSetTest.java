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
import java.util.TreeSet;
import java.util.Iterator;
import java.util.LinkedList;
import org.opendaylight.vtn.core.util.HostAddress;

/**
 * <p>
 *   Unit test class for {@link IpcHostSet}.
 * </p>
 */
public class IpcHostSetTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcHostSet}.
	 *
	 * @param name	The test name.
	 */
	public IpcHostSetTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcHostSet#IpcHostSet(String)}.
	 */
	public void testCtor()
	{
		try {
			new IpcHostSet(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "name is null.");
		}

		Random rand = new Random();
		for (int loop = 0; loop < 10000; loop++) {
			String name = "hset_" + rand.nextInt(10000);

			IpcHostSet hset = new IpcHostSet(name);
			IpcHostSet same_hset = new IpcHostSet(name);
			assertEquals(name, hset.getName());
			assertEquals(name, same_hset.getName());
		}

		// Construct an IpcHostSet instance with invalid name.
		String[] invalidNames = {
			"",
			"too_long_ipc_host_set_name_12345",
			"0123",
			"host_\u3042",
		};

		for (String name: invalidNames) {
			IpcHostSet hset = new IpcHostSet(name);
			assertEquals(name, hset.getName());
		}
	}

	/**
	 * Test case for methods which handle host set.
	 *
	 * <ul>
	 *   <li>{@link IpcHostSet#create()}</li>
	 *   <li>{@link IpcHostSet#destroy()}</li>
	 *   <li>{@link IpcHostSet#exists()}</li>
	 *   <li>{@link IpcHostSet#create(String)}</li>
	 *   <li>{@link IpcHostSet#destroy(String)}</li>
	 *   <li>{@link IpcHostSet#exists(String)}</li>
	 * </ul>
	 */
	public void testHostSet()
	{
		// Specify null host set name.
		try {
			IpcHostSet.create(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class);
		}
		try {
			IpcHostSet.destroy(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class);
		}
		try {
			IpcHostSet.exists(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class);
		}

		// Run tests on a child process.
		runOnChild("hostSetTest");
		runOnChild("staticHostSetTest");
	}

	/**
	 * Test case for methods which handle host address.
	 *
	 * <ul>
	 *   <li>{@link IpcHostSet#add(HostAddress)}</li>
	 *   <li>{@link IpcHostSet#remove(HostAddress)}</li>
	 *   <li>{@link IpcHostSet#contains(HostAddress)}</li>
	 * </ul>
	 */
	public void testHostAddress()
	{
		// Specify null host address.
		IpcHostSet hset = new IpcHostSet("set");
		Class<?> cls = NullPointerException.class;
		try {
			hset.add(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			hset.remove(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			hset.contains(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}

		// The IPC event subsystem is not yet initialized.
		HostAddress haddr = HostAddress.getLocalAddress();
		cls = IpcEventSystemNotReadyException.class;
		try {
			hset.add(haddr);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			hset.remove(haddr);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			hset.contains(haddr);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}

		// Multiple exceptions
		try {
			hset.add(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcEventSystemNotReadyException.class);
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			hset.remove(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcEventSystemNotReadyException.class);
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			hset.contains(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcEventSystemNotReadyException.class);
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}

		// Run test on a child process.
		runOnChild("hostAddressTest");
	}

	/**
	 * Test case for instance methods which handle host set.
	 * This method is run on a child process.
	 */
	static void hostSetTest()
	{
		hostSetTestImpl(new Wrapper());
	}

	/**
	 * Test case for static methods which handle host set.
	 * This method is run on a child process.
	 */
	static void staticHostSetTest()
	{
		hostSetTestImpl(new StaticWrapper());
	}

	/**
	 * Test case for methods which handle host set.
	 * This method is run on a child process.
	 *
	 * @param wrapper	A {@link Wrapper} instance.
	 */
	private static void hostSetTestImpl(Wrapper wrapper)
	{
		// Try to access host set without event subsystem
		// initialization.
		Class<?> cls = IpcEventSystemNotReadyException.class;
		String hname = "hostset";
		try {
			wrapper.create(hname);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			wrapper.destroy(hname);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			wrapper.exists(hname);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}

		// Multiple exceptions
		try {
			wrapper.create(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcEventSystemNotReadyException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			wrapper.destroy(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcEventSystemNotReadyException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			wrapper.exists(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcEventSystemNotReadyException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			wrapper.create("");
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcEventSystemNotReadyException.class);
			clses.add(IllegalArgumentException.class);
			checkException(e, clses);
		}
		try {
			wrapper.destroy("");
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcEventSystemNotReadyException.class);
			clses.add(IpcNoSuchHostSetException.class);
			checkException(e, clses);
		}

		// Initialize the event subsystem.
		IpcEventSystem esys = IpcEventSystem.getInstance();
		try {
			esys.initialize();
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Create host sets.
		String[] names = {
			hname,
			"h",
			"ho",
			"hos",
			"host",
			"host_set",
			"host_123_set",
			"too_long_ipc_host_set_name_1234",
		};
		try {
			for (String name: names) {
				assertFalse(wrapper.exists(name));
				wrapper.create(name);
				assertTrue(wrapper.exists(name));
			}
		}
		catch (Exception e) {
			unexpected(e);
		}

		cls = IpcAlreadyExistsException.class;
		for (String name: names) {
			try {
				wrapper.create(name);
				needException("name=" + name);
			}
			catch (Exception e) {
				checkException(e, cls);
			}
		}

		// Try to create a host set with invalid name.
		String[] invalidNames = {
			"",
			"too_long_ipc_host_set_name_12345",
			"0123",
			"host_\u3042",
		};

		cls = IllegalArgumentException.class;
		for (String name: invalidNames) {
			try {
				wrapper.create(name);
				needException("name=" + name);
			}
			catch (Exception e) {
				checkException(e, cls);
			}

			try {
				assertFalse(wrapper.exists(name));
			}
			catch (Exception e) {
				unexpected(e);
			}
		}

		// Destroy host sets.
		try {
			for (String name: names) {
				assertTrue(wrapper.exists(name));
				wrapper.destroy(name);
				assertFalse(wrapper.exists(name));
			}
		}
		catch (Exception e) {
			unexpected(e);
		}

		cls = IpcNoSuchHostSetException.class;
		for (String name: names) {
			try {
				wrapper.destroy(name);
				needException("name=" + name);
			}
			catch (Exception e) {
				checkException(e, cls);
			}
		}

		// Create host set again.
		try {
			for (String name: names) {
				assertFalse(wrapper.exists(name));
				wrapper.create(name);
				assertTrue(wrapper.exists(name));
			}
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Shut down the IPC event subsystem.
		cls = IpcCanceledException.class;
		try {
			esys.shutdown();
		}
		catch (Exception e) {
			unexpected(e);
		}

		// No more host set can not be created.
		try {
			wrapper.create("newset");
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}

		// Multiple exceptions
		try {
			wrapper.create(null);
			needException();
		} catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			wrapper.create("");
			needException();
		} catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(IllegalArgumentException.class);
			checkException(e, clses);
		}

		// destroy() and exists() must be still available.
		for (int i = 0; i < names.length / 2; i++) {
			try {
				assertTrue(wrapper.exists(names[i]));
				wrapper.destroy(names[i]);
				assertFalse(wrapper.exists(names[i]));
				names[i] = null;
			}
			catch (Exception e) {
				unexpected(e);
			}
		}

		// Disable the IPC event subsystem.
		try {
			esys.disable();
		}
		catch (Exception e) {
			unexpected(e);
		}

		for (String name: names) {
			if (name == null) {
				continue;
			}

			try {
				wrapper.create(name);
				needException();
			}
			catch (Exception e) {
				checkException(e, cls);
			}

			try {
				wrapper.exists(name);
				needException("name=" + name);
			}
			catch (Exception e) {
				checkException(e, cls);
			}

			try {
				wrapper.destroy(name);
				needException("name=" + name);
			}
			catch (Exception e) {
				checkException(e, cls);
			}
		}

		// Multiple exceptions
		try {
			wrapper.create(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			wrapper.destroy(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			wrapper.exists(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			wrapper.create("");
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(IllegalArgumentException.class);
			checkException(e, clses);
		}
		try {
			wrapper.destroy("");
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(IpcNoSuchHostSetException.class);
			checkException(e, clses);
		}
	}

	/**
	 * Test case for methods which handle host address.
	 * This method is run on a child process.
	 */
	static void hostAddressTest()
	{
		// Initialize the event subsystem.
		IpcEventSystem esys = IpcEventSystem.getInstance();
		try {
			esys.initialize();
		}
		catch (Exception e) {
			unexpected(e);
		}

		// The specified IPC host set does not exist.
		IpcHostSet hset = new IpcHostSet("set");
		HostAddress local = HostAddress.getLocalAddress();
		Class<?> cls = IpcNoSuchHostSetException.class;
		try {
			hset.add(local);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			hset.remove(local);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			hset.contains(local);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}

		// Multiple exceptions
		try {
			hset.add(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			hset.remove(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			hset.contains(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}

		// Create two host sets.
		IpcHostSet hset1 = new IpcHostSet("set1");
		try {
			hset.create();
			assertTrue(hset.exists());
			hset1.create();
			assertTrue(hset1.exists());
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Add a local host to `hset'.
		try {
			assertFalse(hset.contains(local));
			assertTrue(hset.add(local));
			assertFalse(hset.add(local));
			assertTrue(hset.contains(local));
			assertFalse(hset1.contains(local));
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Currently only local host is supported.
		TreeSet<HostAddress> addrset = new TreeSet<HostAddress>();
		assertTrue(addrset.add(HostAddress.getByName("/INET4")));
		assertTrue(addrset.add(HostAddress.getByName("192.168.10.1")));
		assertTrue(addrset.add(HostAddress.getByName("/INET6")));
		assertTrue(addrset.add(HostAddress.getByName("ff02::1%1")));

		cls = IpcLibraryException.class;
		for (HostAddress haddr: addrset) {
			try {
				hset.add(haddr);
				needException();
			}
			catch (Exception e) {
				checkException(e, cls);
			}
		}

		// Remove host address.
		try {
			assertTrue(hset.contains(local));
			assertTrue(hset.remove(local));
			assertFalse(hset.remove(local));
			assertFalse(hset.contains(local));
			assertFalse(hset1.contains(local));
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Add local host to both host sets.
		try {
			assertFalse(hset.contains(local));
			assertTrue(hset.add(local));
			assertFalse(hset.add(local));
			assertTrue(hset.contains(local));

			assertFalse(hset1.contains(local));
			assertTrue(hset1.add(local));
			assertFalse(hset1.add(local));
			assertTrue(hset1.contains(local));
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Destroy host set, and recreate.
		try {
			hset1.destroy();
			hset1.create();
		}
		catch (Exception e) {
			unexpected(e);
		}

		try {
			assertFalse(hset1.contains(local));
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Shut down the IPC event subsystem.
		cls = IpcCanceledException.class;
		try {
			esys.shutdown();
		}
		catch (Exception e) {
			unexpected(e);
		}

		// No more host address can not be added.
		try {
			hset1.add(local);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}

		try {
			assertTrue(hset.contains(local));
			assertTrue(hset.remove(local));
			assertFalse(hset.remove(local));
			assertFalse(hset.contains(local));

			assertFalse(hset1.contains(local));
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Disable the IPC event subsystem.
		try {
			esys.disable();
		}
		catch (Exception e) {
			unexpected(e);
		}

		try {
			hset.add(local);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			hset.remove(local);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}
		try {
			hset.contains(local);
			needException();
		}
		catch (Exception e) {
			checkException(e, cls);
		}

		// Multiple exceptions
		hset = new IpcHostSet("new_hset");
		try {
			hset.add(null);
			needException();
		}
		catch (Exception e) {
		        LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			hset.remove(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
		try {
			hset.contains(null);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(IpcCanceledException.class);
			clses.add(IpcNoSuchHostSetException.class);
			clses.add(NullPointerException.class);
			checkException(e, clses);
		}
	}

	/**
	 * Wrapper class which invokes instance methods of {@link IpcHostSet}.
	 */
	private static class Wrapper
	{
		/**
		 * Invoke {@link IpcHostSet#create()}.
		 *
		 * @param name	The name of the IPC host set.
		 */
		void create(String name) throws IpcException
		{
			new IpcHostSet(name).create();
		}

		/**
		 * Invoke {@link IpcHostSet#destroy()}.
		 *
		 * @param name	The name of the IPC host set.
		 */
		void destroy(String name) throws IpcException
		{
			new IpcHostSet(name).destroy();
		}

		/**
		 * Invoke {@link IpcHostSet#exists()}.
		 *
		 * @param name	The name of the IPC host set.
		 * @return	Return value of {@link IpcHostSet#exists()}.
		 */
		boolean exists(String name) throws IpcException
		{
			return new IpcHostSet(name).exists();
		}
	}

	/**
	 * Wrapper class which invokes static methods of {@link IpcHostSet}.
	 */
	private static class StaticWrapper extends Wrapper
	{
		/**
		 * Invoke {@link IpcHostSet#create(String)}.
		 *
		 * @param name	The name of the IPC host set.
		 */
		@Override
		void create(String name) throws IpcException
		{
			IpcHostSet.create(name);
		}

		/**
		 * Invoke {@link IpcHostSet#destroy(String)}.
		 *
		 * @param name	The name of the IPC host set.
		 */
		@Override
		void destroy(String name) throws IpcException
		{
			IpcHostSet.destroy(name);
		}

		/**
		 * Invoke {@link IpcHostSet#exists(String)}.
		 *
		 * @param name	The name of the IPC host set.
		 * @return	Return value of
		 *		{@link IpcHostSet#exists(String)}.
		 */
		@Override
		boolean exists(String name) throws IpcException
		{
			return IpcHostSet.exists(name);
		}
	}
}
