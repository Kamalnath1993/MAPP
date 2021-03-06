#ifdef FF_Style
    FFStyle(ForceField_meam,meam)
#else
#ifndef __MAPP__ff_meam__
#define __MAPP__ff_meam__
#include "ff.h"
namespace MAPP_NS
{
    class ForceField_meam : public ForceFieldMD
    {
        enum{
            i_rho0,
            i_rho1,
            i_rho2,
            i_rho3,
            i_Arho1_1,
            i_Arho1_2,
            i_Arho1_3,
            i_Arho2_1,
            i_Arho2_2,
            i_Arho2_3,
            i_Arho2_4,
            i_Arho2_5,
            i_Arho2_6,
            i_Arho2b,
            i_Arho3_1,
            i_Arho3_2,
            i_Arho3_3,
            i_Arho3_4,
            i_Arho3_5,
            i_Arho3_6,
            i_Arho3_7,
            i_Arho3_8,
            i_Arho3_9,
            i_Arho3_10,
            i_Arho3b_1,
            i_Arho3b_2,
            i_Arho3b_3,
            i_t_ave_1,
            i_t_ave_2,
            i_t_ave_3,
            i_tsq_ave_1,
            i_tsq_ave_2,
            i_tsq_ave_3,
            i_gamma,
            i_dgamma1,
            i_dgamma2,
            i_dgamma3,
            i_fhop};

        
    private:
        int no_types;
        void allocate();
        void deallocate();
        void setup();
        /*---------------------------------*/
        int rho_dim;
        Vec<type0>* rho_ptr;
        Vec<type0>* rho_vec_ptr;
        /*---------------------------------*/
        
        
        
        /*---------------------------------*/
        int v2d[6];
        int v3d[10];
        int vind3d[3][3][3];
        int vind2d[3][3];
        /*---------------------------------*/
        
        /*
         rho dimension is 38!!!!
         **** rho functions (4)
         00: rho0
         01: rho1
         02: rho2
         03: rho3
         
         **** rho_1 terms (3)
         04: Arho1_1
         05: Arho1_2
         06: Arho1_3
         
         **** rho_2 terms (7)
         ******** 1st terms (6)
         07: Arho2_1
         08: Arho2_2
         09: Arho2_3
         10: Arho2_4
         11: Arho2_5
         12: Arho2_6
         ******** 2nd term (1)
         13: Arho2b
         
         **** rho_3 terms
         ******** 1st terms (10)
         14: Arho3_1
         15: Arho3_2
         16: Arho3_3
         17: Arho3_4
         18: Arho3_5
         19: Arho3_6
         20: Arho3_7
         21: Arho3_8
         22: Arho3_9
         23: Arho3_10
         ******** 2nd terms (3)
         24: Arho3b_1
         25: Arho3b_2
         26: Arho3b_3
         
         **** t_ave terms (3)
         27: t_ave_1
         28: t_ave_2
         29: t_ave_3
         
         **** tsq_ave terms (3)
         30: tsq_ave_1
         31: tsq_ave_2
         32: tsq_ave_3
         
         **** Gamma function (1)
         33: gamma
         
         **** dGamma functions (3)
         34: dgamma1
         35: dgamma2
         36: dgamma3
         
         **** fhop term  (1)
         37: fhop
         */
        
        /*---------------------------------*/
        int emb_lin_neg;//checked
        int ialloy;//checked
        int mix_ref_t;//checked
        int bkgd_dyn;//checked
        int augt1;//checked
        int erose_form;//checked
        int nr;
        type0 delr_meam;//checked
        type0 delr_meam_inv;//checked
        type0 rc_meam;//checked
        type0 gsmooth_factor;//checked
        type0 dr;
        type0 dr_inv;
        
        type0*** c_min;//checked
        type0*** c_max;//checked
        type0*** phirar;
        
        type0** re_meam;//checked
        type0** ebound_meam;//checked
        type0** Ec_meam;//checked
        type0** alpha_meam;//checked
        type0** delta_meam;//checked
        type0** attrac_meam;//checked
        type0** repuls_meam;//checked
        int** lattice;//checked
        int** nn2_meam;//checked
        int** zbl_meam;//checked
        
        type0* rho0_meam;//checked
        type0* beta0_meam;//checked
        type0* beta1_meam;//checked
        type0* beta2_meam;//checked
        type0* beta3_meam;//checked
        type0* t0_meam;//checked
        type0* t1_meam;//checked
        type0* t2_meam;//checked
        type0* t3_meam;//checked
        type0* Z_meam;//checked
        type0* rho_ref_meam;//checked
        type0* A_meam;//checked
        int* ibar_meam;//checked
        int* ielt_meam;//checked
        int* type_ref;//not set yet
        
        void get_dens_ref(type0,int,int,type0&,type0&,type0&,type0&,type0&,type0&,type0&,type0&);
        void get_sijk(type0,int,int,int,type0&);
        void get_tavref(type0&,type0&,type0&,type0&,type0&,type0&,type0,type0,type0,type0,type0,type0,type0,int,int,int);
        type0 phi_meam(type0,int,int);
        /*---------------------------------*/
        
        
        /*---------------------------------*/
        int max_pairs;
        type0* scrfcn;//not set yet
        type0* dscrfcn;//not set yet
        type0* fcpair;//not set yet
        /*---------------------------------*/
                

        void reset();

        void fcut(type0,type0&);
        void dfcut(type0,type0&,type0&);
        void dCfunc(type0,type0,type0,type0&);
        void dCfunc2(type0,type0,type0,type0&,type0&);
        
        void G_gam(type0,int,type0,type0&);
        void dG_gam(type0,int,type0,type0&,type0&);
        void get_shpfcn(type0*,int);
        
        void read_global(char*);
        void read_local0(char*);
        void read_local(char*);
        
        void get_Zij(type0&,int);
        void get_Zij2(type0&,type0&,type0&,int,type0,type0);
        type0 zbl(type0,type0,type0);
        type0 erose(type0,type0,type0,type0,type0,type0,int);
        void alloy_params();
        void compute_reference_density();
        void compute_pair_meam();
        void compute_phi(type0,int,int,type0&);
        void compute_phi_dphi(type0,int,int,type0&,type0&);
        
        type0 third,sixth;
        
    protected:
        void force_calc(bool);
        type0 energy_calc();
        void pre_xchng_energy(GCMC*);
        type0 xchng_energy(GCMC*);
        void post_xchng_energy(GCMC*);
    public:
        ForceField_meam();
        ~ForceField_meam();
        void init();
        void fin();
        void init_xchng();
        void fin_xchng();
        void coef(int,char**);
    };
}


#endif
#endif
