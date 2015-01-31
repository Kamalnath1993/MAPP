/*--------------------------------------------
 Created by Sina on 07/23/13.
 Copyright (c) 2013 MIT. All rights reserved.
 --------------------------------------------*/
#ifdef Read_Style
ReadStyle(Read_cfg,cfg)
#else
#ifndef __MAPP__read_cfg__
#define __MAPP__read_cfg__
#include <iostream>
#include "atoms.h"
#include "read.h"
namespace MAPP_NS {
    class Read_cfg : public Read{
    private:
        FILE* cfgfile;
        
        type0** H0;
        type0** eta;
        type0** eta_sq;
        type0** trns;
        type0** H_x;
        type0** H_x_d;
        
        char* line;
        
        type0 basic_length;
        type0 R;
        int entry_count;
        int ext_cfg;
        int header_cmplt;
        int vel_chk;
        int atom_cmplt;
        int last_type;
        
        void read_header();
        void read_atom_md();
        void read_atom_dmd();
        void M3sqroot(type0**,type0**);
        void set_box();
        void add_atom_read_x(int);
        void add_atom_read_x();
        
        int type_n,x_n,x_d_n,id_n,c_n;
        int ch_x_d,ch_type,ch_id,ch_c;
        int dmd_no_types;
        
        int curr_id;
        char* ch_buff;
        VecLst* vec_list;
        
        type0* tmp_buff;
        int tmp_buff_size;
        char* file_name;
        
    protected:
    public:
        Read_cfg(MAPP *,int,char**);
        ~Read_cfg();
    };
}
#endif
#endif

