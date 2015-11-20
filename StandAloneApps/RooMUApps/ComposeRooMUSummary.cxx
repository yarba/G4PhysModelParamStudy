#include <iostream>
#include <string>
#include <vector>
#include "TRandom3.h"
#include "TCanvas.h"
#include "TFile.h"

#include "PlotUtils/MUH1D.h"
#include "PlotUtils/MUApplication.h"
#include "PlotUtils/MUPlotter.h"

using namespace std;
using namespace PlotUtils;

int main( int argc, char** argv )
{

   if ( argc != 3 )
   {
      std::cout << " Missing argument: input file " << std::endl;
      std::cout << " Usage format: ComposeRooMUSummary -i <inputfilename>.root " << std::endl;
      return 1;
   }
   
   TH1::SetDefaultSumw2();

   // std::string hfilename = "G4ModelParamStudy_copy.root";
   std::string hfilename = argv[2];
   
   std::string defaultdir = "G4BertiniDefaultVsHARP";
   std::vector<std::string> variantdirs;
   variantdirs.push_back("G4BertiniModi1VsHARP");
   variantdirs.push_back("G4BertiniModi2VsHARP");
   
//   std::string summarydir = "RooMUHistosSummary"; 
   std::string summarydir = "RooMUHistSummary"; 
      
   TFile* hfile = TFile::Open( hfilename.c_str(), "UPDATE" ); 

   // add directory for RooMUHistos objects
   //
   if ( !( hfile->GetDirectory( summarydir.c_str() ) ) )
   {
      hfile->mkdir( summarydir.c_str() );
   }
   else
   {
      hfile->GetDirectory( summarydir.c_str() )->Clear();
   }
   
   // iterator(s) over all objects in the "default" directory
   //
   TIter     next( hfile->GetDirectory( defaultdir.c_str() )->GetListOfKeys() );
   TKey*     key = (TKey*)next();
   TH1D*      h = 0;
   
   while ( key )
   {
         
      if ( !(TClass::GetClass(key->GetClassName())->InheritsFrom(TH1::Class())) ) 
      {
         key = (TKey*)next();
	 continue;
      }
      h = (TH1D*)key->ReadObj();
      const char* hname = h->GetName();
//      std::string muh1name = "muh1_" + std::string( hname );
      MUH1D* muh1 = new MUH1D( *h, 1. );
      muh1->SetDirectory(0);
//      muh1->RenameHistoAndErrorBands( muh1name.c_str() );
      muh1->SetTitle( h->GetTitle() );
      
      std::vector<TH1D*> hvariants;
      hvariants.reserve( variantdirs.size() );
      hvariants.clear();
      for ( unsigned int i=0; i<variantdirs.size(); ++i )
      {
         std::string hvarname = variantdirs[i] + "/" + std::string( hname );
	 hvariants.push_back( (TH1D*)(hfile->Get(hvarname.c_str())) );	 
      }
            
      std::string muh1vband = "vband_" + std::string( hname );
      muh1->AddVertErrorBand( muh1vband, hvariants );      
      
      hfile->cd( summarydir.c_str() );
      muh1->Write();
      hfile->cd( defaultdir.c_str() );
      key = (TKey*)next();
   }
   
   hfile->Close();
   
   TFile* tstfile = TFile::Open( hfilename.c_str(), "READ" );

   TCanvas* cnv1 = new TCanvas("cnv1","",1000.,600.);
   cnv1->Divide(2,1);
   
   std::string htstname = summarydir + "/" + "piplus_LA_2";
   MUH1D* muh1tst = (MUH1D*)tstfile->Get( htstname.c_str() );
   
   cnv1->cd(1);
   TH1D hcv = muh1tst->GetCVHistoWithError();
   hcv.SetLineColor(kRed);
   hcv.SetLineWidth(3);
   hcv.GetYaxis()->SetRangeUser(0.,165.);
   hcv.DrawCopy("E1hist");

//   size_t nvbands = muh1tst->GetNVertErrorBands();
   std::vector<std::string> vbandnames = muh1tst->GetVertErrorBandNames();
   for ( unsigned int i=0; i<vbandnames.size(); ++i )
   {
      const MUVertErrorBand* vband = muh1tst->GetVertErrorBand( vbandnames[i] );
      int nvbhi = vband->GetNHists();
      for ( int ih=0; ih<nvbhi; ++ih )
      {
         // const TH1D* vbhi = vband->GetHist(ih);
	 TH1D* vbhi = (TH1D*)vband->GetHist(ih); // bad trick - casting awat const !!!
         // TH1D* vbhi = vband->GetHist(ih)->Clone();
	 vbhi->SetLineColor(kBlue+ih*3);
	 vbhi->SetLineStyle(2);
	 vbhi->SetLineWidth(3);
	 vbhi->DrawCopy("histsame");
      }
   }
      
   MUPlotter* muplt = new MUPlotter();
   cnv1->cd(2);
   muplt->DrawMCWithErrorBand( muh1tst );
   
   muplt->MultiPrint( cnv1, "compose-g4-roomu", "gif" ); 
   
   tstfile->Close();
    
   delete muplt;
   delete cnv1;
   
   return 0;

}
