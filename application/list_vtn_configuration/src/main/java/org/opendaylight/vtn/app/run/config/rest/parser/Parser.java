/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
     * Default constructor
     */
    public Parser() {}

    /**
     * Parase the reseived Json Object.
     * @param obj
     * @param json
     *
     * @return
     *         JSON object.
     */
    @SuppressWarnings({"unchecked", "rawtypes" })
    public Object parseJsonObject(Object obj , JSONObject json) throws InstantiationException, IllegalAccessException {

        try {
            if (obj != null && obj.getClass().getAnnotation(JsonObject.class) != null) {

                for (Field field : obj.getClass().getDeclaredFields()) {
                    field.setAccessible(true);

                    JsonElement jsonElement = field.getAnnotation(JsonElement.class);
                    JsonObjectRef jsonObjectRef = field.getAnnotation(JsonObjectRef.class);
                    JsonArray jsonList = field.getAnnotation(JsonArray.class);

                    if (jsonElement != null) {
                        if (json.has(jsonElement.name())) {
                            field.set(obj, json.get(jsonElement.name()));
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
                    } else if (jsonObjectRef != null) {

                        if (json.has(jsonObjectRef.name())) {

                            parseJsonObject(field.get(obj), (JSONObject)json.getJSONObject(jsonObjectRef.name()));
                        }
                    }
                }
            } else {
                LOG.warn("JsonObject is null");
                LOG.debug("EMPTY");
            }
        } catch (SecurityException e) {
            System.out.println("An error has occured...\n\nFor more information, please see the logfile");
            LOG.error("An exception occured - {}", e);
        } catch (JSONException e) {
            System.out.println("An error has occured...\n\nFor more information, please see the logfile");
            LOG.error("An exception occured - {}", e);
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
    public Object parseJsonArray(Class<? extends Object> obj, JSONArray json) {

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
            System.out.println("An error has occured...\n\nFor more information, please see the logfile");
            LOG.error("An exception occured - {}", e);
        }
        return obj;
    }
}
