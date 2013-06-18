/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

/**
 * <p>
 *   The {@code LogFatalHandler} interface defines an interface of fatal
 *   log handler. If a fatal log handler is registered to the PFC-Core
 *   logging system, it will be called when a FATAL level message is logged to
 *   the PFC-Core logging system.
 * </p>
 *
 * @since	C10
 */
public interface LogFatalHandler
{
	/**
	 * A handler which is invoked when a FATAL level message is logged.
	 */
	public void fatalError();
}
