// Analyzer module for to study the effect of Geant4 physics model parameters modifications.
// Can be converted into a Producer if needs be in the future.
//
// Authors: Julia Yarba

// Date: April 2015

// Art includes
//
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"

#include <iostream>

// Services
//
#include "art/Framework/Services/Registry/ServiceHandle.h"
// ---> #include "art/Framework/Services/Optional/RandomNumberGenerator.h"

// G4 (or derived) includes
//
#include "Geant4/G4UImanager.hh"
#include "Geant4/G4CascadeParameters.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Services/G4ModelParamStudy_service.hh"
//#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyRunManager.hh"
#include "Geant4/G4RunManager.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyStepping.hh"

#include "artg4tk/DataProducts/EventGenerators/GenParticle.hh"
#include "artg4tk/DataProducts/EventGenerators/GenParticleCollection.hh"

namespace artg4tk {

  class G4ToyAnalyzer : public art::EDAnalyzer 
  {
  
  public:

    // ctor & dtor
    //
    explicit G4ToyAnalyzer( const fhicl::ParameterSet& );
    virtual ~G4ToyAnalyzer();

    // Overriding analyzer members functions
    //
    virtual void analyze( const art::Event& ) override;
    virtual void beginJob() override;
    virtual void beginRun( const art::Run&  ) override;
    virtual void endRun( const art::Run& ) override;

  private:
  
     // local private methods to deal with 
     // modifications of physics model parameters
     //
     void fillBertiniDefaults();
     void printDefaults();
     void restoreDefaults();
     void modifyParameters();

    // data members
    
    // G4 (or derived) things 
    //
    G4UImanager* fUI;
    // ModelParamStudyRunManager*     fRMPtr;
    G4RunManager*     fRMPtr;
    ModelParamStudyStepping*       fSteppingPtr;

    // Diagnostics level (verbosity)
    //
    int         fVerbosity;
    
    // data members specifically to study parameters modification
    //
    bool                           fDefaultPhysics;
    bool                           fPhysicsModified;
    //
    // NOTE(JVY): Ideally, here should be something like
    //            std::map< std::string, std::map<std::string, *** > >
    //            that'll tie together a model (by name) and a catalog 
    //            of available commands paired with D(efault)
    //            BUT !!! Bertini params are of different types (double, bool, etc.)
    //
    std::vector<std::string>       fCommands2ModifyParam;
    std::vector<std::string>       fDefaultBertiniParam;
    
  };
}

artg4tk::G4ToyAnalyzer::G4ToyAnalyzer( const fhicl::ParameterSet& p )
  : art::EDAnalyzer(p)
{

   std::cout << " IN CTOR OF G4ToyAnalyzer " << std::endl;
   
   fUI              = G4UImanager::GetUIpointer();
   fRMPtr           = 0;
   fSteppingPtr     = 0;
   fVerbosity       = p.get<int>("verbosity",0);
   fDefaultPhysics  = p.get<bool>("defaultPhysics",true);
   fPhysicsModified = false;
   
   fillBertiniDefaults();
     
}

artg4tk::G4ToyAnalyzer::~G4ToyAnalyzer()
{

//   if ( fUI ) delete fUI;
//   fUI = 0;
//   fCommands2ModifyParam.clear();
//   fDefaultBertiniParam.clear();
   
}

// At begin job
//
void artg4tk::G4ToyAnalyzer::beginJob()
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
//   else
//   {
//      std::cout << " RunManager Ptr is OK !!!" << std::endl;
//      const G4String& version = fRMPtr->GetVersionString();
//      std::cout << version << std::endl;
//   }
   
   return;

}

// At begin run
//
void artg4tk::G4ToyAnalyzer::beginRun( const art::Run& r )
{  
   
   if ( fVerbosity > 0 ) printDefaults();
      
   return;

}

// execute the Geant event
//
void artg4tk::G4ToyAnalyzer::analyze( const art::Event& e )
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
   //
   G4Event* g4evt = new G4Event(e.id().event() );

   for ( GenParticleCollection::const_iterator i=primgenparts->begin(); i!=primgenparts->end(); ++i ) 
   {
      int pdgcode = i->pdgId();
      G4ParticleDefinition* g4pd = ptable->FindParticle( pdgcode );
      if ( !g4pd ) continue;
      //
      // G4PrimaryVertex*   g4vtx = new G4PrimaryVertex( i->position()*mm, 0. ); // 4th arg is time(ns)
      //
      CLHEP::Hep3Vector pos(0.,0.,-1300.); // in mm !!!
      G4PrimaryVertex*   g4vtx = new G4PrimaryVertex( pos*CLHEP::mm, 0. ); // 4th arg is time(ns)
      const CLHEP::HepLorentzVector& mom = i->momentum();
      G4PrimaryParticle* g4prim = new G4PrimaryParticle( g4pd, 
                                                         mom.x()*CLHEP::GeV, mom.y()*CLHEP::GeV, mom.z()*CLHEP::GeV, 
							 mom.e()*CLHEP::GeV );        
      g4vtx->SetPrimary(g4prim);
      g4evt->AddPrimaryVertex(g4vtx);
   }
       
   if ( fVerbosity > 0 )  
   {
      std::cout << "Processing art::Event " << e.id().event() << "\n" << std::endl;   
      std::cout << "G4Event " << e.id().event() << " has been generated " << std::endl; 
      g4evt->Print();
      int nprimvtx = g4evt->GetNumberOfPrimaryVertex();
      for ( int i=0; i<nprimvtx; ++i )
      {
         g4evt->GetPrimaryVertex(i)->Print();
      }
      const G4String& version = fRMPtr->GetVersionString();
      std::cout << "Now Process it through " << version << std::endl;
   }
   
   // fRMPtr->ProcessOneEvent( g4evt );
   G4EventManager::GetEventManager()->ProcessOneEvent( g4evt );
   if ( fVerbosity > 0 )
   {
      std::cout << "G4Event " << e.id().event() << " has been processed by EventManager " << std::endl;
   } 
   fRMPtr->AnalyzeEvent( g4evt );
   // fRMPtr->UpdateScoring(); // can NOT use it outside G4RunManager (or deribved) since this method is protected
   
   return;

}

// At end run
//
void artg4tk::G4ToyAnalyzer::endRun( const art::Run& r)
{
   
   return;

}

void artg4tk::G4ToyAnalyzer::fillBertiniDefaults()
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

void artg4tk::G4ToyAnalyzer::printDefaults()
{

   std::cout << "BERTINI Cascade Default Parameters: " << std::endl;
   std::cout << std::endl;
   for ( unsigned int i=0; i<fDefaultBertiniParam.size(); ++i )
   {
      std::cout << fDefaultBertiniParam[i] << std::endl;
   }
   
   return;

}
void artg4tk::G4ToyAnalyzer::restoreDefaults()
{
   
   if ( fDefaultPhysics && fPhysicsModified )
   {
      // run over the container of default Bertini parameters
      // and restore defaults
      for ( unsigned int i=0; i<fDefaultBertiniParam.size(); ++i )
      {
//         std::ostringstream cmd;
//         cmd << (it->first).c_str() << " " << it->second ;
//         fUI->ApplyCommand( cmd.str() );
         fUI->ApplyCommand( (fDefaultBertiniParam[i]).c_str() );
      }
   }
   
   fPhysicsModified = false;
   
   return;

}

void artg4tk::G4ToyAnalyzer::modifyParameters()
{

   if ( fDefaultPhysics ) return; // better if spit a warning !
   
   // run over params to modify
   //
   for ( unsigned int i=0; i<fCommands2ModifyParam.size(); ++i )
   {
      fUI->ApplyCommand( fCommands2ModifyParam[i] );
   }
   
   fPhysicsModified = true;

   return;

}

using artg4tk::G4ToyAnalyzer;
DEFINE_ART_MODULE(G4ToyAnalyzer)
