/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "uncxx/upll_log.hh"

#include "key_tree.hh"

namespace unc {
namespace upll {
namespace keytree {

using std::list;

KeyTree::KeyTree(unc_key_type_t root) : root_node_(root, NULL) {
  all_kt_map_[root] = &root_node_;
}

KeyTree::~KeyTree() {
#if 0  // Do not free as it is a multi-threaded system
  std::map<unc_key_type_t, KeyTreeNode*>::iterator ktn_it = all_kt_map_.begin();
  while (ktn_it != all_kt_map_.end()) {
    KeyTreeNode *ktn = ktn_it->second;
    delete ktn;
    ktn_it++;
  }
  all_kt_map_.clear();
#endif
}

bool KeyTree::AddKeyType(unc_key_type_t parent, unc_key_type_t child) {
  UPLL_LOG_VERBOSE("adding %d %d", parent, child);
  if (all_kt_map_[child]) {
    UPLL_LOG_DEBUG("KT child %d already exists", child);
    return false;
  } else {
    KeyTreeNode *parent_node = GetNode(parent);
    if (parent_node == NULL) {
      UPLL_LOG_DEBUG("%d does not exist", parent);
      return false;
    }

    KeyTreeNode *child_node = new KeyTreeNode(child, parent_node);
    parent_node->children.push_back(child_node);
    all_kt_map_[child] = child_node;
    UPLL_LOG_VERBOSE("added %d %d", parent, child);
  }
  return true;
}

KeyTreeNode *KeyTree::GetNode(unc_key_type_t key_type) {
  std::map<unc_key_type_t, KeyTreeNode*>::iterator it
    = all_kt_map_.find(key_type);
  if (it != all_kt_map_.end())
    return it->second;
  return NULL;
}

const KeyTreeNode *KeyTree::GetNode(unc_key_type_t key_type) const {
  std::map<unc_key_type_t, KeyTreeNode *>::const_iterator it =
      all_kt_map_.find(key_type);
  if (it != all_kt_map_.end())
    return it->second;
  return NULL;
}

bool KeyTree::GetFirstChild(unc_key_type_t parent_kt,
                            unc_key_type_t *child_kt) const {
  const KeyTreeNode *child = NULL;
  const KeyTreeNode *parent = GetNode(parent_kt);
  if (parent != NULL) {
    if (parent->children.size()) {
      child = parent->children.front();
      if (child != NULL) {
        *child_kt = child->key_type;
        return true;
      }
    }
  }
  return false;
}

KeyTreeNode *KeyTree::GetNextSibling(const KeyTreeNode *node) const {
  if (node == NULL) {
    UPLL_LOG_DEBUG("node is null");
    return NULL;
  }
  if (node->parent == NULL) {
    UPLL_LOG_VERBOSE("node parent is null");
    return NULL;
  }

  // get the next sibling for the given node
  std::list<KeyTreeNode*> &children = node->parent->children;

  for (std::list<KeyTreeNode*>::iterator it = children.begin();
       it != children.end(); ++it) {
    UPLL_LOG_VERBOSE("child in iterator: %d", (*it)->key_type);
    if ((*it)->key_type == node->key_type) {
      if (++it != children.end()) {
        UPLL_LOG_VERBOSE("found sibling: %d", (*it)->key_type);
        return (*it);
      } else {
        break;
        // get the next sibling for the given node's parent
        // return GetNextSibling(node->parent);
      }
    }
  }
  return NULL;
}

bool KeyTree::GetNextSibling(unc_key_type_t kt,
                             unc_key_type_t *next_sibling_kt) const {
  const KeyTreeNode *node = GetNextSibling(GetNode(kt));
  if (node) {
    *next_sibling_kt = node->key_type;
    return true;
  }
  return false;
}

bool KeyTree::GetParent(unc_key_type_t kt, unc_key_type_t *parent_kt) {
  KeyTreeNode *node = GetNode(kt);
  if (node) {
    node = node->parent;
    if (node) {
      *parent_kt = node->key_type;
      return true;
    }
  }
  return false;
}

bool KeyTree::IsValidKeyType(unc_key_type_t key_type) const {
  return ( (all_kt_map_.count(key_type)) ? true : false );
}

void KeyTree::PrepareOrderedList() {
  unc_key_type_t kt;
  preorder_list_.clear();
  reverse_preorder_list_.clear();
  KeyTree::PreorderIterator *it = GetPreorderIterator();
  while (it->Next(&kt)) {
    UPLL_LOG_VERBOSE("pushing kt %d", static_cast<int>(kt));
    preorder_list_.push_back(kt);
    reverse_preorder_list_.push_front(kt);
  }
  delete it;
}

// This function does not traverse root node
bool KeyTree::PreorderIterator::Next(unc_key_type_t *next_key_type) {
  if (next_key_type == NULL)
    return false;
  if (next_node_ == NULL)
    return false;

  // find next node
  if (next_node_->children.size()) {
    next_node_ = next_node_->children.front();
    UPLL_LOG_VERBOSE("found child %d", next_node_->key_type);
  } else {  // if no child, get next sibling
    KeyTreeNode *sibling;
    while (!(sibling = keytree_->GetNextSibling(next_node_))) {
      UPLL_LOG_VERBOSE("moving to the parent of %d", next_node_->key_type);
      next_node_ = next_node_->parent;
      if (next_node_ == NULL)
        break;
    }
    next_node_ = sibling;
  }

  if (next_node_) {
    // output the next key type
    *next_key_type = next_node_->key_type;
    UPLL_LOG_VERBOSE("returning %d", *next_key_type);
    return true;
  }
  return false;
}

#if 0
KeyTree::PostorderIterator::PostorderIterator(KeyTree *kt) {
  keytree_= kt;
  // set next_node_ to the left most descendant at the bottom of the tree
  next_node_ = keytree_->GetRoot();
  while (next_node_) {
    if (next_node_->children.empty())
      break;
    next_node_ = next_node_->children.front();
  }
}
#endif

#if 0
// This function does not traverse root node
bool KeyTree::PostorderIterator::Next(unc_key_type_t *next_key_type) {
  if (next_key_type == NULL)
    return false;
  if (next_node_ == NULL)
    return false;
  if (next_node_ == keytree_->GetRoot())
    return false;

  *next_key_type = next_node_->key_type;
  UPLL_LOG_VERBOSE("returning %d", *next_key_type);

  // Now set next_node_ to either sibling or to the parent if no sibling
  KeyTreeNode *next_sibling = keytree_->GetNextSibling(next_node_);
  if (next_sibling != NULL) {
    next_node_ = next_sibling;
    UPLL_LOG_VERBOSE("moving to sibling %d", next_node_->key_type);
    while (next_node_) {
      if (next_node_->children.empty())
        break;
      next_node_ = next_node_->children.front();
    }
    UPLL_LOG_VERBOSE("moving to sibling's descendant %d",
                    ((next_node_) ? next_node_->key_type : 0));
  } else {
    next_node_ = next_node_->parent;
    UPLL_LOG_VERBOSE("moving to the parent %d",
                    ((next_node_) ? next_node_->key_type : 0));
    if (next_node_ == keytree_->GetRoot()) {
      next_node_ = NULL;
    }
  }

  return true;
}
#endif

}  // namespace keytree
}  // namespace upll
}  // namespace unc
