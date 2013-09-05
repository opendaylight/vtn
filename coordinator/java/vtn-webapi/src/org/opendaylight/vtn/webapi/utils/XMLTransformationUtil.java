/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.utils;

import java.io.ByteArrayOutputStream;
import java.io.StringReader;
import java.util.Iterator;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.TransformerException;
import javax.xml.transform.stream.StreamResult;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.enums.ApplicationConstants;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;

/**
 * The Class Transform XML String on the basis of XSLT file. Used XSLT file in
 * this class contains the rules to transform elements to attributes.
 * 
 */
public class XMLTransformationUtil {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(XMLTransformationUtil.class.getName());
	
	/**
	 * Convert all elements to attributes in XML string
	 * 
	 * @param xmlContent
	 *            valid XML formatted string
	 * @return attributed XML string
	 * @throws TransformerException
	 */
	public static String convertAllAttributesToElements(String xmlContent)
			throws TransformerException {
		LOG.trace("Start XMLTransformationUtil#convertAllAttributesToElements()");
		javax.xml.transform.Source xmlSource = new javax.xml.transform.stream.StreamSource(
				new StringReader(xmlContent));
		javax.xml.transform.Source xsltSource = new javax.xml.transform.stream.StreamSource(
				Thread.currentThread().getContextClassLoader()
						.getResource(ApplicationConstants.XSLT_FILE)
						.toExternalForm());
		LOG.trace("Complete XMLTransformationUtil#convertAllAttributesToElements()");
		return transformContent(xmlSource, xsltSource);
	}

	/**
	 * Transformation is performed from elements to attributes
	 * 
	 * @param xmlSource
	 * @param xsltSource
	 * @return
	 * @throws TransformerException
	 */
	private static String transformContent(
			javax.xml.transform.Source xmlSource,
			javax.xml.transform.Source xsltSource) throws TransformerException {
		LOG.trace("Start XMLTransformationUtil#transformContent()");
		String modifiedContent = null;
		javax.xml.transform.TransformerFactory transFact = javax.xml.transform.TransformerFactory
				.newInstance();
		ByteArrayOutputStream outStream = new ByteArrayOutputStream();
		StreamResult result = new javax.xml.transform.stream.StreamResult(
				outStream);
		javax.xml.transform.Transformer trans = transFact
				.newTransformer(xsltSource);
		trans.setOutputProperty(OutputKeys.STANDALONE,
				ApplicationConstants.XML_STANDALONE);
		trans.transform(xmlSource, result);
		byte[] b = ((ByteArrayOutputStream) result.getOutputStream())
				.toByteArray();
		modifiedContent = new String(b);
		// removing any line feed to save bytes.
		modifiedContent = modifiedContent.replaceAll(
				ApplicationConstants.LINE_FEED,
				ApplicationConstants.EMPTY_STRING);
		LOG.trace("Complete XMLTransformationUtil#transformContent()");
		return modifiedContent;
	}

	/**
	 * Convert JSON format to XML format, without any parent tag name in XML
	 * format
	 * 
	 * @param responseJson
	 * @return
	 * @throws JSONException
	 * @throws VtnServiceWebAPIException 
	 */
	public static String convertJsonToXml(Object responseJson)
			throws JSONException, VtnServiceWebAPIException {
		return convertJsonToXml(responseJson, null);
	}

	/**
	 * Convert JSON format to XML format, with given tag name in XML format
	 * 
	 * @param responseJson
	 * @param parentTagName
	 * @return
	 * @throws JSONException
	 * @throws VtnServiceWebAPIException 
	 */
	public static String convertJsonToXml(Object responseJson,
			String parentTagName) throws JSONException, VtnServiceWebAPIException {
		LOG.trace("Start XMLTransformationUtil#convertJsonToXml()");
		StringBuffer xmlString = new StringBuffer();
		// local parameter required for conversion
		JSONArray jsonArray;
		JSONObject jsonObject;
		String jsonKey;
		Iterator<?> jsonKeys;
		int counter, length;
		Object jsonValue;

		// if given object is JSON object
		if (responseJson instanceof JSONObject) {
			// if parent tag is specified
			if (parentTagName != null) {
				xmlString.append(ApplicationConstants.LESS_THAN);
				xmlString.append(parentTagName);
				xmlString.append(ApplicationConstants.GREATER_THAN);
			}
			jsonObject = (JSONObject) responseJson;
			jsonKeys = jsonObject.keys();
			boolean flag = false;
			// iterate for all of the element of JSON object
			while (jsonKeys.hasNext()) {
				jsonKey = jsonKeys.next().toString();
				jsonValue = jsonObject.opt(jsonKey);
				if (jsonValue == null) {
					jsonValue = ApplicationConstants.EMPTY_STRING;
				}
				// if element in JSON object is JSON array type then iterate for
				// all of element recursively
				if (jsonValue instanceof JSONArray) {
					jsonArray = (JSONArray) jsonValue;
					length = jsonArray.length();
					// if there is no element in JSON array
					if (length == 0) {
						xmlString.append(ApplicationConstants.LESS_THAN);
						xmlString.append(jsonKey);
						xmlString.append(ApplicationConstants.SLASH);
						xmlString.append(ApplicationConstants.GREATER_THAN);
					} else {
						xmlString.append(ApplicationConstants.LESS_THAN);
						xmlString.append(jsonKey);
						xmlString.append(ApplicationConstants.GREATER_THAN);
						// get the child name from configuration file and add
						// the same for array items
						String childElementName = null;
						try {
							childElementName = ConfigurationManager
									.getInstance().getConfProperty(jsonKey);
						} catch (VtnServiceWebAPIException e) {
							LOG.error("Property not found for list element");
							if (!(ApplicationConstants.ipaddrs.equals(jsonKey) 
									|| ApplicationConstants.ipv6addr.equals(jsonKey))) {
								throw e;
							} else {
								flag = true;
							}
						}
						// sb.append('[');
						for (counter = 0; counter < length; counter += 1) {
							if(childElementName != null){
								xmlString.append(ApplicationConstants.LESS_THAN);
								xmlString.append(childElementName);
								xmlString.append(ApplicationConstants.GREATER_THAN);
							}
							jsonValue = jsonArray.get(counter);								
							if (jsonValue instanceof JSONArray) {
								xmlString.append(convertJsonToXml(jsonValue));
							} else {
								if (!(counter == length - 1) && flag) {
									jsonValue = jsonValue + ApplicationConstants.COMMA_STR;
								}
								xmlString.append(convertJsonToXml(jsonValue));
							}
							if(childElementName != null){
								xmlString.append(ApplicationConstants.LESS_THAN);
								xmlString.append(ApplicationConstants.SLASH);
								xmlString.append(childElementName);
								xmlString.append(ApplicationConstants.GREATER_THAN);
							}
						}
						// sb.append(']');
						xmlString.append(ApplicationConstants.LESS_THAN);
						xmlString.append(ApplicationConstants.SLASH);
						xmlString.append(jsonKey);
						xmlString.append(ApplicationConstants.GREATER_THAN);
						flag = false;
					}
				} else if (ApplicationConstants.EMPTY_STRING.equals(jsonValue)) {
					xmlString.append(ApplicationConstants.LESS_THAN);
					xmlString.append(jsonKey);
					xmlString.append(ApplicationConstants.SLASH);
					xmlString.append(ApplicationConstants.GREATER_THAN);
				} else {
					xmlString.append(convertJsonToXml(jsonValue, jsonKey));
				}
			}
			if (parentTagName != null) {
				xmlString.append(ApplicationConstants.LESS_THAN);
				xmlString.append(ApplicationConstants.SLASH);
				xmlString.append(parentTagName);
				xmlString.append(ApplicationConstants.GREATER_THAN);
			}
		} else {
			// if entity is a key-value pair - no JSON object or array is there
			if (parentTagName != null) {
				xmlString.append(ApplicationConstants.LESS_THAN);
				xmlString.append(parentTagName);
				xmlString.append(ApplicationConstants.GREATER_THAN);
			}
			xmlString.append(responseJson);
			if (parentTagName != null) {
				xmlString.append(ApplicationConstants.LESS_THAN);
				xmlString.append(ApplicationConstants.SLASH);
				xmlString.append(parentTagName);
				xmlString.append(ApplicationConstants.GREATER_THAN);
			}
		}
		LOG.trace("Complete XMLTransformationUtil#convertJsonToXml()");
		return xmlString.toString();
	}

	/**
	 * pre-process request Json for removing unwanted tags
	 * @param jsonObject
	 * @return
	 * @throws JSONException
	 * @throws VtnServiceWebAPIException
	 */
	public static JSONObject preProcessJson(JSONObject jsonObject) throws JSONException, VtnServiceWebAPIException {
		// pre-process for vtep-group API
		if (ApplicationConstants.vtepgroup.equals(jsonObject.keys().next().toString()) 
				&& jsonObject.getJSONObject(ApplicationConstants.vtepgroup).has(ApplicationConstants.member_vteps)) {
			JSONObject memberVtepJson = jsonObject.getJSONObject(ApplicationConstants.vtepgroup).getJSONObject(ApplicationConstants.member_vteps);
			JSONArray newArray = new JSONArray();
			if (memberVtepJson.has(ApplicationConstants.member_vtep)) {
				// only one instance
				if (memberVtepJson.get(ApplicationConstants.member_vtep) instanceof JSONObject) {
					newArray.put(memberVtepJson
							.getJSONObject(ApplicationConstants.member_vtep));
				} // multiple instances
				else if (memberVtepJson.get(ApplicationConstants.member_vtep) instanceof JSONArray) {
					newArray = memberVtepJson
							.getJSONArray(ApplicationConstants.member_vtep);
				}
			}					
			jsonObject.getJSONObject(ApplicationConstants.vtepgroup).put(ApplicationConstants.member_vteps, newArray);
		}
		return jsonObject;
	}
}
