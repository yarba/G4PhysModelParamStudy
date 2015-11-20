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

namespace artg4tk {

  class ToyAnalyzer : public art::EDAnalyzer 
  {
  
  public:

    // Constructor
    explicit ToyAnalyzer( fhicl::ParameterSet const& );

    // Destructor
    virtual ~ToyAnalyzer();

    // Overriding producer members
    // virtual void produce(art::Event & e) override;
    virtual void analyze( art::Event const& ) override;
    virtual void beginJob() override;
    virtual void beginRun( art::Run const& ) override;
    virtual void endRun( art::Run const& ) override;

  private:
  
    // Diagnostics level (verbosity)
    //
    int         fVerbosity;
    std::string fGreetingMsg;                
    
  };
}

artg4tk::ToyAnalyzer::ToyAnalyzer(fhicl::ParameterSet const& p )
  : art::EDAnalyzer(p)
{


   std::cout << " IN CTOR OF ToyAnalyzer " << std::endl;
   
   fVerbosity = p.get<int>("verbosity",0);
   fGreetingMsg = p.get<std::string>("greetingMsg","HEY BABY !!!");
     
}

artg4tk::ToyAnalyzer::~ToyAnalyzer()
{

}

// At begin job
//
void artg4tk::ToyAnalyzer::beginJob()
{

   std::cout << " NOW IN ToyAnalyzer::beginJob() " << std::endl;
   
   return;

}

// At begin run
//
void artg4tk::ToyAnalyzer::beginRun( art::Run const& r )
{  
   
   std::cout << " NOW IN ToyAnalyzer::beginRun() " << std::endl;
   
   return;

}

// execute the Geant event
//
void artg4tk::ToyAnalyzer::analyze( art::Event const& e )
{
   
   if ( e.id().event() == 1 ) std::cout << fGreetingMsg << std::endl;
   
   if ( fVerbosity > 0 )  std::cout << "Producing event " << e.id().event() << "\n" << std::endl;

   return;

}

// At end run
//
void artg4tk::ToyAnalyzer::endRun( art::Run const& r)
{

   return;

}


using artg4tk::ToyAnalyzer;
DEFINE_ART_MODULE(ToyAnalyzer)
