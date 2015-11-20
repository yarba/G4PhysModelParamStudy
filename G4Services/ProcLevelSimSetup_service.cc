
// Authors: Julia Yarba

// Date: Oct. 2015

// Art includes
//
#include "art/Framework/Core/ModuleMacros.h"

// Services
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "artg4tk/G4PhysModelParamStudy/G4Services/ProcLevelSimSetup_service.hh"

// CLHEP/Random
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RanecuEngine.h"


artg4tk::ProcLevelSimSetupService::ProcLevelSimSetupService( const fhicl::ParameterSet& pset, 
				                             art::ActivityRegistry& )
   : fLogInfo("ProcLevelSimSetupService")
{

   fModelName       = pset.get<std::string>("HadronicModelName");
   fRegion          = 0;
   fTgtMaterialName = pset.get<std::string>("TargetNucleus");
   fTgtMaterial     = 0; 
   fRandomSeed      = pset.get<long>("RNDMSeed",-1);
   if ( fRandomSeed == -1) 
   {
	  // Construct seed from time and pid. (default behavior if 
	  // no seed is provided by the fcl file)
	  // Note: According to Kevin Lynch, the two lines below are not portable. 
	  fRandomSeed = time(0) + getpid();
	  fRandomSeed = ((fRandomSeed & 0xFFFF0000) >> 16) | ((fRandomSeed & 0x0000FFFF) << 16); //exchange upper and lower word
	  fRandomSeed = fRandomSeed % 900000000; // ensure the seed is in the correct range for engine
   }
   
   Init();

   // Non-art way of doing it.
   // However, a service can NOT call createEngine; only EDProducer can.
   // But at the same time we can NOT have it in the Producer because 
   // we may want to run multiple process-level producer in the same job,
   // while Geant4 engine is ** unique ** - better NOT touch more than once.
   // And there's no way to check if Geant4 engine has already been initialized,
   // at least not without hacking ART RNDM machinery.
   //
   // Just for the record, Geant4 runs on the single ** static ** CLHEP engine,
   // it explicitly call shoot() method as opposed to the dynamic fire().
   //
   CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);
   CLHEP::HepRandom::setTheSeed( fRandomSeed ); 
          
}

artg4tk::ProcLevelSimSetupService::~ProcLevelSimSetupService()
{ 

   if ( fRegion ) delete fRegion; 

} 

void artg4tk::ProcLevelSimSetupService::Init()
{

   if(!G4StateManager::GetStateManager()->SetNewState(G4State_PreInit))
      fLogInfo << "G4StateManager PROBLEM! "; //  << G4endl;

//  G4Box* sFrame = new G4Box ("Box", 100.0*cm, 100.0*cm, 100.0*cm);
//  G4LogicalVolume* lFrame = new G4LogicalVolume(sFrame,
//                                                fTgtMaterial, // fMaterial can be NULL at this point - no big deal though...
//						"Box",0,0,0);
  G4PVPlacement* pFrame = new G4PVPlacement(0,G4ThreeVector(),"Box",
                                            new G4LogicalVolume ( new G4Box("Box",1000.0*CLHEP::mm,1000.0*CLHEP::mm,1000.0*CLHEP::mm),
					                          fTgtMaterial, // in general, can be NULL at this point - no big deal...
								  "Box",0,0,0
					                        ),
					    0,false,0);
   assert(pFrame);
   fRegion = new G4Region("Region"); // needed by tracking manager
   pFrame->GetLogicalVolume()->SetRegion(fRegion);
   fRegion->AddRootLogicalVolume( pFrame->GetLogicalVolume() );
//   targetPhys->GetLogicalVolume()->SetRegion(fRegion);
//   fRegion->AddRootLogicalVolume( targetPhys->GetLogicalVolume() );

   std::ostringstream osmat(std::ios_base::out|std::ios_base::app);// to enable appending in output operations
   osmat.clear();
   osmat.str("G4_");
   osmat << fTgtMaterialName;
   G4String g4matname = osmat.str();
   fTgtMaterial = G4NistManager::Instance()->FindOrBuildMaterial(g4matname);

   if (!fTgtMaterial) // stream to mlog !!!
   {
      fLogInfo << "Material <" << g4matname << "> is not found";
      return;
   }

   return;

}

using artg4tk::ProcLevelSimSetupService;

DEFINE_ART_SERVICE(ProcLevelSimSetupService)

