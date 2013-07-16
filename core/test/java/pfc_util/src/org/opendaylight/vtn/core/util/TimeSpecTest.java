/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.util.Arrays;
import java.util.Random;
import java.util.HashSet;
import java.util.Date;

/**
 * <p>
 *   Unit test class for {@link TimeSpec}.
 * </p>
 */
public class TimeSpecTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link TimeSpec}.
	 *
	 * @param name	The test name.
	 */
	public TimeSpecTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for basic routines.
	 */
	public void testTimeSpec()
	{
		// Default constructor.
		TimeSpec ts = new TimeSpec();
		assertEquals(0L, ts.getSeconds());
		assertEquals(0L, ts.getNanoSeconds());
		assertEquals("TimeSpec: 0.0", ts.toString());

		Object obj = ts.clone();
		assertNotSame(obj, ts);
		assertTrue(ts.equals(obj));
		assertEquals(obj.hashCode(), ts.hashCode());
		assertEquals("TimeSpec: 0.0", obj.toString());

		// Specify a pair of seconds and nanoseconds.
		long seconds[] = {
			0L,
			1L,
			10L,
			100L,
			1000L,
			0x80000000L,
			0x800000000L,
			Long.MAX_VALUE,
		};

		for (int i = 0; i < seconds.length; i++) {
			long sec = seconds[i];

			for (long nsec = 0; nsec < TimeSpec.NANO;
			     nsec += 1234567L) {
				ts = new TimeSpec(sec, nsec);
				assertEquals(sec, ts.getSeconds());
				assertEquals(nsec, ts.getNanoSeconds());
				String str = "TimeSpec: " + sec + "." + nsec;
				assertEquals(str, ts.toString());

				obj = ts.clone();
				assertNotSame(obj, ts);
				assertTrue(ts.equals(obj));
				assertEquals(obj.hashCode(), ts.hashCode());
				assertEquals(str, obj.toString());
			}
		}

		// The greatest TimeSpec value.
		ts = new TimeSpec(Long.MAX_VALUE, TimeSpec.NANO - 1L);
		assertEquals(Long.MAX_VALUE, ts.getSeconds());
		assertEquals(TimeSpec.NANO - 1L, ts.getNanoSeconds());
	}

	/**
	 * Test case which ensures that the constructor throws an exception
	 * on error.
	 */
	public void testCtorError()
	{
		TimeSpec ts = null;

		// The number of seconds is negative.
		long[] negatives = {
			-1L,
			-2L,
			-1000L,
			-100000L,
			Long.MIN_VALUE,
		};

		for (int i = 0; i < negatives.length; i++) {
			long sec = negatives[i];

			try {
				ts = new TimeSpec(sec, 0);
				needException();
			}
			catch (IllegalArgumentException e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Negative seconds: " + sec);
			}
		}

		// The number of nanoseconds is negative.
		for (int i = 0; i < negatives.length; i++) {
			long nsec = negatives[i];

			try {
				ts = new TimeSpec(0, nsec);
				needException();
			}
			catch (IllegalArgumentException e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Negative nanoseconds: " + nsec);
			}
		}

		// Too large the number of nanoseconds.
		long[] nanos = {
			TimeSpec.NANO,
			TimeSpec.NANO + 1L,
			TimeSpec.NANO + 1000L,
			TimeSpec.NANO * 3L,
			Long.MAX_VALUE - 1L,
			Long.MAX_VALUE,
		};

		for (int i = 0; i < nanos.length; i++) {
			long nsec = nanos[i];

			try {
				ts = new TimeSpec(0, nsec);
				needException();
			}
			catch (IllegalArgumentException e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Too large nanoseconds: " + nsec);
			}
		}
	}

	/**
	 * Test case for {@link TimeSpec#TimeSpec(long)},
	 * {@link TimeSpec#toMillis()}, and {@link TimeSpec#toDate()}.
	 */
	public void testMillis()
	{
		long[] millis = {
			0L,
			1L,
			2L,
			10L,
			999L,
			1000L,
			1001L,
			999999L,
			1000000L,
			1000001L,
			999999999L,
			1000000000L,
			1000000001L,
			Long.MAX_VALUE - 2,
			Long.MAX_VALUE - 1,
			Long.MAX_VALUE,
		};

		for (int i = 0; i < millis.length; i++) {
			long msec = millis[i];
			TimeSpec ts = new TimeSpec(msec);

			long sec = msec / 1000L;
			long nsec = (msec % 1000L) * 1000000L;
			assertEquals(sec, ts.getSeconds());
			assertEquals(nsec, ts.getNanoSeconds());
			assertEquals(msec, ts.toMillis());

			Date d = ts.toDate();
			assertEquals(msec, d.getTime());
		}
	}

	/**
	 * Test case for {@link TimeSpec#add(TimeSpec, TimeSpec)}.
	 */
	public void testAdd()
	{
		TimeSpec ts1 = new TimeSpec();
		TimeSpec ts2 = new TimeSpec(10L, 20L);
		TimeSpec ts = TimeSpec.add(ts1, ts2);

		assertEquals(10L, ts.getSeconds());
		assertEquals(20L, ts.getNanoSeconds());

		ts1 = ts;
		ts2 = new TimeSpec(100L, 20000L);
		ts = TimeSpec.add(ts1, ts2);
		assertEquals(110L, ts.getSeconds());
		assertEquals(20020L, ts.getNanoSeconds());

		// Carry from nanoseconds.
		ts1 = new TimeSpec(10000L, TimeSpec.NANO - 1L);
		ts2 = new TimeSpec(20000L, 1L);
		ts = TimeSpec.add(ts1, ts2);
		assertEquals(30001L, ts.getSeconds());
		assertEquals(0L, ts.getNanoSeconds());

		ts1 = new TimeSpec(10000L, TimeSpec.NANO - 1L);
		ts2 = new TimeSpec(20000L, 2);
		ts = TimeSpec.add(ts1, ts2);
		assertEquals(30001L, ts.getSeconds());
		assertEquals(1L, ts.getNanoSeconds());

		ts1 = new TimeSpec(10000L, TimeSpec.NANO - 1L);
		ts2 = new TimeSpec(20000L, TimeSpec.NANO - 1L);
		ts = TimeSpec.add(ts1, ts2);
		assertEquals(30001L, ts.getSeconds());
		assertEquals(TimeSpec.NANO - 2L, ts.getNanoSeconds());

		// Arithmetic overflow.
		ts1 = new TimeSpec(Long.MAX_VALUE, TimeSpec.NANO - 1L);
		ts2 = new TimeSpec(0L, 1L);
		try {
			ts = TimeSpec.add(ts1, ts2);
		}
		catch (IllegalArgumentException e) {
			checkException(e, IllegalArgumentException.class,
				       "Negative seconds: " + Long.MIN_VALUE);
		}

		// NULL dereference.
		try {
			ts = TimeSpec.add(null, ts2);
		}
		catch (NullPointerException e) {
			checkException(e, NullPointerException.class, null);
		}
		try {
			ts = TimeSpec.add(ts1, null);
		}
		catch (NullPointerException e) {
			checkException(e, NullPointerException.class, null);
		}
	}

	/**
	 * Test case for {@link TimeSpec#subtract(TimeSpec, TimeSpec)}.
	 */
	public void testSubtract()
	{
		TimeSpec ts1 = new TimeSpec(10L, 20L);
		TimeSpec ts2 = new TimeSpec();
		TimeSpec ts = TimeSpec.subtract(ts1, ts2);

		assertEquals(10L, ts.getSeconds());
		assertEquals(20L, ts.getNanoSeconds());

		ts2 = new TimeSpec(1L, 1L);
		ts = TimeSpec.subtract(ts1, ts2);
		assertEquals(9L, ts.getSeconds());
		assertEquals(19L, ts.getNanoSeconds());

		// Borrow from seconds.
		ts1 = ts;
		ts2 = new TimeSpec(0L, TimeSpec.NANO - 1);
		ts = TimeSpec.subtract(ts1, ts2);
		assertEquals(8L, ts.getSeconds());
		assertEquals(20L, ts.getNanoSeconds());

		// Specify the same objects.
		ts = TimeSpec.subtract(ts, ts);
		assertEquals(0L, ts.getSeconds());
		assertEquals(0L, ts.getNanoSeconds());

		// A subtrahend is greater than a minuend.
		ts1 = new TimeSpec(5L, 1L);
		ts2 = new TimeSpec(6L, 0L);
		try {
			ts = TimeSpec.subtract(ts1, ts2);
			needException();
		}
		catch (IllegalArgumentException e) {
			checkException(e, IllegalArgumentException.class,
				       "Negative seconds: -1");
		}

		ts1 = new TimeSpec(5L, 1L);
		ts2 = new TimeSpec(5L, 2L);
		try {
			ts = TimeSpec.subtract(ts1, ts2);
			needException();
		}
		catch (IllegalArgumentException e) {
			checkException(e, IllegalArgumentException.class,
				       "Negative seconds: -1");
		}

		// NULL dereference.
		try {
			ts = TimeSpec.subtract(null, ts2);
		}
		catch (NullPointerException e) {
			checkException(e, NullPointerException.class, null);
		}
		try {
			ts = TimeSpec.subtract(ts1, null);
		}
		catch (NullPointerException e) {
			checkException(e, NullPointerException.class, null);
		}
	}

	/**
	 * Test case for {@link TimeSpec#compareTo(TimeSpec)}.
	 */
	public void testCompareTo()
	{
		final int nelems = 1000;
		TimeSpec[] array = createTimes(nelems);

		// Sort TimeSpec array.
		Arrays.sort(array);

		long psec = array[0].getSeconds();
		long pnsec = array[0].getNanoSeconds();

		for (int i = 1; i < nelems; i++) {
			TimeSpec ts = array[i];
			long sec = ts.getSeconds();
			long nsec = ts.getNanoSeconds();

			if (sec == psec) {
				assertTrue(pnsec <= nsec);
			}
			else {
				assertTrue(psec <= sec);
			}

			psec = sec;
			pnsec = nsec;
		}
	}

	/**
	 * Test case for {@link TimeSpec#equals(Object)} and
	 * {@link TimeSpec#hashCode()}.
	 */
	public void testEquals()
	{
		final int nelems = 1000;
		TimeSpec[] array = createTimes(nelems);
		HashSet<TimeSpec> set = new HashSet<TimeSpec>();

		// Sort TimeSpec array.
		Arrays.sort(array);

		TimeSpec prev = array[0];
		assertTrue(set.add(prev));

		for (int i = 1; i < nelems; i++) {
			TimeSpec ts = array[i];
			long sec = ts.getSeconds();
			long nsec = ts.getNanoSeconds();

			TimeSpec clone = (TimeSpec)ts.clone();
			assertNotSame(clone, ts);

			if (set.contains(clone)) {
				assertFalse(set.add(ts));
				assertEquals(prev, ts);
			}
			else {
				assertTrue(set.add(ts));
				assertFalse(ts.equals(prev));
			}
			assertFalse(set.add(clone));

			prev = ts;
		}
	}

	/**
	 * Create {@link TimeSpec} array which keeps random time value..
	 *
	 * @param nelems	The number of array elements.
	 * @return		Array of time values.
	 */
	private TimeSpec[] createTimes(int nelems)
	{
		Random  rand = new Random();
		TimeSpec[] array = new TimeSpec[nelems];
		TimeSpec prev = null;

		for (int i = 0; i < nelems; i++) {
			TimeSpec  ts;

			if (prev != null && rand.nextInt(4) == 0) {
				ts = (TimeSpec)prev.clone();
				array[i] = ts;
				continue;
			}

			long sec = rand.nextLong();
			if (sec < 0) {
				sec &= Long.MAX_VALUE;
			}

			long nsec = rand.nextLong();
			if (nsec < 0) {
				nsec &= Long.MAX_VALUE;
			}
			nsec %= TimeSpec.NANO;

			prev = new TimeSpec(sec, nsec);
			array[i] = prev;
		}

		return array;
	}
}
