/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.util;

import java.util.ArrayList;
import java.util.List;

public class OrderedProperties {

	List<String> valueSet = new ArrayList<String>();
	List<String> keySet = new ArrayList<String>();

	public List<String> getValueSet() {
		return valueSet;
	}

	public void setValueSet(List<String> valueSet) {
		this.valueSet = valueSet;
	}

	public List<String> getKeySet() {
		return keySet;
	}

	public void setKeySet(List<String> keySet) {
		this.keySet = keySet;
	}
}
