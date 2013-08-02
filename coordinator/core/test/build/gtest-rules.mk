#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build unit test program using Google Test.
## This makefile is designed to be included after gtest-defs.mk is included.
##

ifdef	GTEST_CONFIG

include $(BLDDIR)/exec-rules.mk

TEST_PROGRAM	= $(OBJ_EXEC)

# Run tests.
check test run:	all FRC
	$(GTEST_ENV) $(TEST_PROGRAM) $(GTEST_FLAGS)

# List test cases.
list:	all
	$(TEST_PROGRAM) --gtest_list_tests $(GTEST_FLAGS)

install:	all

ifdef	TARGET_BLDDIR

ut-rebuild:	FRC
	@for d in $(TARGET_BLDDIR); do					\
	  echo "=== Rebuilding $$d for unit test.";			\
	  $(MAKE) -C $$d DEBUG_BUILD=1 UNIT_TEST=1 || exit 1;		\
	done

# Invoke test with object for unit test.
unit-test ut:	ut-rebuild all
	$(GTEST_ENV) $(TEST_PROGRAM) $(GTEST_FLAGS)

else	# !TARGET_BLDDIR

unit-test ut ut-rebuild:

endif	# TARGET_BLDDIR

else	# !GTEST_CONFIG

GTEST_SKIP_REASON	?= Google Test is not installed.

# Do nothing on "make all".
all:

clean clobber:
	$(RM) -rf $(OBJDIR)

check test list unit-test ut:
	@echo "*** SKIP: $(GTEST_SKIP_REASON)"

.PHONY:	all clean clobber check test

endif	# GTEST_CONFIG
