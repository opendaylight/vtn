/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;

import static junit.framework.TestCase.assertTrue;
/**
 * <p>
 *   {@code BackupFile} is an abstract class which represents a file with
 *   backup.
 * </p>
 */
abstract class BackupFile
{
	/**
	 * <p>
	 *   Path to installed file.
	 * </p>
	 */
	protected File  _file;

	/**
	 * <p>
	 *   Backup file path.
	 * </p>
	 */
	protected File  _backup;

	/**
	 * <p>
	 *   Restore the target file from backup.
	 * </p>
	 */
	synchronized void restore()
	{
		File dst = _file;

		if (dst != null) {
			if (_backup == null) {
				assertTrue(dst.delete());
			}
			else {
				assertTrue(_backup.renameTo(dst));
				_backup = null;
			}

			_file = null;
		}
	}

	/**
	 * <p>
	 *   Create the target file.
	 * </p>
	 *
	 * @param path	Path to the target file.
	 * @return	A {@code PrintWriter} associated with {@code path}.
	 * @throws FileNotFoundException
	 *	Failed to create a new community file.
	 */
	protected synchronized PrintWriter create(String path)
		throws FileNotFoundException
	{
		File src = createBackup(path);
		PrintWriter writer = new PrintWriter(src);
		_file = src;

		return writer;
	}

	/**
	 * <p>
	 *   Back up the current file.
	 * </p>
	 *
	 * @param path	Path to a file to be preserved.
	 * @return	A {@code File} instance associated with the target
	 *		file is returned.
	 */
	protected synchronized File createBackup(String path)
	{
		File src = new File(path);
		if (_backup != null) {
			return src;
		}

		if (!src.exists()) {
			return src;
		}

		File dst;
		for (int i = 0; true; i++) {
			String backup = path + ".bk" + i;
			dst = new File(backup);
			if (!dst.exists()) {
				break;
			}
		}

		assertTrue(src.renameTo(dst));
		_backup = dst;

		return src;
	}
}
