/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include <keytree.hh>
#include <unc/keytype.h>

namespace unc {
namespace vtndrvcache {

uint32_t KeyTree::append_commit_node(ConfigNode* value_list) {
  return 0;
}

uint32_t KeyTree::append_audit_node(
  const std::vector<ConfigNode*>&value_list) {
  return 0;
}

KeyTree* KeyTree::create_cache() {
  return new KeyTree();
}


CommonIterator* KeyTree::get_iterator() {
  return new CommonIterator(this);
}
}  // namespace vtndrvcache
}  // namespace unc
