
#include "artg4tk/G4PhysModelParamStudy/ApplicationBase/ModelParamStudyBase.hh"

#include <iostream>

// Services
//
#include "art/Framework/Services/Registry/ServiceHandle.h"

// G4 (or derived) includes
//
#include "Geant4/G4RunManager.hh"
//#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyRunManager.hh"

#include "Geant4/G4StateManager.hh"
#include "Geant4/G4UImanager.hh"
#include "Geant4/G4CascadeParameters.hh"

#include "artg4tk/G4PhysModelParamStudy/G4Services/G4ModelParamStudy_service.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyStepping.hh"

#include "artg4tk/DataProducts/EventGenerators/GenParticle.hh"
#include "artg4tk/DataProducts/EventGenerators/GenParticleCollection.hh"

artg4tk::ModelParamStudyBase::ModelParamStudyBase( const fhicl::ParameterSet& p )
  : art::EDAnalyzer(p)
{
   
   fUI              = G4UImanager::GetUIpointer();
   fRMPtr           = 0;
   fSteppingPtr     = 0;
   fCurrentG4Event  = 0;
   fVerbosity       = p.get<int>("Verbosity",0);
   fDefaultPhysics  = p.get<bool>("DefaultPhysics",true);
   fCommands2ModifyParam = p.get<std::vector<std::string> >("ModelParamCommands");
   
   fillBertiniDefaults();
     
}

artg4tk::ModelParamStudyBase::~ModelParamStudyBase()
{

   // no need to clean up UI pointer - I don't "new" it, I just "get it"

   if ( fCurrentG4Event ) delete fCurrentG4Event;
   fCommands2ModifyParam.clear();
   fDefaultBertiniParam.clear();
   
}

// At begin job
//
void artg4tk::ModelParamStudyBase::beginJob()
{

   art::ServiceHandle<G4ModelParamStudyService> mpstudy;
   fRMPtr       = mpstudy->GetRunManager();
   fSteppingPtr = mpstudy->GetStepping();

   // NEED TO THROW IF NULL PTR
   //
   if ( !fRMPtr )
   {
      std::cout << " NULL RunManager Ptr !!!" << std::endl;
   }
   
   return;

}

// At begin run
//
void artg4tk::ModelParamStudyBase::beginRun( const art::Run& r )
{  
   
   if ( fVerbosity > 0 ) printDefaults();
      
   return;

}

// execute the Geant event
//
void artg4tk::ModelParamStudyBase::analyze( const art::Event& e )
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
       std::cout << " primgen handle is NOT valid " << std::endl;
       return;
   }
   
   // int size = primgenparts->size();
    
   G4ParticleTable* ptable = G4ParticleTable::GetParticleTable();
   
   // FIXME !!!
   // Need to check who owns this pointer, and whether it should be deleted after processing !!!
   // HINT: Check through G4RunManager && G4EventManager, and see where currentEvent goes...
   // ANS: I think it gets deleted unless it's marked "to be stored/archived"...
   //
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
      std::cout << "Processing art::Event " << e.id().event() << "\n" << std::endl;   
      std::cout << "G4Event " << e.id().event() << " has been generated " << std::endl; 
      fCurrentG4Event->Print();
      int nprimvtx = fCurrentG4Event->GetNumberOfPrimaryVertex();
      for ( int i=0; i<nprimvtx; ++i )
      {
         fCurrentG4Event->GetPrimaryVertex(i)->Print();
      }
      const G4String& version = fRMPtr->GetVersionString();
      std::cout << "Now Process it through " << version << std::endl;
   }
   
   // now either restore defaults or modify parameters if needs be
   // 
   if ( fDefaultPhysics )
   {
      restoreDefaults();
   }
   else
   {
      modifyParameters();
   }
   
   // fRMPtr->ProcessOneEvent( fCurrentG4Event );
   G4EventManager::GetEventManager()->ProcessOneEvent( fCurrentG4Event );
   if ( fVerbosity > 0 )
   {
      std::cout << "G4Event " << e.id().event() << " has been processed by EventManager " << std::endl;
   } 
   fRMPtr->AnalyzeEvent( fCurrentG4Event );
   // fRMPtr->UpdateScoring(); // can NOT use it outside G4RunManager (or deribved) since this method is protected

   fillHisto();
   
   fSteppingPtr->ClearForNextEvent();

   // NOTE(JVY): in principle, one can call G4RunManager::TerminateOneEvent() method,
   //            where a G4Event gets deleted unless it's marked "toBeKept"...
   //            ... which is not the case/goal for this app
   //
   delete fCurrentG4Event;
   fCurrentG4Event = 0;
   
   return;

}
// At end run
//
void artg4tk::ModelParamStudyBase::endRun( const art::Run& r)
{
   
   return;

}

void artg4tk::ModelParamStudyBase::fillBertiniDefaults()
{

   // NOTE(JVY): this procedure is quite boring; need to figure out if there's
   //            a more "algorithmic" approach...
   //
   
   std::ostringstream cmd;
   
   cmd << "/process/had/cascade/doCoalescence " << G4CascadeParameters::doCoalescence();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
/*
// these params/methods are NOT available in G4.9.6-series
// uncomment when switch to 10.1-ref03 or later !!!
   
   cmd << "/process/had/cascade/piNAbsorption " << G4CascadeParameters::piNAbsorption();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/use3BodyMom " << G4CascadeParameters::use3BodyMom();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/usePhaseSpace " << G4CascadeParameters::usePhaseSpace();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
*/
   cmd << "/process/had/cascade/useBestNuclearModel false"; // no corresponding access method; unclear D !!!
                                                            // From G4CascadeParameters.cc: BEST_PAR = (0!=G4NUCMODEL_USE_BEST);
							    // probably means that if env.var. is NOT set, this option isn't in use
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();

   cmd << "/process/had/cascade/useTwoParamNuclearRadius " << G4CascadeParameters::useTwoParam();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/nuclearRadiusScale " << G4CascadeParameters::radiusScale();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/smallNucleusRadius " << G4CascadeParameters::radiusSmall();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/alphaRadiusScale " << G4CascadeParameters::radiusAlpha();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/shadowningRadius " << G4CascadeParameters::radiusTrailing(); 
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/fermiScale " << G4CascadeParameters::fermiScale();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/crossSectionScale " << G4CascadeParameters::xsecScale();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/gammaQuasiDeutScale " << G4CascadeParameters::gammaQDScale();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/cluster2DPmax " << G4CascadeParameters::dpMaxDoublet();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/cluster3DPmax " << G4CascadeParameters::dpMaxTriplet();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();
   cmd << "/process/had/cascade/cluster4DPmax " << G4CascadeParameters::dpMaxAlpha();
   fDefaultBertiniParam.push_back( cmd.str() );
   cmd.str( "" );
   cmd.clear();

   return;

}

void artg4tk::ModelParamStudyBase::printDefaults()
{

   std::cout << "BERTINI Cascade Default Parameters: " << std::endl;
   std::cout << std::endl;
   for ( unsigned int i=0; i<fDefaultBertiniParam.size(); ++i )
   {
      std::cout << fDefaultBertiniParam[i] << std::endl;
   }
   
   return;

}
void artg4tk::ModelParamStudyBase::restoreDefaults()
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
         std::cout << "G4StateManager PROBLEM! " << std::endl;
      // run over the container of default Bertini parameters
      // and restore defaults
      for ( unsigned int i=0; i<fDefaultBertiniParam.size(); ++i )
      {
//         std::ostringstream cmd;
//         cmd << (it->first).c_str() << " " << it->second ;
//         fUI->ApplyCommand( cmd.str() );
         fUI->ApplyCommand( (fDefaultBertiniParam[i]).c_str() );
	 if (fVerbosity > 0 ) std::cout << "Default restored: " << fDefaultBertiniParam[i] << std::endl;
      }
      //
      // Now return G4State back to GeomClosed
      //
      if(!G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed))            
         std::cout << "G4StateManager PROBLEM! " << std::endl;
   }
   
   return;

}

void artg4tk::ModelParamStudyBase::modifyParameters()
{

   if ( fDefaultPhysics ) return; // better if spit a warning !

   // Change G4State to Idle
   // G4UImanager will only apply modifications if
   // 1. G4State_Idle
   // 2. G4State_PreInit
   // 
   // Once run is initialized and prepared to start, the state is set to GeomClosed;
   // for this reason, G4UIcommend::IsAvailable will return false, and G4UImanager will NOT apply it
   //
   if(!G4StateManager::GetStateManager()->SetNewState(G4State_Idle))            
      std::cout << "G4StateManager PROBLEM! " << std::endl;
   
   // run over params to modify
   //
   for ( unsigned int i=0; i<fCommands2ModifyParam.size(); ++i )
   {
      fUI->ApplyCommand( (fCommands2ModifyParam[i]).c_str() );
      if (fVerbosity > 0 ) 
      {
         std::cout << "Modification applied: " << fCommands2ModifyParam[i] << std::endl;
         // std::cout << " Check it !!! " << G4CascadeParameters::radiusScale() << std::endl;
      }
   }
   
   // Now rerutn the State back to GeomClosed
   //
   if(!G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed))            
      std::cout << "G4StateManager PROBLEM! " << std::endl;
   
   return;

}
