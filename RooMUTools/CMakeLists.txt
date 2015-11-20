#
find_library( ROOMULIB NAMES plotutils PATHS $ENV{ROOMU_SYS}/lib NO_DEFAULT_PATH )
## ---> MESSAGE( STATUS "ROOMULIB: ${ROOMULIB}" )
#
include_directories($ENV{ROOMU_SYS})
#
# NOTE(JVY): as of art v1_15_01, the TFileService stuff needs to be linked explicitly
#            it was NOT the case in the v1_12-series, and I do NOT know why the hell this change !...
#            need to talk to M.P. when he's back from "art school"
# 
art_make( MODULE_LIBRARIES art_Framework_Services_Optional_TFileService_service "${ROOMULIB}" )

# install_headers()
