#ifndef G4ModelConfig_SERVICE_HH
#define G4ModelConfig_SERVICE_HH

// Art
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

// fwd declaration(s)
class ModelConfigMapper;

namespace artg4tk
{

class PhysModelConfigService
{

   public:
   
      // ctor & dtor
      //
      // NOTE: the ctor MUST have these 2 inpur args; 
      //       otherwise the system will NOT register it as a "service"
      // 
      PhysModelConfigService( const fhicl::ParameterSet&, art::ActivityRegistry& );
      ~PhysModelConfigService();
      
      void ConfigureModel( const std::string&, const fhicl::ParameterSet& );
      void RestoreDefaults( const std::string& );
                     
   private:
   
      // data members
      //
      ModelConfigMapper*  fConfigMapper;
      bool                fVerbosity;

};

}

using artg4tk::PhysModelConfigService;
DECLARE_ART_SERVICE(PhysModelConfigService,LEGACY)


#endif
