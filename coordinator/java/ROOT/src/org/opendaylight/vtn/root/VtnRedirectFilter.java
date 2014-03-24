/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.root;

import java.io.IOException;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;

/**
 * Filter Class VtnRedirectFilter that forwards all requests corresponding to
 * OpenStack APIs to vtn-webapi application
 */
public class VtnRedirectFilter implements Filter {

	@Override
	public void doFilter(ServletRequest req, ServletResponse res,
			FilterChain chain) throws ServletException, IOException {
		HttpServletRequest request = (HttpServletRequest) req;

		String requestURI = request.getRequestURI();
		String query = request.getQueryString();

		/*
		 * forwards requests corresponding to OpenStack operations
		 */
		if (requestURI.startsWith("/tenants")) {
			if (query != null) {
				requestURI = requestURI + "?" + query;
			}
			req.getServletContext().getContext("/vtn-webapi")
					.getRequestDispatcher(requestURI).forward(req, res);
		}
	}

	/*
	 * Destroy method for Filter - No destruction is required
	 * 
	 * @see javax.servlet.Filter#init(javax.servlet.FilterConfig)
	 */
	@Override
	public void destroy() {
		// No implementation required
	}

	/*
	 * Initialization method for Filter - No initialization is required
	 * 
	 * @see javax.servlet.Filter#init(javax.servlet.FilterConfig)
	 */
	@Override
	public void init(FilterConfig arg0) throws ServletException {
		// No implementation required
	}
}

