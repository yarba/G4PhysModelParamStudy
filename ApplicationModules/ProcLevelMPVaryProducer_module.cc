
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

// --> seems to come via some other inc #include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "messagefacility/MessageLogger/MessageLogger.h" 

#include <iostream>
#include <cmath>
#include <memory>

// G4 includes
#include "Geant4/G4ParticleTable.hh"
#include "Geant4/G4IonTable.hh"
//
#include "Geant4/G4ParticleChange.hh"
#include "Geant4/G4DynamicParticle.hh"
#include "Geant4/G4GenericIon.hh"
//
#include "Geant4/G4MesonConstructor.hh"
#include "Geant4/G4BaryonConstructor.hh"
#include "Geant4/G4IonConstructor.hh"
#include "Geant4/G4LeptonConstructor.hh"
#include "Geant4/G4BosonConstructor.hh"
#include "Geant4/G4Proton.hh"
#include "Geant4/G4Gamma.hh"
#include "Geant4/G4PionMinus.hh"
#include "Geant4/G4PionPlus.hh"
//
#include "Geant4/G4Material.hh"
#include "Geant4/G4IsotopeVector.hh"
#include "Geant4/G4ElementVector.hh"
#include "Geant4/G4NistManager.hh"
//
#include "Geant4/G4HadronCrossSections.hh"
#include "Geant4/G4VCrossSectionDataSet.hh"
#include "Geant4/G4HadronInelasticDataSet.hh"
#include "Geant4/G4BGGNucleonInelasticXS.hh"
#include "Geant4/G4BGGPionInelasticXS.hh"
//
#include "Geant4/G4ForceCondition.hh"
//
#include "Geant4/G4ProcessManager.hh"
#include "Geant4/G4StateManager.hh"
#include "Geant4/G4TrackingManager.hh" // NOTE(JVY): apparetly, this also brings G4Track and G4Step
//
// and finally, the "beef" - Geant4 Hadronic Model Interfaces !
//
#include "Geant4/G4HadronicProcess.hh"
#include "Geant4/G4CascadeInterface.hh"
#include "Geant4/G4INCLXXInterface.hh"
//
#include "artg4tk/G4PhysModelParamStudy/G4Components/ProcessWrapper.hh"

// model setup & params change services 
#include "artg4tk/G4PhysModelParamStudy/G4Services/PhysModelConfig_service.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Services/ProcLevelSimSetup_service.hh"

// Event/Run data products (produced here)
#include "artg4tk/DataProducts/G4DetectorHits/ArtG4tkVtx.hh" // Evt product
#include "artg4tk/DataProducts/G4ModelConfig/ArtG4tkModelConfig.hh" // Run product

// Incoming Event data products
#include "artg4tk/DataProducts/EventGenerators/GenParticle.hh"
#include "artg4tk/DataProducts/EventGenerators/GenParticleCollection.hh"

namespace artg4tk {

  class ProcLevelMPVaryProducer : public art::EDProducer {

  public:

    explicit ProcLevelMPVaryProducer( const fhicl::ParameterSet& pset );
    virtual ~ProcLevelMPVaryProducer();

    virtual void produce( art::Event& event ) override;
    virtual void beginJob()                   override;
    virtual void beginRun( art::Run&  )       override;
    virtual void endRun( art::Run& )          override;

  private:
       
     // special purpose functions
     //
     void initParticles();
     void initProcess();
     void setProjectile( const GenParticle& );
     void resetProjectileKine( const GenParticle& );
     
    // data members
    //
    bool fParticlesInit;
    bool fProcessInit;
    //
    // data members specifically to study parameters modification
    //
    std::string              fModelName;
// -->-->    std::vector<std::string> fModelOptions;
    bool                fDefaultPhysics;
    //
    // internal things needed to setup and process a HAD interaction 
    //
    G4HadronicProcess*  fProcWrapper;
    G4ProcessManager*   fProcManager;
    G4Material*  fTgtMaterial; 
    G4Track*     fTrack;
    G4Step*      fStep;  
        
    //
    // This one is needed since we'll have to insert 
    // model(s) config info as a RunProduct (in beginRun(...))
    //
    ArtG4tkModelConfig* fModelConfig;
    //
    
    // diagnostics printouts
    //
    int         fVerbosity;
    mf::LogInfo fLogInfo;
    
  };

}

artg4tk::ProcLevelMPVaryProducer::ProcLevelMPVaryProducer( const fhicl::ParameterSet& p )
   : fLogInfo("ProcLevelMPVaryProducer")
{
    
   fParticlesInit = false;
   fProcessInit   = false;

   // Check the state
   //
   if(!G4StateManager::GetStateManager()->SetNewState(G4State_PreInit))
      fLogInfo << "G4StateManager PROBLEM! "; //  << G4endl;
   
   
   art::ServiceHandle<ProcLevelSimSetupService> proclevelsetup;
   fModelName = proclevelsetup->GetModelName();
   
   // CRITICAL !!!
   // Model(s) (or at least Bertini) config/params should be in BEFORE
   // the model's ctor is invoked !!!
   //   
   //
   // TRACKED PSet(s) - outputs with different settings will NOT mix
   //
   fhicl::ParameterSet physicscfg = p.get<fhicl::ParameterSet>("HadronicModel");
      
   fDefaultPhysics = physicscfg.get<bool>("DefaultPhysics"); // make it TRACKED !!!
   fModelConfig = new ArtG4tkModelConfig(fDefaultPhysics);
   if ( !fDefaultPhysics )
   {
      fhicl::ParameterSet modelcfg = physicscfg.get<fhicl::ParameterSet>("ModelParameters");
      // mock up tmp pset to look like the one in phys.list-based producer for the RunProduct::Fill(...)
      fhicl::ParameterSet tmp;
      tmp.put( fModelName, modelcfg );
      fModelConfig->Fill( tmp );
      art::ServiceHandle<PhysModelConfigService> physcfgservice;
      if ( !modelcfg.is_empty() ) physcfgservice->ConfigureModel(fModelName,modelcfg);
   }
   
   fProcWrapper = 0;
   fProcManager = 0;
   fTgtMaterial = proclevelsetup->GetTgtMaterial();
   fTrack       = 0;
   fStep        = 0;

   initParticles();
   
   // Set state to Idle for further inits
   //
   if(!G4StateManager::GetStateManager()->SetNewState(G4State_Idle))
      fLogInfo << "G4StateManager PROBLEM! "; //  << G4endl;
   
   initProcess();
      
   fVerbosity       = p.get<int>("Verbosity",0);
           
   // Run product
   // NOTE: Default 2nd arg is art::InEvent
   //
   produces<ArtG4tkModelConfig,art::InRun>();
   
   // technically speaking, a product can be registered with an "instance name: produces<prod>("instancename")
   // but then it needs to be placed into an event as follows: e.put(std::move(prod), "instancename")
   //
   produces<ArtG4tkVtx>();
   
}

artg4tk::ProcLevelMPVaryProducer::~ProcLevelMPVaryProducer()
{

   if ( fProcWrapper ) delete fProcWrapper;
   
   // BUT WE DO NOT DLETE THE PROCESS MANAGER
   
//   if ( fRegion )      delete fRegion;
   if ( fTrack)        delete fTrack;
   if ( fStep )        delete fStep;
   
   delete fModelConfig;
   
}

// At begin job
//
void artg4tk::ProcLevelMPVaryProducer::beginJob()
{
   
   return;

}

// At begin run
//
void artg4tk::ProcLevelMPVaryProducer::beginRun( art::Run& run )
{  
   

// here do proc.level stuff/init ??? Or in the ctor ??? or maybe in the very fisrt event in case !IsFullyInit ??? 
    
   // Last but not least:
   // Insert the RunProduct (model(s) config info)
   //
   std::unique_ptr<ArtG4tkModelConfig> pcfg(new ArtG4tkModelConfig(*fModelConfig));   
   run.put(std::move(pcfg));   

   return;

}

void artg4tk::ProcLevelMPVaryProducer::produce( art::Event& e )
{

   if ( !fParticlesInit || !fProcessInit ) 
   {
      fLogInfo << " Module is NOT properly initialized; bail out " ;
      return;
   }
   
   
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
       fLogInfo << " Handle to Generator Particle(s) is NOT valid "; //  << std::endl;
       return;
   }
   
   if ( primgenparts->empty() )
   {
      fLogInfo << " Generator Partticle(s) empty "; // << std::endl;
      return;
   }  
   
   if ( primgenparts->size() > 1 )
   {
      fLogInfo << " Multiple Generator Particles - only the 1st is taken as a PROJECTILE "; // << std::endl;
   }
    
   GenParticleCollection::const_iterator i=primgenparts->begin();
      
   if ( !fTrack  || ! fStep )
   {
      setProjectile( *i );
   }
   
   if ( i->pdgId() != fTrack->GetParticleDefinition()->GetPDGEncoding() )
   {
      setProjectile( *i );
   }
   
   if ( i->momentum().vect() != fTrack->GetDynamicParticle()->GetMomentum() )
   {
      resetProjectileKine( *i ); // FIXME !!! This still needs to be implemented !!!
   }
   
   if ( !fTrack || !fStep )
   {
      fLogInfo << " Event " << e.id().event() << ": projectile problem - INVALID track and/or step; bail out ";
      return;
   }
      
   // FIXME !!!
   // Need to put in place provisions for AtRestDoIt(...), etc !!! 
   //  
   G4VParticleChange* thechange = fProcWrapper->PostStepDoIt( *fTrack, *fStep );
   
   int nsec = thechange->GetNumberOfSecondaries();
   
   if ( nsec <= 0 ) 
   {
      fLogInfo << " Event " << e.id().event() << ": NO secondaries in HAD interaction " ;
      return;
   }

   // Create an (empty) output data product
   //
   std::unique_ptr<ArtG4tkVtx> firstint(new ArtG4tkVtx());

   // Product Id of the data product to be created 
   // it was said to be needed for persistent pointers 
   // but in practice it doesn't look like it is...
   //
   // art::ProductID firstintID(getProductID<ArtG4tkVtx>(e));

   firstint->SetPosition( CLHEP::HepLorentzVector( fTrack->GetPosition(), fTrack->GetGlobalTime() ) );   
   //
   // FIXME !!!
   //
   // firstint->SetPhysVolumeName( vol->GetName().c_str() );
   firstint->SetPhysVolumeName("");
   //
   firstint->SetMaterialName( fTgtMaterial->GetName().c_str() );
   //
   firstint->SetIncoming( ArtG4tkParticle( fTrack->GetParticleDefinition()->GetPDGEncoding(),
	                                   fTrack->GetMomentum() ) );

   for ( int is=0; is<nsec; ++is )
   {
      G4Track* strk = thechange->GetSecondary(is);
      firstint->AddOutcoming( ArtG4tkParticle( strk->GetParticleDefinition()->GetPDGEncoding(),
	                                       strk->GetMomentum() ) );
   }

   // Put the output collection into the event
   //
   // NOTE: technically speaking, one can also add an "instance name" to a product, 
   //       and put it in as follows: e.put(std::move(prod), "instancename" )
   //       but in order to do so, it needs to be registered with such instance name,
   //       via produces<prod>("instancename")
   //       otherwise event processor will sagfault 
   //
   e.put(std::move(firstint)); 
   
   thechange->Clear();   
   
   return;
    
}

// At end run
//
void artg4tk::ProcLevelMPVaryProducer::endRun( art::Run& )
{
   
   return;

}

void artg4tk::ProcLevelMPVaryProducer::initParticles()
{

   if ( fParticlesInit ) return;
   
   // physics needs to be initialized before the 1st use of particle table,
   // because it constructs particles - otherwise the table is just empty
   //
   G4MesonConstructor pMesonConstructor;
   pMesonConstructor.ConstructParticle();

   G4BaryonConstructor pBaryonConstructor;
   pBaryonConstructor.ConstructParticle();  
  
   // This is needed because starting 9.6.ref05 G4IonTable::CtreateIon(...)
   // explicitly checks if generic ion has a process manager
   //
   G4GenericIon* gion = G4GenericIon::GenericIon();
   gion->SetProcessManager(new G4ProcessManager(gion));    
   //
   G4IonConstructor pIonConstructor;
   pIonConstructor.ConstructParticle();
  
   G4LeptonConstructor pLeptonConstructor;
   pLeptonConstructor.ConstructParticle();
  
   G4BosonConstructor pBosonConstructor;
   pBosonConstructor.ConstructParticle();

   G4ParticleTable* partTable = G4ParticleTable::GetParticleTable();
   partTable->SetReadiness();

   // --> G4IonTable* ionTable = partTable->GetIonTable();
   // --> ionTable->CreateAllIon();
   // --> ionTable->CreateAllIsomer();

   fParticlesInit = true; 
   
   return;

}

void artg4tk::ProcLevelMPVaryProducer::initProcess()
{

   if ( fProcessInit ) return;
   
   std::string mod = fModelName; 
   for( unsigned int i=0; i<mod.size(); ++i ) mod[i] = std::tolower(mod[i]);

   ProcessWrapper* pw = 0;

   if ( mod.find("bertini") != std::string::npos )
   {
      pw = new ProcessWrapper( "BertiniProcessWrapper" ); // I should probably give it a unique name
                                                          // - need to double check how multiple instances of Bertini work !
      G4CascadeInterface* bert = new G4CascadeInterface();
      bert->SetMaxEnergy(15.*CLHEP::GeV);
      pw->RegisterMe(bert);      
   }
   else if ( mod.find("incl++") != std::string::npos || mod.find("inclxx") != std::string::npos )
   {
      pw = new ProcessWrapper( "INCLXXProcessWrapper" );
      G4INCLXXInterface* inclxx = new G4INCLXXInterface();
      inclxx->SetMinEnergy(1.*CLHEP::MeV); // this is what stands in the INCL-based PL for protons 
                                    // it's 0. for pions, but let's make it uniform
      inclxx->SetMaxEnergy(20.*CLHEP::GeV); // this is what stands in the INCL-based PL for p,n,pi
                                     // overlap is typically set at 15-20GeV with a string model 
				     // (higher energy model)
				     //
				     // in principle INCL++ is good up to 3AGeV for p,n,pi, A(<18)
      pw->RegisterMe(inclxx);
   }
    
   if (!pw) // need to stream to mlog !!!
   { 
//      G4cout 
//	     << " generator " << name << " is unavailable"
//	     << G4endl;
//      exit(1);
   } 
        
/* this is for future, when we add ftf and/or qgs

   if ( name.find("lund-str-fragm") != std::string::npos )
   {
      // fProcWrapper->UseG4LundStringFragm(true);
      pw->UseG4LundStringFragm(true);
   }
   pw->Compose();
*/
   
   fProcWrapper = pw;

   fProcessInit = true;
   
   return;

}

void artg4tk::ProcLevelMPVaryProducer::setProjectile( const GenParticle& genpart )
{

   if ( fTrack ) delete fTrack;
   if ( fStep )  delete fStep;

   G4ParticleDefinition* partDef = (G4ParticleTable::GetParticleTable())->FindParticle( genpart.pdgId() );

   // Track
   //
   fTrack = new G4Track( new G4DynamicParticle( partDef, genpart.momentum()*CLHEP::GeV ), // momentum in GeV/c !!! 
                         0., genpart.position()*CLHEP::mm ); // 2nd arg is time (in ns ?) 
   fTrack->SetTouchableHandle( G4TouchableHandle( new G4TouchableHistory() ) );

    // Step
    //
    fStep = new G4Step();
    fStep->SetTrack(fTrack);
    fTrack->SetStep(fStep);
    
    // step points
    //
    // pre-step
    //
    fStep->SetPreStepPoint( new G4StepPoint() );

    fStep->GetPreStepPoint()->SetPosition( genpart.position()*CLHEP::mm );
    fStep->GetPreStepPoint()->SetMaterial( fTgtMaterial );
    fStep->GetPreStepPoint()->SetSafety( 100000.*CLHEP::mm );

    CLHEP::Hep3Vector dir = genpart.momentum().vect();
    dir /= genpart.momentum().vect().mag();
    
    G4ThreeVector bPosition = dir * 0.1*CLHEP::mm;
    bPosition += genpart.position()*CLHEP::mm;
    fStep->SetPostStepPoint( new G4StepPoint() );
    fStep->GetPostStepPoint()->SetPosition( bPosition );
    fStep->GetPostStepPoint()->SetMaterial( fTgtMaterial );
    fStep->GetPostStepPoint()->SetSafety( 100000.*CLHEP::mm );

   return;

}

void artg4tk::ProcLevelMPVaryProducer::resetProjectileKine( const GenParticle& part )
{

   return;

}

DEFINE_ART_MODULE(artg4tk::ProcLevelMPVaryProducer)
