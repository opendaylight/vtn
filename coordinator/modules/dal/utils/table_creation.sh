#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

#/*
# * table_creation.sh
# *   Creates SQL file with table creation queries
# *   Copies the SQL file to /dev/src/sql/
# * Usage : sh table_creation.sh
# */

make clean;
make all;
./upll_create_table.exe > upll_create_table.sql
cp upll_create_table.sql ../../../sql/
