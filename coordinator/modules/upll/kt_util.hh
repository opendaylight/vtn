/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_KT_UTIL_HH_
#define UPLL_KT_UTIL_HH_

#include <string>
#include <vector>
#include <map>

#include "unc/keytype.h"
#include "ipct_st.hh"

namespace unc {
namespace upll {
namespace ipc_util {
using std::string;

typedef struct controller_domain {
  uint8_t *ctrlr;
  uint8_t *domain;
} controller_domain_t;

class KtUtil {
 public:
  static const uint32_t kCtrlrNameLenWith0 = 32;
  static const uint32_t kDrvDomainNameLenWith0 = 41;


  struct KtMsgTemplate {
    // kt_cfg_msg: CANDIDATE, RUNNING, STARTUP
    // Operations: CREATE, DELETE, UPDATE, READ*
    std::vector<IpctSt::IpcStructNum> kt_cfg_msg;   // CANDIDATE/RUNNING/STARTUP
    std::vector<IpctSt::IpcStructNum> kt_import_msg;   // IMPORT
    std::vector<IpctSt::IpcStructNum> kt_cfg_st_msg;   // STATE
    std::vector<IpctSt::IpcStructNum> kt_cfg_ctl_msg;  // Control op
    std::vector<IpctSt::IpcStructNum> kt_cfg_rename_msg;  // rename msg
  };

  static KtUtil *GetKtUtil() {
    if (!singleton_instance_) {
      singleton_instance_ = new KtUtil();
      singleton_instance_->Init();
    }
    return singleton_instance_;
  }

  static const std::vector<IpctSt::IpcStructNum>& GetCfgMsgTemplate(
      unc_key_type_t kt);
  static std::string KtStructToStr(IpctSt::IpcStructNum stnum, void *data);

  // prototype_declaration
  static string IpcStructToStr(const key_unified_nw &key_unified_nw);
  static string IpcStructToStr(const val_unified_nw &val_unified_nw);
  static string IpcStructToStr(const key_unw_label &key_unw_label);
  static string IpcStructToStr(const val_unw_label &val_unw_label);
  static string IpcStructToStr(const key_unw_label_range &key_unw_label_range);
  static string IpcStructToStr(const val_unw_label_range &val_unw_label_range);
  static string IpcStructToStr(const key_unw_spine_domain &key_spine_domain);
  static string IpcStructToStr(const val_unw_spine_domain &val_spine_domain);
  static string IpcStructToStr(const val_unw_spdom_ext &val_spdom_ext);
  static string IpcStructToStr(const val_spdom_st &val_spdom_st);
  static string IpcStructToStr(const val_unw_spine_domain_st &val_spine_st);
  static string IpcStructToStr(const val_unw_spine_domain_assigned_label
                                        &val_unw_spine_domain_assigned_label);
  static string IpcStructToStr(const val_unw_spine_domain_fdbentry
                                        &val_unw_spine_domain_fdbentry);
  static string IpcStructToStr(const val_unw_spine_domain_fdbentry_vtn
                                        &val_unw_spine_domain_fdbentry_vtn);
  static string IpcStructToStr(const key_vtn_unified &key_vtn_unified);
  static string IpcStructToStr(const val_vtn_unified &val_vtn_unified);
  static string IpcStructToStr(const val_ping &val_ping_t);
  static string IpcStructToStr(const val_vtn_neighbor &val_vtn_neighbor_t);
  static string IpcStructToStr(const key_vtn &key_vtn);
  static string IpcStructToStr(const val_vtn &val_vtn);
  static string IpcStructToStr(const val_rename_vtn &val_rename_vtn);
  static string IpcStructToStr(const val_vtn_st &val_vtn_st);
  static string IpcStructToStr(const key_vtn_controller &key_vtn_controller);
  static string IpcStructToStr(const val_vtn_mapping_controller_st &val_st);
  static string IpcStructToStr(const key_vtnstation_controller &key_controller);
  static string IpcStructToStr(const val_vtnstation_controller_st &val__st);
  static string IpcStructToStr(const val_vtnstation_controller_stat &val__st);
  static string IpcStructToStr(const key_vbr &key_vbr);
  static string IpcStructToStr(const val_vbr &val_vbr);
  static string IpcStructToStr(const val_rename_vbr &val_rename_vbr);
  static string IpcStructToStr(const val_vbr_st &val_vbr_st);
  static string IpcStructToStr(const val_vbr_l2_domain_st &val_st);
  static string IpcStructToStr(const val_vbr_l2_domain_member_st &val_st);
  static string IpcStructToStr(const val_vbr_mac_entry_st &val__st);
  static string IpcStructToStr(const key_vbr_if &key_vbr_if);
  static string IpcStructToStr(const val_port_map &val_port_map);
  static string IpcStructToStr(const val_vbr_if &val_vbr_if);
  static string IpcStructToStr(const val_vbr_if_st &val_vbr_if_st);
  static string IpcStructToStr(const key_vlan_map &key_vlan_map);
  static string IpcStructToStr(const val_vlan_map &val_vlan_map);
  static string IpcStructToStr(const key_vbr_portmap &key_vbr_portmap);
  static string IpcStructToStr(const val_vbr_portmap &val_vbr_portmap);
  static string IpcStructToStr(const val_vbr_portmap_st &val_vbr_portmap_st);
  static string IpcStructToStr(const key_vrt &key_vrt);
  static string IpcStructToStr(const val_vrt &val_vrt);
  static string IpcStructToStr(const val_rename_vrt &val_rename_vrt);
  static string IpcStructToStr(const val_vrt_st &val_vrt_st);
  static string IpcStructToStr(const val_vrt_dhcp_relay_st &val_relay_st);
  static string IpcStructToStr(const val_dhcp_relay_if_st &val_st);
  static string IpcStructToStr(const val_vrt_arp_entry_st &valst);
  static string IpcStructToStr(const val_vrt_ip_route_st &val_st);
  static string IpcStructToStr(const key_vrt_if &key_vrt_if);
  static string IpcStructToStr(const val_vrt_if &val_vrt_if);
  static string IpcStructToStr(const val_vrt_if_st &val_vrt_if_st);
  static string IpcStructToStr(const key_static_ip_route &key_route);
  static string IpcStructToStr(const val_static_ip_route &val_route);
  static string IpcStructToStr(const key_dhcp_relay_if &key_if);
  static string IpcStructToStr(const val_dhcp_relay_if &val_if);
  static string IpcStructToStr(const key_dhcp_relay_server &keyserver);
  static string IpcStructToStr(const val_dhcp_relay_server &val_server);
  static string IpcStructToStr(const key_vterm &key_vterm);
  static string IpcStructToStr(const val_vterm &val_vterm);
  static string IpcStructToStr(const val_rename_vterm &val_rename_vterm);
  static string IpcStructToStr(const val_vterm_st &val_vterm_st);
  static string IpcStructToStr(const key_vterm_if &key_vterm_if);
  static string IpcStructToStr(const val_vterm_if &val_vterm_if);
  static string IpcStructToStr(const val_vterm_if_st &val_vterm_if_st);
  static string IpcStructToStr(const key_nwm &key_nwm);
  static string IpcStructToStr(const val_nwm &val_nwm);
  static string IpcStructToStr(const val_nwm_st &val_nwm_st);
  static string IpcStructToStr(const val_nwm_host_st &val_nwm_host_st);
  static string IpcStructToStr(const key_nwm_host &key_nwm_host);
  static string IpcStructToStr(const val_nwm_host &val_nwm_host);
  static string IpcStructToStr(const key_vtep &key_vtep);
  static string IpcStructToStr(const val_vtep &val_vtep);
  static string IpcStructToStr(const val_vtep_st &val_vtep_st);
  static string IpcStructToStr(const key_vtep_if &key_vtep_if);
  static string IpcStructToStr(const val_vtep_if &val_vtep_if);
  static string IpcStructToStr(const val_vtep_if_st &val_vtep_if_st);
  static string IpcStructToStr(const key_vtep_grp &key_vtep_grp);
  static string IpcStructToStr(const val_vtep_grp &val_vtep_grp);
  static string IpcStructToStr(const key_vtep_grp_member &key_vtep_grp_member);
  static string IpcStructToStr(const val_vtep_grp_member &val_vtep_grp_member);
  static string IpcStructToStr(const key_vtunnel &key_vtunnel);
  static string IpcStructToStr(const val_vtunnel &val_vtunnel);
  static string IpcStructToStr(const val_vtunnel_st &val_vtunnel_st);
  static string IpcStructToStr(const key_vtunnel_if &key_vtunnel_if);
  static string IpcStructToStr(const val_vtunnel_if &val_vtunnel_if);
  static string IpcStructToStr(const val_vtunnel_if_st &val_vtunnel_if_st);
  static string IpcStructToStr(const key_vunknown &key_vunknown);
  static string IpcStructToStr(const val_vunknown &val_vunknown);
  static string IpcStructToStr(const key_vunk_if &key_vunk_if);
  static string IpcStructToStr(const val_vunk_if &val_vunk_if);
  static string IpcStructToStr(const key_vlink &key_vlink);
  static string IpcStructToStr(const val_vlink &val_vlink);
  static string IpcStructToStr(const val_vlink_st &val_vlink_st);
  static string IpcStructToStr(const val_rename_vlink &val_rename_vlink);
  static string IpcStructToStr(const key_flowlist &key_flowlist);
  static string IpcStructToStr(const val_flowlist &val_flowlist);
  static string IpcStructToStr(const val_rename_flowlist &val_rename_flowlist);
  static string IpcStructToStr(const key_flowlist_entry &key_flowlist_entry);
  static string IpcStructToStr(const val_flowlist_entry &val_entry);
  static string IpcStructToStr(const pom_stats &pom_stats);
  static string IpcStructToStr(const key_vtn_flowfilter &key_vtn_flowfilter);
  static string IpcStructToStr(const val_flowfilter &val_flowfilter);
  static string IpcStructToStr(const val_vtn_flowfilter_controller_st &val_st);
  static string IpcStructToStr(const val_flowlist_entry_st &valst);
  static string IpcStructToStr(const key_vtn_flowfilter_entry &key_entry);
  static string IpcStructToStr(const val_vtn_flowfilter_entry &val__entry);
  static string IpcStructToStr(const key_vtn_flowfilter_controller &key_ctrlr);
  static string IpcStructToStr(const val_flowfilter_controller &val_controller);
  static string IpcStructToStr(const key_vbr_flowfilter &key_vbr_flowfilter);
  static string IpcStructToStr(const key_vbr_flowfilter_entry &key_entry);
  static string IpcStructToStr(const val_flowfilter_entry &val_ff_entry);
  static string IpcStructToStr(const val_flowfilter_entry_st &val_st);
  static string IpcStructToStr(const key_vbr_if_flowfilter &keyflowfilter);
  static string IpcStructToStr(const key_vbr_if_flowfilter_entry &key_entry);
  static string IpcStructToStr(const key_vrt_if_flowfilter &key_flowfilter);
  static string IpcStructToStr(const key_vrt_if_flowfilter_entry &key_entry);
  static string IpcStructToStr(const key_vterm_if_flowfilter &key_flowfilter);
  static string IpcStructToStr(const key_vterm_if_flowfilter_entry &key_entry);
  static string IpcStructToStr(const key_policingprofile &key_policingprofile);
  static string IpcStructToStr(const val_policingprofile &val_policingprofile);
  static string IpcStructToStr(const val_rename_policingprofile &val__rofile);
  static string IpcStructToStr(const key_policingprofile_entry &key__entry);
  static string IpcStructToStr(const val_policingprofile_entry &val_entry);
  static string IpcStructToStr(const val_policingmap &val_policingmap);
  static string IpcStructToStr(const val_policingmap_controller_st &val_st);
  static string IpcStructToStr(const val_policingmap_switch_st &val_switch_st);
  static string IpcStructToStr(const key_vtn_policingmap_controller &key_ctrlr);
  static string IpcStructToStr(const key_vbr_policingmap_entry &key__entry);
  static string IpcStructToStr(const key_vbrif_policingmap_entry &key_entry);
  static string IpcStructToStr(const val_policingmap_controller &key_entry);
  static string IpcStructToStr(const key_vtermif_policingmap_entry &key_entry);
  static string IpcStructToStr(const val_vbr_expand &val_vbr_expand);
  static string IpcStructToStr(const val_vbr_if_expand &val_vbr_if_expand);
  static string IpcStructToStr(const val_vbr_portmap_expand
                                                    &val_vbr_portmap_expand);
  static string IpcStructToStr(const val_vtunnel_expand &val_vtunnel_expand);
  static string IpcStructToStr(const val_vtunnel_if_expand
                                                 &val_vtunnel_if_expand);
  static string IpcStructToStr(const val_vlink_expand &val_vlink_expand);

  static string IpcStructToStr(const key_ctr &data);
  static string IpcStructToStr(const val_ctr &data);
  static string IpcStructToStr(const val_ctr_st &data);
  static string IpcStructToStr(const key_ctr_domain &data);
  static string IpcStructToStr(const val_ctr_domain &data);
  static string IpcStructToStr(const val_ctr_domain_st &data);
  static string IpcStructToStr(const key_logical_port &data);
  static string IpcStructToStr(const val_logical_port &data);
  static string IpcStructToStr(const val_logical_port_st &data);
  static string IpcStructToStr(const key_boundary &data);
  static string IpcStructToStr(const val_boundary &data);
  static string IpcStructToStr(const val_boundary_st &data);
  static string IpcStructToStr(const val_path_fault_alarm &data);

  static string IpcStructToStr(const pfcdrv_val_vbr_if &data);
  static string IpcStructToStr(const pfcdrv_val_vbrif_vextif &data);
  static string IpcStructToStr(const pfcdrv_val_flowfilter_entry &data);
  static string IpcStructToStr(const pfcdrv_val_vbrif_policingmap &data);
  static string IpcStructToStr(const pfcdrv_val_vbr_portmap &data);
  static string IpcStructToStr(const pfcdrv_val_vtn_controller &data);

  /* VlanmapOnBoundary: New val struct */
  static string IpcStructToStr(const pfcdrv_val_vlan_map &data);

  static string Ipv4AddrToStr(struct in_addr address);
  static string Ipv4AddrToStr(uint32_t address);
  static string Ipv6AddrToStr(struct in6_addr address);
  static string MacAddrToStr(const uint8_t *mac_addr);
  static string ValidArrayToStr(const uint8_t *validarray, int size);
  static string ConfigStatusToStr(const uint8_t *cfgstatus, int size);

  static string IpcStructToStr(const vnpdrv_val_vtunnel &data);
  static string IpcStructToStr(const vnpdrv_val_vtunnel_if &data);
  static string IpcStructToStr(const key_vtn_dataflow &key_vtn_df);
  // convert functions
  static string IpcStructToStr(const key_convert_vbr&);
  static string IpcStructToStr(const val_convert_vbr&);
  static string IpcStructToStr(const key_convert_vbr_if&);
  static string IpcStructToStr(const val_convert_vbr_if&);
  static string IpcStructToStr(const key_convert_vtunnel&);
  static string IpcStructToStr(const val_convert_vtunnel&);
  static string IpcStructToStr(const key_convert_vtunnel_if&);
  static string IpcStructToStr(const val_convert_vtunnel_if&);
  static string IpcStructToStr(const key_convert_vlink&);
  static string IpcStructToStr(const val_convert_vlink&);
  static string IpcStructToStr(const val_vtn_gateway_port&);
  // label allocation functions
  static string IpcStructToStr(const key_gvtnid_label&);
  static string IpcStructToStr(const val_gvtnid_label&);
  static string IpcStructToStr(const key_vbid_label&);
  static string IpcStructToStr(const val_vbid_label&);

  private:
  KtUtil() {}
  void Init();
  static std::map<unc_key_type_t, KtMsgTemplate*> kt_msg_templates_;
  static KtUtil *singleton_instance_;
};


}  // namespace ipc_util
}  // namespace upll
}  // namespace unc
                                                                       // NOLINT
#endif  // UPLL_KT_UTIL_HH_
