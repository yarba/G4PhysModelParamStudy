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

  class AnotherToyAnalyzer : public art::EDAnalyzer 
  {
  
  public:

    // ctor & dtor
    //
    explicit AnotherToyAnalyzer( const fhicl::ParameterSet& );
    virtual ~AnotherToyAnalyzer();

    // Overriding analyzer members functions
    //
    virtual void analyze( const art::Event& ) override;
    virtual void beginJob() override;
    virtual void beginRun( const art::Run&  ) override;
    virtual void endRun( const art::Run& ) override;

  private:
  
    // Diagnostics level (verbosity)
    //
    int         fVerbosity;
    std::string fGreetingMsg;                
    
  };
}

artg4tk::AnotherToyAnalyzer::AnotherToyAnalyzer( const fhicl::ParameterSet& p )
  : art::EDAnalyzer(p)
{


   std::cout << " IN CTOR OF AnotherToyAnalyzer " << std::endl;
   
   fVerbosity = p.get<int>("verbosity",0);
   fGreetingMsg = p.get<std::string>("greetingMsg","HEY GUYS");
     
}

artg4tk::AnotherToyAnalyzer::~AnotherToyAnalyzer()
{

}

// At begin job
//
void artg4tk::AnotherToyAnalyzer::beginJob()
{

   std::cout << " NOW IN AnotherToyAnalyzer::beginJob() " << std::endl;
   
   return;

}

// At begin run
//
void artg4tk::AnotherToyAnalyzer::beginRun( art::Run const& r )
{  
   
   std::cout << " NOW IN AnotherToyAnalyzer::beginRun() " << std::endl;
   std::cout << fGreetingMsg << std::endl;
      
   return;

}

// execute the Geant event
//
void artg4tk::AnotherToyAnalyzer::analyze( art::Event const& e )
{
      
   if ( fVerbosity > 0 )  std::cout << "Producing event " << e.id().event() << "\n" << std::endl;

   return;

}

// At end run
//
void artg4tk::AnotherToyAnalyzer::endRun( art::Run const& r)
{

   std::cout << " NOW IN AnotherToyAnalyzer::endRun() " << std::endl; 
   
   return;

}


using artg4tk::AnotherToyAnalyzer;
DEFINE_ART_MODULE(AnotherToyAnalyzer)
