--
-- Copyright (c) 2012-2014 NEC Corporation
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
    date_time timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP, -- timestamp
    failover_instance integer DEFAULT 0,                    -- num of failovers occured
    operation integer DEFAULT 0                             -- operation that caused failover
);

