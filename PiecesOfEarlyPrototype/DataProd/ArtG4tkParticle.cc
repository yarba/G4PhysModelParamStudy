
#include "artg4tk/G4PhysModelParamStudy/DataProd/ArtG4tkParticle.hh"

#include "Geant4/G4ParticleTable.hh"

#include <assert.h>

artg4tk::ArtG4tkParticle::ArtG4tkParticle()
{

   fPDG = 0;
   
}

artg4tk::ArtG4tkParticle::ArtG4tkParticle( const int pdg, const CLHEP::HepLorentzVector& mom )
{

   fPDG = pdg;
   fMomentum = mom;

}

artg4tk::ArtG4tkParticle::ArtG4tkParticle( const int pdg, const CLHEP::Hep3Vector& mom )
{

   fPDG = pdg;
   assert(fPDG!=0);
   G4ParticleTable* ptable = G4ParticleTable::GetParticleTable();
   G4ParticleDefinition* g4pd = ptable->FindParticle( fPDG );
   assert(g4pd);
   double mass = g4pd->GetPDGMass();
   double e = std::sqrt( mom.mag2() + mass*mass );
   fMomentum.setX( mom.x() );
   fMomentum.setY( mom.y() );
   fMomentum.setZ( mom.z() );
   fMomentum.setE( e );

}

artg4tk::ArtG4tkParticle::ArtG4tkParticle( const artg4tk::ArtG4tkParticle& other )
   : fPDG(other.fPDG), fMomentum(other.fMomentum)
{
}

//artg4tk::ArtG4tkParticle& ArtG4tkParticle::operator=( const ArtG4tkParticle& other )
//{
//
//   fPDG = other.fPDG;
//   fMomentum = other.fMomentum;
//   return *this;
//
//}

void artg4tk::ArtG4tkParticle::SetPDG( const int pdg )
{

   fPDG = pdg;
   return;

}

void artg4tk::ArtG4tkParticle::SetMomentum( const CLHEP::HepLorentzVector& mom )
{

   fMomentum = mom;
   return;

}

void artg4tk::ArtG4tkParticle::SetMomentum( const CLHEP::Hep3Vector& mom )
{

   
   assert(fPDG!=0);
   G4ParticleTable* ptable = G4ParticleTable::GetParticleTable();
   G4ParticleDefinition* g4pd = ptable->FindParticle( fPDG );
   assert(g4pd);
   double mass = g4pd->GetPDGMass();
   double e = std::sqrt( mom.mag2() + mass*mass );
   fMomentum.setX( mom.x() );
   fMomentum.setY( mom.y() );
   fMomentum.setZ( mom.z() );
   fMomentum.setE( e );
  
   return;

}

std::string artg4tk::ArtG4tkParticle::GetName() const 
{

   assert(fPDG!=0);
   G4ParticleTable* ptable = G4ParticleTable::GetParticleTable();
   G4ParticleDefinition* g4pd = ptable->FindParticle( fPDG );
   assert(g4pd);
   return std::string( g4pd->GetParticleName().c_str() );

}
