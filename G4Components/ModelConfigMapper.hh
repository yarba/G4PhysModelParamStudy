#ifndef ModelConfigMapper_H
#define ModelConfigMapper_H 1

#include <string>
#include <vector>
#include <map>

// NOTE: NO namespace, to make it "fwk-neutral"
//

class ModelConfigMapper
{

   public:
   
      // ctor & dtor
      //
      ModelConfigMapper();
      ~ModelConfigMapper();
      
      // member functions
      //
      void PrintDefaults( const std::string& ); 
      void RestoreDefaults( const std::string& );
      //
      void SetVerbosity( const std::string&, const bool& );
      //
      void ChangeParameter( const std::string&, const std::string&, const double&, bool verb=false );
      void ChangeParameter( const std::string&, const std::string&, const float&,  bool verb=false );
      void ChangeParameter( const std::string&, const std::string&, const int& ); // empty now
      //
      void ChangeParameterByRatio( const std::string&, const std::string&, const double&, bool verb=false );
      void ChangeParameterByRatio( const std::string&, const std::string&, const float&,  bool verb=false );

      // (low level) service function
      //
      std::string ToLower( std::string );
            
   private:
   
      // private member functions
      //
      void FillBertiniDefaults();
      void FillINCLXXDefaults();
      // etc. in the future...
      void FillConfigParamMapBertini();
            
      
      // data members
      //
      std::map< std::string, std::string >                        fNameConvention;
      std::string                                                 fBaseCommand; 
      std::map< std::string, std::map<std::string,std::string> >  fDEFAULTS; // NOTE: the reason of doing it as nested map(s) 
                                                                             // is because map has find(...) - simplifies search
									     // otherwise... would this below be more efficient ?
									     // std::map< std::string, std::vector<std::pair> >
      std::map< std::string, std::map<std::string, std::string> > fConfigParameters;

};


#endif
