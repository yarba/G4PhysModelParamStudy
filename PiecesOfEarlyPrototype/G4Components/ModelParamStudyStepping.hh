
#ifndef MPStudyStepping_H
#define MPStudyStepping_H

#include "Geant4/G4UserSteppingAction.hh"

#include <vector>

class G4Interaction;
class G4VPhysicalVolume;

class ModelParamStudyStepping : public G4UserSteppingAction
{

   public:
   
      // ctor & dtor
      //
      ModelParamStudyStepping();
      ~ModelParamStudyStepping();

      // principal method
      //
      virtual void UserSteppingAction( const G4Step* );
      
      // local methods
      //
      void SetTargetPtr( G4VPhysicalVolume* tptr ) { fTargetPtr=tptr; return; }
      void SetStopAfter1stInteraction( bool stop=true ) { fStopAfter1stInt = stop; return; }
      void ClearForNextEvent();
      
      // getters
      //
      const G4Interaction* Get1stInteraction()   const { return fFirstInter; }
      const std::vector<G4Interaction*>& GetOtherInteractions() const { return fOtherInter; }

   private:
   
      G4VPhysicalVolume*          fTargetPtr;
      G4Interaction*              fFirstInter;
      std::vector<G4Interaction*> fOtherInter;
      bool                        fStopAfter1stInt;   
            
};

#endif
