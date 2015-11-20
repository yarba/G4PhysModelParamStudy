#ifndef G4MPStudy_SERVICE_HH
#define G4MPStudy_SERVICE_HH

// Art
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

class G4RunManager;
// class ModelParamStudyRunManager;
class ModelParamStudyStepping;
class ModelParamStudyGeom;

namespace artg4tk
{

class G4ModelParamStudyService
{

   public:
   
      // ctor & dtor 
      G4ModelParamStudyService( const fhicl::ParameterSet&, art::ActivityRegistry& );
      ~G4ModelParamStudyService();
      
      G4RunManager*            GetRunManager() const { return fRunManager; }
//       ModelParamStudyRunManager* GetRunManager() const { return fRunManager; }
      ModelParamStudyStepping*   GetStepping() const { return fStepping; }
      ModelParamStudyGeom*       GetGeometry() const { return fGeometry; }
   
   private:

      G4RunManager* fRunManager;
//      ModelParamStudyRunManager* fRunManager;

      std::string              fPhysList;
      
      // additional data members, specifically to study parameters modification
      //
      ModelParamStudyGeom*     fGeometry;
      ModelParamStudyStepping* fStepping;

      // this seed will be used to init G4 RNDM...
      // ...although we'll have to bypass RandomEngineService and do it directly...
      // ...because a service can NOT access rng() (only producers can)...
      // ...so we'll loose functionalities for reproducibility 
      // but at least we'll be able to set unique seeds in parallel jobs
      //
      long                     fRandomSeed; // keep it here for now...     

};

}

using artg4tk::G4ModelParamStudyService;
DECLARE_ART_SERVICE(G4ModelParamStudyService,LEGACY)


#endif
