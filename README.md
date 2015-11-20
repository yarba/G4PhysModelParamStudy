# G4PhysModelParamStudy

This is a setp of tools that allows to vary intentionally exposed parameters of
selected Geant4 (geant4.cern.ch) hadronic physics models and to study the induced 
systematic uncertanties.

Most of the components are based on the Art framework 
https://web.fnal.gov/project/ArtDoc/Pages/home.aspx) 
and are coded as Art modules.

It is typically used as part of artg4tk toolkit:
https://cdcvs.fnal.gov/redmine/projects/artg4tk

However, several components can be extracted as is and used directly with any
"bare" Geant4-based application.

There are also standalone analysis tools, based on a RooMUHistos package for
statistical analysis and vizzualization of results:
git clone https://github.com/ManyUniverseAna/RooMUHistos
cd RooMUHistos
source env_set.sh 
cd PlotUtils
make
NOTE: RooMUHistos is based on ROOT (root.cern.ch) and requires env.variable
ROOTSYS to be defined.


  
