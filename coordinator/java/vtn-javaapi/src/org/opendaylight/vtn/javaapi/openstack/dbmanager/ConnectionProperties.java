/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.dbmanager;

/**
 * Bean Class for DB Connection Properties
 */
public class ConnectionProperties {

	private String dbDriver, dbURL, dbUsername, dbPassword;

	private int maxPossibleConnections;
	private int initialConnections;

	private boolean waitforUsedConnections;

	/**
	 * Getter for dbDriver
	 * 
	 * @return
	 */
	public String getDbDriver() {
		return dbDriver;
	}

	/**
	 * Setter for dbDriver
	 * 
	 * @return
	 */
	public void setDbDriver(String dbDriver) {
		this.dbDriver = dbDriver;
	}

	/**
	 * Getter for dbURL
	 * 
	 * @return
	 */
	public String getDbURL() {
		return dbURL;
	}

	/**
	 * Setter for dbURL
	 * 
	 * @return
	 */
	public void setDbURL(String dbURL) {
		this.dbURL = dbURL;
	}

	/**
	 * Getter for dbUsername
	 * 
	 * @return
	 */
	public String getDbUsername() {
		return dbUsername;
	}

	/**
	 * Setter for dbUsername
	 * 
	 * @return
	 */
	public void setDbUsername(String dbUsername) {
		this.dbUsername = dbUsername;
	}

	/**
	 * Getter for dbPassword
	 * 
	 * @return
	 */
	public String getDbPassword() {
		return dbPassword;
	}

	/**
	 * Setter for dbPassword
	 * 
	 * @return
	 */
	public void setDbPassword(String dbPassword) {
		this.dbPassword = dbPassword;
	}

	/**
	 * Getter for maxPossibleConnections
	 * 
	 * @return
	 */
	public int getMaxPossibleConnections() {
		return maxPossibleConnections;
	}

	/**
	 * Setter for maxPossibleConnections
	 * 
	 * @return
	 */
	public void setMaxPossibleConnections(int maxPossibleConnections) {
		this.maxPossibleConnections = maxPossibleConnections;
	}

	/**
	 * Getter for initialConnections
	 * 
	 * @return
	 */
	public int getInitialConnections() {
		return initialConnections;
	}

	/**
	 * Setter for initialConnections
	 * 
	 * @return
	 */
	public void setInitialConnections(int initialConnections) {
		this.initialConnections = initialConnections;
	}

	/**
	 * Getter for waitforUsedConnections
	 * 
	 * @return
	 */
	public boolean isWaitforUsedConnections() {
		return waitforUsedConnections;
	}

	/**
	 * Setter for waitforUsedConnections
	 * 
	 * @return
	 */
	public void setWaitforUsedConnections(boolean waitforUsedConnections) {
		this.waitforUsedConnections = waitforUsedConnections;
	}
}
