/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.endpoint;

import java.io.IOException;

import javax.servlet.Servlet;
import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.json.JSONException;
import org.json.JSONObject;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.enums.ApplicationConstants;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;
import org.opendaylight.vtn.webapi.services.VtnServiceWebAPIHandler;
import org.opendaylight.vtn.webapi.utils.DataConverter;
import org.opendaylight.vtn.webapi.utils.InitManager;
import org.opendaylight.vtn.webapi.utils.VtnServiceCommonUtil;
import org.opendaylight.vtn.webapi.utils.VtnServiceWebUtil;

/**
 * The Class VtnServiceWebAPIServle is the end point for all request coming to Web API will be handled here first 
 */
public class VtnServiceWebAPIServlet extends HttpServlet {
	

	/** The Constant serialVersionUID. */
	private static final long serialVersionUID = 9136213504159408769L;
	
	/** The Constant LOGGER. */
	private static final Logger LOG = Logger.getLogger(VtnServiceWebAPIServlet.class.getName());
	
	/** The vtn service web api controller. */
	private VtnServiceWebAPIHandler vtnServiceWebAPIHandler;


	/**
	 * Initialize the HttpServlet and will initialize LOGGING and Configuration.
	 *
	 * @param config the config
	 * @see Servlet#init(ServletConfig)
	 */
    @Override
	public void init(final ServletConfig config){
		try{
			super.init();
			LOG.trace("Servlet initialization starts...");
			InitManager.initialize();
			LOG.trace("Servlet initialized successfully...");
		}catch (VtnServiceWebAPIException vtnException) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(vtnException.getErrorCode()));
		}catch (ServletException se) {
			LOG.error("Servlet Initialization failed error "+se.getMessage());		
		}catch (Exception e) {
			LOG.error("Servlet Initialization failed error " +e.getMessage());
		}
	}
	
    /**
	 * Service Method will be called each time when a request will come to web API and it will validate the same for authentication and authorization
	 * After this will route the call to the specified method.
	 *
	 * @param request the request
	 * @param response the response
	 * @throws ServletException the servlet exception
	 * @see HttpServlet#service(HttpServletRequest request, HttpServletResponse response)
	 */
	@Override
	protected void service(final HttpServletRequest request, final HttpServletResponse response) {
		LOG.trace("Authentication process is initializing.");
		JSONObject serviceErrorJSON = null;
		try{
			vtnServiceWebAPIHandler = new VtnServiceWebAPIHandler();
			vtnServiceWebAPIHandler.validate(request);
			super.service(request, response);
			LOG.trace("Authentication process initialized.");
		} catch (IOException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		} catch (ServletException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		} catch (VtnServiceWebAPIException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
		}catch (Exception e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		} 
		finally{
			if(null != serviceErrorJSON){
				try {
					final String responseString = DataConverter.getConvertedResponse(serviceErrorJSON, VtnServiceCommonUtil.getContentType(request.getRequestURI()));
					response.setStatus(Integer.valueOf(serviceErrorJSON.getJSONObject(ApplicationConstants.ERROR).getString(ApplicationConstants.ERR_CODE)));
					response.getWriter().write(responseString);
				} catch (IOException e) {
					LOG.error("Servlet writer failed error "+e.getMessage());
				} catch (NumberFormatException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				} catch (JSONException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				} catch (VtnServiceWebAPIException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
				}
			}
		}
		
	}
	
	/**
	 * Do get.This method will get the json/xml data from java API and will write the same on servlet output
	 *
	 * @param request the request
	 * @param response the response
	 * @throws ServletException the servlet exception
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 */
	@Override
	protected void doGet(final HttpServletRequest request, final HttpServletResponse response) throws ServletException {
		LOG.trace("HTTP GET method is processing.");
		JSONObject serviceErrorJSON = null;
			try {
				vtnServiceWebAPIHandler = new VtnServiceWebAPIHandler();
				final String responseString = vtnServiceWebAPIHandler.get(request);
				response.setContentType(VtnServiceCommonUtil.getContentType(request.getRequestURI()));
				response.getWriter().write(responseString);
				LOG.trace("HTTP GET method finished processing.");
			}catch (IOException e) {
				LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
				serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			} catch (VtnServiceWebAPIException e) {
				LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
				serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
			}finally{
				if(null != serviceErrorJSON){
					try {
						final String responseString = DataConverter.getConvertedResponse(serviceErrorJSON, VtnServiceCommonUtil.getContentType(request.getRequestURI()));
						response.setStatus(Integer.valueOf(serviceErrorJSON.getJSONObject(ApplicationConstants.ERROR).getString(ApplicationConstants.ERR_CODE)));
						response.getWriter().write(responseString);
					} catch (IOException e) {
						LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					} catch (NumberFormatException e) {
						LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
						serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
					} catch (JSONException e) {
						LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
						serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
					} catch (VtnServiceWebAPIException e) {
						LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
						serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
					}
				}
			}
			
	}

	/**
	 * Do post.This method will be used to post the data either in xml or json to the java api
	 * by converting all the request to JSON and getting response in specified format.
	 *
	 * @param request the request
	 * @param response the response
	 * @throws ServletException the servlet exception
	 * @throws IOException Signals that an I/O exception has occurred.
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	@Override
	protected void doPost(final HttpServletRequest request, final HttpServletResponse response) throws ServletException, IOException {
		LOG.trace("HTTP POST method start processing.");
		JSONObject serviceErrorJSON = null;
		try{
			vtnServiceWebAPIHandler = new VtnServiceWebAPIHandler();
			final String responseString =  vtnServiceWebAPIHandler.post(request);
			response.setContentType(request.getRequestURI());
			response.getWriter().write(responseString);
			LOG.trace("HTTP POST method finished processing.");
		}
		catch (IOException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}catch (VtnServiceWebAPIException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
		}
		finally{
			if(null != serviceErrorJSON){
				try {
					final String responseString = DataConverter.getConvertedResponse(serviceErrorJSON, VtnServiceCommonUtil.getContentType(request.getRequestURI()));
					response.setStatus(Integer.valueOf(serviceErrorJSON.getJSONObject(ApplicationConstants.ERROR).getString(ApplicationConstants.ERR_CODE)));
					response.getWriter().write(responseString);
				} catch (IOException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
				} catch (NumberFormatException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				} catch (JSONException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				} catch (VtnServiceWebAPIException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
				}
			}
		}
		
		
	}

	/**
	 * Do put.This method will be used to post the data either in xml or json to the java api
	 * by converting all the request to JSON and getting response in specified format.
	 *
	 * @param request the request
	 * @param response the response
	 * @see HttpServlet#doPut(HttpServletRequest, HttpServletResponse)
	 */
	@Override
	protected void doPut(final HttpServletRequest request, final HttpServletResponse response) {	
		LOG.trace("HTTP PUT method start processing.");
		JSONObject serviceErrorJSON = null;
		try{
			vtnServiceWebAPIHandler = new VtnServiceWebAPIHandler();
			final String responseString =  vtnServiceWebAPIHandler.put(request);
			response.setContentType(request.getRequestURI());
			response.getWriter().write(responseString);
			LOG.trace("HTTP PUT method finished processing.");
		}
		catch (IOException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}catch (VtnServiceWebAPIException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
		}finally{
			if(null != serviceErrorJSON){
				try {
					final String responseString = DataConverter.getConvertedResponse(serviceErrorJSON, VtnServiceCommonUtil.getContentType(request.getRequestURI()));
					response.setStatus(Integer.valueOf(serviceErrorJSON.getJSONObject(ApplicationConstants.ERROR).getString(ApplicationConstants.ERR_CODE)));
					response.getWriter().write(responseString);
				} catch (IOException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
				} catch (NumberFormatException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				} catch (JSONException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				} catch (VtnServiceWebAPIException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
				}
			}
		}
		

		
	}

	/**
	 * Do delete.This method will be used to post the data either in xml or json to the java api
	 * by converting all the request to JSON and getting response in specified format.
	 *
	 * @param request the request
	 * @param response the response
	 * @see HttpServlet#doDelete(HttpServletRequest, HttpServletResponse)
	 */
	@Override
	protected void doDelete(final HttpServletRequest request, final HttpServletResponse response){
		LOG.trace("HTTP DELETE method start processing.");
		JSONObject serviceErrorJSON = null;
		try{
			vtnServiceWebAPIHandler = new VtnServiceWebAPIHandler();
			final String responseString =  vtnServiceWebAPIHandler.delete(request);
			response.setContentType(request.getRequestURI());
			response.getWriter().write(responseString);
			LOG.trace("HTTP DELETE method finished processing.");
		}
		catch (IOException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}catch (VtnServiceWebAPIException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
			serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
		}finally{
			if(null != serviceErrorJSON){
				try {
					final String responseString = DataConverter.getConvertedResponse(serviceErrorJSON, VtnServiceCommonUtil.getContentType(request.getRequestURI()));
					response.setStatus(Integer.valueOf(serviceErrorJSON.getJSONObject(ApplicationConstants.ERROR).getString(ApplicationConstants.ERR_CODE)));
					response.getWriter().write(responseString);
				} catch (IOException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
				} catch (NumberFormatException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				} catch (JSONException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				} catch (VtnServiceWebAPIException e) {
					LOG.error(VtnServiceCommonUtil.logErrorDetails(e.getErrorCode()));
					serviceErrorJSON = VtnServiceWebUtil.prepareErrResponseJson(e.getErrorCode(), e.getErrorDescription());
				}
			}
		}
		

	}
	

	/**
	 * destroy method to finalize Servlet instances and used objects in the Web API.
	 *
	 * @param request the request
	 * @param response the response
	 * @see HttpServlet#doPut(HttpServletRequest, HttpServletResponse)
	 */
	@Override
	public void destroy() {
		LOG.trace("Servlet instance is now eligible for garbage collection.");
		super.destroy();
	}
	
}
