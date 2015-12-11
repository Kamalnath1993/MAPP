/*--------------------------------------------
 Created by Sina on 07/16/13.
 Copyright (c) 2013 MIT. All rights reserved.
 --------------------------------------------*/
#include "ff.h"
#include "atom_types.h"
#include "timer.h"
#include "error.h"
#include "memory.h"
using namespace MAPP_NS;
/*--------------------------------------------
 constructor
 --------------------------------------------*/
ForceField::
ForceField(MAPP* mapp) : InitPtrs(mapp)
{
    cut_sz=0;
    ns_alloc=0;
    
    if(atoms->tot_natms==0 || atom_types->no_types==0)
        error->abort("system configuration "
        "should be loaded before initiating ff");

    if(atoms->dimension!=3)
        error->abort("the dimension of the box for ff");
    
    cut_off_alloc();
    int dim=atoms->dimension;
    if(dim)
    {
        f=new Vec<type0>(atoms,mapp->x->dim);
        CREATE1D(nrgy_strss,dim*(dim+1)/2+1);
        CREATE1D(nrgy_strss_lcl,dim*(dim+1)/2+1);
        ns_alloc=1;
    }
}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
ForceField::~ForceField()
{
    cut_off_dealloc();
    if(ns_alloc)
    {
        delete [] nrgy_strss_lcl;
        delete [] nrgy_strss;
    }
}
/*--------------------------------------------
 allocate cutoff
 --------------------------------------------*/
void ForceField::cut_off_alloc()
{
    int no_types=atom_types->no_types;
    int cut_sz_=no_types*(no_types+1)/2;
    if(cut_sz_==cut_sz)
        return;
    cut_off_dealloc();
    cut_sz=cut_sz_;
    CREATE1D(cut_sk_sq,cut_sz);
    CREATE1D(cut_sq,cut_sz);
    CREATE1D(cut,cut_sz);
    CREATE1D(rsq_crd,no_types);
}

/*--------------------------------------------
 deallocate cutoff
 --------------------------------------------*/
void ForceField::cut_off_dealloc()
{
    if(cut_sz)
    {
        delete [] cut_sk_sq;
        delete [] cut_sq;
        delete [] cut;
        delete [] rsq_crd;
    }
    cut_sz=0;
}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
type0 ForceField::max_cut()
{
    int arr_size=(atom_types->no_types)
    *(atom_types->no_types+1)/2;
    type0 skin=atoms->get_skin();
    type0 tmp;
    type0 max_cut=0;
    for(int i=0;i<arr_size;i++)
    {
        tmp=sqrt(cut_sq[i])+skin;
        cut_sk_sq[i]=tmp*tmp;
        max_cut=MAX(max_cut,tmp);
    }
    
    return max_cut;
}
/*--------------------------------------------
 
 --------------------------------------------*/
void ForceField::force_calc_timer(bool flag)
{
    timer->start(FORCE_TIME_mode);
    force_calc(flag);
    if(flag)
    {
        type0** H=atoms->H;
        type0 vol=1.0;
        for(int idim=0;idim<atoms->dimension;idim++)
            vol*=H[idim][idim];
        for(int i=1;i<7;i++)
            nrgy_strss[i]/=vol;
    }
    timer->stop(FORCE_TIME_mode);
}
/*--------------------------------------------
 
 --------------------------------------------*/
type0 ForceField::energy_calc_timer()
{
    type0 en;
    timer->start(FORCE_TIME_mode);
    en=energy_calc();
    timer->stop(FORCE_TIME_mode);
    return en;
}
/*--------------------------------------------
 
 --------------------------------------------*/
type0 ForceFieldDMD::imp_cost_grad_timer(bool
chk,type0 m_tol,type0 alpha,type0* a,type0* g)
{
    type0 en;
    timer->start(FORCE_TIME_mode);
    en=imp_cost_grad(chk,m_tol,alpha,a,g);
    timer->stop(FORCE_TIME_mode);
    return en;
}
/*--------------------------------------------
 
 --------------------------------------------*/
type0 ForceFieldDMD::dc_norm_grad_timer
(bool chk,type0* g,type0* g_mod)
{
    type0 en;
    timer->start(FORCE_TIME_mode);
    en=dc_norm_grad(chk,g,g_mod);
    timer->stop(FORCE_TIME_mode);
    return en;
}
/*--------------------------------------------
 
 --------------------------------------------*/
type0 ForceFieldDMD::ddc_norm_timer()
{
    timer->start(FORCE_TIME_mode);
    type0 en=ddc_norm();
    timer->stop(FORCE_TIME_mode);
    return en;
}
/*--------------------------------------------
 
 --------------------------------------------*/
type0 ForceFieldDMD::en_grad_timer(
bool chk,type0* g,type0* g_mod)
{
    type0 en;
    timer->start(FORCE_TIME_mode);
    en=en_grad(chk,g,g_mod);
    timer->stop(FORCE_TIME_mode);
    return en;
}
/*--------------------------------------------
 
 --------------------------------------------*/
void ForceFieldDMD::dc_timer()
{
    timer->start(FORCE_TIME_mode);
    dc();
    timer->stop(FORCE_TIME_mode);
}
/*--------------------------------------------
 
 --------------------------------------------*/
type0 ForceFieldDMD::dc_en_proj_timer
(bool chk,type0* g,type0& g_h)
{
    type0 en;
    timer->start(FORCE_TIME_mode);
    en=dc_en_proj(chk,g,g_h);
    timer->stop(FORCE_TIME_mode);
    return en;
}


