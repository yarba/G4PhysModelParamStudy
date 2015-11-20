#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelConfigMapper.hh"

// Geant4
#include "Geant4/G4PhysicalConstants.hh"
#include "Geant4/G4SystemOfUnits.hh"
//
#include "Geant4/G4CascadeParameters.hh"
//
#include "Geant4/G4RunManager.hh"
#include "Geant4/G4UImanager.hh"
#include "Geant4/G4StateManager.hh"

// system/general
#include <iostream>
#include <cctype>

ModelConfigMapper::ModelConfigMapper()
{

   fNameConvention.insert( std::pair<std::string,std::string>("bertini","cascade") );
   fNameConvention.insert( std::pair<std::string,std::string>("inclxx", "inclxx" ) );
   fBaseCommand = "/process/had/";
   
   FillBertiniDefaults();
   FillINCLXXDefaults();
   
   FillConfigParamMapBertini();

}

ModelConfigMapper::~ModelConfigMapper()
{

   // Need to clean up here !

}

void ModelConfigMapper::PrintDefaults( const std::string& model ) // const
{
     
   std::string mod = model;
   mod = ToLower(mod);
   
   std::map< std::string, std::map<std::string,std::string> >::iterator itr=fDEFAULTS.find(mod);
   
   if ( itr == fDEFAULTS.end() )
   {
      G4cout << " Model " << model << "(" << mod <<") NOT found " << G4endl;
      return;   
   }

   std::map<std::string,std::string>::iterator itr1;
   
   itr1 = fNameConvention.find(mod);
   if ( itr1 == fNameConvention.end() )
   {
      G4cout << " Can NOT find Geant4 internal name for model " << model << "(" << mod <<") " << G4endl;
      return;   
   }
   
   std::string command = fBaseCommand + itr1->second;
   
   G4cout << " ***** Default settings for Geant4 model " << model << G4endl;
   
   itr1 = (itr->second).begin();
   for ( ; itr1!=(itr->second).end(); ++itr1 )
   {
      G4cout << " * " << command << itr1->first << " " << itr1->second << G4endl; 
   }
   
   G4cout << " ***** End of default settings for Geant4 model " << model << G4endl;
   
   return;

}

void ModelConfigMapper::SetVerbosity( const std::string& model, const bool& verb )
{

   std::string mod = model;
   mod = ToLower(mod);
   
   if ( fNameConvention.find(mod) == fNameConvention.end() )
   {
      G4cout << " Can NOT find internal name for model " << mod << G4endl;
      return;
   }
   
   std::map<std::string,std::string>::iterator itr=((fConfigParameters.find(mod))->second).find("verbosity");
   
   std::string command = fBaseCommand + (fNameConvention.find(mod))->second;
   command += itr->second;
   command += " ";
   std::ostringstream cmd;
   cmd << verb;
   if ( verb )
   {
      G4cout << " Turning ON verbosity for model " << model << G4endl;
   }
   else
   {
      G4cout << " Turning OFF verbosity for model " << model << G4endl;
   }
   
   command += cmd.str(); 
   
   G4ApplicationState currentstate = G4StateManager::GetStateManager()->GetCurrentState();
   bool ok = G4StateManager::GetStateManager()->SetNewState(G4State_Idle);
   if ( !ok )
   {
     G4cout << " G4StateManager PROBLEM: can NOT change state to G4State_Idle !" << G4endl;
     return;
   } 
   G4UImanager* uim = G4UImanager::GetUIpointer();   
   uim->ApplyCommand( command.c_str() );   
   ok = G4StateManager::GetStateManager()->SetNewState(currentstate);
      
   return;

}

void ModelConfigMapper::ChangeParameter( const std::string& model, const std::string& param, const double& value, bool verb )
{
   
   // First of all, make sure physics list is NOT setup to the RunManager
   //
   if ( G4RunManager::GetRunManager() ) // check if it exists at all
   {
      if ( G4RunManager::GetRunManager()->GetUserPhysicsList() )
      {
         // bail out with a warning
	 // it's USELESS to change anything after a physics list is set to the run manager
	 // because everything happens at the stage of physics list init (models ctor's)
	 G4cout << " Physics list is already assigned to Run Manager " << G4endl;
	 G4cout << " It is useless to change any model parameters since the changes will not propagate " << G4endl;
	 return;
      }
   
   }
   
   // FIXME !!!
   // Technically speaking, one needs to check if a(ny) physics list is already created
   // because one may create a physics list and assign it to the run manager later...
   // but in such case changing parameters wont make any sense either because they'll
   // NOT propagate to the guts of the model
   // Although we might want to check if it's the physics list ctor or its init...
   
   std::string mod = model;
   mod = ToLower(mod);
   
   std::map< std::string, std::map<std::string,std::string> >::iterator itr = fConfigParameters.find(mod);
   if ( itr == fConfigParameters.end() )
   {
      // bail out with a warning
      G4cout << " Can NOT find model " << model << " (" << mod << ")" << G4endl;
      return;
   }

   std::string par = param;
   par = ToLower(par);
   
   std::map<std::string,std::string>::iterator itr1 = (itr->second).find(par);
   
   if ( itr1 == (itr->second).end() )
   {
      // bail out with a warning
      G4cout << " Can NOT find parameter " << param << " (" << par << ")" << G4endl;
      return;
   }
   
   G4ApplicationState currentstate = G4StateManager::GetStateManager()->GetCurrentState();
   
   // changes propagate through G4UI only if the system is in certain states
   // (Idle or PreInit)
   //
   bool ok = G4StateManager::GetStateManager()->SetNewState(G4State_Idle);
   if ( !ok )
   {
     G4cout << " G4StateManager PROBLEM: can NOT change state to G4State_Idle !" << G4endl;
     return;
   }            
   
   // compose command to apply change
   //
   std::string command = fBaseCommand + (fNameConvention.find(mod))->second;
   command += ( itr1->second + " " );
   std::ostringstream cmd;
   cmd << value;   
   command += cmd.str();
   cmd.str("");
   cmd.clear();
   
   if ( verb )
   {
      G4cout << " New value of parameter " << param << "=" << value << G4endl;
      G4cout << " Applying G4UI command: " << command << G4endl;
   }

   G4UImanager* uim = G4UImanager::GetUIpointer();   
   uim->ApplyCommand( command.c_str() );   

//   if ( par == "radiusscale" )
//   {
//      G4cout << " Cross-check " << par << "=" << G4CascadeParameters::radiusScale() << G4endl;
//   }

   // restore previous state
   //  
   ok = G4StateManager::GetStateManager()->SetNewState(currentstate);

   return;

}

void ModelConfigMapper::ChangeParameter( const std::string& model, const std::string& param, const float& value, bool verb )
{

   double v1 = value;
   ChangeParameter( model, param, v1, verb );
   
   return;

}

void ModelConfigMapper::ChangeParameter( const std::string& model, const std::string& param, const int& value )
{

   return;

}

void ModelConfigMapper::ChangeParameterByRatio( const std::string& model, const std::string& param, const double& ratio, bool verb )
{

   std::string mod = model;
   mod = ToLower(mod);   
   
   std::map< std::string, std::map<std::string,std::string> >::iterator itr = fConfigParameters.find(mod);
   if ( itr == fConfigParameters.end() )
   {
      // bail out with a warning
      //
      G4cout << " Model " << model << "(" << mod <<") NOT found " << G4endl;
      return;
   }

   std::string par = param;
   par = ToLower(par);

   size_t found = par.find("byratio");
   if ( found != std::string::npos )
   {
      // remove "byratio" part from the parameter's name/key
      par.erase( found ); // will go to the end of the string since the D for len=std::string::npos 
   }
   
   std::map<std::string,std::string>::iterator itr1 = (itr->second).find(par);
   
   if ( itr1 == (itr->second).end() )
   {
      // bail out with a warning
      //
      G4cout << " Parameter " << param << "(" << par << ") for model " << model << " NOT found " << G4endl;
      return;
   }
   
   std::map< std::string, std::map<std::string,std::string> >::iterator itr2=fDEFAULTS.find(mod);
   if ( itr2 == fDEFAULTS.end() )
   {
      G4cout << " Can NOT find defaults for model " << model << "(" << mod <<")" << G4endl;
      return;
   }
   
   
   std::map<std::string,std::string>::iterator itr3=(itr2->second).find(itr1->second);
   
   double dvalue = std::atof( (itr3->second).c_str() ); 
   double newvalue = dvalue*ratio;
   
   if ( verb )
   {
      G4cout << " Changing " << model << " parameter " << param << " (" << par << ")" 
             << " by Ratio=" << ratio << ", from Default=" << dvalue << G4endl;
   }
      
   ChangeParameter( model, par, newvalue, verb );
   
//   if ( par == "radiusscale" )
//   {
//      G4cout << " Cross-check " << par << "=" << G4CascadeParameters::radiusScale() << G4endl;
//   }

   return;

}

void ModelConfigMapper::ChangeParameterByRatio( const std::string& model, const std::string& param, const float& ratio, bool verb )
{

   double v1 = ratio;
   ChangeParameterByRatio( model, param, v1, verb );
   return;

}

void ModelConfigMapper::RestoreDefaults( const std::string& model ) 
{
      
   std::string mod = model;
   mod = ToLower(mod);

   std::map< std::string, std::map<std::string,std::string> >::iterator itr=fDEFAULTS.find(mod);
   
   if ( itr == fDEFAULTS.end() )
   {
      G4cout << " Model " << model << "(" << mod <<") NOT found " << G4endl;
      return;   
   }

   // Change G4State to Idle
   // G4UImanager will only apply modifications if
   // 1. G4State_Idle
   // 2. G4State_PreInit
   // 
   // Once run is initialized and prepared to start, the state is set to GeomClosed;
   // for this reason, G4UIcommand::IsAvailable will return false, and G4UImanager will NOT apply it
   //
   
   // save current state (for future restore)
   // 
   G4ApplicationState currentstate = G4StateManager::GetStateManager()->GetCurrentState();

   if(!G4StateManager::GetStateManager()->SetNewState(G4State_Idle))
   {            
      G4cout << "G4StateManager PROBLEM: can NOT change state to G4State_Idle ! " << G4endl; 
      return;
   }   

   std::map<std::string,std::string>::iterator itr1;
   
   itr1 = fNameConvention.find(mod);
   if ( itr1 == fNameConvention.end() )
   {
      G4cout << " Can NOT find Geant4 internal name for model " << model << "(" << mod <<") " << G4endl;
      return;   
   }
   
   std::string command = fBaseCommand + itr1->second;

   G4UImanager* uim = G4UImanager::GetUIpointer();
      
   itr1 = (itr->second).begin();
   for ( ; itr1!=(itr->second).end(); ++itr1 )
   {
      std::string cmd2apply = command + itr1->first + " " + itr1->second;
      uim->ApplyCommand( cmd2apply.c_str() );   
   }
      
//   if(!G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed))
   if( !G4StateManager::GetStateManager()->SetNewState(currentstate) )
   {            
      // fLogInfo << "G4StateManager PROBLEM! "; // << std::endl;
      G4cout << " G4StateManager PROBLEM: can not restore " << currentstate << G4endl; 
      return;
   }

   return;

}

std::string ModelConfigMapper::ToLower( std::string input )
{

  for( unsigned int i=0; i<input.size(); ++i ) input[i] = std::tolower(input[i]);
  return input;

}

void ModelConfigMapper::FillBertiniDefaults()
{
   
   fDEFAULTS.insert( std::pair< std::string, std::map<std::string,std::string> >( "bertini", 
                                                                                  std::map<std::string,std::string>() ) );

   std::map< std::string, std::map<std::string,std::string> >::iterator itr2=fDEFAULTS.find("bertini");

   std::ostringstream cmd;

   // general purpose verbosity switch
   // has nothing to do with the model params/configuration
   // but is useful for some debugging purposes
   //
   cmd << G4CascadeParameters::verbose();
   (itr2->second).insert( std::pair<std::string,std::string>("/verbose",cmd.str()) ); 
   cmd.str( "" );
   cmd.clear();
   
   cmd << G4CascadeParameters::doCoalescence();
   (itr2->second).insert( std::pair<std::string,std::string>("/doCoalescence",cmd.str()) );
   cmd.str( "" );
   cmd.clear();

// these params/methods are NOT available in G4.9.6-series, but only starting 10.1-ref03
//   
   cmd << G4CascadeParameters::piNAbsorption();
   (itr2->second).insert( std::pair<std::string,std::string>("piNAbsorption",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::use3BodyMom();
   (itr2->second).insert( std::pair<std::string,std::string>("/use3BodyMom",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::usePhaseSpace();
   (itr2->second).insert( std::pair<std::string,std::string>("/usePhaseSpace",cmd.str()) );
   cmd.str( "" );
   cmd.clear();

// technically speaking, these parameters are available in 9.6-series, together with their G4UI,  
// but in practice they can only be changed via env.variables, due to some implementation details 
//
   cmd << G4CascadeParameters::useTwoParam();
   (itr2->second).insert( std::pair<std::string,std::string>("/useTwoParamNuclearRadius",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::radiusScale();
   (itr2->second).insert( std::pair<std::string,std::string>("/nuclearRadiusScale",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::radiusSmall();
   (itr2->second).insert( std::pair<std::string,std::string>("/smallNucleusRadius",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::radiusAlpha();
   (itr2->second).insert( std::pair<std::string,std::string>("/alphaRadiusScale",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::radiusTrailing(); 
   (itr2->second).insert( std::pair<std::string,std::string>("/shadowningRadius",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::fermiScale();
   (itr2->second).insert( std::pair<std::string,std::string>("/fermiScale",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::xsecScale();
   (itr2->second).insert( std::pair<std::string,std::string>("/crossSectionScale",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::gammaQDScale();
   (itr2->second).insert( std::pair<std::string,std::string>("/gammaQuasiDeutScale",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::dpMaxDoublet();
   (itr2->second).insert( std::pair<std::string,std::string>("/cluster2DPmax",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::dpMaxTriplet();
   (itr2->second).insert( std::pair<std::string,std::string>("/cluster3DPmax",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << G4CascadeParameters::dpMaxAlpha();
   (itr2->second).insert( std::pair<std::string,std::string>("/cluster4DPmax",cmd.str()) );
   cmd.str( "" );
   cmd.clear();
   cmd << "/usePreCoumpound " << G4CascadeParameters::usePreCompound() ; // false/0 by default
   (itr2->second).insert( std::pair<std::string,std::string>("/usePreCoumpound",cmd.str()) );
   cmd.str("");
   cmd.clear();

   (itr2->second).insert( std::pair<std::string,std::string>("/useBestNuclearModel","false") ); // no corresponding access method; unclear D !!!
                                                                                                // From G4CascadeParameters.cc: BEST_PAR = (0!=G4NUCMODEL_USE_BEST);
				                                                                // probably means that if env.var. is NOT set, this option isn't in use

   return;

}

void ModelConfigMapper::FillINCLXXDefaults()
{

   return;

}

void ModelConfigMapper::FillConfigParamMapBertini()
{

   fConfigParameters.insert( std::pair< std::string, std::map<std::string,std::string> >( "bertini", std::map<std::string,std::string>() ) );
   
   std::map< std::string, std::map<std::string,std::string> >::iterator itr=fConfigParameters.find("bertini");
   
   (itr->second).insert( std::pair<std::string,std::string>("radiusscale","/nuclearRadiusScale") );
   (itr->second).insert( std::pair<std::string,std::string>("xsecscale","/crossSectionScale") );
   (itr->second).insert( std::pair<std::string,std::string>("verbosity","/verbose") );  

   return;

}
