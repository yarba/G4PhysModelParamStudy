#ifndef G4ProcLevelSimSetup_SERVICE_HH
#define G4ProcLevelSimSetup_SERVICE_HH

// Art
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "messagefacility/MessageLogger/MessageLogger.h" 

#include <iostream>
#include <string>

// Geant4
#include "Geant4/G4Material.hh"
#include "Geant4/G4IsotopeVector.hh"
#include "Geant4/G4ElementVector.hh"
#include "Geant4/G4NistManager.hh"
#include "Geant4/G4ForceCondition.hh"
//
#include "Geant4/G4Region.hh"
#include "Geant4/G4Box.hh"
#include "Geant4/G4PVPlacement.hh"
#include "Geant4/G4GRSVolume.hh"
//
#include "Geant4/G4StateManager.hh"


namespace artg4tk
{

class ProcLevelSimSetupService
{

   public:
   
      // ctor & dtor
      //
      // NOTE: the ctor MUST have these 2 inpur args; 
      //       otherwise the system will NOT register it as a "service"
      // 
      ProcLevelSimSetupService( const fhicl::ParameterSet&, art::ActivityRegistry& );
      ~ProcLevelSimSetupService();
      
      G4Material* GetTgtMaterial()      const { return fTgtMaterial; }
      const std::string& GetModelName() const { return fModelName; }
                     
   private:
   
      void Init();
   
      // data members
      //
      std::string  fModelName;
      //
      G4Region*    fRegion;
      std::string  fTgtMaterialName; // in fact, it's target nucleus...
      G4Material*  fTgtMaterial; // in fact, it's target nucleus... 
      //
      long         fRandomSeed;
      
      mf::LogInfo fLogInfo;
      
      

};

}

using artg4tk::ProcLevelSimSetupService;
DECLARE_ART_SERVICE(ProcLevelSimSetupService,LEGACY)


#endif
