/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <pfcxx/module.hh>
#include <pfc/rbtree.h>
#include <pfc/synch.h>
#include <pfc/debug.h>
#include <pfc/log.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>

namespace pfc {
namespace core {

/*
 * Return module instance associated with the specified module descriptor.
 */
#define PMODULE_INSTANCE(mod)\
  (reinterpret_cast<Module*>(PMODULE_OBJECT_PTR(mod)))


  /*
   * * Module object entry which keep a pair of module class pointer type name and
   * * module instance. This must be defined as POD because it is passed to
   * * offsetof().
   * */
  extern "C" {
    typedef struct {
      const char *mo_name;  /* type identifier */
      pfc_ptr_t mo_object;  /* pointer to module instance */
      pfc_rbnode_t mo_node; /* Red-Black Tree node */
    } module_object_t;
  }

#define PMODULE_OBJECT_NODE2PTR(node)\
  PFC_CAST_CONTAINER((node), module_object_t, mo_node)

  /*
   * * Module::Module(const pfc_modattr_t *attr)
   * * Constructor of abstract module class.
   * */
  Module::Module(const pfc_modattr_t *attr)
      : _attr(attr), _name(_attr->pma_name) {}

  /*
   * * Module::~Module(void)
   * * Destructor of abstract module class.
   * */
  Module::~Module(void) {}


}  // namespace core
}  // namespace pfc
