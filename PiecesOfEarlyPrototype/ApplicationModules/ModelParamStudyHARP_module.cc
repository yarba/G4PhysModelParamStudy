
#include "artg4tk/G4PhysModelParamStudy/ApplicationBase/ModelParamStudyBase.hh"

#include <iostream>

// Services
//
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "artg4tk/G4PhysModelParamStudy/G4Services/G4ModelParamStudy_service.hh"


// G4 (or derived) includes
//
// --> #include "Geant4/G4RunManager.hh"
//#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyRunManager.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyStepping.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyGeom.hh"
#include "Geant4/G4ParticleTable.hh"
#include "Geant4/G4HadronCrossSections.hh"

#include "artg4tk/DataProducts/G4DetectorHits/G4Interaction.hh"
#include "Geant4/G4Track.hh"

#include "TFile.h"
#include "TDirectory.h"
#include "TH1D.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "artg4tk/DataProducts/EventGenerators/GenParticle.hh"
#include "artg4tk/DataProducts/EventGenerators/GenParticleCollection.hh"

namespace artg4tk {

  class ModelParamStudyHARP : public ModelParamStudyBase 
  {
  
  public:
    // ctor & dtor
    //
    explicit ModelParamStudyHARP( const fhicl::ParameterSet& );
    virtual ~ModelParamStudyHARP();
    
    virtual void analyze( const art::Event& ) override;
    virtual void beginJob() override;
    // virtual void beginRun( const art::Run&  ) override; // no need
    // virtual void endRun( const art::Run& ) override;    // no need
    virtual void endJob() override;
    virtual void fillHisto() override;
  
  private:
  
     void fill1stInteraction();
     void fillOtherInteractions();
     void initXSecOnTarget( const art::Event& );
     
     TH1D*              fNSec;
     std::vector<TH1D*> fHistoSecPiMinusFW; 
     std::vector<TH1D*> fHistoSecPiPlusFW; 
     std::vector<TH1D*> fHistoSecPiMinusLA; 
     std::vector<TH1D*> fHistoSecPiPlusLA; 
     std::vector<TH1D*> fHistoSecProtonFW;
     std::vector<TH1D*> fHistoSecProtonLA;
      
     int                fNThetaBinsFW;
     double             fThetaMinFW;
     double             fDeltaThetaFW;   
     int                fNThetaBinsLA;
     double             fThetaMinLA;
     double             fDeltaThetaLA;  
     
     // FIXME !!!
     // In a general case, this aspect needs to be carefully designed
     // because a "generic" analyzer is not supposed to know in what
     // material an interaction took place, and the xsec, etc.
     //
     //int    fTargetA;
     //int    fTargetZ;
     double fXSecOnTarget; 
     bool   fXSecInit;
  
  };
}

artg4tk::ModelParamStudyHARP::ModelParamStudyHARP( const fhicl::ParameterSet& p )
  : artg4tk::ModelParamStudyBase(p),
    fNThetaBinsFW(4), fThetaMinFW(0.05), fDeltaThetaFW(0.05),
    fNThetaBinsLA(9), fThetaMinLA(0.35), fDeltaThetaLA(0.2),
    /*fTargetA(0), fTargetZ(0),*/ fXSecOnTarget(0.), fXSecInit(false)
{
}

artg4tk::ModelParamStudyHARP::~ModelParamStudyHARP()
{

   // do I need any delete's here ?!
   // or will TFileService take care of that ?!?!

}

void artg4tk::ModelParamStudyHARP::analyze( const art::Event& e )
{

   artg4tk::ModelParamStudyBase::analyze(e);
   
   if ( !fXSecInit ) initXSecOnTarget(e);
   
   return;

}

void artg4tk::ModelParamStudyHARP::beginJob()
{

   artg4tk::ModelParamStudyBase::beginJob();
   
   TH1::SetDefaultSumw2();
      
   art::ServiceHandle<art::TFileService> tfs;
   fNSec = tfs->make<TH1D>( "NSec", "Number of secondary per inelastic interaction", 100, 0., 100 );

   G4double thetaMin = 0.;
   G4double thetaMax = 0.;
   std::string theta_bin_fw;
   std::string theta_bin_la;

   std::string htitle = ""; // FIXME !!!
   std::string hname  = "";

   double parbins_fw[] = { 0.5, 1.0, 1.5, 2., 2.5, 3., 3.5, 4., 5., 6.5, 8. };
   int    nparbins_fw = sizeof(parbins_fw) / sizeof(double) - 1;
      
   for ( G4int i=0; i<fNThetaBinsFW; i++ )
   {
      thetaMin = fThetaMinFW + fDeltaThetaFW*i;
      thetaMax = thetaMin + fDeltaThetaFW;
      
      std::ostringstream osTitle1;
      std::ostringstream osTitle2;
      std::ostringstream osTitle3;
      
      osTitle1.clear();
      osTitle1 << thetaMin;
      theta_bin_fw = osTitle1.str() + " < theta < ";
      osTitle2.clear();
      osTitle2 << thetaMax;
      theta_bin_fw += osTitle2.str();
      theta_bin_fw += "(rad)";
   
      osTitle3.clear();
      osTitle3 << i;
      
      hname = "piminus_FW_" + osTitle3.str();         
      fHistoSecPiMinusFW.push_back( tfs->make<TH1D>( hname.c_str(), htitle.c_str(), nparbins_fw, parbins_fw ) );

      hname = "piplus_FW_" + osTitle3.str();
      fHistoSecPiPlusFW.push_back( tfs->make<TH1D>( hname.c_str(), htitle.c_str(), nparbins_fw, parbins_fw ) );

      hname = "proton_FW_" + osTitle3.str();
      fHistoSecProtonFW.push_back( tfs->make<TH1D>( hname.c_str(), htitle.c_str(),nparbins_fw, parbins_fw ) ); 
   
   }

   double parbins_la[] = { 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.6, 0.7, 0.8 };
   int    nparbins_la = sizeof(parbins_la) / sizeof(double) - 1;

   for ( G4int i=0; i<fNThetaBinsLA; i++ )
   {
     
      thetaMin = fThetaMinLA + fDeltaThetaLA*i;
      thetaMax = thetaMin + fDeltaThetaLA; 
     
      std::ostringstream osTitle1;
      std::ostringstream osTitle2;
      std::ostringstream osTitle3;

      osTitle1.clear();
      osTitle1 << thetaMin;
      theta_bin_la = osTitle1.str() + " < theta < ";
      osTitle2.clear();
      osTitle2 << thetaMax;
      theta_bin_la += osTitle2.str();
      theta_bin_la += "(rad)";
      
      osTitle3.clear();
      osTitle3 << i;
      
      hname = "piminus_LA_" + osTitle3.str();         
      fHistoSecPiMinusLA.push_back( tfs->make<TH1D>( hname.c_str(), htitle.c_str(), nparbins_la, parbins_la ) );

      hname = "piplus_LA_" + osTitle3.str();
      fHistoSecPiPlusLA.push_back( tfs->make<TH1D>( hname.c_str(), htitle.c_str(), nparbins_la, parbins_la ) );

      hname = "proton_LA_" + osTitle3.str();
      fHistoSecProtonLA.push_back( tfs->make<TH1D>( hname.c_str(), htitle.c_str(), nparbins_la, parbins_la ) );

   }

//    _directory = gDirectory;
//    std::cout << "******************************We are in the directory named: " << gDirectory->GetName() << std::endl;
//    _file = gDirectory->GetFile();
// NOTE: one can get the file directly from TFileService 

   return;

}

void artg4tk::ModelParamStudyHARP::endJob()
{

   if ( fNSec->GetEntries() <= 0 ) return;

   // here I can also pull up whatever xsec I might need
   // but in order to do so, I might need to store material & beam particle
   
   G4double xbin = 1.;
   G4double norm = 1.;
   G4double scale = 1.;
   
   norm = fNSec->Integral();
   
   xbin = (G4double)fNSec->GetBinWidth(1);
   scale = 1. / (xbin*norm);
   fNSec->Scale( scale ) ;

   // NOTE: no need to Write each histo as TFileService will do it   

   for ( size_t i=0; i<fHistoSecPiMinusFW.size(); ++i )
   {
      scale = fXSecOnTarget / (norm*fDeltaThetaFW);
      fHistoSecPiMinusFW[i]->Scale(scale,"width");
   }
   for ( size_t i=0; i<fHistoSecPiMinusLA.size(); ++i )
   {
      scale = fXSecOnTarget / (norm*fDeltaThetaLA);
      fHistoSecPiMinusLA[i]->Scale(scale,"width");
   }

   // secondary pi+
   //
   for ( size_t i=0; i<fHistoSecPiPlusFW.size(); ++i )
   {
      scale = fXSecOnTarget / (norm*fDeltaThetaFW);
      fHistoSecPiPlusFW[i]->Scale(scale,"width");
   }
   for ( size_t i=0; i<fHistoSecPiPlusLA.size(); ++i )
   {
      scale = fXSecOnTarget / (norm*fDeltaThetaLA);
      fHistoSecPiPlusLA[i]->Scale(scale,"width");
   }

   // secondary proton
   //
   for ( size_t i=0; i<fHistoSecProtonFW.size(); ++i )
   {
      scale = fXSecOnTarget / (norm*fDeltaThetaFW);
      fHistoSecProtonFW[i]->Scale( scale, "width" );
   }
   for ( size_t i=0; i<fHistoSecProtonLA.size(); ++i )
   {
      scale = fXSecOnTarget / (norm*fDeltaThetaLA);
      fHistoSecProtonLA[i]->Scale( scale, "width" );
   }

   // artg4tk::ModelParamStudyBase::endJob(); // FIXME !!! See later if it's needed for anything...
   
   return;

}

void artg4tk::ModelParamStudyHARP::fillHisto() 
{

   fill1stInteraction();
   fillOtherInteractions();
   return;

}

void artg4tk::ModelParamStudyHARP::fill1stInteraction()
{

   const G4Interaction* interaction = fSteppingPtr->Get1stInteraction();
   bool isFirst = interaction->IsFirstInteraction(); // double check
   
   if ( !isFirst ) return;
    
   int NSec = interaction->GetNumberOfSecondaries();
      
   if ( NSec > 0 ) fNSec->Fill( (double)NSec ); 

   const G4DynamicParticle* sec = 0;
   for (G4int i=0; i<NSec; i++) 
   {
        sec = interaction->GetSecondary(i)->GetDynamicParticle();			
	const G4String& pname = sec->GetDefinition()->GetParticleName();

	double pmom = sec->GetTotalMomentum() / CLHEP::GeV ;
	double theta = (sec->GetMomentum()).theta();
	
	if ( theta < fThetaMinFW ) continue;
	if ( theta < fThetaMinFW+fDeltaThetaFW*fNThetaBinsFW )
	{
	   G4int ith = ( theta - fThetaMinFW ) / fDeltaThetaFW;
	   if ( pname == "pi-" )
	   {
	      if ( isFirst ) fHistoSecPiMinusFW[ith]->Fill( pmom ); //, xsec/millibarn );
	   }
	   else if ( pname == "pi+" )
	   {
	      if ( isFirst ) fHistoSecPiPlusFW[ith]->Fill( pmom ); //, xsec/millibarn );
	   }
	   else if ( pname == "proton" )
	   {
	      fHistoSecProtonFW[ith]->Fill( pmom ); //, xsec/millibarn );
	   }
	}
	
	if ( theta < fThetaMinLA ) continue;
	if ( theta > fThetaMinLA+fDeltaThetaLA*fNThetaBinsLA ) continue;
	G4int    itheta = ( theta - fThetaMinLA ) / fDeltaThetaLA;
	if ( itheta < 0 || itheta >= fNThetaBinsLA ) continue;
	        
	if ( pname == "pi-" )
	{
	   if ( isFirst )
	   {
	      fHistoSecPiMinusLA[itheta]->Fill( pmom );
	   }
	}
	else if ( pname == "pi+" )
	{
	   if ( isFirst )
	   {
	      fHistoSecPiPlusLA[itheta]->Fill( pmom );
	   }
	   else if ( pname == "proton" )
	   {
	      fHistoSecProtonLA[itheta]->Fill( pmom );
	   }
	}

   }
      
   return;

}

void artg4tk::ModelParamStudyHARP::fillOtherInteractions()
{
   return;
}

void artg4tk::ModelParamStudyHARP::initXSecOnTarget( const art::Event& e )
{

// alternative access... but it takes dynamic casting which is not so nice...
//
//   const ModelParamStudyGeom* geom = dynamic_cast<const ModelParamStudyGeom*>(G4RunManager::GetRunManager()->GetUserDetectorConstruction());
//
   art::ServiceHandle<G4ModelParamStudyService> mpstudy;
   ModelParamStudyGeom* geom = mpstudy->GetGeometry();
   const G4Element* elm = geom->GetCurrentMaterial()->GetElement(0);
//
// cash here A and Z for further xsec calculation(s)
//
   int A = (int)(elm->GetN()+0.5);
   int Z = (int)(elm->GetZ()+0.5);
      
   assert(A>0);
   assert(Z>0);

   art::Handle<GenParticleCollection> primgenparts;
   e.getByLabel( "PrimaryGenerator", primgenparts );
   if ( !primgenparts.isValid() )
   {
       std::cout << " primgen handle is NOT valid " << std::endl;
       return;
   }
   
   // in this case, we need to make sure there's ONLY ONE BEAM PARTICLE !!!
   //
   assert(primgenparts->size()==1);
    
   G4ParticleTable* ptable = G4ParticleTable::GetParticleTable();
   GenParticleCollection::const_iterator i=primgenparts->begin();
   int pdgcode = i->pdgId();
   G4ParticleDefinition* g4pd = ptable->FindParticle( pdgcode );
   assert(g4pd);
   // const CLHEP::HepLorentzVector& mom = i->momentum();
   G4DynamicParticle g4pdyn( g4pd, (i->momentum()*CLHEP::GeV) );   
   fXSecOnTarget = (G4HadronCrossSections::Instance())->GetInelasticCrossSection( &g4pdyn, Z, A );
   
   fXSecOnTarget /= CLHEP::millibarn;
      
   fXSecInit = true;

   return;

}

using artg4tk::ModelParamStudyHARP;
DEFINE_ART_MODULE(ModelParamStudyHARP)
