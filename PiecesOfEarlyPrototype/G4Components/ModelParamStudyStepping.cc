
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyStepping.hh"
// --> old inc --> #include "artg4tk/G4PhysModelParamStudy/DataProd/G4Interaction.hh"
#include "artg4tk/DataProducts/G4DetectorHits/G4Interaction.hh"
#include "Geant4/G4Track.hh"
#include "Geant4/G4Step.hh"

ModelParamStudyStepping::ModelParamStudyStepping()
   : G4UserSteppingAction()
{

   fTargetPtr = 0;
   fFirstInter = new G4Interaction( true );
   fStopAfter1stInt = true;
   
}

ModelParamStudyStepping::~ModelParamStudyStepping()
{
   if ( fFirstInter ) delete fFirstInter;
   for ( unsigned int i=0; i<fOtherInter.size(); ++i )
   {
      if ( fOtherInter[i] ) delete fOtherInter[i];
   }
   fOtherInter.clear();
}


void ModelParamStudyStepping::UserSteppingAction( const G4Step* theStep ) 
{
   
   G4VPhysicalVolume* vol = theStep->GetTrack()->GetVolume();
   
   if ( vol != fTargetPtr ) return; // we're outside primary target
   
   // G4cout << " Target materials: " << vol->GetLogicalVolume()->GetMaterial()->GetName() << G4endl;
   
   const std::vector<const G4Track*>* secs = theStep->GetSecondaryInCurrentStep();
   int nsec = secs->size();
   
   // check if anything generated in this step; if not, just return (nothing to do)
   //
   if ( nsec <=0 ) return;
      
   // check if primary track looses idenity
   //
   bool is_1st_inelastic_int = false;
   if ( theStep->GetTrack()->GetTrackStatus() != fAlive )
   {
      if ( theStep->GetTrack()->GetParentID() == 0 )
      {
         is_1st_inelastic_int = true;
         fFirstInter->SetIncomingTrack( theStep->GetTrack() );
/*
	 
	 std::cout << " Stepping: 1st hadronic interaction " << std::endl;
	 std::cout << " Stepping: volume ptr = " << vol << std::endl;
	 std::cout << " Stepping: volume name = " << vol->GetName() << std::endl;
	 std::cout << " Stepping: position = " << theStep->GetTrack()->GetPosition().x() << " "
	                                       << theStep->GetTrack()->GetPosition().y() << " "
					       << theStep->GetTrack()->GetPosition().z() << std::endl;
         std::cout << " Stepping: total momentum = " << theStep->GetTrack()->GetMomentum().mag() << std::endl;
	 std::cout << " Stepping: re-check total momentum = " 
	           << theStep->GetTrack()->GetDynamicParticle()->GetTotalMomentum() << std::endl;
         std::cout << " Stepping: re-check pre-step total momentum = " 
	           << theStep->GetPreStepPoint()->GetMomentum().mag() << std::endl;
*/
	 
	 
	 if ( fStopAfter1stInt ) theStep->GetTrack()->SetTrackStatus( fStopAndKill );
         for ( int i=0; i<nsec; i++ )
         {
	    G4Track* tr = new G4Track( *((*secs)[i]) );
            fFirstInter->AddSecondary( tr );
            if ( fStopAfter1stInt ) tr->SetTrackStatus( fStopAndKill );	    
	 }	 
      }
   }
   
// test printouts
//   G4cout << " In volume: " << vol->GetName() << G4endl;
//   G4cout << " Number of Secondaries: " << nsec << G4endl;   
//   const G4ThreeVector& curpos = theStep->GetPostStepPoint()->GetPosition();
//   G4cout << " PostStepPoint " << curpos.x() << " " << curpos.y() << " " << curpos.z() << G4endl;
//   G4cout << " 1st inelastic int: " << is_1st_inelastic_int << G4endl;
   
   if ( is_1st_inelastic_int && fStopAfter1stInt ) return;
   
   // we get here if it's a secondary(etc) interactions, and we're configured to go on
      
   fOtherInter.push_back( new G4Interaction() );
   fOtherInter.back()->SetIncomingTrack( theStep->GetTrack() );
      
   for ( int i=0; i<nsec; i++ )
   {
      G4Track* tr = new G4Track( *((*secs)[i]) );
      fOtherInter.back()->AddSecondary( tr );
   }      

   return;
   
}

void ModelParamStudyStepping::ClearForNextEvent()
{

   int nsc = fFirstInter->GetNumberOfSecondaries();
   if ( nsc > 0  )
   {
         for ( int i=0; i<nsc; i++) 
         {   
            delete fFirstInter->GetSecondary(i);
         } 
         fFirstInter->Clear();    
   }
   
   int nother = fOtherInter.size();
   for ( int i=0; i<nother; ++i )
   {
      int nsc1 = fOtherInter[i]->GetNumberOfSecondaries();
      if ( nsc1 > 0 )
      {
         for ( int j=0; j<nsc1; ++j) 
         {   
            delete fOtherInter[i]->GetSecondary(j);
         } 
         fOtherInter[i]->Clear();    
      }
   }

   return;

}
