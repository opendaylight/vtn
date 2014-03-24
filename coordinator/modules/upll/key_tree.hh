/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_KEY_TREE_HH_
#define UPLL_KEY_TREE_HH_

#include <string>
#include <list>
#include <map>

#include "unc/keytype.h"

namespace unc {
namespace upll {
namespace keytree {

/**
 * NOTE: KeyTree is assumed to be created at the time of initialization of the
 * process and never modified after that. For this reason KeyTree is not
 * implemented as thread safe.
 */

struct KeyTreeNode {
  unc_key_type_t key_type;
  KeyTreeNode *parent;
  std::list<KeyTreeNode*> children;
  KeyTreeNode(unc_key_type_t key_type, KeyTreeNode *parent) {
    this->key_type = key_type;
    this->parent = parent;
  }
};

class KeyTree {
 public:
  friend class PreorderIterator;
#if 0
  friend class PostorderIterator;
#endif

  class PreorderIterator {
   public:
    bool Next(unc_key_type_t *next_key_type);
    explicit PreorderIterator(KeyTree *kt) {
      keytree_= kt;
      next_node_ = keytree_->GetRoot();
    }
   private:
    KeyTree *keytree_;
    KeyTreeNode *next_node_;
  };

#if 0
  class PostorderIterator {
   public:
    bool Next(unc_key_type_t *next_key_type);
    explicit PostorderIterator(KeyTree *kt);
   private:
    KeyTree *keytree_;
    KeyTreeNode *next_node_;
  };
#endif


  explicit KeyTree(unc_key_type_t root);
  virtual ~KeyTree();
  bool AddKeyType(unc_key_type_t parent, unc_key_type_t child);
  bool IsValidKeyType(unc_key_type_t key_type) const;
  void PrepareOrderedList();
  const std::list<unc_key_type_t> *get_preorder_list() {
    return &preorder_list_;
  }
  const std::list<unc_key_type_t> *get_reverse_postorder_list() {
    return &reverse_preorder_list_;
  }
  bool GetFirstChild(unc_key_type_t parent_kt, unc_key_type_t *child_kt) const;
  bool GetNextSibling(unc_key_type_t kt, unc_key_type_t *next_sibling_kt) const;
  bool GetParent(unc_key_type_t kt, unc_key_type_t *parent_kt);
  inline PreorderIterator *GetPreorderIterator() {
    return (new PreorderIterator(this));
  }
#if 0
  inline PostorderIterator *GetPostorderIterator() {
    return (new PostorderIterator(this));
  }
#endif

 private:
  KeyTreeNode *GetRoot() { return &root_node_; }
  KeyTreeNode *GetNode(unc_key_type_t key_type);
  const KeyTreeNode *GetNode(unc_key_type_t key_type) const;
  KeyTreeNode *GetNextSibling(const KeyTreeNode *child) const;
  KeyTreeNode root_node_;
  std::map<unc_key_type_t, KeyTreeNode*> all_kt_map_;
  std::list<unc_key_type_t> preorder_list_;
  std::list<unc_key_type_t> reverse_preorder_list_;
};

}  // namespace keytree
}  // namespace upll
}  // namespace unc

#endif  // UPLL_KEY_TREE_HH_
