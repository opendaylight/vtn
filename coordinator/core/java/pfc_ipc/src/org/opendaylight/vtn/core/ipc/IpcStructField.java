/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

/**
 * <p>
 *   The {@code IpcStructField} class provides information about a single
 *   field in the IPC structure.
 * </p>
 *
 * @see		IpcStruct#getField(String, String)
 * @see		IpcStruct#getField(String)
 * @since	C10
 */
public final class IpcStructField
{
	/**
	 * The name of IPC structure that contains the field.
	 */
	private final String  _structName;

	/**
	 * The name of the structure field.
	 */
	private final String  _name;

	/**
	 * Data type identifier of the field.
	 */
	private final int  _type;

	/**
	 * The length of array field.
	 * Zero is set for non-array field.
	 */
	private final int  _arrayLength;

	/**
	 * The name of nested IPC structure.
	 * null is set if the field type is not {@link IpcDataUnit#STRUCT}.
	 */
	private final String  _nestedName;

	/**
	 * Construct a new instance.
	 *
	 * @param stname	The name of IPC structure that contains
	 *			the field.
	 * @param name		The field name.
	 * @param type		Data type identifier of the field.
	 * @param alen		The length of array field. Zero must be
	 *			specified if the field is non-array field.
	 * @param nested	The name of nested IPC structure.
	 *			null must be specified if the field type
	 *			is not {@link IpcDataUnit#STRUCT}.
	 */
	IpcStructField(String stname, String name, int type, int alen,
		       String nested)
	{
		_structName = stname;
		_name = name;
		_type = type;
		_arrayLength = alen;
		_nestedName = nested;
	}

	/**
	 * Return the name of IPC structure which contains this field.
	 *
	 * @return	The name of IPC structure.
	 */
	public String getStructureName()
	{
		return _structName;
	}

	/**
	 * Return the name of this structure field.
	 *
	 * @return	The name of IPC structure field.
	 */
	public String getName()
	{
		return _name;
	}

	/**
	 * Return the type identifier of this field.
	 *
	 * @return	The type identifier defined by {@link IpcDataUnit},
	 *		such as {@link IpcDataUnit#INT8}.
	 */
	public int getType()
	{
		return _type;
	}

	/**
	 * Determine whether this field is an array field or not.
	 *
	 * @return	True if this field is an array field.
	 *		Otherwise false.
	 */
	public boolean isArray()
	{
		return (_arrayLength != 0);
	}

	/**
	 *  Return the length of an array field.
	 *
	 * @return	The length of array field.
	 *		Zero is returned if this field is not an array field.
	 */
	public int getArrayLength()
	{
		return _arrayLength;
	}

	/**
	 * Return the name of nested IPC structure.
	 *
	 * @return	The name of nested structure.
	 *		null is returned if this field is not a nested
	 *		structure field.
	 */
	public String getNestedStructureName()
	{
		return _nestedName;
	}

	/**
	 * Return a string which represents this field.
	 *
	 * @return	A string which represents this field.
	 */
	@Override
	public String toString()
	{
		return "ipc_struct " + _structName + "." + _name;
	}
}
