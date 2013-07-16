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
 *   Unit test class for {@link IpcEventConfiguration}.
 * </p>
 */
public class IpcEventConfigurationTest extends TestBase
{
	/**
	 * Random number generator.
	 */
	private final Random  _rand = new Random();

	/**
	 * Create JUnit test case for {@link IpcEventConfiguration}.
	 *
	 * @param name	The test name.
	 */
	public IpcEventConfigurationTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcEventConfiguration#setIdleTimeout(int)} and
	 * {@link IpcEventConfiguration#getIdleTimeout()}.
	 */
	public void testIdleTimeout()
	{
		IpcEventConfiguration conf = new IpcEventConfiguration();

		assertEquals(0, conf.getIdleTimeout());
		for (int loop = 0; loop < 1000; loop++) {
			int value = _rand.nextInt();

			conf.setIdleTimeout(value);
			assertEquals(value, conf.getIdleTimeout());

			assertEquals(0, conf.getMaxThreads());
			assertEquals(0, conf.getConnectInterval());
			assertEquals(0, conf.getTimeout());
		}

		conf.setIdleTimeout(0);
		assertEquals(0, conf.getIdleTimeout());
	}

	/**
	 * Test case for {@link IpcEventConfiguration#setMaxThreads(int)} and
	 * {@link IpcEventConfiguration#getMaxThreads()}.
	 */
	public void testMaxThreads()
	{
		IpcEventConfiguration conf = new IpcEventConfiguration();

		assertEquals(0, conf.getMaxThreads());
		for (int loop = 0; loop < 1000; loop++) {
			int value = _rand.nextInt();

			conf.setMaxThreads(value);
			assertEquals(value, conf.getMaxThreads());

			assertEquals(0, conf.getIdleTimeout());
			assertEquals(0, conf.getConnectInterval());
			assertEquals(0, conf.getTimeout());
		}

		conf.setMaxThreads(0);
		assertEquals(0, conf.getMaxThreads());
	}

	/**
	 * Test case for {@link IpcEventConfiguration#setConnectInterval(int)}
	 * and {@link IpcEventConfiguration#getConnectInterval()}.
	 */
	public void testConnectInterval()
	{
		IpcEventConfiguration conf = new IpcEventConfiguration();

		assertEquals(0, conf.getConnectInterval());
		for (int loop = 0; loop < 1000; loop++) {
			int value = _rand.nextInt();

			conf.setConnectInterval(value);
			assertEquals(value, conf.getConnectInterval());

			assertEquals(0, conf.getIdleTimeout());
			assertEquals(0, conf.getMaxThreads());
			assertEquals(0, conf.getTimeout());
		}

		conf.setConnectInterval(0);
		assertEquals(0, conf.getConnectInterval());
	}

	/**
	 * Test case for {@link IpcEventConfiguration#setTimeout(int)}
	 * and {@link IpcEventConfiguration#getTimeout()}.
	 */
	public void testTimeout()
	{
		IpcEventConfiguration conf = new IpcEventConfiguration();

		assertEquals(0, conf.getTimeout());
		for (int loop = 0; loop < 1000; loop++) {
			int value = _rand.nextInt();

			conf.setTimeout(value);
			assertEquals(value, conf.getTimeout());

			assertEquals(0, conf.getIdleTimeout());
			assertEquals(0, conf.getMaxThreads());
			assertEquals(0, conf.getConnectInterval());
		}

		conf.setTimeout(0);
		assertEquals(0, conf.getTimeout());
	}
}
