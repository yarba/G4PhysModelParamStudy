
// Authors: Julia Yarba

// Date: Oct. 2015

// Art includes
//
#include "art/Framework/Core/ModuleMacros.h"

// Services
#include "art/Framework/Services/Registry/ServiceHandle.h"

#include "artg4tk/G4PhysModelParamStudy/G4Services/PhysModelConfig_service.hh"
#include "artg4tk/G4PhysModelParamStudy/G4Components/ModelConfigMapper.hh"


artg4tk::PhysModelConfigService::PhysModelConfigService( const fhicl::ParameterSet& pset, 
				                         art::ActivityRegistry& )
{

   fConfigMapper = new ModelConfigMapper();
   fVerbosity = pset.get<bool>("Verbosity",false);
   if ( fVerbosity )
   {
      // FIXME !!!
      // Need to find out a more "automated" way to handle it...
      //
      fConfigMapper->PrintDefaults("bertini");
// --> too much output this way; better do only if model's verbosty is requested fConfigMapper->SetVerbosity("bertini",fVerbosity);
      // ---> for future --> fConfigMapper->PrintDefaults("inclxx");
   }
          
}

artg4tk::PhysModelConfigService::~PhysModelConfigService()
{

   delete fConfigMapper;
 
} 

void artg4tk::PhysModelConfigService::ConfigureModel( const std::string& mname, 
                                                      const fhicl::ParameterSet& mpset )
{

   std::string mod = mname;
   mod = fConfigMapper->ToLower(mod);
   if ( mod == "bertini" )
   {
      // this is a "hack" and kind of "violation" of the PSet philosophy but...
      //
      std::vector<std::string> keys = mpset.get_keys();
      for ( unsigned int i=0; i<keys.size(); ++i )
      {
          std::string thekey = keys[i];
	  thekey = fConfigMapper->ToLower(thekey);
	  if ( thekey.find("use") != std::string::npos ) continue; // those are int or bool !
	                                                           // will add/treat them later
	  if ( thekey == "verbosity" )
	  {
	     fConfigMapper->SetVerbosity( "bertini", mpset.get<bool>(keys[i],false) );
	     continue;
	  }	  
	  double value = mpset.get<double>(keys[i]);
	  if ( thekey.find("byratio") != std::string::npos )
	  {
	     //  change by ratio
	     fConfigMapper->ChangeParameterByRatio( "bertini", thekey, value, fVerbosity );
	  }
	  else
	  {
	    // change by value
	    fConfigMapper->ChangeParameter( "bertini", thekey, value, fVerbosity );
	  }
      }
   }

   return;

}

void artg4tk::PhysModelConfigService::RestoreDefaults( const std::string& model )
{

   fConfigMapper->RestoreDefaults( model );
   return;

}

using artg4tk::PhysModelConfigService;

DEFINE_ART_SERVICE(PhysModelConfigService)

