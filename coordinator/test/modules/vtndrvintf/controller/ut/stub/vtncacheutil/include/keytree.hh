/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef KEYTREE_CONFIG_HH_
#define KEYTREE_CONFIG_HH_

#include <confignode.hh>
#include <vector>
#include <utility>
#include <map>
#include <string>

namespace unc {
namespace vtndrvcache {

class CommonIterator;

class KeyTree {
 public:
  friend class CommonIterator;

  static KeyTree* create_cache();

  uint32_t append_audit_node(const std::vector<ConfigNode*>&value_list);

  uint32_t append_commit_node(ConfigNode* value_list);

  CommonIterator* get_iterator();

  ConfigNode* operator[](int itemIndex) {
    return cfg_node_list_[itemIndex];
  }

  uint32_t get_nodelist_keytree(std::vector<ConfigNode*>&value_list) {
    return 0;
  }

  int cfg_list_count() {
    return cfg_node_list_.size();
  }

  typedef std::vector<ConfigNode*> Config_node_list;
  Config_node_list cfg_node_list_;
  RootNode node_tree_;
};

class CommonIterator {
  KeyTree *aggregate_;
  int currentIndex_;

 public:
  explicit CommonIterator(KeyTree *aggregate)
      : aggregate_(aggregate)
        , currentIndex_(0) {
        }
  ConfigNode* FirstItem() {
    currentIndex_ = 0;
    return (*aggregate_)[currentIndex_];
  }


  ConfigNode* NextItem() {
    currentIndex_ += 1;

    if (IsDone() == false) {
      return (*aggregate_)[currentIndex_];
    } else {
      return NULL;
    }
  }

  ConfigNode* AuditFirstItem() {
    currentIndex_ = 0;
    aggregate_->get_nodelist_keytree(aggregate_->cfg_node_list_);
    return (*aggregate_)[currentIndex_];
  }

  ConfigNode* CurrentItem() {
    return (*aggregate_)[currentIndex_];
  }

  bool IsDone() {
    if (currentIndex_ < aggregate_->cfg_list_count()) {
      return false;
    }
    return true;
  }
};
}  // namespace vtndrvcache
}  // namespace unc
#endif


