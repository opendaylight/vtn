/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.util.Random;
import java.util.List;
import java.util.LinkedList;
import java.util.Enumeration;
import java.net.InetAddress;
import java.net.Inet6Address;
import java.net.NetworkInterface;
import junit.framework.Test;
import junit.framework.TestCase;
import org.opendaylight.vtn.core.CoreSystem;
import org.opendaylight.vtn.core.util.TimeSpec;

/**
 * Base class of test cases.
 */
public abstract class TestBase extends TestCase
{
	/**
	 * Java command option to specify data model.
	 */
	private final static String  _mode;

	/**
	 * Path to a JAR file which contains test classes.
	 */
	private final static String  _jarFile;

	/**
	 * {@code true} only if this host is IPv6 ready.
	 */
	protected final static boolean  _inet6;

	/**
	 * List to preserve output of a child process spawned by
	 * {@link #runOnChild(String, Object[])}.
	 */
	private List<String>  _childOut;

	/**
	 * List to preserve error output of a child process spawned by
	 * {@link #runOnChild(String, Object[])}.
	 */
	private List<String>  _childErr;

	/**
	 * Load native library for test.
	 */
	static {
		CoreSystem.loadLibrary("pfc_ipc_test_jni");
		_mode = getJavaModeOption();
		_jarFile = getJarFilePath();
		try {
			_inet6 = isInet6Ready();
		}
		catch (Exception e) {
			throw new IllegalStateException
				("Unable to determine IPv6 interface.");
		}
	}

	/**
	 * <p>
	 *   Ensure that JNI library is loaded.
	 * </p>
	 */
	static void loadLibrary() {}

	/**
	 * Create JUnit test case.
	 *
	 * @param name	The test name.
	 */
	public TestBase(String name)
	{
		super(name);
	}

	/**
	 * Tear down the test environment.
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
	 * Ensure that required exception was thrown.
	 *
	 * @param e	An exception to be tested.
	 * @param cls	Required exception class.
	 */
	protected static void checkException(Exception e, Class<?> cls)
	{
		checkException(e, cls, null);
	}

	/**
	 * Ensure that required exception was thrown.
	 *
	 * @param e	An exception to be tested.
	 * @param cls	Required exception class.
	 * @param msg	Required message in an exception.
	 */
	protected static void checkException(Exception e, Class<?> cls,
					     String msg)
	{
		assertEquals(cls, e.getClass());
		if (msg != null) {
			assertEquals(msg, e.getMessage());
		}
	}

	/**
	 * Ensure that required exception was thrown.
	 *
	 * @param e	An exception to be tested.
	 * @param clses	Required exception classes.
	 */
	protected static void checkException(Exception e, List<Class<?>> clses)
	{
		boolean result = false;
		Class<?> ecls = e.getClass();

		for (Class<?> cls: clses) {
			if (cls == ecls) {
				result = true;
				break;
			}
		}
		assertTrue(result);
	}

	/**
	 * Throw an error which indicates an exception is required.
	 */
	protected static void needException()
	{
		needException(null);
	}

	/**
	 * Throw an error which indicates an exception is required.
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
	 * Throw an error which indicates an unexpected exception is caught.
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
	 * Set list to preserve output of a child process spawned by
	 * {@link #runOnChild(String, Object[])}.
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
	 * Return a java command option to specify data model.
	 *
	 * @return	A java command option.
	 */
	private static native String getJavaModeOption();

	/**
	 * Return a path to JAR file which contains this test.
	 *
	 * @return	A path to JAR file which contains this test.
	 */
	private static native String getJarFilePath();

	/**
	 * Return a {@link TimeSpec} instance which contains the current
	 * system time.
	 *
	 * @return	A {@link TimeSpec} instance.
	 */
	static native TimeSpec getCurrentTime();

	/**
	 * <p>
	 *   Return a number associated with the given error number.
	 * </p>
	 *
	 * @param name	A string which represents error number, such as
	 *		<strong>EINVAL</strong>.
	 * @return	A number associated with {@code errno}.
	 */
	static native int getErrorNumber(String name);

	/**
	 * <p>
	 *   Convert the given long value into a string as a pointer value.
	 * </p>
	 * <p>
	 *   This method returns a string converted by "%p" printf(3)
	 *   conversion specifier.
	 * </p>
	 *
	 * @param value	A long value to be converted.
	 * @return	A string which represents {@code value} as a pointer.
	 */
	static native String toHexPointer(long value);

	/**
	 * <p>
	 *   Remove the file specified by the given path.
	 * </p>
	 *
	 * @param path	A file path to be removed.
	 * @throws IllegalStateException
	 *      Failed to remove the given file.
	 */
	static native void removePath(String path);

	/**
	 * Determine whether this host is IPv6 ready.
	 *
	 * @return	{@code true} only if this host is IPv6 ready.
	 */
	private static boolean isInet6Ready() throws Exception
	{
		for (Enumeration<NetworkInterface> en =
			     NetworkInterface.getNetworkInterfaces();
		     en.hasMoreElements();) {
			NetworkInterface nif = en.nextElement();
			for (Enumeration<InetAddress> ien =
				     nif.getInetAddresses();
			     ien.hasMoreElements();) {
				InetAddress iaddr = ien.nextElement();
				if (!iaddr.isLoopbackAddress() &&
				    (iaddr instanceof Inet6Address)) {
					return true;
				}
			}
		}

		return false;
	}

	/**
	 * <p>
	 *   Get a value in the private field of the given object.
	 * </p>
	 *
	 * @param obj		The target object.
	 * @param name		The name of the target private field.
	 * @return		An object at the specified field.
	 */
	protected static Object getFieldValue(Object obj, String name)
	{
		try {
			Class<?> cls = obj.getClass();
			Field field = cls.getDeclaredField(name);
			field.setAccessible(true);

			return field.get(obj);
		}
		catch (Exception e) {
			throw new IllegalStateException
				("Unable to access field: " + name, e);
		}
	}

	/**
	 * Create an additional data array which contains all supported
	 * data types.
	 *
	 * @param rand	Random number generator.
	 */
	static IpcDataUnit[] createData(Random rand)
	{
		byte[] raw4 = new byte[4];
		byte[] raw16 = new byte[16];
		byte[] bin = new byte[rand.nextInt(128) + 1];
		rand.nextBytes(bin);

		IpcStruct psdump = new IpcStruct("pfcd_psdump");
		for (String field: psdump.getFieldNames()) {
			psdump.setInt(field, rand.nextInt());
		}

		IpcDataUnit[] data = {
			new IpcInt8((byte)rand.nextInt(0x100)),
			new IpcUint8((byte)rand.nextInt(0x100)),
			new IpcInt16((short)rand.nextInt(0x10000)),
			new IpcUint16((short)rand.nextInt(0x10000)),
			new IpcInt32(rand.nextInt()),
			new IpcUint32(rand.nextInt()),
			new IpcInt64(rand.nextLong()),
			new IpcUint64(rand.nextLong()),
			new IpcFloat(rand.nextFloat()),
			new IpcDouble(rand.nextDouble()),
			IpcInetAddress.create(raw4),
			IpcInetAddress.create(raw16),
			new IpcString(),
			new IpcString(""),
			new IpcString("test:" + rand.nextInt()),
			new IpcBinary(),
			new IpcBinary(new byte[0]),
			new IpcBinary(bin),
			new IpcNull(),
			psdump,
		};

		return data;
	}

	/**
	 * Create an additional data array which has no body.
	 */
	static IpcDataUnit[] createEmptyData()
	{
		LinkedList<IpcDataUnit> list = new LinkedList<IpcDataUnit>();

		byte[] empty = new byte[0];
		for (int loop = 0; loop < 10; loop++) {
			list.add(new IpcBinary());
			list.add(new IpcString());
			list.add(new IpcNull());
			list.add(new IpcBinary(empty));
		}

		return list.toArray(new IpcDataUnit[0]);
	}

	/**
	 * Ensure that the given two data are identical.
	 *
	 * @param expected	An expected value.
	 * @param data		A data to be tested.
	 */
	static void checkEquals(IpcDataUnit expected, IpcDataUnit data)
	{
		assertNotSame(expected, data);

		if (data.getType() != IpcDataUnit.STRUCT) {
			assertEquals(expected, data);

			return;
		}

		IpcStruct exstruct = (IpcStruct)expected;
		if (!(data instanceof IpcStruct)) {
			fail("IPC structure is expected, but " +
			     data.getClass());
		}

		IpcStruct struct = (IpcStruct)data;
		assertEquals(exstruct.getName(), struct.getName());
		for (String name: exstruct.getFieldNames()) {
			IpcStructField field = exstruct.getField(name);
			int alen = field.getArrayLength();

			if (alen == 0) {
				checkEquals(exstruct.get(name),
					    struct.get(name));
				continue;
			}

			for (int i = 0; i < alen; i++) {
				checkEquals(exstruct.get(name, i),
					    struct.get(name, i));
			}
		}
	}

	/**
	 * Entry point of a child process spawned by
	 * {@link #runOnChild(String, Object[])}.
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
