/**
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package com.vtn.app.run.config.rest.parser;

import java.lang.reflect.Field;
import java.lang.reflect.ParameterizedType;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import com.vtn.app.run.config.json.annotations.JsonArray;
import com.vtn.app.run.config.json.annotations.JsonElement;
import com.vtn.app.run.config.json.annotations.JsonObject;
import com.vtn.app.run.config.json.annotations.JsonObjectRef;



public class Parser {

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
                            System.out.println(field.getGenericType());
                            System.out.println((ParameterizedType)field.getGenericType());
                            System.out.println(((ParameterizedType)field.getGenericType()).getActualTypeArguments()[0]);
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
                System.out.println("EMPTY");
            }
        } catch (SecurityException e) {
            e.printStackTrace();
        } catch (JSONException e) {
            e.printStackTrace();
        }
        System.out.println("PARSER OUT:" + obj);
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

        }
        return obj;
    }
}
