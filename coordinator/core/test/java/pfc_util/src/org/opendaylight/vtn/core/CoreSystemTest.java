/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import org.opendaylight.vtn.core.util.TestBase;

/**
 * <p>
 *   Unit test class for {@link CoreSystem}.
 * </p>
 */
public class CoreSystemTest extends TestBase
{
	/**
	 * <p>
	 *   Create JUnit test case for {@link CoreSystem}.
	 * </p>
	 *
	 * @param name	The test name.
	 */
	public CoreSystemTest(String name)
	{
		super(name);
	}

	/**
	 * <p>
	 *   Ensure that all constants which represent build configuration are
	 *   initialized correctly.
	 * </p>
	 * 
	 */
	public void testConstants()
	{
		// Name of constants to be tested.
		String[] names = {
			"PRODUCT_NAME",
			"VERSION",
			"VERSION_MAJOR",
			"VERSION_MINOR",
			"VERSION_REVISION",
			"VERSION_PATCHLEVEL",
			"DEBUG",
			"INST_PREFIX",
			"INST_BINDIR",
			"INST_DATADIR",
			"INST_LIBDIR",
			"INST_LIBEXECDIR",
			"INST_LOCALSTATEDIR",
			"INST_SBINDIR",
			"INST_SYSCONFDIR",
			"JAVA_LIBDIR",
			"JNI_LIBDIR",
		};

		for (String name: names) {
			checkConfiguration(name);
		}
	}

	/**
	 * <p>
	 *   Ensure that a constant provided by {@link CoreSystem} class
	 *   is initialized correctly.
	 * </p>
	 *
	 * @param name	The name of constant to be tested.
	 */
	private void checkConfiguration(String name)
	{
		Field field = null;
		try {
			field = CoreSystem.class.getField(name);
		}
		catch (Exception e) {
			fail("Unable to get CoreSystem field: " + name +
			     ": " + e);
		}

		int mod = field.getModifiers();
		final int mask = (Modifier.PUBLIC | Modifier.FINAL |
				  Modifier.STATIC);
		assertEquals(mask, mod & mask);

		Class<?> type = field.getType();
		if (String.class.equals(type)) {
			// String field.
			Object value = null;
			try {
				value = field.get(null);
			}
			catch (Exception e) {
				fail("Unable to get string field value: " +
				     name + ": " + e);
			}

			assertEquals("Unexpected configuration: " + name,
				     getConfiguration(name), value);
		}
		else if (boolean.class.equals(type)) {
			// Boolean field.
			int value = 0;
			try {
				value = (field.getBoolean(null)) ? 1 : 0;
			}
			catch (Exception e) {
				fail("Unable to get boolean field value: " +
				     name + ": " + e);
			}

			assertEquals("Unexpected configuration: " + name,
				     getIntConfiguration(name), value);
		}
		else {
			// Integer field.
			assertEquals("Unexpected field=" + name,
				     int.class, type);

			int value = 0;
			try {
				value = field.getInt(null);
			}
			catch (Exception e) {
				fail("Unable to get integer field value: " +
				     name + ": " + e);
			}

			assertEquals("Unexpected configuration: " + name,
				     getIntConfiguration(name), value);
		}
	}

	/**
	 * <p>
	 *   Return a string which represents build configuration associated
	 *   with the given key.
	 * </p>
	 *
	 * @param key	A key string to specify build configuration.
	 * @return	A string associated with the given key.
	 * @throws IllegalStateException
	 *	{@code key} is an unknown key string.
	 */
	private native String getConfiguration(String key);

	/**
	 * <p>
	 *   Return an integer value which represents build configuration
	 *   associated with the given key.
	 * </p>
	 *
	 * @param key	A key string to specify build configuration.
	 * @return	An integer value associated with the given key.
	 * @throws IllegalStateException
	 *	{@code key} is an unknown key string.
	 */
	private native int getIntConfiguration(String key);
}
