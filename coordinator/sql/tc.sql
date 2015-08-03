--
-- Copyright (c) 2012-2015 NEC Corporation
-- All rights reserved.
-- 
-- This program and the accompanying materials are made available under the
-- terms of the Eclipse Public License v1.0 which accompanies this
-- distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
--

CREATE TABLE TC_UNC_CONF_TABLE (
    auto_save boolean    DEFAULT TRUE,                      -- auto save option
    date_time timestamp  NOT NULL DEFAULT CURRENT_TIMESTAMP -- timestamp
);

CREATE TABLE TC_RECOVERY_TABLE (
    database  integer DEFAULT 0,                            -- database to be audited
    operation integer DEFAULT 0,                            -- operation that caused failover
    abort_version bigint DEFAULT 0,                         -- abort_version during failiver
    save_version  bigint DEFAULT 0,                         -- save version during failover
    failover_instance integer DEFAULT 0,                    -- num of failovers occured
    date_time timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP, -- timestamp
    config_mode smallint  DEFAULT 0,                        -- config_mode (global/real/virtual/vtn)
    vtn_name  varchar(32) DEFAULT ' ',                      -- vtn name
    global_mode_dirty boolean DEFAULT FALSE                 -- global_mode_dirty (in Global Mode Acquisition)
);

