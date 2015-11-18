#include "md.h"
#include "error.h"
#include "memory.h"
#include "thermo_dynamics.h"
using namespace MAPP_NS;

/*--------------------------------------------
 constructor
 --------------------------------------------*/
MD::MD(MAPP* mapp):InitPtrs(mapp)
{
    ns_alloc=0;
    
    if(forcefield==NULL)
        error->abort("ff should be "
        "initiated before md");
    
    if(mapp->mode!=MD_mode)
        error->abort("md works only "
        "for md mode");
    
    char** args;
    int nargs=mapp->parse_line("KE Temp. "
    "PE S_xx S_yy S_zz S_yz S_zx S_xy",args);
    ke_idx=0;
    temp_idx=1;
    pe_idx=2;
    stress_idx=3;
    
    
    thermo=new ThermoDynamics(mapp,nargs,args);
    for(int i=0;i<nargs;i++)
        delete [] args[i];
    if(nargs)
        delete [] args;
    
    int dim=atoms->dimension;
    if(dim)
    {
        CREATE1D(nrgy_strss,dim*(dim+1)/2+1);
        ns_alloc=1;
    }

}
/*--------------------------------------------
 constructor
 --------------------------------------------*/
MD::~MD()
{
    delete thermo;
    if(ns_alloc)
        delete [] nrgy_strss;
}

