
// Authors: Julia Yarba

// Date: April 2015

// Art includes
//
#include "art/Framework/Core/ModuleMacros.h"

#include "Geant4/G4RunManager.hh"

// #include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyRunManager.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyGeom.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyStepping.hh"

#include "Geant4/G4GeometryManager.hh"
#include "Geant4/G4StateManager.hh"
#include "Geant4/FTFP_BERT.hh"
#include "Geant4/QGSP_FTFP_BERT.hh"
#include "Geant4/QGSP_BERT.hh"

// Services
#include "art/Framework/Services/Registry/ServiceHandle.h"
// ---> #include "artg4tk/services/PhysicsListHolder_service.hh"
// #include "art/Framework/Services/Optional/RandomNumberGenerator.h"

#include "artg4tk/G4PhysModelParamStudy/G4Services/G4ModelParamStudy_service.hh"

// ---> #include "messagefacility/MessageLogger/MessageLogger.h"

// Category for this file
// static std::string msgctg = "G4ModelParamStudyServiceService";

artg4tk::G4ModelParamStudyService::G4ModelParamStudyService( const fhicl::ParameterSet& pset, 
				                             art::ActivityRegistry& )
   :  fPhysList(pset.get<std::string>("PhysicsList","QGSP_FTFP_BERT"))
{

   fRunManager = new G4RunManager();
//   fRunManager = new ModelParamStudyRunManager();

  
  // Get the physics list and pass it to Geant and initialize the list if necessary
  //
  // FIXME !!!
  // For some reason, it works with artg4tk_Main but not here...
  // ... perhaps it's because of the (random?) order of instantiating Services ?
  //
  // art::ServiceHandle<PhysicsListHolderService> physicsListHolder;
  // fRunManager->SetUserInitialization( physicsListHolder->makePhysicsList() );
  //
  if ( fPhysList == "FTFP_BERT" )
  {
     fRunManager->SetUserInitialization( new FTFP_BERT() );
  }
  else if ( fPhysList == "QGSP_BERT" )
  {
     fRunManager->SetUserInitialization( new QGSP_BERT() );
  }
  else if ( fPhysList == "QGSP_FTFP_BERT" )
  {
     fRunManager->SetUserInitialization( new QGSP_FTFP_BERT() );
  }

  // Declare the detector construction to Geant
  // First, get the corresponding PSet for Geometry
  //
  const fhicl::ParameterSet& psgeom = pset.get<fhicl::ParameterSet>("TargetGeom");
  fGeometry = new ModelParamStudyGeom( psgeom );
  fRunManager->SetUserInitialization( fGeometry );
  fRunManager->GeometryHasBeenModified();

  fStepping = new ModelParamStudyStepping();
  fStepping->SetStopAfter1stInteraction( true );
  fRunManager->SetUserAction( fStepping ); // FIXME !!! Do I transfer ownership ???
  //
  // These things below are "left-over" from experimenting with custom run managers
  //
  // fRunManager->GeometryHasBeenModified();
  //    
  // fRunManager->InitializeGeometry();
  // fRunManager->InitializePhysics();  // NOTE(JVY) how will it play vs physicsListHolder->initializePhysicsList();     ?????

   fRunManager->Initialize();
   
   fRunManager->ConfirmBeamOnCondition();
   fRunManager->ConstructScoringWorlds();
   fRunManager->RunInitialization(); // this is part of BeamOn 
                                       // and needs be done (at least) to set GeomClosed status 
      
   fStepping->SetTargetPtr( fGeometry->GetTarget() );

   fRandomSeed = pset.get<long>("RNDMSeed", -1);
   if ( fRandomSeed == -1) 
   {
	  // Construct seed from time and pid. (default behavior if 
	  // no seed is provided by the fcl file)
	  // Note: According to Kevin Lynch, the two lines below are not portable. 
	  fRandomSeed = time(0) + getpid();
	  fRandomSeed = ((fRandomSeed & 0xFFFF0000) >> 16) | ((fRandomSeed & 0x0000FFFF) << 16); //exchange upper and lower word
	  fRandomSeed = fRandomSeed % 900000000; // ensure the seed is in the correct range for engine
   }

   // fRndmEngine = new CLHEP::RanecuEngine; 
   // CLHEP::HepRandom::setTheEngine( fRndmEngine );   
   CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);
   CLHEP::HepRandom::setTheSeed( fRandomSeed ); 
   
   // NOTE: RandomEngine can be set in an EDProducer but NOT in a Service !!!
   //
   // Set up the random number engine.
   // See the documentation in RandomNumberHeader.h for how this works. 
   // Note that @createEngine@ is a member function of our base class 
   // (actually, a couple of base classes deep!).
   // Note that the name @G4Engine@ is special. 
   // art::ServiceHandle<art::RandomNumberGenerator>& rng();
   // rng()->createEngine( fRandomSeed, "G4Engine");
          
}

artg4tk::G4ModelParamStudyService::~G4ModelParamStudyService()
{
 
   fRunManager->RunTermination();
   
   // do proper deletes here !
   
//
// Do NOT delete Stepping and/or Geometry as RUnManager will take care of that !
// If you delete them by hands, the RunManager dtor will segfault !
// NOTE that RunManager will delete Geometry PROPERLY - it'll open GeometryStore
//   if ( fStepping )   delete fStepping;
//   if ( fGeometry )   delete fGeometry;
   if ( fRunManager ) delete fRunManager;

} 

using artg4tk::G4ModelParamStudyService;

DEFINE_ART_SERVICE(G4ModelParamStudyService)

