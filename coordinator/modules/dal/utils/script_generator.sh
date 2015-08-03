#
# Copyright (c) 2014-2015 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

#/*
# * script_generator.sh
# *   Creates SQL file with Upgrade, Downgrade, Create and Delete
# *   Copies the create SQL file to /dev/src/sql/
# *   Copies the upgrade SQL file to /dev/src/sql/upgrade/
# *   Copies the downgrade SQL file to /dev/src/sql/downgrade/
# *   Copies the delete SQL file to current working directory
# * Usage : sh table_creation.sh available_version(U13/U14/...)


#!/bin/sh

version=${1^^}  #converting string to uppercase

if [ "$version" == "U13" ] \
   || [ "$version" == "U14" ] \
   || [ "$version" == "U16" ] \
   || [ "$version" == "U17" ];
then
  make clean;
  make all;
  ./upll_upgrade_table.exe $version
  ./upll_downgrade_table.exe $version
  ./upll_create_table.exe $version
  ./upll_optimize_table.exe $version
  ./upll_delete_table.exe
else
  echo ""
  echo "<< Please enter the correct available version >>"
  echo "**  usage: $0 version(U13/U14/U16/U17...) **"
  echo ""
fi

rm -rf *.exe
