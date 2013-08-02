/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.tomcat.server;

import org.apache.catalina.Lifecycle;
import org.apache.catalina.LifecycleEvent;
import org.apache.catalina.LifecycleListener;
import org.apache.juli.logging.Log;
import org.apache.juli.logging.LogFactory;

/**
 * <p>
 *   Handle life cycle event for the Tomcat server.
 * </p>
 * <p>
 *   This event listener is used to initialize and finalize global resources
 *   for UNC service in the Tomcat server.
 * </p>
 */
public class StateListener implements LifecycleListener
{
	/**
	 * <p>
	 *   Invoked when the state of life cycle of the Tomcat server has
	 *   been changed.
	 * </p>
	 *
	 * @param event	A {@code LifecycleEvent} which represents the state of
	 *		the Tomcat server.
	 */
	@Override
	public void lifecycleEvent(LifecycleEvent event)
	{
		String type = event.getType();
		Log log = LogFactory.getLog(getClass());

		if (log.isDebugEnabled()) {
			log.debug("Received " + type + " event.");
		}

		if (Lifecycle.BEFORE_INIT_EVENT.equals(type)) {
			// Initialize global resources.
			GlobalResourceManager.getInstance().initialize();
		}
		else if (Lifecycle.BEFORE_STOP_EVENT.equals(type)) {
			// Trigger the resource shutdown.
			GlobalResourceManager.getInstance().startShutdown();
		}
		else if (Lifecycle.AFTER_STOP_EVENT.equals(type)) {
			// Disable global resources.
			GlobalResourceManager.getInstance().disable();
		}
	}
}
