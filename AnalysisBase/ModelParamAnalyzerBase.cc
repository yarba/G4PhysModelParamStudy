
#include "artg4tk/G4PhysModelParamStudy/AnalysisBase/ModelParamAnalyzerBase.hh"

// Run/Eevent data products
// NOTE(JVY): NO NEED to include since they all come via header
//
// --> -->  #include "artg4tk/DataProducts/G4DetectorHits/ArtG4tkVtx.hh"        // Event data product
// --> --> #include "artg4tk/DataProducts/G4DetectorHits/ArtG4tkParticle.hh"        // Event data product// --> --> 
// --> --> #include "artg4tk/DataProducts/G4ModelConfig/ArtG4tkModelConfig.hh" // Run data product

// G4-specific headers
#include "Geant4/G4Material.hh"
#include "Geant4/G4NistManager.hh"
#include "Geant4/G4ParticleTable.hh"
#include "Geant4/G4DynamicParticle.hh"
#include "Geant4/G4HadronCrossSections.hh"

// Root-specific headers
#include "TObjString.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "art/Framework/Services/Optional/TFileService.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <memory>


artg4tk::ModelParamAnalyzerBase::ModelParamAnalyzerBase( const fhicl::ParameterSet& p )
  : art::EDAnalyzer(p),
    fXSecInit(false),
    fLogInfo("ModelParamAnalyzer")
{

   fProdLabel = p.get<std::string>("ProductLabel");
   
}

artg4tk::ModelParamAnalyzerBase::~ModelParamAnalyzerBase()
{
   // do I need any delete's here ?!
   // or will TFileService take care of that ?!?!
}


void artg4tk::ModelParamAnalyzerBase::beginRun( const art::Run& r )
{

   art::Handle<ArtG4tkModelConfig> physcfg;
   r.getByLabel( fProdLabel, physcfg );
   if ( !physcfg.isValid() )
   {
      fLogInfo << " handle to run product (model/physics config) is NOT valid"; // << std::endl;
      return;
   }   
 
   art::ServiceHandle<art::TFileService> tfs;

   // NOTE-1(JVY): The TObjArray (of TObjString's) will be created in the right directory of the TFile.
   //              However, giving it an explicit name here will NOT help the I/O - see another note later.
   //
   fModelConfig = tfs->make<TObjArray>();

   std::vector<std::string> cfgmodels = physcfg->GetConfiguredModels();
   for ( unsigned int i=0; i<cfgmodels.size(); ++i )
   {
      const std::vector< std::pair<std::string,double> >* mcfg = physcfg->GetConfig( cfgmodels[i] );
      for ( unsigned int ii=0; ii<mcfg->size(); ++ii )
      {
	 std::string cfginfo = cfgmodels[i] + ": ";
	 cfginfo += ( (*mcfg)[ii].first + "=" );
	 std::ostringstream os;
	 os << (*mcfg)[ii].second;
	 cfginfo += os.str();
	 fModelConfig->Add( new TObjString( cfginfo.c_str() ) );
      }  
   }
      
   // NOTE-2(JVY): One must explicitly call the Write(...) method on the TObjArray (unlike e.g. TH-whatever),
   //              and give theobject's name at this point, plus one should specify to use a "single key" option
   //              on write, because otherwise Root will dump it as a sequence of whatever objects one stores
   //              in the array, using the same name for each one, so accessing them will be a headache.
   //
   fModelConfig->Write("Geant4ModelConfig",1); // NOTE(JVY): 2nd arg tells TObjArray to be written 
                                               //            with the use of a "singke key", i.e. as an TObjArray,
					       //            not a sequence of TObjects (TObjString's)
        
   return;

}

void artg4tk::ModelParamAnalyzerBase::initXSecOnTarget( const std::string& mname,
                                                        const artg4tk::ArtG4tkParticle& part )
{

// NOTE/FIXME (JV): probably it's better to take the beam particle 
//                  directly from the generator record, via the event !!!

   G4Material* mat = G4NistManager::Instance()->FindOrBuildMaterial( mname );
   assert(mat);
   
   const G4Element* elm = mat->GetElement(0);  

   int A = (int)(elm->GetN()+0.5);
   int Z = (int)(elm->GetZ()+0.5);
   assert(A);
   assert(Z);
   
   G4ParticleTable* ptable = G4ParticleTable::GetParticleTable();
   G4ParticleDefinition* g4pd = ptable->FindParticle( part.GetPDG() );
   assert(g4pd);
      
//   G4DynamicParticle g4pdyn( g4pd, (part.GetMomentum().vect())*CLHEP::GeV );
   G4DynamicParticle g4pdyn( g4pd, part.GetMomentum().vect() );
   fXSecOnTarget = (G4HadronCrossSections::Instance())->GetInelasticCrossSection( &g4pdyn, Z, A );
   
   fXSecOnTarget /= CLHEP::millibarn;
      
   fXSecInit = true;
   
   return;

}

