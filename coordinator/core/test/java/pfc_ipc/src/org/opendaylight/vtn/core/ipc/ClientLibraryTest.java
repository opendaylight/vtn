/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.LinkedList;
import org.opendaylight.vtn.core.util.HostAddress;
import org.opendaylight.vtn.core.util.TimeSpec;
import java.io.File;
import java.io.FileReader;
import java.io.LineNumberReader;
import org.opendaylight.vtn.core.util.LogConfiguration;
import org.opendaylight.vtn.core.util.LogSystem;

/**
 * <p>
 *   Unit test class for {@link ClientLibrary}.
 * </p>
 */
public class ClientLibraryTest extends TestBase
{
	/**
	 * Temporary directory path.
	 */
	private File  _tmpDir;

	/**
	 * <p>
	 *   Create JUnit test case for {@link ClientLibraryTest}.
	 * </p>
	 *
	 * @param name	The test name.
	 */
	public ClientLibraryTest(String name)
	{
		super(name);
	}

	/**
	 * Tear down the test.
	 */
	@Override
	protected void tearDown()
	{
		if (_tmpDir != null) {
			removePath(_tmpDir.getAbsolutePath());
		}

		super.tearDown();
	}

	/**
	 * <p>
	 *   Test case for {@link ClientLibrary#disable()}.
	 * </p>
	 */
	public void testDisable()
	{
		runOnChild("disableTest");
	}

	/**
	 * <p>
	 *   Test case for {@link ClientSession#setLogEnabled(boolean)}.
	 * </p>
	 * <p>
	 *   Note that this test create run/ipc/.test/log/ipc_test.log file
	 *   under the modifiable system data directory (Default: 
	 *   /usr/local/pfc/var) and delete its file.  The directories under
	 *   .test is deleted if and only if directories has no entry.
	 * </p>
	 */
	public void testLog()
	{
		try {
			logTest();
		}
		catch (Exception e) {
			unexpected(e);
		}
	}


	/**
	 * <p>
	 *   Ensure that {@link ClientLibrary#disable()} cancels all ongoing
	 *   IPC client sessions.
	 * </p>
	 * <p>
	 *   This method is invoked on a child process.
	 * </p>
	 *
	 * @throws Exception	An unexpected exception was thrown.
	 */
	static void disableTest() throws Exception
	{
		ChannelAddress chaddr = new ChannelAddress("foo");
		IpcConnection altconn = null;
		try {
			altconn = AltConnection.open(chaddr);
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Disable IPC client library.
		assertFalse(ClientLibrary.isDisabled());
		ClientLibrary.disable();
		assertTrue(ClientLibrary.isDisabled());

		// Ensure that IPC client is no longer available.
		IpcConnection[] conns = {
			DefaultConnection.getInstance(),
			altconn,
		};

		Class<?> cls = IpcClientDisabledException.class;
		for (IpcConnection conn: conns) {
			ClientSession sess = conn.createSession("bar", 0);
			try {
				sess.invoke();
				needException();
			}
			catch (Exception e) {
				checkException(e, cls);
			}
			finally {
				sess.destroy();
			}
		}

		// Connection should be able to be closed.
		try {
			altconn.close();
		}
		catch (Exception e) {
			unexpected(e);
		}
	}

	/**
	 * <p>
	 *   Ensure that ClientLibrary can control log system.
	 * </p>
	 * <p>
	 *   Note that this test checks only to enable/disalbe log system.
	 *   Contents in log is not checked.
	 * </p>
	 *
	 * @throws Exception	An unexpected exception was thrown.
	 */
	private void logTest() throws Exception
	{
		String testDirName = "ClientLibraryTest.logdir";
		File dir = new File(testDirName).getAbsoluteFile();
		assertTrue(dir.mkdir());
		_tmpDir = dir;

		// Initialize log file and log system.
		String testFileName = ".ipc_test.log";
		File testFile = new File(dir, testFileName);
		testFile.delete();

		LogConfiguration logConf =
			new LogConfiguration("ipc_test", testFile);

		LogSystem logSys = LogSystem.getInstance();
		logSys.initialize(logConf);

		// First, log system is disabled by default settings.
		generateLogs();
		int numLineLog = checkLogFile(testFile);
		assertEquals(numLineLog, 0);
 
                // Logs are outputed by enabling log system.
		ClientLibrary.setLogEnabled(true);
		generateLogs();
		numLineLog = checkLogFile(testFile);
		assertTrue(numLineLog != 0);
		
		// Logs are not outputed by disabling log system.
		ClientLibrary.setLogEnabled(false);
		generateLogs();
		assertEquals(checkLogFile(testFile), numLineLog);

		// Shut down the log system.
		logSys.shutdown();
	}

	/**
	 * Let the system generate some logs.
	 */
	private void generateLogs()
	{
		// Let the IPC event subsystem logs error message.
		IpcEventSystem esys = IpcEventSystem.getInstance();
		IpcEventConfiguration conf = new IpcEventConfiguration();
		conf.setTimeout(-1);

		try {
			esys.initialize(conf);
			needException();
		}
		catch (Exception e) {
			checkException(e, IllegalArgumentException.class);
		}
	}

	/**
	 * <p>
	 *   Get the number of log lines.
	 * </p>
	 *
	 * @param logFile	A target log file.
	 * @throws Exception	An unexpected exception was thrown.
	 */
	private int checkLogFile(File logFile) throws Exception
	{
		LineNumberReader fin =
			new LineNumberReader(new FileReader(logFile));
		String	line;
		int	noLine = 0;

		while (null != (line = fin.readLine())) {
			// If log contents should be checked,
			// Add check function in this position
		}
		noLine = fin.getLineNumber();
		fin.close();
		return noLine;
	}
}
