#ifdef FF_Style
    FFStyle(ForceField_eam,eam)
#else
#ifndef __MAPP__ff_eam__
#define __MAPP__ff_eam__
#include "ff.h"
#include "eam_file_reader.h"
namespace MAPP_NS
{
    class ForceField_eam: public ForceFieldMD
    {
    private:
        Vec<type0>* rho_ptr;
    protected:
        int nr,nrho;
        type0 dr,drho,dr_inv,drho_inv,rho_max;
        type0*** F_arr;
        type0*** phi_r_arr;
        type0*** rho_arr;
        
        int** type2rho;
        int** type2phi;
        
        EAMFileReader* eam_reader;
        
                
        /*--------------------------------------------*/
        type0* drhoi_dr;
        type0* drhoj_dr;
        int max_pairs;
        /*--------------------------------------------*/
        void force_calc(int,type0*);
        type0 energy_calc();
    public:
        ForceField_eam(MAPP *);
        ~ForceField_eam();
        void init();
        void fin();
        void coef(int,char**);

    };
    

    
}
#endif
#endif
