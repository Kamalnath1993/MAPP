#ifdef FF_Style
FFStyle(ForceField_eam_dmd_c,eam_dmd_c)
#else

#ifndef __MAPP__ff_eam_dmd_c__
#define __MAPP__ff_eam_dmd_c__

#include <stdio.h>
#include <iostream>
#include "ff.h"
#include "atoms.h"
namespace MAPP_NS
{
    class ForceField_eam_dmd_c : public ForceField
    {
    private:
        int no_types;
    protected:
        TYPE0*** F_arr;
        TYPE0*** phi_r_arr;
        TYPE0*** rho_arr;
        

        
        int eam_mode;
        int nr,nrho;
        TYPE0 dr,drho,dr_inv,drho_inv,rho_max;

        void set_setfl(int,char**);
        void set_funcfl(int,char**);
        void set_fs(int,char**);
        int line_read(FILE*,char*&);
        void setup_delta_e(char*);
        
        void allocate();
        int allocated;
        void clean_up();
        
        void set_arrays();
        void interpolate(int,TYPE0,TYPE0**);
        TYPE0 interpolate(TYPE0*,int,TYPE0,int);
        void interpolate_m(int,TYPE0,TYPE0**);
        
        int** type2rho;
        int** type2phi;
        
        int f_n,type_n,x_n,c_n,E_n,dE_n,ddE_n,
        n_n,s_n,crd_n,t_n,v_n,c_d_n,m_n;
        TYPE0 cut_sq_0;
        TYPE0 cut_sq_mod_0;
        TYPE0 rc,mod_rc,kbT,beta;
        TYPE0* c_0;
        TYPE0* delta_e;
        
        /*--------------------------------------------*/
        TYPE0* rho;
        TYPE0* drho_dr;
        TYPE0* drho_dalpha;
        TYPE0* phi;
        TYPE0* dphi_dr;
        TYPE0* dphi_dalpha;
        int max_pairs;
        /*--------------------------------------------*/
        
        
        TYPE0* xi;
        TYPE0* wi_0;
        TYPE0* wi_1;
        TYPE0* wi_2;
        TYPE0 alpha_min,alpha_max;
        int no_i;
        void set_weight_abs(int);
        void rho_calc(TYPE0,TYPE0,int,int);
        void phi_calc(TYPE0,TYPE0,int,int);
        /*--------------------------------------------*/
        TYPE0 r_crd,rsq_crd;
        int** neigh_lst;
        int* neigh_lst_sz;
        int neigh_lst_sz_sz;
        /*--------------------------------------------*/
        
        TYPE0 mat(TYPE0,TYPE0,TYPE0,TYPE0,int);
        TYPE0 dmat0(TYPE0,TYPE0,TYPE0,TYPE0,int);
        TYPE0 dmat1(TYPE0,TYPE0,TYPE0,TYPE0,int);
        TYPE0 calc_ent(TYPE0);
        
    public:
        ForceField_eam_dmd_c(MAPP *);
        ~ForceField_eam_dmd_c();
        void force_calc(int,TYPE0*);
        TYPE0 energy_calc();
        void init();
        void fin();
        void coef(int,char**);
        
        void create_2nd_neigh_lst();
        TYPE0 g_calc(int,TYPE0,TYPE0*,TYPE0*);
        void c_d_calc();
    };
    
    
    
}

#endif
#endif