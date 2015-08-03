--
-- Copyright (c) 2012-2015 NEC Corporation
-- All rights reserved.
-- 
-- This program and the accompanying materials are made available under the
-- terms of the Eclipse Public License v1.0 which accompanies this
-- distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
--

-- 			DT_STARTUP DT_CANDIDATE	DT_RUNNING/DT_STATE DT_IMPORT
-- UNC_KT_CONTROLLER		yes	yes		yes		no
-- UNC_KT_DOMAIN		yes	yes		yes		yes
-- UNC_KT_BOUNDARY		yes	yes		yes		no
-- UNC_KT_LOGICAL_PORT		no	no		yes		yes
-- UNC_KT_LOGICAL_MEMBER_PORT	no	no		yes		yes
-- UNC_KT_SWITCH		no	no		yes		yes
-- UNC_KT_PORT			no	no		yes		yes
-- UNC_KT_LINK			no	no		yes		yes

-- variable_declaration ::= variable_name variable_type

create table if not exists s_controller_table (
  controller_name varchar(32) primary key, 
  type smallint not null,
  version varchar(32),
  description varchar(128), 
  ip_address bigint, 
  user_name varchar(32), 
  password varchar(257), 
  enable_audit smallint default 0,
  actual_version varchar(32),
  oper_status smallint default 0,
  port integer,
  valid char(10),
  cs_row_status smallint,
  cs_attr char(10), 
  commit_number bytea,
  commit_date bytea,
  commit_application varchar(256),
  valid_commit_version char(3),
  actual_controllerid varchar(32),
  actual_ctrid_valid char(1)
  );

create table if not exists s_ctr_domain_table(
  controller_name varchar(32) , 
  domain_name varchar(32) , 
  type smallint default 0 , 
  description varchar(128) , 
  oper_status smallint default 1 , 
  valid char(3),
  cs_row_status smallint, 
  cs_attr char(3),        
  primary key(controller_name,domain_name)
  );

create table if not exists s_boundary_table(
  boundary_id varchar(32) primary key,
  description varchar(128),
  controller_name1 varchar(32),
  domain_name1 varchar(32),
  logical_port_id1 bytea,
  controller_name2 varchar(32),
  domain_name2 varchar(32),
  logical_port_id2 bytea,
  oper_status smallint default 1,
  valid char(8),
  cs_row_status smallint,
  cs_attr char(8)
  );

create table if not exists c_controller_table (
  controller_name varchar(32) primary key, 
  type smallint not null,
  version varchar(32),
  description varchar(128), 
  ip_address bigint, 
  user_name varchar(32), 
  password varchar(257), 
  enable_audit smallint default 0,
  actual_version varchar(32),
  oper_status smallint default 0,
  port integer,
  valid char(10),
  cs_row_status smallint,
  cs_attr char(10), 
  commit_number bytea,
  commit_date bytea,
  commit_application varchar(256),
  valid_commit_version char(3),
  actual_controllerid varchar(32),
  actual_ctrid_valid char(1)
  );

create table if not exists c_ctr_domain_table(
  controller_name varchar(32) , 
  domain_name varchar(32) , 
  type smallint default 0 , 
  description varchar(128) , 
  oper_status smallint default 1 , 
  valid char(3),
  cs_row_status smallint, 
  cs_attr char(3),        
  primary key(controller_name,domain_name)
  );

create table if not exists c_boundary_table(
  boundary_id varchar(32) primary key,
  description varchar(128),
  controller_name1 varchar(32),
  domain_name1 varchar(32),
  logical_port_id1 bytea,
  controller_name2 varchar(32),
  domain_name2 varchar(32),
  logical_port_id2 bytea,
  oper_status smallint default 1,
  valid char(8),
  cs_row_status smallint,
  cs_attr char(8)
  );

create table if not exists r_controller_table (
  controller_name varchar(32) primary key, 
  type smallint not null,
  version varchar(32),
  description varchar(128), 
  ip_address bigint, 
  user_name varchar(32), 
  password varchar(257), 
  enable_audit smallint default 0,
  actual_version varchar(32),
  oper_status smallint default 0,
  port integer,
  valid char(10),
  cs_row_status smallint default 5,
  cs_attr char(10), 
  commit_number bytea,
  commit_date bytea,
  commit_application varchar(256),
  valid_commit_version char(3),
  actual_controllerid varchar(32),
  actual_ctrid_valid char(1)
  );

create table if not exists r_ctr_domain_table(
  controller_name varchar(32) , 
  domain_name varchar(32) , 
  type smallint default 0 , 
  description varchar(128) , 
  oper_status smallint default 1 , 
  valid char(3),
  cs_row_status smallint default 5, 
  cs_attr char(3),        
  primary key(controller_name,domain_name)
  );

create table if not exists r_boundary_table(
  boundary_id varchar(32) primary key,
  description varchar(128),
  controller_name1 varchar(32),
  domain_name1 varchar(32),
  logical_port_id1 bytea,
  controller_name2 varchar(32),
  domain_name2 varchar(32),
  logical_port_id2 bytea,
  oper_status smallint default 1,
  valid char(8),
  cs_row_status smallint default 5,
  cs_attr char(8)
  );

create table if not exists r_logicalport_table(
  controller_name varchar(32) , 
  domain_name varchar(32) ,
  port_id bytea,
  description varchar(128), 
  port_type smallint, 
  switch_id bytea,
  physical_port_id varchar(32),
  oper_down_criteria smallint,
  oper_status smallint default 0, 
  valid char(6), 
  primary key(controller_name,domain_name,port_id)
  );

create table if not exists r_logical_member_port_table(
  controller_name varchar(32) , 
  domain_name varchar(32) ,
  port_id bytea,
  switch_id bytea,
  physical_port_id varchar(32),
  primary key(controller_name,domain_name,
  port_id,switch_id,physical_port_id)
  );

create table if not exists r_switch_table(
  controller_name varchar(32) , 
  switch_id bytea, 
  description varchar(128), 
  model varchar(16), 
  ip_address bigint,
  ipv6_address bytea,
  admin_status smallint default 1,
  domain_name varchar(32) ,
  oper_status smallint default 0,
  manufacturer varchar(256),
  hardware varchar(256), 
  software varchar(256), 
  alarms_status bytea,
  valid char(11), 
  primary key(controller_name,switch_id) 
  );

create table if not exists r_port_table(
  controller_name varchar(32) , 
  switch_id bytea ,
  port_id varchar(32) , 
  port_number bigint , 
  description varchar(128),
  admin_status smallint default 1, 
  direction smallint default 2,  
  trunk_allowed_vlan integer,
  oper_status smallint default 0, 
  mac_address bytea, 
  duplex smallint,
  speed bytea,
  alarms_status bytea,
  logical_port_id bytea,
  valid char(11), 
  connected_switch bytea,
  connected_port varchar(32),
  connected_controller varchar(32),
  connectedneighbor_valid char(3),
  primary key (controller_name, switch_id, port_id)
  );

create table if not exists r_link_table(
  controller_name varchar(32) , 
  switch_id1 bytea, 
  port_id1 varchar(32) , 
  switch_id2 bytea, 
  port_id2 varchar(32) , 
  description varchar(128), 
  oper_status smallint default 0, 
  valid char(2),
  primary key (controller_name,switch_id1,port_id1,switch_id2,port_id2)
  );

create table if not exists i_ctr_domain_table(
  controller_name varchar(32) , 
  domain_name varchar(32) , 
  type smallint default 0 , 
  description varchar(128) , 
  oper_status smallint default 1 , 
  valid char(3),
  cs_row_status smallint default 5, 
  cs_attr char(3),        
  primary key(controller_name,domain_name)
  );

create table if not exists i_logicalport_table(
  controller_name varchar(32) , 
  domain_name varchar(32) ,
  port_id bytea,
  description varchar(128), 
  port_type smallint, 
  switch_id bytea,
  physical_port_id varchar(32),
  oper_down_criteria smallint,
  oper_status smallint default 0, 
  valid char(6), 
  primary key(controller_name,domain_name,port_id)
  );

create table if not exists i_logical_member_port_table(
  controller_name varchar(32) , 
  domain_name varchar(32) ,
  port_id bytea,
  switch_id bytea,
  physical_port_id varchar(32),
  primary key(controller_name,domain_name,
  port_id,switch_id,physical_port_id)
  );

create table if not exists i_switch_table(
  controller_name varchar(32) , 
  switch_id bytea, 
  description varchar(128), 
  model varchar(16), 
  ip_address bigint,
  ipv6_address bytea,
  admin_status smallint default 1,
  domain_name varchar(32) ,
  oper_status smallint default 0,
  manufacturer varchar(256),
  hardware varchar(256), 
  software varchar(256), 
  alarms_status bytea,
  valid char(11), 
  primary key(controller_name,switch_id) 
  );

create table if not exists i_port_table(
  controller_name varchar(32) , 
  switch_id bytea ,
  port_id varchar(32) , 
  port_number bigint , 
  description varchar(128),
  admin_status smallint default 1, 
  direction smallint default 2,  
  trunk_allowed_vlan integer,
  oper_status smallint default 0, 
  mac_address bytea, 
  duplex smallint,
  speed bytea,
  alarms_status bytea,
  logical_port_id bytea,
  valid char(11), 
  connected_switch bytea,
  connected_port varchar(32),
  connected_controller varchar(32),
  connectedneighbor_valid char(3),
  primary key (controller_name, switch_id, port_id)
  );

create table if not exists i_link_table(
  controller_name varchar(32) , 
  switch_id1 bytea, 
  port_id1 varchar(32) , 
  switch_id2 bytea, 
  port_id2 varchar(32) , 
  description varchar(128), 
  oper_status smallint default 0, 
  valid char(2),
  primary key (controller_name,switch_id1,port_id1,switch_id2,port_id2)
  );
