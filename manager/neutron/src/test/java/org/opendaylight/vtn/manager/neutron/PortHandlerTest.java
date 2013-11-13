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

/**
 * JUnit test for {@link PortHandler}
 */
public class PortHandlerTest extends TestCase {
  /**
   * Test method for
   * {@link PortHandler#neutronPortCreated(NeutronPort port)}.
   */
  @Test
  public void testNeutronPortCreated() {
    PortHandler nh = new PortHandler();
    Assert.assertTrue(nh != null);
    nh.neutronPortCreated(null);
  }
}
