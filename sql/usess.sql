--
-- Copyright (c) 2012-2013 NEC Corporation
-- All rights reserved.
-- 
-- This program and the accompanying materials are made available under the
-- terms of the Eclipse Public License v1.0 which accompanies this
-- distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
--

CREATE TABLE tbl_unc_usess_user (
    uname       varchar(32) NOT NULL,                           -- user name
    passwd_hash int4        NOT NULL,                           -- passwoed hash type.
    passwd      varchar     NOT NULL,                           -- password(hash format).
    usertype    int4        NOT NULL,                           -- user type.
    expiration  date,                                           -- expiration date(Unused).
    created     timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP, -- date of create.
    modified    timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP, -- date of modified.

    PRIMARY KEY(uname)
);

CREATE TABLE tbl_unc_usess_enable (
    mode        int4        NOT NULL,                           -- mode kind.
    passwd_hash int4        NOT NULL,                           -- passwoed hash type.
    passwd      varchar     NOT NULL,                           -- password(hash format).
    created     timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP, -- date of create.
    modified    timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP, -- date of modified.

    PRIMARY KEY(mode)
);


-- tbl_unc_usess_user initialize.
INSERT INTO tbl_unc_usess_user 
    (uname, passwd_hash, passwd, usertype, expiration, created, modified)
    VALUES ('UNC_CLI_ADMIN', 6, '$6$40081705$GCCJKhuVOFT8i97mtM.SyLPFPTmgr2EAQ28Ot6x3Y5gxEVkpQ.YiqKN60RIOlDQVIxeQmrsGIOpbXYUKlaov4.', 2, NULL, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);

INSERT INTO tbl_unc_usess_user 
    (uname, passwd_hash, passwd, usertype, expiration, created, modified)
    VALUES ('UNC_WEB_ADMIN', 6, '$6$05200703$AaQ4lLgaRr397St9Y4I40jvhQMgAqWTmx5YeDL4NeLtYDkMyB8PT8UPSULpKDBDmTjra.OsSMi38fdUj2UyN91', 2, NULL, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);

INSERT INTO tbl_unc_usess_user 
    (uname, passwd_hash, passwd, usertype, expiration, created, modified)
    VALUES ('UNC_WEB_OPER', 6, '$6$05072017$Hf3uUjQB0t/5s.mJTnQPqxJb2f0le5MsaphYCM8zQc.zYxDstlUnLQu1UWJcDUfmkTJ4QiNeTPPBfrI3eTEDD.', 1, NULL, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);

-- tbl_unc_usess_enable initialize.
INSERT INTO tbl_unc_usess_enable
    (mode, passwd_hash, passwd, created, modified)
    VALUES (2, 6, '$6$20130517$65c80/Ll8Brma/dmODecE9iUJu3I5wYSCeF.fbyB9PglGFHid64XGMaCC6XKwIvC0P35JxDnABGbfebZWp3B8.', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);

\q
