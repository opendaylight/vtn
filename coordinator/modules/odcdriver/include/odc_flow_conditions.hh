/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __FLOWCONDITIONS_HH__
#define __FLOWCONDITIONS_HH__
#include <string>
#include <list>
#include <odc_rest.hh>
namespace unc {
namespace odcdriver {
using namespace unc::restjson;

struct dst {
public:
  int from_;
  int to_;
  dst ():
    from_ (-1),
    to_ (-1)
  {}

  ~dst () {
  }

};

struct ethernet {
public:
  int type_;
  int vlan_;
  int vlanpri_;
  std::string src_;
  std::string dst_;
  ethernet ():
    type_ (-1),
    vlan_ (-1),
    vlanpri_ (-1),
    src_ (""),
    dst_ ("")
  {}

  ~ethernet () {
  }

};


struct icmp {
public:
  int type_;
  int code_;
  icmp ():
    type_ (-1),
    code_ (-1)
  {}

  ~icmp () {
  }

};

struct inet4 {
public:
  int srcsuffix_;
  int dstsuffix_;
  int protocol_;
  int dscp_;
  std::string src_;
  std::string dst_;
  inet4 ():
    srcsuffix_ (0),
    dstsuffix_ (0),
    protocol_ (-1),
    dscp_ (-1),
    src_ (""),
    dst_ ("")
  {}

  ~inet4 () {
  }

};

struct inetMatch {
public:
  inet4*  inet4_;
  inetMatch ():
    inet4_ (NULL)
  {}

  ~inetMatch () {
    if (inet4_)
      delete inet4_;
  }

};



struct src {
public:
  int from_;
  int to_;
  src ():
    from_ (-1),
    to_ (-1)
  {}

  ~src () {
  }

};

struct tcp {
public:
  src*  src_;
  dst*  dst_;
  tcp ():
    src_ (NULL),
    dst_ (NULL)
  {}

  ~tcp () {
    if (src_)
      delete src_;
    if (dst_)
      delete dst_;
  }

};

struct udp {
public:
  src*  src_;
  dst*  dst_;
  udp ():
    src_ (NULL),
    dst_ (NULL)
  {}

  ~udp () {
    if (src_)
      delete src_;
    if (dst_)
      delete dst_;
  }

};

struct l4Match {
public:
  tcp*  tcp_;
  udp*  udp_;
  icmp*  icmp_;
  l4Match ():
    tcp_ (NULL),
    udp_ (NULL),
    icmp_ (NULL)
  {}

  ~l4Match () {
    if (tcp_)
      delete tcp_;
    if (udp_)
      delete udp_;
    if (icmp_)
      delete icmp_;
  }
};

struct match {
public:
  int index_;
  ethernet*  ethernet_;
  inetMatch*  inetMatch_;
  l4Match*  l4Match_;
  match ():
    index_ (0),
    ethernet_ (NULL),
    inetMatch_ (NULL),
    l4Match_ (NULL)
  {}

  ~match () {
    if (ethernet_)
      delete ethernet_;
    if (inetMatch_)
      delete inetMatch_;
    if (l4Match_)
      delete l4Match_;
  }

};

struct flowcondition {
public:
  std::string name_;
  std::list<struct match*> match_;
  flowcondition ():
    name_ ("")
  {}

  ~flowcondition () {
    std::list<struct match*>::iterator iter=match_.begin();
    while ( iter != match_.end () ) {
      if (*iter)
        delete *iter;
      iter++;
    }
  }
};

struct flowConditions {
public:
  std::list<flowcondition*> flowcondition_;
  flowConditions ()
  {}

  ~flowConditions () {
    std::list<flowcondition*>::iterator iter=flowcondition_.begin();
    while ( iter != flowcondition_.end () ) {
      if (*iter)
        delete *iter;
      iter++;
    }
  }

};

class flowConditionsUtil {
public:
  UncRespCode GetValue( json_object* in,inet4* out);
  UncRespCode SetValue( json_object* out,inet4* in);
  UncRespCode GetValue( json_object* in,tcp* out);
  UncRespCode SetValue( json_object* out,tcp* in);
  UncRespCode GetValue( json_object* in,udp* out);
  UncRespCode SetValue( json_object* out,udp* in);
  UncRespCode GetValue( json_object* in,icmp* out);
  UncRespCode SetValue( json_object* out,icmp* in);
  UncRespCode GetValue( json_object* in,ethernet* out);
  UncRespCode SetValue( json_object* out,ethernet* in);
  UncRespCode GetValue( json_object* in,inetMatch* out);
  UncRespCode SetValue( json_object* out,inetMatch* in);
  UncRespCode GetValue( json_object* in,l4Match* out);
  UncRespCode SetValue( json_object* out,l4Match* in);
  UncRespCode GetValue( json_object* in,src* out);
  UncRespCode SetValue( json_object* out,src* in);
  UncRespCode GetValue( json_object* in,dst* out);
  UncRespCode SetValue( json_object* out,dst* in);
  UncRespCode GetValue( json_object* in,flowcondition* out);
  UncRespCode SetValue( json_object* in,flowcondition* out);
  UncRespCode GetValue( json_object* in,flowConditions* out);
  UncRespCode SetValue( json_object* in,flowConditions* out);
  UncRespCode GetValue( json_object* in,match* out);
  UncRespCode SetValue( json_object* in,match* out);
};

}
}
#endif
