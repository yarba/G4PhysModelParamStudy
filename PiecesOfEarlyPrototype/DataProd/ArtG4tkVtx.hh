#ifndef ArtG4tkVtx_H
#define ArtG4tkVtx_H

// no need as they come via ArtG4tkParticle
//#include "CLHEP/Vector/LorentzVector.h"
// #include "CLHEP/Vector/ThreeVector.h"

#include "artg4tk/G4PhysModelParamStudy/DataProd/ArtG4tkParticle.hh"

#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"

#include<string>
// #include <ostream>

namespace artg4tk {

class ArtG4tkVtx 
{

   public:
   
      // NOTE(JVY): it might also be useful to add status:
      //            none, decay, interaction (em, had, elastic, inelastic,...)
      
      ArtG4tkVtx() : fPhysVolumeName(""), fMaterialName(""), fIncoming(), fOutcoming() {}
      ArtG4tkVtx( const CLHEP::HepLorentzVector& pos, 
                  const std::string& pvname, 
		  const std::string& mname ) : fPosition(pos), fPhysVolumeName(pvname), fMaterialName(mname),
		                               fIncoming(), fOutcoming() {}
      ArtG4tkVtx( const ArtG4tkVtx& rhs ) : fPosition(rhs.fPosition),
                                            fPhysVolumeName(rhs.fPhysVolumeName),
                                            fMaterialName(rhs.fMaterialName),
					    fIncoming(rhs.fIncoming),
					    fOutcoming(rhs.fOutcoming) {}
      ~ArtG4tkVtx() {}
      
      void Clear() { fPosition=CLHEP::HepLorentzVector(0.,0.,0.,0);
                     fPhysVolumeName=""; fMaterialName="";
		     fIncoming=ArtG4tkParticle(); fOutcoming.clear();
		     return; }
      
      void SetPosition( const CLHEP::HepLorentzVector& pos )     { fPosition=pos; return; }
      void SetPhysVolumeName( const std::string& pvname )        { fPhysVolumeName=pvname; return; }
      void SetMaterialName( const std::string& mname )           { fMaterialName=mname; return; }

      void SetIncoming( const ArtG4tkParticle& pin )             { fIncoming=pin; return; }
      void SetIncoming( const int pdg, const CLHEP::Hep3Vector& mom ) { fIncoming.SetPDG(pdg); fIncoming.SetMomentum(mom); return; }
      void SetIncoming( const int pdg, const CLHEP::HepLorentzVector& mom ) { fIncoming.SetPDG(pdg); fIncoming.SetMomentum(mom); return; }
      void AddOutcoming( const ArtG4tkParticle& pout ) { fOutcoming.push_back( pout ); return; }
      
      const CLHEP::HepLorentzVector&         GetPosition()            const { return fPosition; }
      const std::string&                     GetPhysVolumeName()      const { return fPhysVolumeName; }
      const std::string&                     GetMaterialName()        const { return fMaterialName; }    
      const ArtG4tkParticle&                 GetIncoming()            const { return fIncoming; }
            int                              GetNumOutcoming()        const { return fOutcoming.size(); }
      const std::vector<ArtG4tkParticle>&    GetAllOutcoming()        const { return fOutcoming; }
      const ArtG4tkParticle&                 GetOutcoming( int i )    const { return fOutcoming[i]; }

      // Patch up dictionary generation.
      // NOTE(from GenParticle, by HW):
      // There is source in art::PtrVector that should be protected and gccxml trying to instantiate it.
      // Until that is fixed, we need to provide a an operator< or else the dictionary generation fails.
      // So we provide a bogus version here. This is never actually used but,if gccxml cannot find 
      // the method, it quits.
      //
      bool operator<( const ArtG4tkVtx& ) const { return true; }
      
   private:
      
      // data members
      //
      CLHEP::HepLorentzVector         fPosition;
      std::string                     fPhysVolumeName;
      std::string                     fMaterialName;
      //art::Ptr<ArtG4tkParticle>       fIncoming;
      //art::PtrVector<ArtG4tkParticle> fOutcoming;
      ArtG4tkParticle                 fIncoming;
      std::vector<ArtG4tkParticle>    fOutcoming;
     
};

}

#endif
