#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of Upll module.
##

#include ../defs.mk

ODBCINC = /usr/local/include

INC =	-I$(CURDIR)/../.. \
	-I$(ODBCINC)

CREATE_SRC =	../dal_schema.cc \
		table_creation.cc

DELETE_SRC =	../dal_schema.cc \
		table_deletion.cc

UPGRADE_SRC =	../dal_schema.cc \
		table_upgrade.cc

DOWNGRADE_SRC =	../dal_schema.cc \
		table_downgrade.cc

OPTIMIZE_SRC =  ../dal_schema.cc \
                table_optimize.cc

OPTS = -m64 -g -Wall -Wextra

CREATE_EXE = upll_create_table.exe
DELETE_EXE = upll_delete_table.exe
UPGRADE_EXE = upll_upgrade_table.exe
DOWNGRADE_EXE = upll_downgrade_table.exe
OPTIMIZE_EXE = upll_optimize_table.exe

all: $(CREATE_EXE) $(DELETE_EXE) $(UPGRADE_EXE) $(DOWNGRADE_EXE) $(OPTIMIZE_EXE)

$(CREATE_EXE): $(SRC)
	g++ $(CREATE_SRC) $(INC) $(OPTS) -o$(CREATE_EXE)

$(DELETE_EXE): $(SRC)
	g++ $(DELETE_SRC) $(INC) $(OPTS) -o$(DELETE_EXE)

$(UPGRADE_EXE): $(SRC)
	g++ $(UPGRADE_SRC) $(INC) $(OPTS) -o$(UPGRADE_EXE)

$(DOWNGRADE_EXE): $(SRC)
	g++ $(DOWNGRADE_SRC) $(INC) $(OPTS) -o$(DOWNGRADE_EXE)

$(OPTIMIZE_EXE): $(SRC)
	g++ $(OPTIMIZE_SRC) $(INC) $(OPTS) -o$(OPTIMIZE_EXE)

clean:
	rm -f $(CREATE_EXE) $(DELETE_EXE) $(UPGRADE_EXE) $(DOWNGRADE_EXE) $(OPTIMIZE_EXE) *.sql

#include ../rules.mk
