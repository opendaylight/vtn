/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *    table_downgrade.hh
 *    Contians downgrade schema information for tables
 *
 */


#ifndef _TABLE_DOWNGRADE_HH
#define _TABLE_DOWNGRADE_HH


#include <iostream>
#include <fstream>

using namespace std;

ofstream upll_downgrade_file;

std::string u12u13_error_check =
  "DO\n"
  "$check_u13_config$\n"
  "BEGIN\n"
  "IF EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name "
    "= 'su_vterminal_tbl') THEN\n"
  "  IF EXISTS (SELECT 1 FROM su_vterminal_tbl) THEN\n"
  "    RAISE EXCEPTION 'vTerminal configuration exist. "
    "Delete vTerminal configuration before doing downgrade';\n"
  "  END IF;\n"
  "END IF;\n\n"
  "IF EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = "
    "'ru_vterminal_tbl') THEN\n"
  "  IF EXISTS (SELECT 1 FROM ru_vterminal_tbl) THEN\n"
  "    RAISE EXCEPTION 'vTerminal configuration exist. "
    "Delete vTerminal configuration before doing downgrade';\n"
  "  END IF;\n"
  "END IF;\n\n"
  "IF EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = "
    "'ca_vterminal_tbl') THEN\n"
  "  IF EXISTS (SELECT 1 FROM ca_vterminal_tbl) THEN\n"
  "    RAISE EXCEPTION 'vTerminal configuration exist. "
    "Delete vTerminal configuration before doing downgrade';\n"
  "  END IF;\n"
  "END IF;\n"
  "END\n"
  "$check_u13_config$;";


#endif  //_TABLE_DOWNGRADE_HH
