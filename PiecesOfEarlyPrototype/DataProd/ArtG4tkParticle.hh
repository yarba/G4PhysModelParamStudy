#ifndef ArtG4tkParticle_H
#define ArtG4tkParticle_H

#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"

#include<string>

#include <ostream>

namespace artg4tk {

class ArtG4tkParticle
{

   public:
   
      ArtG4tkParticle();
      ArtG4tkParticle( const int, const CLHEP::HepLorentzVector& );
      ArtG4tkParticle( const int, const CLHEP::Hep3Vector& );
      ArtG4tkParticle( const ArtG4tkParticle& );
      ~ArtG4tkParticle() {}

      ArtG4tkParticle& operator=( const ArtG4tkParticle& other ) { fPDG=other.fPDG; fMomentum=other.fMomentum; return *this; }
      
      void SetPDG( const int );
      void SetMomentum( const CLHEP::HepLorentzVector& );
      void SetMomentum( const CLHEP::Hep3Vector& );
      
      int                            GetPDG() const { return fPDG; }
      std::string                    GetName() const;
      const CLHEP::HepLorentzVector& GetMomentum() const { return fMomentum; }
      

      // Patch up dictionary generation.
      // NOTE(from GenParticle, by HW):
      // There is source in art::PtrVector that should be protected and gccxml trying to instantiate it.
      // Until that is fixed, we need to provide a an operator< or else the dictionary generation fails.
      // So we provide a bogus version here. This is never actually used but,if gccxml cannot find 
      // the method, it quits.
      //
      bool operator<( const ArtG4tkParticle& ) const { return true; }
   
   private:
   
      // data members
      //
      int                     fPDG;
      CLHEP::HepLorentzVector fMomentum;
      

};

}

#endif
