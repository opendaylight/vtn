/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.net.InetAddress;
import java.net.UnknownHostException;

/**
 * <p>
 *   The {@code IpcStruct} is an IPC additional data unit class which
 *   represents an IPC structure.
 * </p>
 * <p>
 *   The name of structure field is used as a key to access a value in the
 *   IPC structure. But the name of inner structure field nested in the
 *   structure can not be used as a key.
 *   To access inner structure fields, another {@code IpcStruct} instance
 *   which maps inner structure must be obtained by {@link #getInner(String)}
 *   or {@link #getInner(String, int)}.
 * </p>
 * <dl>
 *   <dt>IPC structure definition</dt>
 *   <dd>
 *     <pre>
 *ipc_struct foo
 *{
 *        UINT8   foo_u8;
 *        UINT32  foo_u32;
 *}
 *
 *ipc_struct bar
 *{
 *        foo     bar_foo[5];
 *        INT8    bar_s8;
 *}
 *
 *ipc_struct baz
 *{
 *        bar     baz_bar;
 *        UINT8   baz_u8[4];
 *}</pre>
 *   </dd>
 *   <dt>Examples</dt>
 *   <dd>
 *     <ul>
 *       <li>
 *         Determine the length of array field {@code bar.bar_foo}.
 *         <blockquote><pre>
 *IpcStruct bar = new IpcStruct("bar");
 *IpcStructField field = bar.getField("bar_foo");
 *int length = field.getArrayLength();  // 5 is returned.</pre>
 *         </blockquote>
 *       </li>
 *       <li>
 *         Set 10 to {@code baz.baz_bar.bar_foo[3].foo_u8}.
 *         <blockquote><pre>
 *IpcStruct baz = new IpcStruct("baz");
 *IpcStruct foo = baz.getInner("baz_bar").getInner("bar_foo", 3);
 *foo.set("foo_u8", (byte)10);</pre>
 *         </blockquote>
 *       </li>
 *     </ul>
 *   </dd>
 * </dl>
 * <p>
 *   Note that this class is not synchronized. If multiple threads access
 *   an {@code IpcStruct} instance concurrently, it must be synchronized
 *   externally.
 * </p>
 *
 * @see		IpcStructField
 * @since	C10
 */
public final class IpcStruct extends IpcDataUnit implements Cloneable
{
	/**
	 * The name of this IPC structure.
	 */
	private final String  _name;

	/**
	 * The contents of this IPC structure.
	 */
	private long  _buffer;

	/**
	 * Base offset of the structure contents in {@link #_buffer}.
	 */
	private int  _baseOffset;

	/**
	 * IPC structure information.
	 */
	private long  _info;

	/**
	 * Load IPC structure information.
	 */
	static {
		ClientLibrary.load();
		load();
	}

	/**
	 * Get an {@link IpcStructField} instance which contains information
	 * about the specified IPC structure field.
	 *
	 * @param stname	The name of IPC structure.
	 * @param name		The field name contained in the IPC structure
	 *			specified by {@code stname}.
	 * @return		An {@link IpcStructField} instance which
	 *			contains information about IPC structure
	 *			field.
	 * @throws NullPointerException
	 *	Either {@code stname} or {@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code stname} is not a valid IPC structure name.
	 * @throws IllegalArgumentException
	 *	The IPC structure specified by {@code stname} does not contain
	 *	the field named {@code name}.
	 */
	public static native IpcStructField getField(String stname,
						     String name);

	/**
	 * <p>
	 *   Return a {@link String} array which contains all field names
	 *   defined in the specified structure.
	 * </p>
	 * <p>
	 *   Note that field names in a returned array are sorted by field name
	 *   in dictionary order, not defined order.
	 * </p>
	 *
	 * @param stname	The name of IPC structure.
	 * @return		A {@code String} array which contains all
	 *			field names defined in the specified structure.
	 * @throws NullPointerException
	 *	{@code stname} is null.
	 * @throws IllegalArgumentException
	 *	{@code stname} is not a valid IPC structure name.
	 */
	public static native String[] getFieldNames(String stname);

	/**
	 * Construct an empty IPC structure.
	 * All fields are initialized with zero or NULL.
	 *
	 * @param name	The name of the IPC structure.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	The IPC structure specified by {@code name} is not defined.
	 */
	public IpcStruct(String name)
	{
		_name = name;
		_baseOffset = 0;
		initialize(name);
		assert _buffer != 0;
		assert _info != 0;
	}

	/**
	 * Construct an IPC structure with specifying initialized buffer.
	 *
	 * @param name		The name of IPC structure.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 */
	IpcStruct(String name, long buffer, int base, long info)
	{
		_name = name;
		_baseOffset = base;
		_buffer = buffer;
		_info = info;
	}

	/**
	 * Return the name of this IPC structure.
	 *
	 * @return	The name of this structure.
	 */
	public String getName()
	{
		return _name;
	}

	/**
	 * Return the type identifier associated with this data.
	 *
	 * @return	{@link IpcDataUnit#STRUCT} is always returned.
	 */
	@Override
	public int getType()
	{
		return STRUCT;
	}

	/**
	 * Return a {@code String} which represents this IPC structure.
	 *
	 * @return	A {@code String} which represents this value.
	 */
	@Override
	public String toString()
	{
		return "ipc_struct " + _name;
	}

	/**
	 * <p>
	 *   Return a value at the field specified by the field name.
	 * </p>
	 * <p>
	 *   This method returns an instance of {@link IpcDataUnit} which
	 *   contains a value at the specified field.
	 *   If data type of the specified field is
	 *   {@link IpcDataUnit#STRUCT}, an {@code IpcStruct} instance which
	 *   contains value of the specified inner structure field is returned.
	 *   Note that this method always returns a deep copy of the structure
	 *   field specified by {@code name}. If you want to get shallow copy
	 *   of an inner structure field, {@link #getInner(String)} must be
	 *   used instead.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #get(String, int)} must be used to get an array
	 *   element in this structure.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		An {@link IpcDataUnit} instance which contains
	 *			a value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 */
	public IpcDataUnit get(String name)
	{
		return get(name, -1, _buffer, _baseOffset, _info, true);
	}

	/**
	 * <p>
	 *   Return a value at the array field specified by the field name.
	 * </p>
	 * <p>
	 *   This method returns an instance of {@link IpcDataUnit} which
	 *   contains a value at the specified field.
	 *   If data type of the specified field is
	 *   {@link IpcDataUnit#STRUCT}, an {@code IpcStruct} instance which
	 *   contains value of the specified inner structure field is returned.
	 *   Note that this method always returns a deep copy of the structure
	 *   field specified by {@code name}. If you want to get shallow copy
	 *   of an inner structure field, {@link #getInner(String, int)} must
	 *   be used instead.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @return		An {@link IpcDataUnit} instance which contains
	 *			a value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public IpcDataUnit get(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		return get(name, index, _buffer, _baseOffset, _info, true);
	}

	/**
	 * <p>
	 *   Return an {@code IpcStruct} instance which represents a nested
	 *   structure in this structure.
	 * </p>
	 * <p>
	 *   This method is used to access inner structure field nested in
	 *   this structure. An {@code IpcStruct} instance returned by this
	 *   method shares the entity of IPC structure with this instance.
	 *   So any changes to an {@code IpcStruct} instance returned by
	 *   this method affects this instance. In other words, concurrent
	 *   access to an {@code IpcStruct} instance returned by this method
	 *   and this instance must be synchronized externally.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #getInner(String, int)} must be used to get an
	 *   {@code IpcStruct} instance associated with an array element in
	 *   this structure.
	 * </p>
	 *
	 * @param name		The field name associated with nested structure
	 *			in this structure.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a nested structure field name in this
	 *	structure.
	 */
	public IpcStruct getInner(String name)
	{
		try {
			return (IpcStruct)get(name, -1, _buffer,
					      _baseOffset, _info, false);
		}
		catch (ClassCastException e) {
			throw new IllegalArgumentException
				("Not a nested structure field: " + name);
		}
	}

	/**
	 * <p>
	 *   Return an {@code IpcStruct} instance which represents an element
	 *   of nested structure array in this structure.
	 * </p>
	 * <p>
	 *   This method is used to access inner structure array field nested
	 *   in this structure. An {@code IpcStruct} instance returned by this
	 *   method shares the entity of IPC structure with this instance.
	 *   So any changes to an {@code IpcStruct} instance returned by
	 *   this method affects this instance. In other words, concurrent
	 *   access to an {@code IpcStruct} instance returned by this method
	 *   and this instance must be synchronized externally.
	 * </p>
	 *
	 * @param name		The field name associated with nested structure
	 *			in this structure.
	 * @param index		The target array index.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a nested structure field name in this
	 *	structure.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public IpcStruct getInner(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		try {
			return (IpcStruct)get(name, index, _buffer,
					      _baseOffset, _info, false);
		}
		catch (ClassCastException e) {
			throw new IllegalArgumentException
				("Not a nested structure field: " + name);
		}
	}

	/**
	 * <p>
	 *   Return a {@code byte} value at the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #getByte(String, int)} must be used to get a byte
	 *   array element in this structure.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code byte} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public byte getByte(String name)
	{
		return getByte(name, -1, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code byte} array element at the field specified by
	 *   the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @return		A {@code byte} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public byte getByte(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		return getByte(name, index, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code short} value at the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT16} or {@link IpcDataUnit#UINT16}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #getShort(String, int)} must be used to get a short
	 *   array element in this structure.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code short} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public short getShort(String name)
	{
		return getShort(name, -1, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code short} array element at the field specified by
	 *   the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT16} or {@link IpcDataUnit#UINT16}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @return		A {@code short} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public short getShort(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		return getShort(name, index, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return an {@code int} value at the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT32} or {@link IpcDataUnit#UINT32}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #getInt(String, int)} must be used to get an int
	 *   array element in this structure.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		An {@code int} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public int getInt(String name)
	{
		return getInt(name, -1, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return an {@code int} array element at the field specified by
	 *   the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT32} or {@link IpcDataUnit#UINT32}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @return		An {@code int} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public int getInt(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		return getInt(name, index, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code long} value at the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT64} or {@link IpcDataUnit#UINT64}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #getLong(String, int)} must be used to get a long
	 *   array element in this structure.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code long} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public long getLong(String name)
	{
		return getLong(name, -1, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code long} array element at the field specified by
	 *   the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT64} or {@link IpcDataUnit#UINT64}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @return		A {@code long} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public long getLong(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		return getLong(name, index, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code float} value at the field specified by the field
	 *   name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#FLOAT}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #getFloat(String, int)} must be used to get a float
	 *   array element in this structure.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code float} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public float getFloat(String name)
	{
		return getFloat(name, -1, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code float} array element at the field specified by
	 *   the field name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#FLOAT}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @return		A {@code float} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public float getFloat(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		return getFloat(name, index, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code double} value at the field specified by the field
	 *   name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#DOUBLE}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #getDouble(String, int)} must be used to get a
	 *   double array element in this structure.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code double} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public double getDouble(String name)
	{
		return getDouble(name, -1, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code double} array element at the field specified by
	 *   the field name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#DOUBLE}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @return		A {@code double} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public double getDouble(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		return getDouble(name, index, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return an {@code InetAddress} instance which represents a value
	 *   at the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#IPV4} or {@link IpcDataUnit#IPV6}
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #getInetAddress(String, int)} must be used to get an
	 *   IP address array element in this structure.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		An {@code InetAddress} instance at the
	 *			specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public InetAddress getInetAddress(String name)
	{
		byte[] raw = getInetAddress(name, -1, _buffer, _baseOffset,
					    _info);

		try {
			return InetAddress.getByAddress(raw);
		}
		catch (UnknownHostException e) {
			// This should never happen.
			throw new IllegalStateException
				("Unable to create an InetAddress: len=" +
				 raw.length, e);
		}
	}

	/**
	 * <p>
	 *   Return an {@code InetAddress} instance which represents an array
	 *   element at the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#IPV4} or {@link IpcDataUnit#IPV6}
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @return		An {@code InetAddress} instance at the
	 *			specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public InetAddress getInetAddress(String name, int index)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		byte[] raw = getInetAddress(name, index, _buffer, _baseOffset,
					    _info);

		try {
			return InetAddress.getByAddress(raw);
		}
		catch (UnknownHostException e) {
			// This should never happen.
			throw new IllegalStateException
				("Unable to create an InetAddress: len=" +
				 raw.length, e);
		}
	}

	/**
	 * <p>
	 *   Return a {@code byte} array which contains all elements in the
	 *   array field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code byte} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public byte[] getByteArray(String name)
	{
		return getByteArray(name, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code short} array which contains all elements in the
	 *   array field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT16} or {@link IpcDataUnit#UINT16}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code short} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public short[] getShortArray(String name)
	{
		return getShortArray(name, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return an {@code int} array which contains all elements in the
	 *   array field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT32} or {@link IpcDataUnit#UINT32}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		An {@code int} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public int[] getIntArray(String name)
	{
		return getIntArray(name, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code long} array which contains all elements in the
	 *   array field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT64} or {@link IpcDataUnit#UINT64}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code long} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public long[] getLongArray(String name)
	{
		return getLongArray(name, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code float} array which contains all elements in the
	 *   array field specified by the field name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#FLOAT}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code float} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public float[] getFloatArray(String name)
	{
		return getFloatArray(name, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return a {@code double} array which contains all elements in the
	 *   array field specified by the field name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#DOUBLE}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code double} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public double[] getDoubleArray(String name)
	{
		return getDoubleArray(name, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Return an {@code InetAddress} array which contains all elements
	 *   in the array field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#IPV4} or {@link IpcDataUnit#IPV6}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		An {@code InetAddress} array which contains
	 *			all elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public InetAddress[] getInetAddressArray(String name)
	{
		int alen = getField(name).getArrayLength();
		if (alen == 0) {
			throw new IllegalArgumentException
				("\"" + name + "\" is not an array field.");
		}

		InetAddress[] addrs = new InetAddress[alen];

		for (int i = 0; i < alen; i++) {
			addrs[i] = getInetAddress(name, i);
		}

		return addrs;
	}

	/**
	 * <p>
	 *   Return a {@code byte} array specified by the field name as a
	 *   {@code String}.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 *   Note that a string in the specified array field must be encoded
	 *   in US-ASCII or UTF-8. Specifying array field which contains
	 *   a byte sequence encoded in unsupported encoding results in
	 *   undefined behavior.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @return		A {@code String} constructed from the
	 *			{@code byte} array elements in the
	 *			specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public String getString(String name)
	{
		return getString(name, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a value in the specified {@link IpcDataUnit} instance to
	 *   the field specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be exactly matched
	 *   to the type of {@link IpcDataUnit} instance. Therefore,
	 *   neither an {@link IpcString} nor {@link IpcBinary} instance
	 *   can be specified to {@code value} because those types can not
	 *   defined as IPC structure field type.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #set(String, int, IpcDataUnit)} must be used to
	 *   set a value into an array element.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, IpcDataUnit value)
	{
		value.setStruct(this, name);
	}

	/**
	 * <p>
	 *   Set a value in the specified {@link IpcDataUnit} instance
	 *   into an array element at the field specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be exactly matched
	 *   to the type of {@link IpcDataUnit} instance. Therefore,
	 *   neither an {@link IpcString} nor {@link IpcBinary} instance
	 *   can be specified to {@code value} because those types can not
	 *   defined as IPC structure field type.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void set(String name, int index, IpcDataUnit value)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		value.setStruct(this, name, index);
	}

	/**
	 * <p>
	 *   Set a {@code byte} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #set(String, int, byte)} must be used to set a
	 *   value into a {@code byte} array element.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, byte value)
	{
		setByte(name, -1, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code byte} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, byte)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setByte(String name, byte value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set a {@code byte} value into a {@code byte} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void set(String name, int index, byte value)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		setByte(name, index, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code byte} value into a {@code byte} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int, byte)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void setByte(String name, int index, byte value)
	{
		set(name, index, value);
	}

	/**
	 * <p>
	 *   Set a {@code short} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT16} or {@link IpcDataUnit#UINT16}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #set(String, int, short)} must be used to set a
	 *   value into a {@code short} array element.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, short value)
	{
		setShort(name, -1, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code short} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT16} or {@link IpcDataUnit#UINT16}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, short)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setShort(String name, short value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set a {@code short} value into a {@code short} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT16} or {@link IpcDataUnit#UINT16}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void set(String name, int index, short value)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		setShort(name, index, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code short} value into a {@code short} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT16} or {@link IpcDataUnit#UINT16}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int, short)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void setShort(String name, int index, short value)
	{
		set(name, index, value);
	}

	/**
	 * <p>
	 *   Set an {@code int} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT32} or {@link IpcDataUnit#UINT32}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #set(String, int, int)} must be used to set a
	 *   value into an {@code int} array element.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, int value)
	{
		setInt(name, -1, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set an {@code int} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT32} or {@link IpcDataUnit#UINT32}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setInt(String name, int value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set an {@code int} value into an {@code int} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT32} or {@link IpcDataUnit#UINT32}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void set(String name, int index, int value)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		setInt(name,index, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set an {@code int} value into an {@code int} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT32} or {@link IpcDataUnit#UINT32}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int, int)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void setInt(String name, int index, int value)
	{
		set(name, index, value);
	}

	/**
	 * <p>
	 *   Set a {@code long} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT64} or {@link IpcDataUnit#UINT64}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #set(String, int, long)} must be used to set a
	 *   value into a {@code long} array element.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, long value)
	{
		setLong(name, -1, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code long} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT64} or {@link IpcDataUnit#UINT64}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, long)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setLong(String name, long value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set a {@code long} value into a {@code long} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT64} or {@link IpcDataUnit#UINT64}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void set(String name, int index, long value)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		setLong(name, index, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code long} value into a {@code long} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT64} or {@link IpcDataUnit#UINT64}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int, long)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void setLong(String name, int index, long value)
	{
		set(name, index, value);
	}

	/**
	 * <p>
	 *   Set a {@code float} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#FLOAT}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #set(String, int, float)} must be used to set a
	 *   value into a {@code float} array element.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, float value)
	{
		setFloat(name, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code float} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#FLOAT}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, float)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setFloat(String name, float value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set a {@code float} value into a {@code float} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#FLOAT}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void set(String name, int index, float value)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		setFloat(name, index, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code float} value into a {@code float} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#FLOAT}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int, float)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void setFloat(String name, int index, float value)
	{
		set(name, index, value);
	}

	/**
	 * <p>
	 *   Set a {@code double} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#DOUBLE}.
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an array
	 *   field. {@link #set(String, int, double)} must be used to
	 *   set a value into a {@code double} array element.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, double value)
	{
		setDouble(name, -1, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code double} value to the field specified by the field
	 *   name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#DOUBLE}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, double)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setDouble(String name, double value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set a {@code double} value into a {@code double} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#DOUBLE}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void set(String name, int index, double value)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		setDouble(name, index, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code double} value into a {@code double} array element at
	 *   the field specified by the field name.
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#DOUBLE}.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int, double)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void setDouble(String name, int index, double value)
	{
		set(name, index, value);
	}

	/**
	 * <p>
	 *   Set an {@code InetAddress} instance into the field specified by
	 *   the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#IPV4} or {@link IpcDataUnit#IPV6}
	 * </p>
	 * <p>
	 *   Note that the field specified by {@code name} must not be an
	 *   array field. {@link #set(String, int, InetAddress)} must be used
	 *   to set a value into an IP address array element.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, InetAddress value)
	{
		setInetAddress(name, -1, value.getAddress(), _buffer,
			       _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set an {@code InetAddress} instance into the field specified by
	 *   the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#IPV4} or {@link IpcDataUnit#IPV6}
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, InetAddress)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setInetAddress(String name, InetAddress value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set an {@code InetAddress} instance into an IP address array
	 *   element at the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#IPV4} or {@link IpcDataUnit#IPV6}
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void set(String name, int index, InetAddress value)
	{
		if (index < 0) {
			throw new ArrayIndexOutOfBoundsException
				("Negative array index: " + index);
		}

		setInetAddress(name, index, value.getAddress(), _buffer,
			       _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set an {@code InetAddress} instance into an IP address array
	 *   element at the field specified by the field name.
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#IPV4} or {@link IpcDataUnit#IPV6}
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int, InetAddress)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param index		The target array index.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	An illegal array index is specified to {@code index}.
	 */
	public void setInetAddress(String name, int index, InetAddress value)
	{
		set(name, index, value);
	}

	/**
	 * <p>
	 *   Set all elements in  a {@code byte} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 *   In addition, the length of {@code value} must be the same
	 *   as the capacity of the specified field.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code byte} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, byte[] value)
	{
		setByteArray(name, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set all elements in  a {@code byte} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, byte[])}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code byte} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setByteArray(String name, byte[] value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set all elements in a {@code short} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT16} or {@link IpcDataUnit#UINT16}.
	 *   In addition, the length of {@code value} must be the same
	 *   as the capacity of the specified field.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code short} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, short[] value)
	{
		setShortArray(name, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set all elements in a {@code short} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, short[])}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code short} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setShortArray(String name, short[] value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set all elements in an {@code int} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT32} or {@link IpcDataUnit#UINT32}.
	 *   In addition, the length of {@code value} must be the same
	 *   as the capacity of the specified field.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		An {@code int} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, int[] value)
	{
		setIntArray(name, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set all elements in an {@code int} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, int[])}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		An {@code int} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setIntArray(String name, int[] value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set all elements in a {@code long} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT64} or {@link IpcDataUnit#UINT64}.
	 *   In addition, the length of {@code value} must be the same
	 *   as the capacity of the specified field.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code long} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, long[] value)
	{
		setLongArray(name, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set all elements in a {@code long} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, long[])}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code long} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setLongArray(String name, long[] value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set all elements in a {@code float} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#FLOAT}.
	 *   In addition, the length of {@code value} must be the same
	 *   as the capacity of the specified field.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code float} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, float[] value)
	{
		setFloatArray(name, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set all elements in a {@code float} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, float[])}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code float} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setFloatArray(String name, float[] value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set all elements in a {@code double} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be
	 *   {@link IpcDataUnit#DOUBLE}.
	 *   In addition, the length of {@code value} must be the same
	 *   as the capacity of the specified field.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code double} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, double[] value)
	{
		setDoubleArray(name, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set all elements in a {@code double} array to the array field
	 *   specified by the field name.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, double[])}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code double} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setDoubleArray(String name, double[] value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set all elements in an {@code InetAddress} array to the array
	 *   field specified by the field name.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#IPV4} or {@link IpcDataUnit#IPV6}.
	 *   In addition, the length of {@code value} must be the same
	 *   as the capacity of the specified field.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		An {@code InetAddress} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws NullPointerException
	 *	{@code value} contains at least one null entry.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void set(String name, InetAddress[] value)
	{
		IpcStructField field = getField(name);

		int alen = field.getArrayLength();
		if (alen == 0) {
			throw new IllegalArgumentException
				("\"" + name + "\" is not an array field.");
		}

		int addrlen;
		int type = field.getType();
		if (type == IPV4) {
			addrlen = 4;
		}
		else if (type == IPV6) {
			addrlen = 16;
		}
		else {
			throw new IllegalArgumentException
				("Data type of the field \"" + name +
				 "\" is neither IPV4 nor IPV6: type=" +
				 IpcDataUnit.getTypeName(type));
		}

		if (alen != value.length) {
			throw new IllegalArgumentException
				("Array length(" + value.length +
				 ") does not match the field " + name +
				 "[" + alen + "].");
		}

		// Constructs byte array which contains all raw addresses.
		byte[] bytes = new byte[addrlen * alen];
		int dstpos = 0;
		for (int i = 0; i < alen; i++) {
			byte[] raw = value[i].getAddress();
			if (raw.length != addrlen) {
				throw new IllegalArgumentException
					("Unexpected address length at " + i +
					 ": length=" + raw.length);
			}

			System.arraycopy(raw, 0, bytes, dstpos, addrlen);
			dstpos += addrlen;
		}

		setInetAddressArray(name, bytes, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set all elements in an {@code InetAddress} array to the array
	 *   field specified by the field name.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, InetAddress[])}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		An {@code InetAddress} array to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws NullPointerException
	 *	{@code value} contains at least one null entry.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	public void setInetAddressArray(String name, InetAddress[] value)
	{
		set(name, value);
	}

	/**
	 * <p>
	 *   Set a {@code String} into {@code byte} array field.
	 * </p>
	 * <p>
	 *   The data type of the specified field must be either
	 *   {@link IpcDataUnit#INT8} or {@link IpcDataUnit#UINT8}.
	 * </p>
	 * <p>
	 *   This method encodes the specified string in UTF-8.
	 *   If the number of UTF-8 characters in {@code value} is less than
	 *   the capacity of the array field, a string terminator
	 *   ({@code '\0'}) is also set to the array field.
	 *   Note that no string terminator is appended if the number of
	 *   UTF-8 characters in {@code value} equals the capacity of the
	 *   array field.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	The number of UTF-8 characters in {@code value} exceeds the
	 *	capacity of the array field.
	 */
	public void set(String name, String value)
	{
		setString(name, value, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Set a {@code String} into {@code byte} array field.
	 * </p>
	 * <p>
	 *   This method is an alias for {@link #set(String, String)}.
	 * </p>
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	The number of UTF-8 characters in {@code value} exceeds the
	 *	capacity of the array field.
	 */
	public void setString(String name, String value)
	{
		set(name, value);
	}

	/**
	 * Return an {@link IpcStructField} instance which contains information
	 * about structure field.
	 *
	 * @param name		The field name in this structure.
	 * @return		An {@code IpcStructField} instance which
	 *			contains information about IPC structure
	 *			field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	This IPC structure does not contain the field named
	 *	{@code name}.
	 */
	public IpcStructField getField(String name)
	{
		return getFieldImpl(_name, name, _info);
	}

	/**
	 * <p>
	 *   Return a {@code String} array which contains all field names
	 *   defined in this structure.
	 * </p>
	 * <p>
	 *   Note that field names in a returned array are sorted by field name
	 *   in dictionary order, not defined order.
	 * </p>
	 *
	 * @return	A {@code String} array which contains all defined
	 *		field names.
	 */
	public String[] getFieldNames()
	{
		return getFieldNamesImpl(_info);
	}

	/**
	 * <p>
	 *   Returns a deep copy of this IPC structure.
	 * </p>
	 * <p>
	 *   An {@code IpcStruct} object returned by this method keeps the
	 *   copy of this IPC structure, but they don't share the entity of
	 *   the IPC structure data.
	 * </p>
	 *
	 * @return	A deep copy of this IPC structure.
	 */
	@Override
	public Object clone()
	{
		IpcStruct struct = null;
		try {
			struct = (IpcStruct)super.clone();
		}
		catch (CloneNotSupportedException e) {
			// This should never happen.
			throw new InternalError("Unable to clone IpcStruct.");
		}

		// Create a deep copy of IPC structure.
		boolean succeeded = false;
		try {
			struct._buffer = deepCopy(_buffer, _baseOffset, _info);
			struct._baseOffset = 0;
			succeeded = true;
		}
		finally {
			if (!succeeded) {
				// Invalidate buffer and structure information
				// in a dead clone.
				struct._buffer = 0;
				struct._info = 0;
			}
		}

		return struct;
	}

	/**
	 * Append this data to the end of the additional data in the
	 * specified IPC client session
	 *
	 * Note that this method must be called by an internal method of
	 * {@link ClientSession}.
	 *
	 * @param session	An IPC client session handle.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on the specified
	 *	session.
	 * @throws IpcShutdownException
	 *	The state of the specified session is <strong>DISCARD</strong>.
	 *	This exception indicates that the specified session is no
	 *	longer available.
	 * @throws IpcBadStateException
	 *	The state of the specified session is <strong>RESULT</strong>.
	 * @throws IpcTooBigDataException
	 *	No more additional data can not be added.
	 */
	@Override
	void addClientOutput(long session) throws IpcException
	{
		addClientOutput(session, _buffer, _baseOffset, _info);
	}

	/**
	 * <p>
	 *   Copy whole contents of this structure to the specified structure
	 *   field.
	 * </p>
	 * <p>
	 *   Data type of the specified field must be
	 *   {@link IpcDataUnit#STRUCT}, and the structure name must be
	 *   matched to the name of this structure.
	 * </p>
	 *
	 * @param target	The target {@link IpcStruct} instance.
	 * @param name		The name of the target field in {@code target}.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a valid field name.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	@Override
	void setStruct(IpcStruct target, String name)
	{
		setStruct(target, name, -1);
	}

	/**
	 * <p>
	 *   Copy whole contents of this structure into an array element at
	 *   the specified structure field.
	 * </p>
	 * <p>
	 *   Data type of the specified field must be
	 *   {@link IpcDataUnit#STRUCT}, and the structure name must be
	 *   matched to the name of this structure.
	 * </p>
	 *
	 * @param target	The target {@link IpcStruct} instance.
	 * @param name		The name of the target field in {@code target}.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a valid field name.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name and {@code index} is
	 *	a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name and {@code index} is
	 *	not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	@Override
	void setStruct(IpcStruct target, String name, int index)
	{
		if (target._buffer == _buffer) {
			// The specified structure shares the buffer with
			// this instance. So there is no need to copy.
			return;
		}

		target.setStructField(name, index, target._buffer,
				      target._baseOffset, target._info,
				      _buffer, _baseOffset, _info);
	}

	/**
	 * Set a {@code byte} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param reqtype	Required data type. Strict data type check is
	 *			omitted if a negative value is specified.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	void setByte(String name, int index, int reqtype, byte value)
	{
		setByte(name, index, reqtype, value, _buffer, _baseOffset,
			_info);
	}

	/**
	 * Set a {@code short} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param reqtype	Required data type. Strict data type check is
	 *			omitted if a negative value is specified.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	void setShort(String name, int index, int reqtype, short value)
	{
		setShort(name, index, reqtype, value, _buffer, _baseOffset,
			 _info);
	}

	/**
	 * Set an {@code int} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param reqtype	Required data type. Strict data type check is
	 *			omitted if a negative value is specified.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	void setInt(String name, int index, int reqtype, int value)
	{
		setInt(name, index, reqtype, value, _buffer, _baseOffset,
		       _info);
	}

	/**
	 * Set a {@code long} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param reqtype	Required data type. Strict data type check is
	 *			omitted if a negative value is specified.
	 * @param value		A value to be set.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	void setLong(String name, int index, int reqtype, long value)
	{
		setLong(name, index, reqtype, value, _buffer, _baseOffset,
			_info);
	}

	/**
	 * Finalize this {@code IpcStruct} instance.
	 *
	 * @throws Throwable
	 *	An error occurs while finalization.
	 */
	@Override
	protected void finalize() throws Throwable
	{
		finalize(_buffer, _info);
	}

	/**
	 * Load information about IPC structures.
	 *
	 * @throws IllegalStateException
	 *	IPC structure information could not be loaded.
	 */
	private static native void load();

	/**
	 * Initialize this instance.
	 * All fields are initialized with zero or NULL.
	 *
	 * @param name	The name of the IPC structure.
	 * @throws IllegalArgumentException
	 *	The IPC structure specified by {@code name} is not defined.
	 */
	private native void initialize(String name);

	/**
	 * Finalize this {@code IpcStruct} instance.
	 *
	 * @param buffer	IPC structure buffer.
	 * @param info		IPC structure information.
	 */
	private native void finalize(long buffer, long info);

	/**
	 * Append this data to the end of the additional data in the
	 * specified IPC client session
	 *
	 * @param session	An IPC client session handle.
	 * @param buffer	IPC structure buffer.
	 * @param offset	Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on the specified
	 *	session.
	 * @throws IpcShutdownException
	 *	The state of the specified session is <strong>DISCARD</strong>.
	 *	This exception indicates that the specified session is no
	 *	longer available.
	 * @throws IpcBadStateException
	 *	The state of the specified session is <strong>RESULT</strong>.
	 * @throws IpcTooBigDataException
	 *	No more additional data can not be added.
	 */
	private native void addClientOutput(long session, long buffer,
					    int offset, long info)
		throws IpcException;

	/**
	 * Return a value at the structure field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @param deep          Return a deep copy if {@code true}.
	 *			Otherwise a shallow copy is returned.
	 * @return		An {@link IpcDataUnit} instance which contains
	 *			a value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native IpcDataUnit get(String name, int index, long buffer,
				       int base, long info, boolean deep);

	/**
	 * Return a {@code byte} value at the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code byte} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native byte getByte(String name, int index, long buffer,
				    int base, long info);

	/**
	 * Return a {@code short} value at the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code short} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native short getShort(String name, int index, long buffer,
				      int base, long info);

	/**
	 * Return an {@code int} value at the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		An {@code int} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native int getInt(String name, int index, long buffer,
				  int base, long info);

	/**
	 * Return a {@code long} value at the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code long} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native long getLong(String name, int index, long buffer,
				    int base, long info);

	/**
	 * Return a {@code float} value at the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code float} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native float getFloat(String name, int index, long buffer,
				      int base, long info);

	/**
	 * Return a {@code double} value at the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code double} value at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native double getDouble(String name, int index, long buffer,
					int base, long info);

	/**
	 * Return a {@code byte} array which contains raw IP address at the
	 * specified field.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code byte} array at the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native byte[] getInetAddress(String name, int index,
					     long buffer, int base, long info);

	/**
	 * Return a {@code byte} array which contains all elements in the
	 * array field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code byte} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native byte[] getByteArray(String name, long buffer, int base,
					   long info);

	/**
	 * Return a {@code short} array which contains all elements in the
	 * array field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code short} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native short[] getShortArray(String name, long buffer,
					     int base, long info);

	/**
	 * Return an {@code int} array which contains all elements in the
	 * array field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		An {@code int} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native int[] getIntArray(String name, long buffer, int base,
					 long info);

	/**
	 * Return a {@code long} array which contains all elements in the
	 * array field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code long} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native long[] getLongArray(String name, long buffer, int base,
					   long info);

	/**
	 * Return a {@code float} array which contains all elements in the
	 * array field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code float} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native float[] getFloatArray(String name, long buffer,
					     int base, long info);

	/**
	 * Return a {@code double} array which contains all elements in the
	 * array field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code double} array which contains all
	 *			elements in the specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native double[] getDoubleArray(String name, long buffer,
					       int base, long info);

	/**
	 * Return a {@code byte} array specified by the field name as a
	 * {@code String}.
	 *
	 * @param name		The field name in this structure.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A {@code String} constructed from the
	 *			{@code byte} array elements in the
	 *			specified field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native String getString(String name, long buffer, int base,
					long info);

	/**
	 * Set a {@code byte} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param reqtype	Required data type. Strict data type check is
	 *			omitted if a negative value is specified.
	 * @param value		A value to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native void setByte(String name, int index, int reqtype,
				    byte value, long buffer, int base,
				    long info);

	/**
	 * Set a {@code short} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param reqtype	Required data type. Strict data type check is
	 *			omitted if a negative value is specified.
	 * @param value		A value to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native void setShort(String name, int index, int reqtype,
				     short value, long buffer, int base,
				     long info);

	/**
	 * Set an {@code int} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param reqtype	Required data type. Strict data type check is
	 *			omitted if a negative value is specified.
	 * @param value		A value to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native void setInt(String name, int index, int reqtype,
				   int value, long buffer, int base,
				   long info);

	/**
	 * Set a {@code long} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param reqtype	Required data type. Strict data type check is
	 *			omitted if a negative value is specified.
	 * @param value		A value to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native void setLong(String name, int index, int reqtype,
				    long value, long buffer, int base,
				    long info);

	/**
	 * Set a {@code float} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param value		A value to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native void setFloat(String name, int index, float value,
				     long buffer, int base, long info);

	/**
	 * Set a {@code double} value to the structure field specified by
	 * the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param value		A value to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native void setDouble(String name, int index, double value,
				      long buffer, int base, long info);

	/**
	 * Set a {@code byte} array which represents raw IP address to the
	 * structure field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param value		A value to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native void setInetAddress(String name, int index,
					   byte[] value, long buffer,
					   int base, long info);

	/**
	 * Copy whole contents of this structure to the specified structure
	 * field.
	 *
	 * @param name		The name of the target field.
	 * @param index		An array index. -1 must be specified if
	 *			{@code name} is not an array field name.
	 * @param dstbuf	Buffer of the target IPC structure.
	 * @param dstbase	Base offset of the target IPC structure.
	 * @param dstinfo	Information of the target IPC structure.
	 * @param srcbuf	Buffer of the source IPC structure.
	 * @param srcbase	Base offset of the source IPC structure.
	 * @param srcinfo	Information of the source IPC structure.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is a negative integer.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name in this structure and
	 *	{@code index} is not a negative integer.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code name} is an array field name in this structure and
	 *	{@code index} is greater than or equal to its length.
	 */
	private native void setStructField(String name, int index,
					   long dstbuf, int dstbase,
					   long dstinfo, long srcbuf,
					   int srcbase, long srcinfo);

	/**
	 * Set all elements in a {@code byte} array to the array field
	 * specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code byte} array to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native void setByteArray(String name, byte[] value,
					 long buffer, int base, long info);

	/**
	 * Set all elements in a {@code short} array to the array field
	 * specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code short} array to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native void setShortArray(String name, short[] value,
					  long buffer, int base, long info);

	/**
	 * Set all elements in an {@code int} array to the array field
	 * specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param value		An {@code int} array to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native void setIntArray(String name, int[] value,
					long buffer, int base, long info);

	/**
	 * Set all elements in a {@code long} array to the array field
	 * specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code long} array to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native void setLongArray(String name, long[] value,
					 long buffer, int base, long info);

	/**
	 * Set all elements in a {@code float} array to the array field
	 * specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code float} array to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native void setFloatArray(String name, float[] value,
					  long buffer, int base, long info);

	/**
	 * Set all elements in a {@code double} array to the array field
	 * specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code double} array to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	The length of {@code value} does not match the capacity of
	 *	the specified field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 */
	private native void setDoubleArray(String name, double[] value,
					   long buffer, int base, long info);

	/**
	 * Set all elements in a {@code byte} array to the IPV4 or IPV6 array
	 * field specified by the field name.
	 *
	 * @param name		The field name in this structure.
	 * @param value		A {@code byte} array to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 */
	private native void setInetAddressArray(String name, byte[] value,
						long buffer, int base,
						long info);

	/**
	 * Set a {@code String} into {@code byte} array field.
	 *
	 * @param name		The field name in this structure.
	 * @param value		A value to be set.
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @throws NullPointerException
	 *	Either {@code name} or {@code value} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name in this structure.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	The number of UTF-8 characters in {@code value} exceeds the
	 *	capacity of the array field.
	 */
	private native void setString(String name, String value, long buffer,
				      int base, long info);

	/**
	 * Return an {@link IpcStructField} instance which contains information
	 * about structure field.
	 *
	 * @param stname	The name of this structure.
	 * @param name		The field name in this structure.
	 * @param info		IPC structure information.
	 * @return		An {@code IpcStructField} instance which
	 *			contains information about IPC structure
	 *			field.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	This IPC structure does not contain the field named
	 *	{@code name}.
	 */
	private native IpcStructField getFieldImpl(String stname, String name,
						   long info);

	/**
	 * Return a {@code String} array which contains all field names
	 * defined in this structure.
	 *
	 * @param info	IPC structure information.
	 * @return	A {@code String} array which contains all defined
	 *		field names.
	 */
	private native String[] getFieldNamesImpl(long info);

	/**
	 * Create a copy of buffer for this IPC structure.
	 *
	 * @param buffer	IPC structure buffer.
	 * @param base		Base offset of the IPC structure contents.
	 * @param info		IPC structure information.
	 * @return		A copy of IPC structure buffer specified by
	 *			{@code buffer} and {@code base}.
	 */
	private native long deepCopy(long buffer, int base, long info);
}
