
#include "artg4tk/G4PhysModelParamStudy/AnalysisBase/ModelParamAnalyzerBase.hh"

#include "art/Framework/Core/ModuleMacros.h"

#include "art/Framework/Principal/Event.h"

// Run/Eevent data products
#include "artg4tk/DataProducts/G4DetectorHits/ArtG4tkVtx.hh"        // Event data product

// Root-specific headers
#include "TH1D.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "art/Framework/Services/Optional/TFileService.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <memory>

namespace artg4tk {

   class AnalyzerHARP : public ModelParamAnalyzerBase {
   
   public:
   
      explicit AnalyzerHARP( const fhicl::ParameterSet& );
      virtual ~AnalyzerHARP();
      
      virtual void analyze( const art::Event& event ) override;
      virtual void beginJob()                         override;
      virtual void endJob()                           override;
         
   private:
         
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
            
      // diagnostics output
      //
// -->      mf::LogInfo fLogInfo; // now in Base... do I nead a separate one for each analyzer ???
        
   };

}

artg4tk::AnalyzerHARP::AnalyzerHARP( const fhicl::ParameterSet& p )
  : artg4tk::ModelParamAnalyzerBase(p), 
    fNThetaBinsFW(4), fThetaMinFW(0.05), fDeltaThetaFW(0.05),
    fNThetaBinsLA(9), fThetaMinLA(0.35), fDeltaThetaLA(0.2) 
    // fLogInfo("AnalyzerHARP") // well, maybe each module does need its's own logger ???
{
}

artg4tk::AnalyzerHARP::~AnalyzerHARP()
{
   // do I need any delete's here ?!
   // or will TFileService take care of that ?!?!
}

void artg4tk::AnalyzerHARP::beginJob()
{

   TH1::SetDefaultSumw2();
      
   art::ServiceHandle<art::TFileService> tfs;
   fNSec = tfs->make<TH1D>( "NSec", "Number of secondary per inelastic interaction", 100, 0., 100 );
   
   double thetaMin = 0.;
   double thetaMax = 0.;
   std::string theta_bin_fw;
   std::string theta_bin_la;

   std::string htitle = ""; // FIXME !!!
   std::string hname  = "";

   double parbins_fw[] = { 0.5, 1.0, 1.5, 2., 2.5, 3., 3.5, 4., 5., 6.5, 8. };
   int    nparbins_fw = sizeof(parbins_fw) / sizeof(double) - 1;
      
   for ( int i=0; i<fNThetaBinsFW; i++ )
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

   for ( int i=0; i<fNThetaBinsLA; i++ )
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

void artg4tk::AnalyzerHARP::endJob()
{
   
   if ( !fXSecInit ) return;
   
   if ( fNSec->GetEntries() <= 0 ) return;

   // here I can also pull up whatever xsec I might need
   // but in order to do so, I might need to store material & beam particle
   
   double xbin = 1.;
   double norm = 1.;
   double scale = 1.;
   
   norm = fNSec->Integral();
   
   xbin = fNSec->GetBinWidth(1);
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

   return;

}

void artg4tk::AnalyzerHARP::analyze( const art::Event& e )
{
      
   art::Handle<ArtG4tkVtx> firstint;
   e.getByLabel( fProdLabel, firstint );
   if ( !firstint.isValid() )
   {
      fLogInfo << " handle to 1st hadronic interaction is NOT valid"; // << std::endl;
      return;
   }
   
   if ( !fXSecInit ) initXSecOnTarget( firstint->GetMaterialName(), firstint->GetIncoming() );
   
   // const std::vector<ArtG4tkParticle>& secs = firstint->GetAllOutcoming();
   // int nsec = secs.size();
   int nsec = firstint->GetNumOutcoming();
   if ( nsec > 0 ) fNSec->Fill( (double)nsec );
   
   for ( int ip=0; ip<nsec; ++ip )
   {
      const ArtG4tkParticle& sec = firstint->GetOutcoming( ip );
      
      std::string pname = sec.GetName();
      double pmom = sec.GetMomentum().vect().mag();
      
      pmom /= CLHEP::GeV;
      double theta = sec.GetMomentum().vect().theta();
      
      if ( theta < fThetaMinFW ) continue;
      if ( theta < fThetaMinFW+fDeltaThetaFW*fNThetaBinsFW )
      {
         int ith = ( theta - fThetaMinFW ) / fDeltaThetaFW;
         if ( pname == "pi-" )
         {
	      fHistoSecPiMinusFW[ith]->Fill( pmom ); //, xsec/millibarn );
         }
         else if ( pname == "pi+" )
         {
	      fHistoSecPiPlusFW[ith]->Fill( pmom ); //, xsec/millibarn );
         }
         else if ( pname == "proton" )
         {
	      fHistoSecProtonFW[ith]->Fill( pmom ); //, xsec/millibarn );
	 }
      }
      
      if ( theta < fThetaMinLA ) continue;
      if ( theta > fThetaMinLA+fDeltaThetaLA*fNThetaBinsLA ) continue;
      int    itheta = ( theta - fThetaMinLA ) / fDeltaThetaLA;
      if ( itheta < 0 || itheta >= fNThetaBinsLA ) continue;
      
      if ( pname == "pi-" )
      {
         fHistoSecPiMinusLA[itheta]->Fill( pmom );
      }
      else if ( pname == "pi+" )
      {
         fHistoSecPiPlusLA[itheta]->Fill( pmom );
      }
      else if ( pname == "proton" )
      {
         fHistoSecProtonLA[itheta]->Fill( pmom );
      }    

   } // end loop over secondaries

   return;
   
}

using artg4tk::AnalyzerHARP;
DEFINE_ART_MODULE(AnalyzerHARP)
