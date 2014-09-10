/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __FLOWFILTERLIST_HH__
#define __FLOWFILTERLIST_HH__
#include <string>
#include <stdio.h>
#include <list>
#include <odc_rest.hh>
using namespace unc::restjson;
namespace unc {
namespace odcdriver {

struct destination {
public:
  std::string tenant_;
  std::string bridge_;
  std::string router_;
  std::string terminal_;
  std::string interface_;
  destination ():
    tenant_ (""),
    bridge_ (""),
    router_ (""),
    terminal_ (""),
    interface_ ("")
  {}

  ~destination () {
  }

};

struct dldst {
public:
  std::string address_;
  dldst ():
    address_ ("")
  {}

  ~dldst () {
  }

};

struct dlsrc {
public:
  std::string address_;
  dlsrc ():
    address_ ("")
  {}

  ~dlsrc () {
  }

};

struct drop {
public:
  drop () {}

  ~drop () {
  }

};

struct dscp {
public:
  int dscp_;
  dscp ():
    dscp_ (-1)
  {}

  ~dscp () {
  }

};





struct icmpcode {
public:
  int code_;
  icmpcode ():
    code_ (-1)
  {}

  ~icmpcode () {
  }

};

struct icmptype {
public:
  int type_;
  icmptype ():
    type_ (-1)
  {}

  ~icmptype () {
  }

};

struct inet4dst {
public:
  std::string address_;
  inet4dst ():
    address_ ("")
  {}

  ~inet4dst () {
  }

};

struct inet4src {
public:
  std::string address_;
  inet4src ():
    address_ ("")
  {}

  ~inet4src () {
  }

};

struct pass {
public:
  pass ()
  {}

  ~pass () {
  }

};

struct redirect {
public:
  destination*  destination_;
  bool output_;
  redirect ():
    destination_ (NULL),
    output_ (false)
  {}

  ~redirect () {
    if (destination_)
      delete destination_;
  }

};

struct tpdst {
public:
  int type_;
  tpdst ():
    type_ (-1)
  {}

  ~tpdst () {
  }

};

struct tpsrc {
public:
  int type_;
  tpsrc ():
    type_ (-1)
  {}

  ~tpsrc () {
  }

};

struct vlanpcp {
public:
  int priority_;
  vlanpcp ():
    priority_ (-1)
  {}

  ~vlanpcp () {
  }

};

struct filterType {
public:
  pass*  pass_;
  drop*  drop_;
  redirect*  redirect_;
  filterType ():
    pass_ (NULL),
    drop_ (NULL),
    redirect_ (NULL)
  {}

  ~filterType () {
    if (pass_)
      delete pass_;
    if (drop_)
      delete drop_;
    if (redirect_)
      delete redirect_;
  }

};

struct action {
public:
  dlsrc*  dlsrc_;
  dldst*  dldst_;
  vlanpcp*  vlanpcp_;
  inet4src*  inet4src_;
  inet4dst*  inet4dst_;
  dscp*  dscp_;
  tpsrc*  tpsrc_;
  tpdst*  tpdst_;
  icmpcode*  icmpcode_;
  icmptype*  icmptype_;
  action ():
    dlsrc_ (NULL),
    dldst_ (NULL),
    vlanpcp_ (NULL),
    inet4src_ (NULL),
    inet4dst_ (NULL),
    dscp_ (NULL),
    tpsrc_ (NULL),
    tpdst_ (NULL),
    icmpcode_ (NULL),
    icmptype_ (NULL)
  {}

  ~action () {
    if (dlsrc_)
      delete dlsrc_;
    if (dldst_)
      delete dldst_;
    if (vlanpcp_)
      delete vlanpcp_;
    if (inet4src_)
      delete inet4src_;
    if (inet4dst_)
      delete inet4dst_;
    if (dscp_)
      delete dscp_;
    if (tpsrc_)
      delete tpsrc_;
    if (tpdst_)
      delete tpdst_;
    if (icmpcode_)
      delete icmpcode_;
    if (icmptype_)
      delete icmptype_;
  }

};

struct flowfilter {
public:
  int index_;
  std::string condition_;
  filterType*  filterType_;
  std::list<action*> action_;
  flowfilter ():
    index_ (0),
    condition_ (""),
    filterType_ (NULL)
  {}

  ~flowfilter () {
    if (filterType_)
      delete filterType_;
    std::list<action*>::iterator iter=action_.begin();
    while ( iter != action_.end () ) {
      if (*iter)
        delete (*iter);
      iter++;
    }
  }

};


struct flowfilterlist {
public:
  std::list<flowfilter*> flowfilter_;
  flowfilterlist () {}

  ~flowfilterlist () {
    std::list<flowfilter*>::iterator iter=flowfilter_.begin();
    while ( iter != flowfilter_.end () ) {
      if (*iter)
        delete (*iter);
      iter++;
    }
  }

};

class flowfilterlistUtil {
public:
  UncRespCode GetValue( json_object* in,dlsrc* out);
  UncRespCode SetValue( json_object* out,dlsrc* in);
  UncRespCode GetValue( json_object* in,dldst* out);
  UncRespCode SetValue( json_object* out,dldst* in);
  UncRespCode GetValue( json_object* in,vlanpcp* out);
  UncRespCode SetValue( json_object* out,vlanpcp* in);
  UncRespCode GetValue( json_object* in,inet4src* out);
  UncRespCode SetValue( json_object* out,inet4src* in);
  UncRespCode GetValue( json_object* in,inet4dst* out);
  UncRespCode SetValue( json_object* out,inet4dst* in);
  UncRespCode GetValue( json_object* in,dscp* out);
  UncRespCode SetValue( json_object* out,dscp* in);
  UncRespCode GetValue( json_object* in,tpsrc* out);
  UncRespCode SetValue( json_object* out,tpsrc* in);
  UncRespCode GetValue( json_object* in,tpdst* out);
  UncRespCode SetValue( json_object* out,tpdst* in);
  UncRespCode GetValue( json_object* in,icmpcode* out);
  UncRespCode SetValue( json_object* out,icmpcode* in);
  UncRespCode GetValue( json_object* in,icmptype* out);
  UncRespCode SetValue( json_object* out,icmptype* in);
  UncRespCode GetValue( json_object* in,pass* out);
  UncRespCode SetValue( json_object* out,pass* in);
  UncRespCode GetValue( json_object* in,drop* out);
  UncRespCode SetValue( json_object* out,drop* in);
  UncRespCode GetValue( json_object* in,redirect* out);
  UncRespCode SetValue( json_object* out,redirect* in);
  UncRespCode GetValue( json_object* in,filterType* out);
  UncRespCode SetValue( json_object* out,filterType* in);
  UncRespCode GetValue( json_object* in,destination* out);
  UncRespCode SetValue( json_object* out,destination* in);
  UncRespCode GetValue( json_object* in,flowfilterlist* out);
  UncRespCode SetValue( json_object* out,flowfilterlist* in);
  UncRespCode GetValue( json_object* in,action* out);
  UncRespCode SetValue( json_object* out,action* in);
  UncRespCode GetValue( json_object* in,flowfilter* out);
  UncRespCode SetValue( json_object* out,flowfilter* in);
};

}
}
#endif
