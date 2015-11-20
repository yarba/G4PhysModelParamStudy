#ifndef MPStudyBase_H
#define MPStudyBase_H

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"

class G4UImanager;
class G4RunManager;
class ModelParamStudyStepping;
class G4Event;

namespace artg4tk {

  class ModelParamStudyBase : public art::EDAnalyzer 
  {
  
  public:

    // ctor & dtor
    //
    explicit ModelParamStudyBase( const fhicl::ParameterSet& );
    virtual ~ModelParamStudyBase();

    // Overriding analyzer members functions
    //
    virtual void analyze( const art::Event& ) override;
    virtual void beginJob() override;
    virtual void beginRun( const art::Run&  ) override;
    virtual void endRun( const art::Run& ) override;
    
    virtual void fillHisto() = 0;

  protected:
  
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
    G4UImanager*             fUI;
    // ModelParamStudyRunManager*     fRMPtr;
    G4RunManager*            fRMPtr;
    ModelParamStudyStepping* fSteppingPtr;
    G4Event*                 fCurrentG4Event;

    // Diagnostics level (verbosity)
    //
    int         fVerbosity;
    
    // data members specifically to study parameters modification
    //
    bool                           fDefaultPhysics;
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
} // end namespace artg4tk

#endif
