/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import junit.framework.Test;
import junit.framework.TestSuite;

/**
 * <p>
 *   Bundle of all JUnit tests for org.opendaylight.vtn.core.ipc package.
 * </p>
 */
public class AllTests
{
	/**
	 * Return test suits which contains all JUnit tests for
	 * org.opendaylight.vtn.core.ipc package.
	 *
	 * @return	{@code Test} instance which contains tests.
	 */
	public static Test suite()
	{
		TestSuite  suite = new TestSuite("All core.ipc JUnit tests");

		suite.addTest(new TestSuite(ClientLibraryTest.class));
		suite.addTest(new TestSuite(IpcInt8Test.class));
		suite.addTest(new TestSuite(IpcInt16Test.class));
		suite.addTest(new TestSuite(IpcInt32Test.class));
		suite.addTest(new TestSuite(IpcInt64Test.class));
		suite.addTest(new TestSuite(IpcUint8Test.class));
		suite.addTest(new TestSuite(IpcUint16Test.class));
		suite.addTest(new TestSuite(IpcUint32Test.class));
		suite.addTest(new TestSuite(IpcUint64Test.class));
		suite.addTest(new TestSuite(IpcFloatTest.class));
		suite.addTest(new TestSuite(IpcDoubleTest.class));
		suite.addTest(new TestSuite(IpcInet4AddressTest.class));
		suite.addTest(new TestSuite(IpcInet6AddressTest.class));
		suite.addTest(new TestSuite(IpcStringTest.class));
		suite.addTest(new TestSuite(IpcBinaryTest.class));
		suite.addTest(new TestSuite(IpcNullTest.class));
		suite.addTest(IpcStructTest.suite());
		suite.addTest(new TestSuite(ChannelAddressTest.class));
		suite.addTest(new TestSuite(IpcHostSetTest.class));
		suite.addTest(new TestSuite(IpcEventMaskTest.class));
		suite.addTest(new TestSuite(IpcEventAttributeTest.class));
		suite.addTest(new TestSuite(IpcEventConfigurationTest.class));

		return suite;
	}
}
