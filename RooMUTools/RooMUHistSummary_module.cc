//
// Authors: Julia Yarba

// Date: May 2015

#include <string>
#include <vector>

// Art includes
//
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"

// Services
//
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"

// Root 
//
#include "TFile.h"
#include "TDirectory.h"
#include "TH1D.h"
#include "TClass.h"

// RooMUHistos 
//
#include "PlotUtils/MUH1D.h"
#include "PlotUtils/MUApplication.h"

#include <iostream>

namespace artg4tk {

  class RooMUHistSummary : public art::EDAnalyzer 
  {
  
  public:

    // Constructor
    explicit RooMUHistSummary( fhicl::ParameterSet const& );

    // Destructor
    virtual ~RooMUHistSummary();

    virtual void analyze( art::Event const& ) override;
    virtual void beginJob()                   override;
    virtual void beginRun( art::Run const& )  override;
    virtual void endRun( art::Run const& )    override;
    virtual void endJob()                     override;

  private:
  
    // Diagnostics level (verbosity)
    //
    int                      fVerbosity;
    std::string              fG4DefaultDir;
    std::vector<std::string> fG4VariantDirs;
    
  };
}

artg4tk::RooMUHistSummary::RooMUHistSummary(fhicl::ParameterSet const& p )
  : art::EDAnalyzer(p)
{

   fVerbosity = p.get<int>("Verbosity",0);
   fG4DefaultDir  = p.get<std::string>("G4DefaultDirectory"); 
   fG4VariantDirs = p.get<std::vector<std::string> >("G4VariantDirectories");
   
   TH1::SetDefaultSumw2();
   
   // look in the PSet, module label would be there, most likely module_label data member
     
}

artg4tk::RooMUHistSummary::~RooMUHistSummary()
{

}

// At begin job
//
void artg4tk::RooMUHistSummary::beginJob()
{
   
   return;

}

void artg4tk::RooMUHistSummary::endJob()
{
   
   art::ServiceHandle<art::TFileService> tfs;

   TFile& hfile = tfs->file();
         
   // NOTE(JVY): at this point, it'll point at the directory that corresponds
   //            to the label of the last module that called TFileService
   // const TString oldDir = gDirectory->GetPath();
      
   // NOTE(JVY): While in memory, those are TObject(s) (OBJ), not TKey(s) (KEY) !!!
   //
   // TIter  next( hfile.GetDirectory( fG4DefaultDir.c_str() )->GetListOfKeys() );
   TIter  next( hfile.GetDirectory( fG4DefaultDir.c_str() )->GetList() );
         
   // TKey*     key = (TKey*)next();
   TObject* obj = next();
   TH1D*     h = 0;
   
//   while ( key )
   while ( obj )
   {

      // if ( !(TClass::GetClass(key->GetClassName())->InheritsFrom(TH1::Class())) ) 
      if ( !(TClass::GetClass(obj->ClassName())->InheritsFrom(TH1::Class())) ) 
      {
         // key = (TKey*)next();
	 obj = next();
	 continue;
      }
      // h = (TH1D*)key->ReadObj();
      h = (TH1D*)obj;
      const char* hname = h->GetName();            
//      std::string muh1name = "muh1_" + std::string( hname );
      // --> MUH1D* muh1 = new MUH1D( *h, 1. );
      // --> muh1->SetDirectory(0);
      PlotUtils::MUH1D* muh1 = tfs->make<PlotUtils::MUH1D>( *h, 1. );
      // NOTE(JVY): and now, gDirectory->GetPath() will point at this module's subdir !
//      muh1->RenameHistoAndErrorBands( muh1name.c_str() );
      muh1->SetTitle( h->GetTitle() );
      std::vector<TH1D*> hvariants;
      hvariants.reserve( fG4VariantDirs.size() );
      hvariants.clear();
      for ( unsigned int i=0; i<fG4VariantDirs.size(); ++i )
      {
         std::string hvarname = fG4VariantDirs[i] + "/" + std::string( hname );
	 hvariants.push_back( (TH1D*)(hfile.Get(hvarname.c_str())) );	 
      }
      std::string muh1vband = "vband_" + std::string( hname );
      muh1->AddVertErrorBand( muh1vband, hvariants );      
      // not necessary... as it produces all the same complains about missing dictionaries
      // for pair<string,PlotUtils::MULatErrorBand*>, pair<string,TMatrixT<double>*>, pair<string,TH1D*>
      // muh1->Write();  
      hfile.cd( fG4DefaultDir.c_str() );
      // NOTE(JVY): and here, gDirectory->GetPath() will point at fG4DefaultDir !
      // key = (TKey*)next();
      obj = next();
   }
   
   return;

}

// At begin run
//
void artg4tk::RooMUHistSummary::beginRun( art::Run const& r )
{  
      
   return;

}

//
void artg4tk::RooMUHistSummary::analyze( art::Event const& e )
{
      
   if ( fVerbosity > 0 )  std::cout << "RooMUSummary: at event " << e.id().event() << "\n" << std::endl;

   return;

}

// At end run
//
void artg4tk::RooMUHistSummary::endRun( art::Run const& r)
{

   return;

}

using artg4tk::RooMUHistSummary;
DEFINE_ART_MODULE(RooMUHistSummary)
