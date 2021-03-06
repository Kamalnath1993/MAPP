#include "ff.h"
#include "md.h"
#include "error.h"
#include "memory.h"
#include "script_reader.h"
#include "thermo_dynamics.h"
using namespace MAPP_NS;

/*--------------------------------------------
 constructor
 --------------------------------------------*/
MD::MD():
nrgy_strss(forcefield->nrgy_strss)
{
    boltz=dt=hplanck=0.0;
    if(forcefield==NULL)
        error->abort("ff should be "
        "initiated before md");
    
    if(mode!=MD_mode)
        error->abort("md works only "
        "for md mode");
    
    char** args;
    int nargs=ScriptReader::parse_line("KE Temp. "
    "PE S[0][0] S[1][1] S[2][2] S[2][1] S[2][0] S[1][0]",args);
    ke_idx=0;
    temp_idx=1;
    pe_idx=2;
    stress_idx=3;
    
    
    thermo=new ThermoDynamics(nargs,args);
    for(int i=0;i<nargs;i++)
        delete [] args[i];
    if(nargs)
        delete [] args;

}
/*--------------------------------------------
 constructor
 --------------------------------------------*/
MD::~MD()
{
    delete thermo;
}

