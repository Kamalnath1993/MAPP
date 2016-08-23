/*--------------------------------------------
 Created by Sina on 06/29/16.
 Copyright (c) 2013 MIT. All rights reserved.
 --------------------------------------------*/
#ifndef __MAPP__gcmc__
#define __MAPP__gcmc__
#include "init.h"
namespace MAPP_NS
{
    enum{NOEX_MODE,INS_MODE,DEL_MODE};
    enum{MINE_FLAG,INTERACT_FLAG,NONEINTERACT_FLAG};
    /*--------------------------------------------
     allocation for this constructor has 3 levels:
     
     0. buffers that have constant size during the
     life of this object. They are allocated in
     constructor and destroyed destructor:
     
     cut_s;
     s_lo_ph;
     s_hi_ph;
     cell_size;
     ncells_per_dim;
     cell_denom;
     icell_coord;
     jcell_coord;
     nimages_per_dim; 
     *nimages_per_dim;
     ins_s_trials; (!! NOT *ins_s_trials)
     rel_neigh_lst_coord;
     rel_neigh_lst;
     
     1. buffers whose size are dependent on box 
     dimensions and domain dimensions:
     
     head_atm;
     *ins_s_trials;
     ins_cell;
     ins_cell_coord;
     ins_buff;
     ins_cell;
     del_lst;
     
     2. buffers whose sizes are decided on fly
     
     del_ids;
     
     --------------------------------------------*/
    
    
    class GCMC:protected InitPtrs
    {
    private:
    protected:
        class ForceFieldMD* ff;
        
        const int dim;
        
        int igas,gas_id,ngas;
        md_type gas_type;
        type0 vol;
        //constants
        type0 gas_mass,beta,kbT,T,mu,lambda,sigma,z_fac;
        
        int& natms;
        int& natms_ph;
        type0 cut;
        type0**& cut_sq;
        type0*& s_lo;
        type0*& s_hi;
        
        // size dim
        type0* s_buff;
        type0* vel_buff;
        type0* cut_s;
        type0* s_lo_ph;
        type0* s_hi_ph;
        
        int** nimages_per_dim;
        type0** s_trials;

        
        int del_idx;
        
        int* del_ids;
        int del_ids_sz,del_ids_cpcty;
        int max_id;

        Vec<type0>* s_vec_p;

        
        
        class Random* random;
        
        
        int itrial_atm,ntrial_atms,max_ntrial_atms;
        
        virtual void ins_succ()=0;
        virtual void del_succ()=0;
        virtual void box_setup();
        virtual void box_dismantle();
        void add_del_id(int*,int);
        int get_new_id();

    public:
        GCMC(MAPP*,dmd_type,type0,type0,int);
        ~GCMC();
        
        virtual void init();
        virtual void fin();
        virtual void xchng(bool,int)=0;
        virtual void next_iatm()=0;
        virtual void next_jatm()=0;
        virtual void next_icomm()=0;
        
        virtual void reset_iatm()=0;
        virtual void reset_jatm()=0;
        virtual void reset_icomm()=0;
        
        int iatm;
        int niatms;
        md_type& itype;
        type0* ix;
        type0* jx;
        int jatm;
        md_type jtype;
        type0 rsq;
        int xchng_mode;
        int dof_diff;
        int tot_ngas;
        bool im_root;

        Vec<int>* tag_vec_p;
        int icomm;
        type0* lcl_vars;
        type0* vars;
        MPI_Comm* curr_comm;
        int curr_root;
        bool root_succ;
    };
    
    
    
    
    
    
    
    
    
    
    
    

}
#endif

