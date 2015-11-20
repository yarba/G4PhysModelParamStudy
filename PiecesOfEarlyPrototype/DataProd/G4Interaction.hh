//
//               __        __ __  __  __  
//   ____ ______/ /_____ _/ // / / /_/ /__
//  / __ `/ ___/ __/ __ `/ // /_/ __/ //_/
// / /_/ / /  / /_/ /_/ /__  __/ /_/ ,<   
// \__,_/_/   \__/\__, /  /_/  \__/_/|_|  
//               /____/                  
//
// artg4tk: art based Geant 4 Toolkit
// 
//=============================================================================
// G4Interaction.hh: holds outcome of an interaction + checks if 1st interaction or not
// Author: Julia Yarba (Fermilab)
//=============================================================================
#ifndef G4Interaction_H
#define G4Interaction_H 

#include "Geant4/G4VParticleChange.hh"

class G4Interaction : public G4VParticleChange
{

   public:
   
      G4Interaction() : fIsFirstInter(false), fIncomingTrack(0) {}
      G4Interaction( bool isFirst ) : fIsFirstInter( isFirst ), fIncomingTrack(0) {}
      virtual ~G4Interaction() { if (fIncomingTrack) delete fIncomingTrack; fIncomingTrack=0; }
      
      void           SetIncomingTrack( G4Track* trk ) { if (fIncomingTrack) delete fIncomingTrack;
                                                        fIncomingTrack=new G4Track(*trk);
						        fIncomingTrack->SetTouchableHandle(trk->GetTouchableHandle());
							fIncomingTrack->SetStep( new G4Step( *(trk->GetStep()) ) );
						        return; }
      bool           IsFirstInteraction() const { return fIsFirstInter; }
      const G4Track* GetIncomingTrack()   const { return fIncomingTrack; }

   private:
   
      bool     fIsFirstInter;
      G4Track* fIncomingTrack;

};

#endif
