# services CMakeLists

# NOTE-1(JVY): as of art v1_15_01, the TFileService stuff needs to be linked explicitly
#            it was NOT the case in the v1_12-series, and I do NOT know why the hell this change !...
#            need to talk to M.P. when he's back from "art school"
#
# NOTE-2(JVY): for the moment, no need for "${XERCESCLIB}"
# 
art_make( MODULE_LIBRARIES  
          art_Framework_Services_Optional_TFileService_service 
	  "artg4tk_G4PhysModelParamStudy_AnalysisBase"
	  "${G4_LIB_LIST}" )

#install_headers()
