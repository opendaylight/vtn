/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core;

import junit.framework.Test;
import junit.framework.TestSuite;

/**
 * <p>
 *   Bundle of all JUnit tests for org.opendaylight.vtn.core package.
 * </p>
 */
public class AllTests
{
	/**
	 * <p>
	 *   Return test suits which contains all JUnit tests for
	 *   org.opendaylight.vtn.core package.
	 * </p>
	 *
	 * @return	{@code Test} instance which contains tests.
	 */
	public static Test suite()
	{
		TestSuite  suite = new TestSuite("All core JUnit tests");

		suite.addTest(new TestSuite(CoreSystemTest.class));
		suite.addTest(org.opendaylight.vtn.core.util.AllTests.suite());

		return suite;
	}
}
