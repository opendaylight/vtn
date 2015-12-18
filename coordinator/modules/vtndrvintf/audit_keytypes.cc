/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <audit_keytypes.h>

namespace unc {
namespace driver {

audit_key_type audit_key[AUDIT_KT_SIZE] = {
                               {UNC_KT_VTN, UNC_KT_ROOT},
                               //{UNC_KT_FLOWLIST, UNC_KT_ROOT},
                               {UNC_KT_VBRIDGE, UNC_KT_VTN},
                               {UNC_KT_VTERMINAL, UNC_KT_VTN},
                               //{UNC_KT_VTN_FLOWFILTER, UNC_KT_VTN},
                               {UNC_KT_VTERM_IF, UNC_KT_VTERMINAL},
                               {UNC_KT_VBR_IF, UNC_KT_VBRIDGE},
                               //{UNC_KT_VBR_FLOWFILTER, UNC_KT_VBRIDGE},
                               //{UNC_KT_VBRIF_FLOWFILTER, UNC_KT_VBR_IF},
                               {UNC_KT_VBR_VLANMAP, UNC_KT_VBRIDGE},
                               //{UNC_KT_VTERMIF_FLOWFILTER, UNC_KT_VTERM_IF},
                             };
}  // namespace driver
}  //  namespace unc
