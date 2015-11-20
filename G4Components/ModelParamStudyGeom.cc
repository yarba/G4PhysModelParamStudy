#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelParamStudyGeom.hh"

#include "Geant4/G4PhysicalConstants.hh"
#include "Geant4/G4SystemOfUnits.hh"

#include "Geant4/G4Material.hh"
#include "Geant4/G4NistManager.hh"

#include "Geant4/G4Box.hh"
#include "Geant4/G4Tubs.hh"
//#include "Geant4/G4Cons.hh"
#include "Geant4/G4LogicalVolume.hh"
#include "Geant4/G4ThreeVector.hh"
#include "Geant4/G4PVPlacement.hh"
#include "Geant4/G4GeometryManager.hh"
#include "Geant4/G4PhysicalVolumeStore.hh"
#include "Geant4/G4LogicalVolumeStore.hh"
#include "Geant4/G4SolidStore.hh"

#include "artg4tk/pluginDetectors/gdml/HadInteractionSD.hh"

// #include <fstream>
#include <string>
#include <iostream>
#include <sstream>
// #include <iomanip>

ModelParamStudyGeom::ModelParamStudyGeom()
   : G4VUserDetectorConstruction(),
     fShape("G4Box"),
     fMatName(""),
     fMaterial(0),
     fWorld(0), fSubWorld(0),
     fLogTarget(0), fPhysTarget(0)
{
   
   fTargetSize.reserve(3);
   fTargetPos.reserve(3);
   for ( int i=0; i<3; ++i )
   {
      fTargetSize[i] = 100.;
      fTargetPos[i]  = 0.;
   }

}
ModelParamStudyGeom::ModelParamStudyGeom( const fhicl::ParameterSet& pset )
   : G4VUserDetectorConstruction()
{

   fTargetSize = pset.get<std::vector<double> >("targetSize");
   fTargetPos  = pset.get<std::vector<double> >("targetPosition");
   assert(fTargetSize.size()==3);
   assert(fTargetPos.size()==3);
   for ( int i=0; i<3; ++i )
   {
      fTargetSize[i] *= mm;
      fTargetPos[i]  *= mm;
   }
   // std::string mat = pset.get<std::string>("targetMaterial");
   ResetMaterial( pset.get<std::string>("targetMaterial") );
   fShape      = pset.get<std::string>("targetShape");
   
}

ModelParamStudyGeom::~ModelParamStudyGeom()
{

   // Do I need to delete log/phys volumes ?
   // Or are they taken care of by someone else ?

}

G4Material* ModelParamStudyGeom::ResetMaterial( G4String mat )
{
   

   if ( mat.find("G4") != std::string::npos ) 
   {
      fMatName = mat ;
      fMaterial = G4NistManager::Instance()->FindOrBuildMaterial(fMatName);
      if (!fMaterial) 
      {
         G4cout << "Material <" << fMatName << "> is not found" << G4endl;
         exit(1);
       }
      return fMaterial;
   }

   std::ostringstream osMat(std::ios_base::out|std::ios_base::app);
   osMat.clear();
   osMat.str("G4_");
   osMat << mat;
   fMatName = osMat.str();

   fMaterial = G4NistManager::Instance()->FindOrBuildMaterial(fMatName);
   if (!fMaterial) 
   {
      G4cout << "Material <" << fMatName << "> is not found" << G4endl;
      exit(1);
   }
      
   return fMaterial;

}
void ModelParamStudyGeom::ResetMaterial( G4Material* mat )
{

   if ( !mat )
   {
      G4cout << "Invalid Material" << G4endl;
      exit(1);
   }
   
   
   if ( !mat->GetName() )
   {
      G4cout << "Invalid Material" << G4endl;
      exit(1);      
   }
    
   fMaterial = mat;
   fMatName = mat->GetName();

   return;

}

void ModelParamStudyGeom::ResetGeom()
{

   // Clean old geometry, if any.
   //
   G4GeometryManager::GetInstance()->OpenGeometry();
   G4PhysicalVolumeStore::GetInstance()->Clean();
   G4LogicalVolumeStore::GetInstance()->Clean();
   G4SolidStore::GetInstance()->Clean();

   return;

}

G4VPhysicalVolume* ModelParamStudyGeom::Construct()
{

   ResetGeom();
   
   G4Material* mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
   
   // G4Box* solidWorld = new G4Box( "world_solid", 10.*m, 10.*m, 10.*m );
   fWorld = new G4PVPlacement( 0, G4ThreeVector(), "world_phys",
                               new G4LogicalVolume( new G4Box( "world_solid", 10.*m, 10.*m, 10.*m ),
			                            mat, "world_log", 0, 0, 0 ), // mat=Vacuum !!!
			       0,
			       false, 0 );
   
   // G4Box* solidSubWorld = new G4Box( "subworld_solid", 9.999*m, 9.999*m, 9.999*m );
   fSubWorld = new G4PVPlacement( 0, G4ThreeVector(), "subworld_phys",
                                  new G4LogicalVolume( new G4Box( "subworld_solid", 9.999*m, 9.999*m, 9.999*m ),
				                       mat, "subworld_log", 0, 0, 0 ), // mat=Vacuum again
				  fWorld,
				  false, 0 );

   ConstructTarget();
   
   return fWorld;

}

G4VPhysicalVolume* ModelParamStudyGeom::ConstructTarget()
{

  // Clean old geometry, if any.
  //
/*
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();
*/

  G4CSGSolid* solid = 0;
  
  // TMP THINGS
  //
  if ( fShape != "G4Box" && fShape != "G4Tubs" )
  {
     throw cet::exception("DetectorHolderService") << " For now, only G4Box or G4Tubs is supported; your shape = " << fShape << "\n";
  }
  
 
  if ( fShape == "G4Box" )
  {
     solid = new G4Box ( "target_solid", 
                         0.5*fTargetSize[0], 
			 0.5*fTargetSize[1], 
			 0.5*fTargetSize[2] ); // size is already done in the right units (ctor)
  }
  else if ( fShape == "G4Tubs" )
  {
     solid = new G4Tubs( "solid", fTargetSize[0], fTargetSize[1], 0.5*fTargetSize[2], 0., 2.*pi );
  }
  
  if ( !solid )
  {
     throw cet::exception("DetectorHolderService") << "Can NOT construct target (solid); check shape and/or size\n";
  }
  
  fLogTarget = new G4LogicalVolume( solid, fMaterial, "target_log", 0, 0, 0);
  fLogTarget->SetSensitiveDetector( new artg4tk::HadInteractionSD("HadInteractionSD") );
  
  fPhysTarget = new G4PVPlacement( 0, G4ThreeVector(fTargetPos[0],fTargetPos[1],fTargetPos[2]), "target_phys",
                                   fLogTarget,
				   fSubWorld, // mother physical volume, which can be NULL
				   false, 0 );

  if ( !fPhysTarget )
  {
     throw cet::exception("DetectorHolderService") << "Can NOT construct target phys.volume\n"; 
  }

  return fPhysTarget; 

}

G4double ModelParamStudyGeom::GetRMax() const
{

   double RMax = 0;
   
   if ( fShape == "G4Box" )
   {
      RMax = 0.5 * std::min( fTargetSize[0], fTargetSize[1] );
   }
   else if ( fShape == "G4Tubs" )
   {
      RMax = fTargetSize[1] ;
   }
   
   return RMax;

}
