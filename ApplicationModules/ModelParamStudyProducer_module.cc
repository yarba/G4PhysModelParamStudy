
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

// needed when doing geom via GDML
//
// --> --> #include "artg4tk/services/DetectorHolder_service.hh"

#include "artg4tk/services/PhysicsListHolder_service.hh"
// --> seems to come via some other inc #include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "messagefacility/MessageLogger/MessageLogger.h" 

#include <iostream>
#include <cmath>
#include <memory>

// #include <vector>

// G4 (or derived) includes
//
#include "Geant4/G4RunManager.hh"
#include "Geant4/G4SDManager.hh"
#include "Geant4/G4GeometryManager.hh"
#include "Geant4/G4StateManager.hh"
//
// --> #include "Geant4/G4UImanager.hh"
// --> #include "Geant4/G4CascadeParameters.hh"
//
#include "artg4tk/G4PhysModelParamStudy/G4Services/PhysModelConfig_service.hh"
//
// NOTE(JVY): This isn't needed if we do via GDML; comment out then
//
#include "artg4tk/geantInit/ArtG4DetectorConstruction.hh"
//
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyGeom.hh" 
#include "artg4tk/pluginDetectors/gdml/HadInteractionSD.hh"
//
// --> comes via HadInteractionSD.hh --> #include "artg4tk/DataProducts/G4DetectorHits/ArtG4tkVtx.hh" // Evt product
//
#include "artg4tk/DataProducts/G4ModelConfig/ArtG4tkModelConfig.hh" // Run product
//
#include "artg4tk/DataProducts/EventGenerators/GenParticle.hh"
#include "artg4tk/DataProducts/EventGenerators/GenParticleCollection.hh"

namespace artg4tk {

  class ModelParamStudyProducer : public art::EDProducer {

  public:

    explicit ModelParamStudyProducer( const fhicl::ParameterSet& pset );
    virtual ~ModelParamStudyProducer();

    virtual void produce( art::Event& event ) override;
    virtual void beginJob()                   override;
    virtual void beginRun( art::Run&  )       override;
    virtual void endRun( art::Run& )          override;

  private:
       
     void clear();

    // data members
    
    // Diagnostics level (verbosity)
    //
    int         fVerbosity;
    
    // G4 (or derived) things 
    //
    G4RunManager*            fRM;
    //
    // data members specifically to study parameters modification
    //
    bool                           fDefaultPhysics;

    //
    // NOTE(JV): This isn't needed if we do via GDML; comment out then
    // 
    fhicl::ParameterSet      fPSetGeom;
    //
    // This one is needed since we'll have to insert 
    // model(s) config info as a RunProduct (in beginRun(...))
    //
    ArtG4tkModelConfig*      fModelConfig;
    //
    HadInteractionSD*        f1stHadIntSD;
    G4Event*                 fCurrentG4Event;
    //
    long                     fRandomSeed;
    
    // diagnostics printouts
    //
    mf::LogInfo fLogInfo;
    

  };

}

artg4tk::ModelParamStudyProducer::ModelParamStudyProducer( const fhicl::ParameterSet& p )
   : fLogInfo("ModelParamStudyProducer")
{
    
   fVerbosity       = p.get<int>("Verbosity",0);

   // Check the state
   //
   if(!G4StateManager::GetStateManager()->SetNewState(G4State_PreInit))
      fLogInfo << "G4StateManager PROBLEM! "; //  << G4endl;

   fRM = 0;
/*
   // Get the run manager and check if it's been initialized in any way
   // NOTE-1: attempt to call the ctor more than once will result
   //         in a fatal throw on the G4 side (in the ctor itself)
   // NOTE-2: attempt to call G4RunManager::GetRunManager() BEFORE
   //         it's initialized (via new) will result in return of
   //         a NULL pointer !
   //
   fRM              = new G4RunManager();
*/
   
   // CRITICAL !!!
   // Model(s) (or at least Bertini) config/params should be in BEFORE
   // Physics List is instantiated/initialized !!!
   //   
   //
   // TRACKED PSet(s) - outputs with different settings will NOT mix
   //
   fhicl::ParameterSet physicscfg = p.get<fhicl::ParameterSet>("HadronicModelParameters");
//   fDefaultPhysics = physicscfg.get<bool>("DefaultPhysics",true);
   fDefaultPhysics = physicscfg.get<bool>("DefaultPhysics"); // make it TRACKED !!!
   fModelConfig = new ArtG4tkModelConfig(fDefaultPhysics);
   if ( !fDefaultPhysics )
   {
      fModelConfig->Fill( physicscfg );
      art::ServiceHandle<PhysModelConfigService> physcfgservice;
      fhicl::ParameterSet modelcfg = physicscfg.get<fhicl::ParameterSet>("Bertini");
      if ( !modelcfg.is_empty() ) physcfgservice->ConfigureModel("Bertini",modelcfg);
      // modelcfg = physicscfg.get<fhicl::ParameterSet>("INCLXX");
      // if ( !modelcfg.is_empty() ) physcfgservice->ConfigureModel("INCLXX",modelcfg);
      // Etc.
   }

   // Detector geometry business
   //
   // NOTE(JVY): This isn't necessary we we do via GDML; comment out then
   //
   fPSetGeom = p.get<fhicl::ParameterSet>("TargetGeom");

   //
   // NOTE(JVY): This is needed when doing geom via GDML
   //
// --> -->   art::ServiceHandle<DetectorHolderService> detholder;
// --> -->   detholder->initialize();
// --> -->   detholder->constructAllLVs();
// --> -->   detholder->callArtProduces(this);

   f1stHadIntSD     = 0;

   fCurrentG4Event  = 0;

   fRandomSeed      = p.get<long>("RNDMSeed",-1);
   if ( fRandomSeed == -1) 
   {
	  // Construct seed from time and pid. (default behavior if 
	  // no seed is provided by the fcl file)
	  // Note: According to Kevin Lynch, the two lines below are not portable. 
	  fRandomSeed = time(0) + getpid();
	  fRandomSeed = ((fRandomSeed & 0xFFFF0000) >> 16) | ((fRandomSeed & 0x0000FFFF) << 16); //exchange upper and lower word
	  fRandomSeed = fRandomSeed % 900000000; // ensure the seed is in the correct range for engine
   }
   createEngine( fRandomSeed, "G4Engine" ); // inherited from "EngineCreator" base class
        
   // Run product
   // NOTE: Default 2nd arg is art::InEvent
   //
   produces<ArtG4tkModelConfig,art::InRun>();
   
   // technically speaking, a product can be registered with an "instance name: produces<prod>("instancename")
   // but then it needs to be placed into an event as follows: e.put(std::move(prod), "instancename")
   //
   // NOTE(JVY): This is needed when doing WITHOUT GDML 
   //
   produces<ArtG4tkVtx>();
   
}

artg4tk::ModelParamStudyProducer::~ModelParamStudyProducer()
{

   // no need to clean up UI pointer - I don't "new" it, I just "get it"

   if ( fCurrentG4Event ) delete fCurrentG4Event;
   
   fRM->RunTermination();
   delete fRM;
   delete fModelConfig;
   
}

// At begin job
//
void artg4tk::ModelParamStudyProducer::beginJob()
{

/* moved to beginRun
   G4SDManager* sdman = G4SDManager::GetSDMpointer();
   f1stHadIntSD = dynamic_cast<HadInteractionSD*>(sdman->FindSensitiveDetector("HadInteractionSD"));
*/
   
   return;

}

// At begin run
//
void artg4tk::ModelParamStudyProducer::beginRun( art::Run& run )
{  
   
   // Get the run manager and check if it's been initialized in any way
   // NOTE-1: attempt to call the ctor more than once will result
   //         in a fatal throw on the G4 side (in the ctor itself)
   // NOTE-2: attempt to call G4RunManager::GetRunManager() BEFORE
   //         it's initialized (via new) will result in return of
   //         a NULL pointer !
   //
   fRM              = new G4RunManager();

   // Setup phys.list to Geant and initialize the list if necessary
   //
   art::ServiceHandle<PhysicsListHolderService> physicsListHolder;
   fRM->SetUserInitialization( physicsListHolder->makePhysicsList() );

   // Declare the detector construction to Geant
   //
   // NOTE(JVY): This isn't needed if we do via GDML
   //
   fRM->SetUserInitialization( new ModelParamStudyGeom( fPSetGeom ) );
   //
   // This IS needed to do via GDML
   //
// --> -->   fRM->SetUserInitialization( new ArtG4DetectorConstruction() ); // this GDML-related - will retrieve from detholder, etc.

   fRM->GeometryHasBeenModified();

   // inits
   //
   fRM->Initialize();   
   fRM->ConfirmBeamOnCondition();
   fRM->ConstructScoringWorlds();
   fRM->RunInitialization(); // this is part of BeamOn 
                             // and needs be done (at least) to set GeomClosed status 

   G4SDManager* sdman = G4SDManager::GetSDMpointer();
   //
   // NOTE(JVY): This is the way to do WITHOUT GDML
   //
   f1stHadIntSD = dynamic_cast<HadInteractionSD*>(sdman->FindSensitiveDetector("HadInteractionSD"));
   //
   // This is how to via GDML
   //
// --> -->   f1stHadIntSD = dynamic_cast<HadInteractionSD*>(sdman->FindSensitiveDetector("TargetVolume_HadInteraction"));
    
    
   // Last but not least:
   // Insert the RunProduct (model(s) config info)
   //
   std::unique_ptr<ArtG4tkModelConfig> pcfg(new ArtG4tkModelConfig(*fModelConfig));   
   run.put(std::move(pcfg));   

   return;

}

void artg4tk::ModelParamStudyProducer::produce( art::Event& e )
{

   // first of all, fetch event from prinamry generator
   // (pgun from artg4tk/EventGenerators)
   //
   // do it "ByLabel" !!!
   // NOTE: in the config /artg4tk/G4PhysModelParamStudy/fcl/toy.fcl
   //       (module_type) EventGenerator is labeled "PrimaryGenerator"
   //       ...in principle, one can make it configurable what label 
   //       this particular analyzer wants...
   //
   art::Handle<GenParticleCollection> primgenparts;
   e.getByLabel( "PrimaryGenerator", primgenparts );
   if ( !primgenparts.isValid() )
   {
       fLogInfo << " primgen handle is NOT valid "; //  << std::endl;
       return;
   }

   G4ParticleTable* ptable = G4ParticleTable::GetParticleTable();
   
   fCurrentG4Event = new G4Event(e.id().event() );

   for ( GenParticleCollection::const_iterator i=primgenparts->begin(); i!=primgenparts->end(); ++i ) 
   {
      int pdgcode = i->pdgId();
      G4ParticleDefinition* g4pd = ptable->FindParticle( pdgcode );
      if ( !g4pd ) continue;
      //
      // G4PrimaryVertex*   g4vtx = new G4PrimaryVertex( i->position()*mm, 0. ); // 4th arg is time(ns)
      //
      // FIXME !!!
      // This needs to be configurable !!!
      //
      CLHEP::Hep3Vector pos(0.,0.,-1300.); // in mm !!!
      G4PrimaryVertex*   g4vtx = new G4PrimaryVertex( pos*CLHEP::mm, 0. ); // 4th arg is time(ns)
      const CLHEP::HepLorentzVector& mom = i->momentum();
      G4PrimaryParticle* g4prim = new G4PrimaryParticle( g4pd, 
                                                         mom.x()*CLHEP::GeV, mom.y()*CLHEP::GeV, mom.z()*CLHEP::GeV, 
							 mom.e()*CLHEP::GeV );        
      g4vtx->SetPrimary(g4prim);
      fCurrentG4Event->AddPrimaryVertex(g4vtx);
   }
   
   if ( fVerbosity > 0 )  
   {
      fLogInfo << "Processing art::Event " << e.id().event(); //  << "\n" << std::endl;   
      fLogInfo << "G4Event " << e.id().event() << " has been generated "; // << std::endl; 
      fCurrentG4Event->Print();
      int nprimvtx = fCurrentG4Event->GetNumberOfPrimaryVertex();
      for ( int i=0; i<nprimvtx; ++i )
      {
         fCurrentG4Event->GetPrimaryVertex(i)->Print();
      }
      // const G4String& version = fRM->GetVersionString();
      // fLogInfo << "Now Process it through " << version; // << std::endl;
   }
   
   G4EventManager::GetEventManager()->ProcessOneEvent( fCurrentG4Event );
   if ( fVerbosity > 0 )
   {
      fLogInfo << "G4Event " << e.id().event() << " has been processed by EventManager "; // << std::endl;
   } 
   fRM->AnalyzeEvent( fCurrentG4Event );
   // fRM->UpdateScoring(); // can NOT use it outside G4RunManager (or deribved) since this method is protected

//--------------------------

   // get the "hits" from SensitiveDetector (see also beginRun)
   //
   if ( !f1stHadIntSD ) 
   {
      fLogInfo << " Invalid HadInteractionSD ptr "; // << std::endl;
      return;
   }

   const ArtG4tkVtx& inter = f1stHadIntSD->Get1stInteraction();
   
   if ( inter.GetNumOutcoming() <= 0 )
   {
      clear();
      return;
   }
   
   // Create a (no longer empty!) output data product
   //
   std::unique_ptr<ArtG4tkVtx> firstint(new ArtG4tkVtx(inter));

   // Product Id of the data product to be created 
   // it was said to be needed for persistent pointers 
   // but in practice it doesn't look like it is...
   //
   // art::ProductID firstintID(getProductID<ArtG4tkVtx>(e));

   // Put the output collection into the event
   //
   // NOTE: technically speaking, one can also add an "instance name" to a product, 
   //       and put it in as follows: e.put(std::move(prod), "instancename" )
   //       but in order to do so, it needs to be registered with such instance name,
   //       via produces<prod>("instancename")
   //       otherwise event processor will sagfault 
   //
   e.put(std::move(firstint)); 

   clear();
   return;
    
}

// At end run
//
void artg4tk::ModelParamStudyProducer::endRun( art::Run& )
{
   
   return;

}

/*

void artg4tk::ModelParamStudyProducer::fillBertiniDefaults()
{

   // NOTE(JVY): this procedure is quite boring; need to figure out if there's
   //            a more "algorithmic" approach...
   //
   
   std::ostringstream cmd;
   
   // general purpose verbosity switch
   // has nothing to do with the model params/configuration
   // but is useful for some debugging purposes
   //
   cmd << "process/had/cascade/verbose " << G4CascadeParameters::verbose();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   
   cmd << "/process/had/cascade/doCoalescence " << G4CascadeParameters::doCoalescence();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();

// these params/methods are NOT available in G4.9.6-series, but only starting 10.1-ref03
//   
   cmd << "/process/had/cascade/piNAbsorption " << G4CascadeParameters::piNAbsorption();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/use3BodyMom " << G4CascadeParameters::use3BodyMom();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/usePhaseSpace " << G4CascadeParameters::usePhaseSpace();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();

// these parameters are "technically" available in 9.6-series, together with their G4UI,  
// but in practice they can only be changed via env.variables, due to specific implementation 
//
   cmd << "/process/had/cascade/useTwoParamNuclearRadius " << G4CascadeParameters::useTwoParam();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/nuclearRadiusScale " << G4CascadeParameters::radiusScale();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/smallNucleusRadius " << G4CascadeParameters::radiusSmall();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/alphaRadiusScale " << G4CascadeParameters::radiusAlpha();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/shadowningRadius " << G4CascadeParameters::radiusTrailing(); 
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/fermiScale " << G4CascadeParameters::fermiScale();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/crossSectionScale " << G4CascadeParameters::xsecScale();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/gammaQuasiDeutScale " << G4CascadeParameters::gammaQDScale();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/cluster2DPmax " << G4CascadeParameters::dpMaxDoublet();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/cluster3DPmax " << G4CascadeParameters::dpMaxTriplet();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/cluster4DPmax " << G4CascadeParameters::dpMaxAlpha();
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd<< "/process/had/cascade/usePreCoumpound " << G4CascadeParameters::usePreCompound() ; // false/0 by default
   fDefaultBertini.push_back( cmd.str() );
   cmd.str("");
   cmd.clear();

   cmd << "/process/had/cascade/useBestNuclearModel false"; // no corresponding access method; unclear D !!!
                                                            // From G4CascadeParameters.cc: BEST_PAR = (0!=G4NUCMODEL_USE_BEST);
							    // probably means that if env.var. is NOT set, this option isn't in use
   fDefaultBertini.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
  
   return;

}
void artg4tk::ModelParamStudyProducer::printDefaults()
{

   fLogInfo << "BERTINI Cascade Default Parameters: \n"; // << std::endl;
   //std::cout << std::endl;
   for ( unsigned int i=0; i<fDefaultBertini.size(); ++i )
   {
      fLogInfo << fDefaultBertini[i] << "\n"; // << std::endl;
   }
   
   return;

}
void artg4tk::ModelParamStudyProducer::restoreDefaults()
{
   
   if ( fDefaultPhysics )
   {
      // Change G4State to Idle
      // G4UImanager will only apply modifications if
      // 1. G4State_Idle
      // 2. G4State_PreInit
      // 
      // Once run is initialized and prepared to start, the state is set to GeomClosed;
      // for this reason, G4UIcommend::IsAvailable will return false, and G4UImanager will NOT apply it
      //
      if(!G4StateManager::GetStateManager()->SetNewState(G4State_Idle))            
         fLogInfo << "G4StateManager PROBLEM! "; // << std::endl;
      // run over the container of default Bertini parameters
      // and restore defaults
      for ( unsigned int i=0; i<fDefaultBertini.size(); ++i )
      {
//         std::ostringstream cmd;
//         cmd << (it->first).c_str() << " " << it->second ;
//         fUI->ApplyCommand( cmd.str() );
         fUI->ApplyCommand( (fDefaultBertini[i]).c_str() );
	 if (fVerbosity > 0 ) fLogInfo << "Default restored: " << fDefaultBertini[i] << "\n"; // << std::endl;
      }
      //
      // Now return G4State back to GeomClosed
      //
      if(!G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed))            
         fLogInfo << "G4StateManager PROBLEM! "; // << std::endl;
   }
   
   return;

}

void artg4tk::ModelParamStudyProducer::modifyParameters( bool changestate )
{

   if ( fDefaultPhysics ) return; // better if spit a warning !

   // the Instance() will call the ctor, which in turn will instanciate
   // the associated G4CascadeParamMesseger and the its (sub)directory
   // of the UI commands
   //
   // if not "touched" before, the (sub)directory of the Bertini commands
   // directory will be empty, and no command passed in via G4UI/ApplyCommand
   // will be found, so changes will never apply
   //
   // it's possible that the UI directory is instnatiated somehow via RunManager
   // because when running with a physics list the issue doesn't seem to show
   // but it did come up in a process-level job/app
   //
   G4CascadeParameters::Instance();
   
   // Change G4State to Idle because...
   // ...once initialized, G4UImanager will only apply modifications if
   // 1. G4State_Idle
   // 2. G4State_PreInit
   // 
   // Once run is initialized and prepared to start, the state is set to GeomClosed;
   // for this reason, G4UIcommend::IsAvailable will return false, and G4UImanager will NOT apply it
   //
   if ( changestate )
   {
      if(!G4StateManager::GetStateManager()->SetNewState(G4State_Idle))            
         fLogInfo << "G4StateManager PROBLEM! "; // << std::endl;
   }
   
   // run over params to modify
   //
   for ( unsigned int i=0; i<fCommands2ModifyBertini.size(); ++i )
   {
      fUI->ApplyCommand( (fCommands2ModifyBertini[i]).c_str() );
      if (fVerbosity > 0 ) 
      {
         fLogInfo << "Modification applied: " << fCommands2ModifyBertini[i] << "\n"; // << std::endl;
      }
      std::cout << "Modification applied: " << fCommands2ModifyBertini[i] << "\n" << std::endl;
   }
   
   // Now rerutn the State back to GeomClosed
   //
   if ( changestate )
   {
      if(!G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed))            
         fLogInfo << "G4StateManager PROBLEM! "; // << std::endl;
   }
   
   return;

}

*/

void artg4tk::ModelParamStudyProducer::clear()
{

   // 
   // NOTE(JVY): This is needed when we go WITHOUT GDML AND its fillEventWithArtHits stuff;
   //            if/when we go WITH it, HadInteractionSD::clear() will be called from there,
   //            so there wont be a ny need to clear here
   //
   f1stHadIntSD->clear();
   
   // NOTE(JVY): in principle, one can call G4RunManager::TerminateOneEvent() method,
   //            where a G4Event gets deleted unless it's marked "toBeKept"...
   //            ... which is not the case/goal for this app
   //
   delete fCurrentG4Event;
   fCurrentG4Event = 0;

   return;

}

DEFINE_ART_MODULE(artg4tk::ModelParamStudyProducer)
