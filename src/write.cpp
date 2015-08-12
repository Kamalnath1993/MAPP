#include "write.h"
#include "timer.h"

using namespace MAPP_NS;
/*--------------------------------------------
 constructor
 --------------------------------------------*/
Write::Write(MAPP* mapp):InitPtrs(mapp)
{
    last_write_step=-1;
}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
Write::~Write()
{
    
}
/*--------------------------------------------
 init before a run
 --------------------------------------------*/
void Write::init()
{
    init_indv();
    if(last_write_step!=step_no)
    {
        timer->start(WRITE_TIME_mode);
        write_file(step_no);
        timer->stop(WRITE_TIME_mode);
        last_write_step=step_no;
    }
    write_step=step_no+write_step_tally;


}
/*--------------------------------------------
 write the file
 --------------------------------------------*/
void Write::write()
{
    if(write_step!=step_no)
        return;
    timer->start(WRITE_TIME_mode);
    write_file(step_no);
    timer->stop(WRITE_TIME_mode);
    last_write_step=step_no;
    write_step=step_no+write_step_tally;
}
/*--------------------------------------------
 after a run
 --------------------------------------------*/
void Write::fin()
{

    if(last_write_step!=step_no)
    {
        timer->start(WRITE_TIME_mode);
        write_file(step_no);
        timer->stop(WRITE_TIME_mode);
    }
    last_write_step=step_no;
    fin_indv();
}
