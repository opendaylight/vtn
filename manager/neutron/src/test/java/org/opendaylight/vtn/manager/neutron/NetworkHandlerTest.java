/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.neutron;

import junit.framework.TestCase;

import org.junit.Assert;
import org.junit.Test;

import org.opendaylight.vtn.manager.neutron.NetworkHandler;

/**
 * JUnit test for {@link NetworkHandler}
 */
public class NetworkHandlerTest extends TestCase {
  @Test
  public void testNeutronNetworkCreated() {
    NetworkHandler nh = new NetworkHandler();
    Assert.assertTrue(nh != null);
    nh.neutronNetworkCreated(null);
  }
}
