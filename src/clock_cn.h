#ifdef Clock_Style
    ClockStyle(Clock_cn,cn)
#else
#ifndef __MAPP__clock_cn__
#define __MAPP__clock_cn__
#include <stdio.h>
#include "clock.h"
namespace MAPP_NS {
    class Clock_cn :public Clock
    {
    private:
    protected:
        int c_n,c_d_n,dof_tot,dof_lcl;
        int max_iter,no_steps;
        TYPE0 min_gamma,gamma_red,slope;
        TYPE0 m_tol,a_tol,e_tol;
        TYPE0 min_del_t,max_del_t,initial_del_t;
        TYPE0 eq_ratio;

        TYPE0 beta;
        TYPE0 err,err_prefac;
        
        
        TYPE0* t;
        
        TYPE0** dy;
        
        TYPE0* y_0;
        TYPE0* y;
        TYPE0* dy0;
        TYPE0* a;
        TYPE0* g;
        TYPE0* c0;
        TYPE0* g0;
        TYPE0* h;

        TYPE0 solve(TYPE0);
        int interpolate(TYPE0);
        void ord_dt(TYPE0&);
        
    public:
        Clock_cn(MAPP *,int,char**);
        ~Clock_cn();
        void run();
        void init();
        void fin();
        
    };
}
#endif
#endif 