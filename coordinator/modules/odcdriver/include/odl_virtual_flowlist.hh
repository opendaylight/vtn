#ifndef __ODL_VIRTUAL_FLOWLIST_PARSER__
#define __ODL_VIRTUAL_FLOWLIST_PARSER__

#include <odl_flowlist.hh>

namespace unc {
namespace odcdriver {

class VirtParserFlowlist {

public:
 VirtParserFlowlist() {}
 ~VirtParserFlowlist() {}

 UncRespCode get_ip_type(json_object *in, key_flowlist_t &key, val_flowlist_t &val) {
   val.ip_type=UPLL_FLOWLIST_TYPE_IP;
   val.valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_VALID;
   return UNC_RC_SUCCESS;
 }
};
}
}
#endif
