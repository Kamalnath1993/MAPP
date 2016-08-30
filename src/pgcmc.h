/*--------------------------------------------
 Created by Sina on 06/29/16.
 Copyright (c) 2013 MIT. All rights reserved.
 --------------------------------------------*/
#ifndef __MAPP__pgcmc__
#define __MAPP__pgcmc__
#include "gcmc.h"
namespace MAPP_NS
{
    
    class PGCMC:public GCMC
    {
    private:
#ifdef MPP_DEBUG
        type0 tot_du_test;
#endif
        
        int* N_c;
        int* N_s;
        int* N_curr_comms;
        int* i_curr_comms;
        bool* prll_dim;
        
        int max_n_cncrcy;
        int* max_N_cncrcy;
        
        int comm_buff_size;
        byte* comm_buff;
        
        int ip;
        int n_p;
        int* op_vec;
        int* p_vec;
        int* N_p;
        int* B_p;
        
        int n_prll;
        int* N_prll;
        int* B_prll;
        
        int n_pcomm;
        int* N_pcomm;
        int* B_pcomm;

        int n_comm;
        int* N_comm;
        int* B_comm;

        
        const int m;
        
        //static stuff
        //allocate in constructor
        //deallocate in destructor
        int* N_cells;
        int* B_cells;
        int* icell_coord;
        int* jcell_coord;
        type0* cell_size;


        //dynamic determined by m
        //allocate in constructor
        //deallocate in destructor
        int* rel_neigh_lst_coord;
        
        //dynamic determined by box
        //allocate in setup_box
        //deallocate in dismantle_box
        int* head_atm;
        
        //dynamic determined by box & comm
        //allocate in setup_box
        //deallocate in dismantle_box
        int** cell_coord_buff;
        type0** s_x_buff;
        type0** s_buff;
        
        //dynamic determined by comm
        //allocate in setup_comm
        //deallocate in dismantle_comm
        int* ntrial_atms;
        int* roots;
        int* gcmc_mode;
        MPI_Comm* comms;
        MPI_Comm** curr_comms;
        int** comm_id;
        type0** lcl_vars_comm;
        type0** vars_comm;
        int* success;
        int* int_buff;
        int int_buff_sz;
        
        int jatm_next;
        int iself;
        int ineigh;
        int nneighs;
        
        
        int icell,jcell;
        int n_cells;
        
    
        Vec<int>* cell_vec_p;
        Vec<int>* next_vec_p;
        
        void find_cell_no(type0*&,int&);
        void find_cell_coord(type0*&,int*&);
                
        
        void prep_s_x_buff();

                
        void next_jatm_reg();
        void next_jatm_self();
        void (PGCMC::*next_jatm_p)();
        
        void attmpt();
        
        /*---------------------------------------------------------------------*/
        

        
        class Random* lcl_random;
        int n_vars;
        int n_s;
        
        void comms_setup(int,int);
        void comms_dismantle();
        void create_comm_pattern();
        int n_curr_comms;
        void success2tag();
        void reset_tag();
        int prev_p,next_p,origin_p;
        type0 delta_u;
        int new_id;
        void decide();
        void finalize();
        /*---------------------------------------------------------------------*/

    protected:
        void ins_succ();
        void del_succ();
        void box_setup();
        void box_dismantle();

        
        
    public:
        PGCMC(MAPP*,int,dmd_type,type0,type0,int);
        ~PGCMC();
        
        void init();
        void fin();
        
        
        void xchng(bool,int);
        
        void next_iatm();
        void next_jatm();
        void next_icomm();
        
        void reset_iatm();
        void reset_jatm();
        void reset_icomm();

    };
    

    
}
#endif
