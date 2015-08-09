/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.parser;

import java.lang.reflect.Field;
import java.lang.reflect.ParameterizedType;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.opendaylight.vtn.app.run.config.rest.client.VTNClientException;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

public class Parser {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(Parser.class);

    /**
     * Error message.
     */
    private static final String  ERROR_MESSAGE = "\n\nAn error has occured...";

    /**
     * Default constructor
     */
    public Parser() {}

    /**
     * Type conversion of parsed json object.
     *
     * @param object
     * @param field
     * @param fieldValue
     */
    private void parseToRespectiveObject(Object object, Field field,
            String fieldValue) {
        try {
            String fieldTypeName = field.getType().getName();
            if ((field != null) && (field.getType().isPrimitive())) {
                if (fieldTypeName.equals("long")) {
                    field.set(object, Long.parseLong(fieldValue));
                } else if (fieldTypeName.equals("double")) {
                    field.set(object, Double.parseDouble(fieldValue));
                } else if (fieldTypeName.equals("float")) {
                    field.set(object, Float.parseFloat(fieldValue));
                } else if (fieldTypeName.equals("int")) {
                    field.set(object, Integer.parseInt(fieldValue));
                } else if (fieldTypeName.equals("short")) {
                    field.set(object, Short.parseShort(fieldValue));
                } else if (fieldTypeName.equals("byte")) {
                    field.set(object, Byte.parseByte(fieldValue));
                } else if (fieldTypeName.equals("boolean")) {
                    field.set(object, Boolean.parseBoolean(fieldValue));
                }
            } else if (fieldTypeName.equals("java.lang.String")) {
                field.set(object, fieldValue);
            }
        } catch (IllegalArgumentException | IllegalAccessException e) {
            LOG.error("Exception in parseToRespectiveObject()", e);
        }
    }

    /**
     * Parase the reseived Json Object.
     * @param obj
     * @param json
     *
     * @return
     *         JSON object.
     */
    @SuppressWarnings({"unchecked", "rawtypes" })
    public Object parseJsonObject(Object obj , JSONObject json) throws InstantiationException, IllegalAccessException, VTNClientException {

        try {
            if (obj != null && obj.getClass().getAnnotation(JsonObject.class) != null) {

                for (Field field : obj.getClass().getDeclaredFields()) {
                    field.setAccessible(true);

                    JsonElement jsonElement = field.getAnnotation(JsonElement.class);
                    JsonObjectRef jsonObjectRef = field.getAnnotation(JsonObjectRef.class);
                    JsonArray jsonList = field.getAnnotation(JsonArray.class);

                    if (jsonElement != null) {
                        if (json.has(jsonElement.name())) {
                            if (!field.getType().getName().equals(json.get(jsonElement.name()).getClass().getName())) {
                                parseToRespectiveObject(obj, field, String.valueOf(json.get(jsonElement.name())));
                            } else {
                                field.set(obj, json.get(jsonElement.name()));
                            }
                        }

                    } else if (jsonList != null) {

                        if (json.has(jsonList.name())) {

                            JSONArray jsonArray = (JSONArray)json.get(jsonList.name());
                            LOG.debug("" + field.getGenericType());
                            LOG.debug("" + (ParameterizedType)field.getGenericType());
                            LOG.debug("" + ((ParameterizedType)field.getGenericType()).getActualTypeArguments()[0]);
                            Class<?> listClass = (Class<?>)((ParameterizedType)field.getGenericType()).getActualTypeArguments()[0];

                            for (int i = 0; i < jsonArray.length(); i++) {
                                Object listObj = listClass.newInstance();
                                ((List)field.get(obj)).add(listObj);
                                parseJsonObject(listObj, (JSONObject)jsonArray.get(i));
                            }
                        }
                    } else if (jsonObjectRef != null &&
                               json.has(jsonObjectRef.name())) {

                        parseJsonObject(field.get(obj), (JSONObject)json.getJSONObject(jsonObjectRef.name()));
                    }
                }
            } else {
                LOG.warn("JsonObject is null");
                LOG.debug("EMPTY");
            }
        } catch (SecurityException | JSONException e) {
            LOG.error("An exception occured - {}", e);
            throw new VTNClientException(ERROR_MESSAGE);
        }

        LOG.debug("PARSER OUT:" + obj);
        LOG.debug("JsonObject parser is done");
        return obj;
    }

    /**
     * Parase the reseived Json Array.
     * @param obj
     * @param json
     *
     * @return
     *         JSON object.
     */
    @SuppressWarnings({"unchecked", "rawtypes"})
    public Object parseJsonArray(Class<? extends Object> obj, JSONArray json) throws VTNClientException {

        try {
            if (obj.getAnnotation(JsonObject.class) != null) {

                for (Field field : obj.getDeclaredFields()) {
                    field.setAccessible(true);
                    JSONArray jsonArray = json;
                    Class<?> listClass = (Class<?>)((ParameterizedType)field.getGenericType()).getActualTypeArguments()[0];
                    for (int i = 0; i < jsonArray.length(); i++) {
                        Object listObj =  listClass.newInstance();
                        ((List)field.get(obj)).add(listObj);
                        parseJsonObject(listObj, (JSONObject)jsonArray.get(i));
                    }
                }
            }
        } catch (Exception e) {
            LOG.error("An exception occured - ", e);
            throw new VTNClientException(ERROR_MESSAGE);
        }
        return obj;
    }
}
