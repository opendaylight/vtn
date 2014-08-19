@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

REM
REM Copyright (c) 2014 NEC Corporation
REM All rights reserved.
REM
REM This program and the accompanying materials are made available under the
REM terms of the Eclipse Public License v1.0 which accompanies this
REM distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
REM

SET RUN_CONTROLLER=run_controller.bat
SET CONFDIR=configuration
SET CONF_INITIAL=%CONFDIR%\initial
SET CONF_INIT_AVAIL=%CONFDIR%\initial.available

SET OF13=0
SET ARGUMENTS=

:ARGLOOP
IF "%~1" NEQ "" (
    SET arg=%~1
    IF "!arg!"=="-of10" (
        SET OF13=0
        GOTO :NEXTARG
    )
    IF "!arg!"=="-of13" (
        SET OF13=1
        GOTO :NEXTARG
    )
    SET ARGUMENTS= !ARGUMENTS! %1

:NEXTARG
    SHIFT
    GOTO :ARGLOOP
)

REM Clean up optional configuration files.
FOR /F "usebackq" %%f IN (`DIR /B "%CONF_INIT_AVAIL%"`) DO (
    SET cfile=%CONF_INITIAL%\%%f
    IF EXIST !cfile! DEL !cfile!
)

REM Set up openflow plugin.
SET FILTER="^^(?^!org\.opendaylight\.(openflowplugin^|openflowjava^|controller\.sal-compatibility)).^*"

IF "%OF13%" NEQ "0" (
    SET FILTER="^^(?^!org\.opendaylight\.controller\.(thirdparty\.org\.openflow^|protocol_plugins\.openflow)).^*"
    FOR /F "usebackq" %%f IN (`DIR /B "%CONF_INIT_AVAIL%\*.xml"`) DO (
        COPY %CONF_INIT_AVAIL%\%%f %CONF_INITIAL%\%%f
    )
)

%RUN_CONTROLLER% "-Dfelix.fileinstall.filter=!FILTER!" %ARGUMENTS%
