//
// Build a dictionary.
//
#include <vector>
#include "artg4tk/G4PhysModelParamStudy/DataProd/ArtG4tkVtx.hh"
#include "artg4tk/G4PhysModelParamStudy/DataProd/ArtG4tkParticle.hh"
#include "art/Persistency/Common/Wrapper.h"
template class std::vector<artg4tk::ArtG4tkParticle>;
template class art::Wrapper<artg4tk::ArtG4tkVtx>;
