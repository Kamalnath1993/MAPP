#include <stdlib.h>
#include "command_grid.h"
#include "atoms.h"
#include "error.h"
#include "memory.h"
using namespace MAPP_NS;
/*--------------------------------------------
 constructor
 --------------------------------------------*/
Command_grid::Command_grid(int nargs
,char** args)
{
    if(nargs!=__dim__+1)
        error->abort("grid command needs "
        "%d arguments",__dim__);
    
    int n[__dim__];

    for(int i=0;i<__dim__;i++)
        n[i]=atoi(args[i+1]);
    
    int tmp=1;
    for(int i=0;i<__dim__;i++)
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

    atoms->man_grid(n);


}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
Command_grid::~Command_grid()
{

}

