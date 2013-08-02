/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.util.Date;

/**
 * <p>
 *   The {@code TimeSpec} class keeps an immutable time value represented
 *   by a pair of seconds and nanoseconds.
 * </p>
 *
 * @since	C10
 */
public final class TimeSpec implements Cloneable, Comparable<TimeSpec>
{
	/**
	 * The number of milliseconds per a second.
	 */
	public final static long  MILLI = 1000L;

	/**
	 * The number of microseconds per a second.
	 */
	public final static long  MICRO = 1000000L;

	/**
	 * The number of nanoseconds per a second.
	 */
	public final static long  NANO = 1000000000L;

	/**
	 * The number of seconds.
	 */
	private final long  _seconds;

	/**
	 * The number of nanoseconds.
	 */
	private final long  _nanoSeconds;

	/**
	 * Construct a {@code TimeSpec} instance which represents the sum of
	 * the specified two time values.
	 *
	 * @param augend	An augend.
	 * @param addend	An addend.
	 * @return	The sum of {@code augend} and {@code addend}.
	 * @throws NullPointerException
	 *	{@code augend} or {@code addend} is null.
	 * @throws IllegalArgumentException
	 *	The sum of the specified values is too large.
	 */
	public static TimeSpec add(TimeSpec augend, TimeSpec addend)
	{
		long sec = augend._seconds + addend._seconds;
		long nsec = augend._nanoSeconds + addend._nanoSeconds;
		long div = nsec / NANO;
		sec += div;
		nsec -= (div * NANO);

		return new TimeSpec(sec, nsec);
	}

	/**
	 * Construct a {@code TimeSpec} instance which represents the
	 * difference of the specified two time values.
	 * Note that {@code minuend} must be greater than or equal to
	 * {@code subtrahend}.
	 *
	 * @param minuend	A minuend.
	 * @param subtrahend	A subtrahend.
	 * @return	The difference of {@code minuend} and
	 *		{@code subtrahend}.
	 * @throws NullPointerException
	 *	{@code augend} or {@code addend} is null.
	 * @throws IllegalArgumentException
	 *	{@code subtrahend} is greater than {@code minuend}.
	 */
	public static TimeSpec subtract(TimeSpec minuend, TimeSpec subtrahend)
	{
		long sec = minuend._seconds - subtrahend._seconds;
		long minnsec = minuend._nanoSeconds;
		long subnsec = subtrahend._nanoSeconds;
		long nsec = minnsec - subnsec;

		if (minnsec < subnsec) {
			nsec += NANO;
			sec--;
		}

		return new TimeSpec(sec, nsec);
	}

	/**
	 * Construct a time value of zero.
	 */
	public TimeSpec()
	{
		_seconds = 0;
		_nanoSeconds = 0;
	}

	/**
	 * Construct a time value specified by a pair of seconds and
	 * nanoseconds.
	 *
	 * @param sec		The number of seconds.
	 * @param nsec		The number of nanoseconds.
	 * @throws IllegalArgumentException
	 *	{@code sec} or {@code nsec} is a negative value.
	 * @throws IllegalArgumentException
	 *	{@code nsec} is greater than or equal to {@link #NANO}.
	 */
	public TimeSpec(long sec, long nsec)
	{
		if (sec < 0) {
			throw new IllegalArgumentException
				("Negative seconds: " + sec);
		}
		if (nsec < 0) {
			throw new IllegalArgumentException
				("Negative nanoseconds: " + nsec);
		}
		if (nsec >= NANO) {
			throw new IllegalArgumentException
				("Too large nanoseconds: " + nsec);
		}

		_seconds = sec;
		_nanoSeconds = nsec;
	}

	/**
	 * Construct a time value represented by the number of milliseconds.
	 *
	 * @param msec	The number of milliseconds.
	 * @throws IllegalArgumentException
	 *	{@code msec} is negative.
	 * @throws IllegalArgumentException
	 *	{@code msec} is too large.
	 */
	public TimeSpec(long msec)
	{
		if (msec < 0) {
			throw new IllegalArgumentException
				("Negative milliseconds: " + msec);
		}

		long sec = msec / MILLI;
		if (sec < 0) {
			throw new IllegalArgumentException
				("Too large msec: " + msec);
		}

		long mod = msec - (sec * MILLI);

		_seconds = sec;
		_nanoSeconds = mod * (NANO / MILLI);
	}

	/**
	 * Return the number of seconds kept by this object.
	 *
	 * @return	The number of seconds kept by this object.
	 */
	public long getSeconds()
	{
		return _seconds;
	}

	/**
	 * Return the number of nanoseconds kept by this object.
	 *
	 * @return	The number of nanoseconds kept by this object.
	 */
	public long getNanoSeconds()
	{
		return _nanoSeconds;
	}

	/**
	 * <p>
	 *   Convert a time value kept by this object into the number of
	 *   milliseconds.
	 * </p>
	 * <p>
	 *   Note that this method causes arithmetic overflow if the number
	 *   of seconds kept by this object is greater than
	 *   {@code 0x20c49ba5e353f7L}.
	 * </p>
	 */
	public long toMillis()
	{
		long divisor = NANO / MILLI;
		long msec = (_seconds * MILLI) +
			((_nanoSeconds + (divisor >>> 1)) / divisor);

		return msec;
	}

	/**
	 * <p>
	 *   Convert this instance into a {@code Date} instance.
	 * </p>
	 * <p>
	 *   Note that {@code Date} keeps a time with milliseconds precision.
	 *   So the conversion to {@code Date} instance always downgrade
	 *   precision to milliseconds.
	 * </p>
	 * <p>
	 *   If this instance does not contains a realtime clock,
	 *   {@code Date} methods on a returned value may result in undefined
	 *   behavior.
	 * </p>
	 *
	 * @return	A converted {@code Date} instance.
	 */
	public Date toDate()
	{
		return new Date(toMillis());
	}

	/**
	 * Return a clone of this object.
	 *
	 * @return	A clone of this object.
	 */
	@Override
	public Object clone()
	{
		try {
			TimeSpec ts = (TimeSpec)super.clone();

			return ts;
		}
		catch (CloneNotSupportedException e) {
			// This should never happen.
			throw new InternalError("Unable to clone TimeSpec.");
		}
	}

	/**
	 * Compare this object to the specified {@code TimeSpec} object.
	 *
	 * @param ts	A {@code TimeSpec} object to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if this time value is less than, equal to,
	 *		or greater than the specified time value respectively.
	 */
	@Override
	public int compareTo(TimeSpec ts)
	{
		long tsec = ts._seconds;

		if (_seconds < tsec) {
			return -1;
		}
		if (_seconds > tsec) {
			return 1;
		}

		long tnsec = ts._nanoSeconds;
		if (_nanoSeconds == tnsec) {
			return 0;
		}

		return (_nanoSeconds < tnsec) ? -1 : 1;
	}

	/**
	 * Determine whether the specified object is identical to this time
	 * value.
	 *
	 * @param obj	An object to be compared.
	 * @return	True is returned if the specified object is identical
	 *		to this time value.
	 *		Otherwise false is returned.
	 */
	@Override
	public boolean equals(Object obj)
	{
		if (obj instanceof TimeSpec) {
			TimeSpec ts = (TimeSpec)obj;

			return (_seconds == ts._seconds &&
				_nanoSeconds == ts._nanoSeconds);
		}

		return false;
	}

	/**
	 * Return a hash code for this time value.
	 *
	 * @return	A hash code for this time value.
	 */
	@Override
	public int hashCode()
	{
		long	s = _seconds ^ (_seconds >>> 32);
		long	n = _nanoSeconds ^ (_nanoSeconds >>> 32);

		return (int)(s ^ n);
	}

	/**
	 * Return a string representation of this time value.
	 *
	 * @return	A string representation of this time value.
	 */
	@Override
	public String toString()
	{
		return "TimeSpec: " + _seconds + "." + _nanoSeconds;
	}
}
