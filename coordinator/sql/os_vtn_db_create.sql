--
-- Copyright (c) 2013-2015 NEC Corporation
-- All rights reserved.
-- 
-- This program and the accompanying materials are made available under the
-- terms of the Eclipse Public License v1.0 which accompanies this
-- distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
--

CREATE TABLE os_controller_tbl
(
  os_controller_id character varying(31) NOT NULL,
  os_controller_counter integer NOT NULL DEFAULT 1,
  CONSTRAINT pk_os_controller PRIMARY KEY (os_controller_counter),
  CONSTRAINT ck_os_controller CHECK (os_controller_counter = 1)
);

CREATE TABLE os_free_counter_tbl
(
  os_res_id character varying(7) NOT NULL,
  os_vtn_name character varying(31) NOT NULL,
  os_res_counter integer NOT NULL,
  CONSTRAINT pk_os_free_counter PRIMARY KEY (os_vtn_name, os_res_id, os_res_counter)
); 

CREATE TABLE os_vtn_tbl
(
  os_vtn_id integer NOT NULL,
  os_vtn_name character varying(31) NOT NULL,
  os_vtn_status integer NOT NULL,
  CONSTRAINT pk_os_vtn PRIMARY KEY (os_vtn_name),
  CONSTRAINT max_val CHECK (os_vtn_id >= 0)
);

CREATE TABLE os_vbr_tbl
(
  os_vbr_id integer NOT NULL,
  os_vtn_name character varying(31) NOT NULL,
  os_vbr_name character varying(31) NOT NULL,
  os_vbr_status integer NOT NULL,
  CONSTRAINT pk_os_vbr PRIMARY KEY (os_vtn_name, os_vbr_name),
  CONSTRAINT fk_os_vbr FOREIGN KEY (os_vtn_name)
      REFERENCES os_vtn_tbl (os_vtn_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT max_val CHECK (os_vbr_id >= 0)
);
 
CREATE TABLE os_vbr_if_tbl
(
  os_vbr_if_id integer NOT NULL,
  os_vtn_name character varying(31) NOT NULL,
  os_vbr_name character varying(31) NOT NULL,
  os_vbr_if_name character varying(31) NOT NULL,
  os_map_type character(7) NOT NULL,
  os_logical_port_id character varying(319) NOT NULL,
  os_vbr_if_status integer NOT NULL,
  CONSTRAINT pk_os_vbr_if PRIMARY KEY (os_vtn_name, os_vbr_name, os_vbr_if_name),
  CONSTRAINT fk_os_vbr_if FOREIGN KEY (os_vtn_name, os_vbr_name)
      REFERENCES os_vbr_tbl (os_vtn_name, os_vbr_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT max_val CHECK (os_vbr_if_id >= 0)
);

CREATE TABLE os_vrt_tbl
(
  os_vtn_name character varying(31) NOT NULL,
  os_vrt_name character varying(31) NOT NULL,
  CONSTRAINT pk_os_vrt PRIMARY KEY (os_vtn_name, os_vrt_name),
  CONSTRAINT fk_os_vrt FOREIGN KEY (os_vtn_name)
      REFERENCES os_vtn_tbl (os_vtn_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT uk_vtn_name UNIQUE (os_vtn_name)
);

CREATE TABLE os_vrt_if_tbl
(
  os_vrt_if_id integer NOT NULL,
  os_vtn_name character varying(31) NOT NULL,
  os_vrt_name character varying(31) NOT NULL,
  os_vrt_if_name character varying(31) NOT NULL,
  os_vbr_name character varying(31) NOT NULL,
  CONSTRAINT pk_os_vrt_if PRIMARY KEY (os_vtn_name, os_vrt_name, os_vrt_if_name),
  CONSTRAINT fk_os_vrt_if FOREIGN KEY (os_vtn_name, os_vrt_name)
      REFERENCES os_vrt_tbl (os_vtn_name, os_vrt_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT max_val CHECK (os_vrt_if_id > 0)
);

CREATE TABLE os_vrt_route_tbl
(
  os_vtn_name character varying(31) NOT NULL,
  os_vrt_name character varying(31) NOT NULL,
  os_vrt_route_name character varying(47) NOT NULL,
  CONSTRAINT pk_os_vrt_route PRIMARY KEY (os_vtn_name, os_vrt_name, os_vrt_route_name),
  CONSTRAINT fk_os_vrt_route FOREIGN KEY (os_vtn_name, os_vrt_name)
      REFERENCES os_vrt_tbl (os_vtn_name, os_vrt_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE os_fl_tbl
(
  os_fl_id integer NOT NULL,
  os_fl_name character varying(32) NOT NULL,
  os_fl_status integer NOT NULL,
  CONSTRAINT pk_os_fl PRIMARY KEY (os_fl_name),
  CONSTRAINT max_val CHECK (os_fl_id >= 0)
);

CREATE TABLE os_ff_vbr_tbl
(
  os_vtn_name character varying(31) NOT NULL,
  os_vbr_name character varying(31) NOT NULL,
  os_vbr_if_name character varying(31) NOT NULL,
  os_fl_name character varying(32) NOT NULL,
  CONSTRAINT pk_os_ff_vbr PRIMARY KEY (os_vtn_name, os_vbr_name, os_vbr_if_name, os_fl_name),
  CONSTRAINT fk_os_ff_vbr_fl FOREIGN KEY (os_fl_name)
      REFERENCES os_fl_tbl (os_fl_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT fk_os_ff_vbr_if FOREIGN KEY (os_vtn_name, os_vbr_name, os_vbr_if_name)
      REFERENCES os_vbr_if_tbl (os_vtn_name, os_vbr_name, os_vbr_if_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE os_ff_vrt_tbl
(
  os_vtn_name character varying(31) NOT NULL,
  os_vrt_name character varying(31) NOT NULL,
  os_vrt_if_name character varying(31) NOT NULL,
  os_fl_name character varying(32) NOT NULL,
  os_vbr_name character varying(31) NOT NULL,
  CONSTRAINT pk_os_ff_vrt PRIMARY KEY (os_vtn_name, os_vrt_name, os_vrt_if_name, os_fl_name),
  CONSTRAINT fk_os_ff_vrt_fl FOREIGN KEY (os_fl_name)
      REFERENCES os_fl_tbl (os_fl_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT fk_os_ff_vrt_if FOREIGN KEY (os_vtn_name, os_vrt_name, os_vrt_if_name)
      REFERENCES os_vrt_if_tbl (os_vtn_name, os_vrt_name, os_vrt_if_name) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
);
