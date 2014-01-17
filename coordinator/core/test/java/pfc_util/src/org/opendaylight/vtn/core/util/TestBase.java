/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.lang.reflect.Method;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.util.List;
import java.util.LinkedList;
import junit.framework.Test;
import junit.framework.TestCase;
import org.opendaylight.vtn.core.CoreSystem;

/**
 * <p>
 *   Base class of test cases.
 * </p>
 */
public class TestBase extends TestCase
{
	/**
	 * <p>
	 *   Java command option to specify data model.
	 * </p>
	 */
	private final static String  _mode;

	/**
	 * <p>
	 *   Path to a JAR file which contains test classes.
	 * </p>
	 */
	private final static String  _jarFile;

	/**
	 * <p>
	 *   List to preserve output of a child process spawned by
	 *   {@link #runOnChild(String, Object...)}.
	 * </p>
	 */
	private List<String>  _childOut;

	/**
	 * <p>
	 *   List to preserve error output of a child process spawned by
	 *   {@link #runOnChild(String, Object...)}.
	 * </p>
	 */
	private List<String>  _childErr;

	/**
	 * <p>
	 *   Load native library for test.
	 * </p>
	 */
	static {
		CoreSystem.loadLibrary("pfc_util_test_jni");
		_mode = getJavaModeOption();
		_jarFile = getJarFilePath();
	}

	/**
	 * <p>
	 *   Create JUnit test case.
	 * </p>
	 *
	 * @param name	The test name.
	 */
	public TestBase(String name)
	{
		super(name);
	}

	/**
	 * <p>
	 *   Tear down the test environment.
	 * </p>
	 */
	@Override
	protected void tearDown()
	{
		_childOut = null;
		_childErr = null;

		// Run GC to ensure that object finalizer never crashes the
		// Java VM.
		System.gc();
	}

	/**
	 * <p>
	 *   Ensure that required exception was thrown.
	 * </p>
	 *
	 * @param t	A throwable to be tested.
	 * @param cls	Required exception class.
	 */
	protected static void checkException(Throwable t, Class<?> cls)
	{
		checkException(t, cls, null);
	}

	/**
	 * <p>
	 *   Ensure that required exception was thrown.
	 * </p>
	 *
	 * @param t	A throwable to be tested.
	 * @param cls	Required exception class.
	 * @param msg	Required message in an exception.
	 */
	protected static void checkException(Throwable t, Class<?> cls,
					     String msg)
	{
		assertEquals(cls, t.getClass());

		if (msg != null) {
			assertEquals(msg, t.getMessage());
		}
	}

	/**
	 * <p>
	 *   Throw an error which indicates an exception is required.
	 * </p>
	 */
	protected static void needException()
	{
		needException(null);
	}

	/**
	 * <p>
	 *   Throw an error which indicates an exception is required.
	 * </p>
	 *
	 * @param obj	An object to be displayed.
	 */
	protected static void needException(Object obj)
	{
		String msg = "An execption must be thrown";

		if (obj == null) {
			msg += ".";
		}
		else {
			msg += ": " + obj;
		}
		fail(msg);
	}

	/**
	 * <p>
	 *   Throw an error which indicates an unexpected exception was caught.
	 * </p>
	 *
	 * @param t	A throwable.
	 */
	protected static void unexpected(Throwable t)
	{
		throw new AssertionError("Unexpected exception: " + t, t);
	}

	/**
	 * <p>
	 *   Return a list which contains command line arguments to spawn a
	 *   new Java process.
	 * </p>
	 * <p>
	 *   To spawn a new Java process, the caller must append the main
	 *   class name to the returned list.
	 * </p>
	 *
	 * @return	List of command line arguments.
	 */
	static List<String> getJavaArguments()
	{
		LinkedList<String> list = new LinkedList<String>();

		String java = System.getProperty("java.home") + "/bin/java";
		list.add(java);
		list.add(_mode);
		list.add("-classpath");
		list.add(_jarFile);

		String prop = "pflow.core.libpath";
		String libpath = System.getProperty(prop);
		if (libpath != null) {
			list.add("-D" + prop + "=" + libpath);
		}

		return list;
	}

	/**
	 * <p>
	 *   Set list to preserve output of a child process spawned by
	 *   {@link #runOnChild(String, Object...)}.
	 * </p>
	 *
	 * @param out	A list to preserve outputs to the standard output.
	 * @param err	A list to preserve outputs to the standard error
	 *		output.
	 */
	protected void setChildOutput(List<String> out, List<String> err)
	{
		_childOut = out;
		_childErr = err;
	}

	/**
	 * <p>
	 *   Run the specified method on a child process.
	 * </p>
	 *
	 * @param method	The name of method to run.
	 *			Note that the method specified by the name
	 *			must be a static method. Class of the
	 *			method is determined by {@code getClass()}
	 *			of this instance.
	 * @param args		Parameters to be passed to test method.
	 */
	protected void runOnChild(String method, Object ... args)
	{
		List<String> list = TestBase.getJavaArguments();

		// Set main class.
		list.add(TestBase.class.getName());

		// Set class and method name of the target.
		list.add(getClass().getName());
		list.add(method);

		// Set parameters for method.
		for (Object arg: args) {
			list.add(arg.toString());
		}

		// Run the specified method on a child.
		ProcessBuilder builder = new ProcessBuilder(list);
		Process p = null;

		try {
			p = builder.start();
		}
		catch (Exception e) {
			unexpected(e);
		}

		try {
			BufferedReader reader = new BufferedReader
				(new InputStreamReader(p.getInputStream()));
			for (String line = reader.readLine(); line != null;
			     line = reader.readLine()) {
				if (_childOut != null) {
					_childOut.add(line);
				}
				else {
					fail("Unexpected output to stdout: " +
					     line);
				}
			}

			reader = new BufferedReader
				(new InputStreamReader(p.getErrorStream()));
			for (String line = reader.readLine(); line != null;
			     line = reader.readLine()) {
				if (_childErr != null) {
					_childErr.add(line);
				}
				else {
					fail("Unexpected output to stderr: " +
					     line);
				}
			}
		}
		catch (Exception e) {
			unexpected(e);
		}

		try {
			assertEquals(0, p.waitFor());
		}
		catch (Exception e) {
			unexpected(e);
		}
	}

	/**
	 * <p>
	 *   Remove the file specified by the given path.
	 * </p>
	 *
	 * @param path	A file path to be removed.
	 * @throws IllegalStateException
	 *	Failed to remove the given file.
	 */
	static native void removePath(String path);

	/**
	 * <p>
	 *   Return a java command option to specify data model.
	 * </p>
	 *
	 * @return	A java command option.
	 */
	private static native String getJavaModeOption();

	/**
	 * <p>
	 *   Return a path to JAR file which contains this test.
	 * </p>
	 *
	 * @return	A path to JAR file which contains this test.
	 */
	private static native String getJarFilePath();


	/**
	 * <p>
	 *   Entry point of a child process spawned by
	 *   {@link #runOnChild(String, Object...)}.
	 * </p>
	 *
	 * @param args	Command line arguments.
	 */
	public static void main(String[] args)
	{
		String clname = args[0];
		String mname = args[1];
		Class<?>[] argTypes;
		Object[] margs;
		if (args.length > 2) {
			argTypes = new Class<?>[]{String[].class};
			String[] sargs = new String[args.length - 2];
			System.arraycopy(args, 2, sargs, 0, sargs.length);
			margs = new Object[] {sargs};
		}
		else {
			argTypes = (Class<?>[])null;
			margs = null;
		}

		try {
			Class<?> cls = Class.forName(args[0]);
			Method method = cls.getDeclaredMethod(mname, argTypes);
			method.invoke(null, margs);
		}
		catch (Exception e) {
			unexpected(e);
		}

		// Run GC to ensure that object finalizer never crashes the
		// Java VM.
		System.gc();
		try {
			Thread.sleep(50);
		}
		catch (InterruptedException e) {}
	}
}
