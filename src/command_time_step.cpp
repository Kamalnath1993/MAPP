#include <stdlib.h>
#include "command_time_step.h"
#include "global.h"
#include "md.h"
#include "error.h" 
using namespace MAPP_NS;
/*--------------------------------------------
 constructor
 --------------------------------------------*/
Command_time_step::Command_time_step(
int nargs,char** args)
{
    if(md==NULL)
        error->abort("before adjusting the "
        "time_step, ensemble should be initialized");
    
    if(nargs!=2)
        error->abort("time_step should have 1argument");
    
    type0 dt=atof(args[1]);
    if(dt<=0.0)
        error->abort("time_step should be"
        " greater than 0.0");
    
    md->dt=dt;
    
}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
Command_time_step::~Command_time_step()
{

}

