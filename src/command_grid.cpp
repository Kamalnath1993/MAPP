#include "command_grid.h"
#include "atoms.h"
#include "error.h"
#include "memory.h"
using namespace MAPP_NS;
using namespace std;
/*--------------------------------------------
 constructor
 --------------------------------------------*/
Command_grid::Command_grid(MAPP* mapp,int narg,char** args)
:InitPtrs(mapp)
{
    int dimension=atoms->dimension;
    if(narg!=dimension+1)
        error->abort("grid command needs "
        "%d arguements",dimension);
    
    int* n;
    CREATE1D(n,dimension);
    for(int i=0;i<dimension;i++)
        n[i]=atoi(args[i+1]);
    
    int tmp=1;
    for(int i=0;i<dimension;i++)
    {
        n[i]=atoi(args[i+1]);
        if(n[i]<1)
            error->abort("number of processors "
            "in dimension %d for grid command "
            "cannot be less than 1",i);
        tmp*=n[i];
    }

    if(tmp!=atoms->tot_p)
        error->abort("for grid command total "
        "number of processors should be equal "
        "to the product of arguments");

    atoms->man_grid_proc(n);
}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
Command_grid::~Command_grid()
{

}
