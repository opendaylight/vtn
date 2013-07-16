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
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * <p>
 *   Unit test class for {@link IpcEventMask}.
 * </p>
 */
public class IpcEventMaskTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcEventMask}.
	 *
	 * @param name	The test name.
	 */
	public IpcEventMaskTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcEventMask#IpcEventMask()}.
	 */
	public void testCtorFilled()
	{
		IpcEventMask mask = new IpcEventMask();

		assertEquals(IpcEventMask.MASK_FILLED, mask.getMask());
		assertTrue(mask.isFilled());
	}

	/**
	 * Test case for {@link IpcEventMask#IpcEventMask(int)}.
	 */
	public void testCtorInt()
	{
		for (int type = IpcEvent.MIN_TYPE; type <= IpcEvent.MAX_TYPE;
		     type++) {
			IpcEventMask mask = new IpcEventMask(type);

			assertEquals(1L << type, mask.getMask());
		}

		checkIllegalType(new IllegalTypeTest() {
			public void run(int type)
			{
				new IpcEventMask(type);
			}
		});
	}

	/**
	 * Test case for {@link IpcEventMask#IpcEventMask(long)}.
	 */
	public void testCtorLong()
	{
		IpcEventMask mask = new IpcEventMask(IpcEventMask.MASK_EMPTY);
		assertEquals(IpcEventMask.MASK_EMPTY, mask.getMask());
		assertTrue(mask.isEmpty());

		mask = new IpcEventMask(IpcEventMask.MASK_FILLED);
		assertEquals(IpcEventMask.MASK_FILLED, mask.getMask());
		assertTrue(mask.isFilled());

		Random rand = new Random();

		for (int loop = 0; loop < 1000; loop++) {
			long l = rand.nextLong();
			mask = new IpcEventMask(l);
			assertEquals(l, mask.getMask());
		}
	}

	/**
	 * Test case for {@link IpcEventMask#clone()}.
	 */
	public void testClone()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 1000; loop++) {
			long l = rand.nextLong();
			IpcEventMask mask = new IpcEventMask(l);
			assertEquals(l, mask.getMask());

			IpcEventMask c = (IpcEventMask)mask.clone();
			assertTrue(c != mask);
			assertEquals(l, c.getMask());
		}
	}

	/**
	 * Test case for {@link IpcEventMask#equals(Object)} and
	 * {@link IpcEventMask#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcEventMask> set = new HashSet<IpcEventMask>();
		HashSet<Integer> iset = new HashSet<Integer>();
		HashSet<Long> lset = new HashSet<Long>();
		Random rand = new Random();

		IpcEventMask mask = new IpcEventMask();
		assertTrue(mask.equals(mask));
		assertTrue(mask.equals(new IpcEventMask()));
		assertTrue(mask.equals(mask.clone()));
		assertFalse(mask.equals(null));
		assertFalse(mask.equals(new Object()));
		assertTrue(set.add(mask));
		assertTrue(lset.add(IpcEventMask.MASK_FILLED));
		assertFalse(set.add(mask));

		mask = new IpcEventMask();
		mask.clear();
		assertTrue(mask.equals(mask));
		assertFalse(mask.equals(new IpcEventMask()));
		assertTrue(mask.equals(mask.clone()));
		assertFalse(mask.equals(null));
		assertFalse(mask.equals(new Object()));
		assertTrue(set.add(mask));
		assertTrue(lset.add(IpcEventMask.MASK_EMPTY));
		assertFalse(set.add(mask));

		for (int loop = 0; loop < 100000; loop++) {
			int i = rand.nextInt(IpcEvent.MAX_TYPE + 1);
			mask = new IpcEventMask(i);
			assertTrue(mask.equals(mask));
			assertTrue(mask.equals(new IpcEventMask(i)));
			assertTrue(mask.equals(mask.clone()));
			assertFalse(mask.equals(null));
			assertFalse(mask.equals(new Object()));
			assertFalse(mask.equals(new Integer(i)));

			if (iset.add(i)) {
				assertTrue(set.add(mask));
			}
			else {
				assertFalse(set.add(mask));
			}
		}

		for (Iterator<Integer> it = iset.iterator(); it.hasNext();) {
			mask = new IpcEventMask(it.next().intValue());
			assertFalse(set.add(mask));
		}

		for (int loop = 0; loop < 100000; loop++) {
			long l = rand.nextLong();
			mask = new IpcEventMask(l);
			assertTrue(mask.equals(mask));
			assertTrue(mask.equals(new IpcEventMask(l)));
			assertTrue(mask.equals(mask.clone()));
			assertFalse(mask.equals(null));
			assertFalse(mask.equals(new Object()));
			assertFalse(mask.equals(new Long(l)));

			if (lset.add(l)) {
				assertTrue(set.add(mask));
			}
			else {
				assertFalse(set.add(mask));
			}
		}

		for (Iterator<Long> it = lset.iterator(); it.hasNext();) {
			mask = new IpcEventMask(it.next().longValue());
			assertFalse(set.add(mask));
		}
	}

	/**
	 * Test case for {@link IpcEventMask#add(int)}.
	 */
	public void testAdd()
	{
		final IpcEventMask mask =
			new IpcEventMask(IpcEventMask.MASK_EMPTY);
		long value = IpcEventMask.MASK_EMPTY;

		for (int type = IpcEvent.MIN_TYPE; type <= IpcEvent.MAX_TYPE;
		     type++) {
			assertEquals(value, mask.getMask());
			assertFalse(mask.contains(type));
			mask.add(type);
			value |= (1L << type);
			assertEquals(value, mask.getMask());
			assertTrue(mask.contains(type));
			mask.add(type);
			assertEquals(value, mask.getMask());
			assertTrue(mask.contains(type));
		}

		mask.clear();
		assertEquals(IpcEventMask.MASK_EMPTY, mask.getMask());

		checkIllegalType(new IllegalTypeTest() {
			public void run(int type)
			{
				mask.add(type);
			}
		});

		assertEquals(IpcEventMask.MASK_EMPTY, mask.getMask());
	}

	/**
	 * Test case for {@link IpcEventMask#set(Integer[])}.
	 */
	public void testSet()
	{
		final IpcEventMask mask = new IpcEventMask();
		mask.clear();

		// Argument array is null.
		try {
			mask.set((Integer[])null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class);
		}
		assertTrue(mask.isEmpty());

		// Argument array contains null and a valid number.
		try {
			mask.set(0, null, 1);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class);
		}
		assertTrue(mask.isEmpty());

		// Argument array contains an invalid object.
		checkIllegalType(new IllegalTypeTest() {
			public void run(int type)
			{
				mask.set(1, 2, 3, type, 5);
			}
		});
		assertTrue(mask.isEmpty());

		// Argument array contains null and an invalid object.
		try {
			mask.set(null, IpcEvent.MIN_TYPE - 100, 0);
			needException();
		}
		catch (Exception e) {
			LinkedList<Class<?>> clses = new LinkedList<Class<?>>();
			clses.add(NullPointerException.class);
			clses.add(IllegalArgumentException.class);
			checkException(e, clses);
		}

		// No argument.
		mask.fill();
		assertTrue(mask.isFilled());
		mask.set();
		assertTrue(mask.isEmpty());

		// One argument.
		for (int type = IpcEvent.MIN_TYPE; type <= IpcEvent.MAX_TYPE;
		     type++) {
			mask.fill();
			assertTrue(mask.isFilled());
			mask.set(type);
			long value = (1L << type);
			assertEquals(value, mask.getMask());
			assertTrue(mask.contains(type));
			mask.set(type);
			assertEquals(value, mask.getMask());
			assertTrue(mask.contains(type));
		}

		// Multiple arguments.
		Random rand = new Random();
		for (int loop = 0; loop < 100; loop++) {
			long value = 0;
			mask.fill();
			assertTrue(mask.isFilled());

			int nargs = rand.nextInt(5) + 2;
			if (nargs == 2) {
				int arg1 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg1);
				int arg2 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg2);

				mask.set(arg1, arg2);
				assertTrue(mask.contains(arg1));
				assertTrue(mask.contains(arg2));
			}
			else if (nargs == 3) {
				int arg1 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg1);
				int arg2 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg2);
				int arg3 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg3);

				mask.set(arg1, arg2, arg3);
				assertTrue(mask.contains(arg1));
				assertTrue(mask.contains(arg2));
				assertTrue(mask.contains(arg3));
			}
			else if (nargs == 4) {
				int arg1 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg1);
				int arg2 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg2);
				int arg3 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg3);
				int arg4 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg4);

				mask.set(arg1, arg2, arg3, arg4);
				assertTrue(mask.contains(arg1));
				assertTrue(mask.contains(arg2));
				assertTrue(mask.contains(arg3));
				assertTrue(mask.contains(arg4));
			}
			else if (nargs == 5) {
				int arg1 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg1);
				int arg2 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg2);
				int arg3 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg3);
				int arg4 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg4);
				int arg5 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg5);

				mask.set(arg1, arg2, arg3, arg4, arg5);
				assertTrue(mask.contains(arg1));
				assertTrue(mask.contains(arg2));
				assertTrue(mask.contains(arg3));
				assertTrue(mask.contains(arg4));
				assertTrue(mask.contains(arg5));
			}
			else {
				assertEquals(6, nargs);
				int arg1 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg1);
				int arg2 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg2);
				int arg3 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg3);
				int arg4 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg4);
				int arg5 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg5);
				int arg6 = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << arg6);

				mask.set(arg1, arg2, arg3, arg4, arg5, arg6);
				assertTrue(mask.contains(arg1));
				assertTrue(mask.contains(arg2));
				assertTrue(mask.contains(arg3));
				assertTrue(mask.contains(arg4));
				assertTrue(mask.contains(arg5));
				assertTrue(mask.contains(arg6));
			}

			assertEquals(value, mask.getMask());
		}

		// Specify event types as array.
		for (int loop = 0; loop < 100; loop++) {
			long value = 0;
			mask.fill();
			assertTrue(mask.isFilled());

			int nargs = rand.nextInt(IpcEvent.MAX_TYPE + 1) + 1;
			Integer[] args = new Integer[nargs];
			for (int i = 0; i < args.length; i++) {
				args[i] = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << args[i].intValue());
			}

			mask.set(args);
			for (Integer type: args) {
				assertTrue(mask.contains(type.intValue()));
			}

			assertEquals(value, mask.getMask());
		}

		// Specify duplicate event types as array.
		for (int loop = 0; loop < 100; loop++) {
			long value = 0;
			mask.fill();
			assertTrue(mask.isFilled());

			int nargs = rand.nextInt(10000 + 1);
			Integer[] args = new Integer[nargs];
			for (int i = 0; i < args.length; i++) {
				args[i] = rand.nextInt(IpcEvent.MAX_TYPE + 1);
				value |= (1L << args[i].intValue());
			}

			mask.set(args);
			for (Integer type: args) {
				assertTrue(mask.contains(type.intValue()));
			}

			assertEquals(value, mask.getMask());
		}
	}

	/**
	 * Test case for {@link IpcEventMask#remove(int)}.
	 */
	public void testRemove()
	{
		final IpcEventMask mask = new IpcEventMask();
		long value = IpcEventMask.MASK_FILLED;

		for (int type = IpcEvent.MIN_TYPE; type <= IpcEvent.MAX_TYPE;
		     type++) {
			assertEquals(value, mask.getMask());
			assertTrue(mask.contains(type));
			mask.remove(type);
			value &= ~(1L << type);
			assertEquals(value, mask.getMask());
			assertFalse(mask.contains(type));
			mask.remove(type);
			assertEquals(value, mask.getMask());
			assertFalse(mask.contains(type));
		}

		mask.fill();
		assertEquals(IpcEventMask.MASK_FILLED, mask.getMask());

		checkIllegalType(new IllegalTypeTest() {
			public void run(int type)
			{
				mask.remove(type);
			}
		});

		assertEquals(IpcEventMask.MASK_FILLED, mask.getMask());
	}

	/**
	 * Test case for {@link IpcEventMask#contains(int)}.
	 */
	public void testContains()
	{
		final IpcEventMask mask = new IpcEventMask(IpcEventMask.MASK_EMPTY);
		long value = IpcEventMask.MASK_EMPTY;
		Random rand = new Random();

		for (int loop = 0; loop < 100000; loop++) {
			int type = rand.nextInt(IpcEvent.MAX_TYPE + 1);
			long m = 1L << type;
			boolean contains = ((value & m) != 0);
			assertEquals(contains, mask.contains(type));

			if (rand.nextBoolean()) {
				mask.add(type);
				assertTrue(mask.contains(type));
				value |= m;
			}
			else {
				mask.remove(type);
				assertFalse(mask.contains(type));
				value &= ~m;
			}
		}

		mask.fill();
		assertEquals(IpcEventMask.MASK_FILLED, mask.getMask());

		for (int type = -100; type < IpcEvent.MIN_TYPE; type++) {
			try {
				mask.contains(type);
				needException("type=" + type);
			}
			catch (Exception e) {
				checkException(e, IllegalArgumentException.class);
			}
		}

		checkIllegalType(new IllegalTypeTest() {
			public void run(int type)
			{
				mask.contains(type);
			}
		});
	}

	/**
	 * Test case for {@link IpcEventMask#fill()} and
	 * {@link IpcEventMask#isFilled()}.
	 */
	public void testFill()
	{
		for (int type = IpcEvent.MIN_TYPE; type <= IpcEvent.MAX_TYPE;
		     type++) {
			IpcEventMask mask = new IpcEventMask(type);

			assertFalse(mask.isFilled());
			assertEquals(1L << type, mask.getMask());

			mask.fill();
			assertTrue(mask.isFilled());
			assertEquals(IpcEventMask.MASK_FILLED, mask.getMask());
		}
	}

	/**
	 * Test case for {@link IpcEventMask#clear()} and
	 * {@link IpcEventMask#isEmpty()}.
	 */
	public void testClear()
	{
		for (int type = IpcEvent.MIN_TYPE; type <= IpcEvent.MAX_TYPE;
		     type++) {
			IpcEventMask mask = new IpcEventMask(type);

			assertFalse(mask.isEmpty());
			assertEquals(1L << type, mask.getMask());

			mask.clear();
			assertTrue(mask.isEmpty());
			assertEquals(IpcEventMask.MASK_EMPTY, mask.getMask());
		}
	}

	/**
	 * Verify the specified task throws an exception which indicates
	 * an invalid event type is specified.
	 *
	 * @param test	A {@link IllegalTypeTest} to be tested.
	 */
	private void checkIllegalType(IllegalTypeTest test)
	{
		// Negative event types.
		for (int type = -100; type < IpcEvent.MIN_TYPE; type++) {
			try {
				test.run(type);
				needException("type=" + type);
			}
			catch (Exception e) {
				checkIllegalType(e, type);
			}
		}

		// Too large event types.
		for (int type = IpcEvent.MAX_TYPE + 1; type <= 1000; type++) {
			try {
				test.run(type);
				needException("type=" + type);
			}
			catch (Exception e) {
				checkIllegalType(e, type);
			}
		}
	}

	/**
	 * Verify an exception which indicates that an invalid event type is
	 * specified.
	 *
	 * @param e	An exception to be tested.
	 * @param type	An IPC event type.
	 */
	private void checkIllegalType(Exception e, int type)
	{
		checkException(e, IllegalArgumentException.class,
			       "Invalid IPC event type: " + type);
	}

	/**
	 * Interface to ensure that specifying an illegal event type throws
	 * an exception.
	 */
	private interface IllegalTypeTest
	{
		/**
		 * Run a test that throws an {@code IllegalArgumentException}.
		 *
		 * @param type	An illegal event type.
		 */
		public void run(int type);
	}
}
