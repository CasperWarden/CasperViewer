# -*- cmake -*-
include(Prebuilt)

set(LLCONVEXDECOMP_INCLUDE_DIRS ${LIBS_PREBUILT_DIR}/include)
  
use_prebuilt_binary(llconvexdecompositionstub)
set(LLCONVEXDECOMP_LIBRARY llconvexdecompositionstub)
