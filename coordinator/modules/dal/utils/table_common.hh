/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *   table_common.hh
 *   Contians common accessible functions prototypes to generate schema
 *
 */

#ifndef _TABLE_COMMON_HH_
#define _TABLE_COMMON_HH_


std::string default_rows_ =
  "\n      '(' || quote_literal('save_op_version')  || ', ' || quote_literal('0') || '),'"
  "\n      '(' || quote_literal('abort_op_version') || ', ' || quote_literal('0') || ');';\n";


#endif  // _TABLE_COMMON_HH_

