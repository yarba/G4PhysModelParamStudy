#ifndef MPStudyGeom_H
#define MPStudyGeom_H 1

// G4
#include "Geant4/G4VUserDetectorConstruction.hh"
#include "Geant4/globals.hh"       

// Art
#include "fhiclcpp/ParameterSet.h"

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;


class ModelParamStudyGeom : public G4VUserDetectorConstruction {

public:

  ModelParamStudyGeom();
  ModelParamStudyGeom( const fhicl::ParameterSet& );
  ~ModelParamStudyGeom();
  
  G4VPhysicalVolume*         ConstructTarget();
  virtual G4VPhysicalVolume* Construct();
  void                       ResetGeom();

  void        SetDimentions( G4double x1, G4double x2, G4double x3 ) { fTargetSize[0]=x1; fTargetSize[1]=x2, fTargetSize[2]=x3; return; }
  void        SetX( G4double val ) { fTargetSize[0]=val; return; }
  void        SetY( G4double val ) { fTargetSize[1]=val; return; }
  void        SetZ( G4double val ) { fTargetSize[2]=val; return; }
  void        SetRMin( G4double val ) { fTargetSize[0]=val; return; }
  void        SetRMax( G4double val ) { fTargetSize[1]=val; return; }
  void        SetShape( G4String val ) { fShape=val; return; }
  
  G4Material* ResetMaterial( G4String mat );
  void        ResetMaterial( G4Material* mat ); 

  G4Material*        GetCurrentMaterial() const { return fMaterial; }
  G4VPhysicalVolume* GetTarget()          const { return fPhysTarget; }
  G4double           GetRMax()    const;
  G4double           GetZLength() const { return fTargetSize[2]; } 

private:

  std::vector<G4double> fTargetSize;
  std::vector<G4double> fTargetPos;
  
  G4String    fShape;
    
  G4String    fMatName;
  G4Material* fMaterial;
  
  G4VPhysicalVolume* fWorld;
  G4VPhysicalVolume* fSubWorld;

  // the real stuff
  //
  G4LogicalVolume*   fLogTarget;
  G4VPhysicalVolume* fPhysTarget;

};

#endif
