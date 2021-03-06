/*--------------------------------------------
 Created by Sina on 06/27/14.
 Copyright (c) 2014 MIT. All rights reserved.
 --------------------------------------------*/
#ifndef __MAPP__atom_types__
#define __MAPP__atom_types__
#include "type_def.h"
namespace MAPP_NS
{
    class AtomTypes 
    {
    private:
    protected:
    public:
        AtomTypes();
        ~AtomTypes();
        
        int no_types;
        char** atom_names;
        type0** clr_rad;
        type0* mass;
        
        int add_type(type0,char*);
        int find_type(const char*);
        int find_type_exist(char*);
        void assign_color_rad(char*,type0*);
        dmd_type get_dmd_type(int,dmd_type*,type0*);
    };
}
#endif 
