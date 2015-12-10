#include <stdlib.h>
#include <limits>
#include "dmd_mbdf.h"
#include "ff.h"
#include "error.h"
#include "memory.h"
#include "write.h"
#include "thermo_dynamics.h"
using namespace MAPP_NS;
/*--------------------------------------------
 constructor
 --------------------------------------------*/
DMD_mbdf::DMD_mbdf(MAPP* mapp,int nargs
,char** args):DMDImplicit(mapp)
{
    max_order=5;
    
    if(nargs>2)
    {
        if(nargs%2!=0)
            error->abort("every keyword in dmd mbdf should be followed by it's value");
        int iarg=2;
        while(iarg<nargs)
        {
            if(strcmp(args[iarg],"max_step")==0)
            {
                iarg++;
                max_step=atoi(args[iarg]);
                iarg++;
            }
            else if(strcmp(args[iarg],"max_iter")==0)
            {
                iarg++;
                max_iter=atoi(args[iarg]);
                iarg++;
            }
            else if(strcmp(args[iarg],"max_order")==0)
            {
                iarg++;
                max_order=atoi(args[iarg]);
                iarg++;
            }
            else if(strcmp(args[iarg],"m_tol")==0)
            {
                iarg++;
                m_tol=atof(args[iarg]);
                iarg++;
            }
            else if(strcmp(args[iarg],"a_tol")==0)
            {
                iarg++;
                a_tol=atof(args[iarg]);
                iarg++;
            }
            else if(strcmp(args[iarg],"min_del_t")==0)
            {
                iarg++;
                min_del_t=atof(args[iarg]);
                iarg++;
            }
            else
                error->abort("unknown keyword in dmd mbdf: %s",args[iarg]);
        }
    }
    
    if(max_step<0)
        error->abort("max_step in dmd mbdf should be greater than 0");
    if(max_iter<=0)
        error->abort("max_iter in dmd mbdf should be greater than 0");
    if(max_order<=0)
        error->abort("max_order in dmd mbdf should be greater than 0");
    if(m_tol<=0.0)
        error->abort("m_tol in dmd mbdf should be greater than 0.0");
    if(a_tol<=0.0)
        error->abort("a_tol in dmd mbdf should be greater than 0.0");
    if(min_del_t<=0.0)
        error->abort("min_del_t in dmd mbdf should be greater than 0.0");
    
}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
DMD_mbdf::~DMD_mbdf()
{
}
/*--------------------------------------------
 init
 --------------------------------------------*/
void DMD_mbdf::allocate()
{
    vecs_1=new Vec<type0>*[max_order+2];
    for(int ivec=0;ivec<max_order+2;ivec++)
        vecs_1[ivec]=new Vec<type0>(atoms,c_dim);
    
    CREATE1D(alpha_y,max_order+1);
    CREATE1D(dalpha_y,max_order+1);
    CREATE1D(alph_err,max_order+2);
    CREATE1D(t,max_order+1);
    CREATE1D(y,max_order+1);
}
/*--------------------------------------------
 init
 --------------------------------------------*/
void DMD_mbdf::deallocate()
{
    
    delete [] y;
    delete [] t;
    delete [] alph_err;
    delete [] dalpha_y;
    delete [] alpha_y;

    for(int ivec=0;ivec<max_order+2;ivec++)
        delete vecs_1[ivec];
    delete [] vecs_1;
}
/*--------------------------------------------
 restart a simulation
 --------------------------------------------*/
inline void DMD_mbdf::restart(type0& del_t,int& q)
{
    reset();
    for(int i=0;i<max_order+1;i++)
        y[i]=vecs_1[i]->begin();
    dy=vecs_1[max_order+1]->begin();
    
    iter_dcr_cntr=-1;
    type0* c=mapp->c->begin();
    type0* c_d=mapp->c_d->begin();
    type0 sum,sum_lcl;
    sum_lcl=0.0;
    for(int i=0;i<ncs;i++)
        if(c[i]>=0.0)
            sum_lcl+=c_d[i]*c_d[i];
    sum=0.0;
    MPI_Allreduce(&sum_lcl,&sum,1,MPI_TYPE0,MPI_SUM,world);
    sum=sqrt(sum/nc_dofs);
    del_t=MIN(2.0*a_tol/sum,1.0e-3*(max_t-tot_t));
    
    
    if(del_t>max_t-tot_t)
        del_t=max_t-tot_t;
    else
    {
        if(max_t-tot_t<=2.0*min_del_t)
        {
            del_t=max_t-tot_t;
        }
        else
        {
            if(del_t<min_del_t)
                del_t=min_del_t;
            else if(del_t>=max_t-tot_t-min_del_t)
                del_t=max_t-tot_t-min_del_t;
        }
    }
    
    memcpy(y[0],mapp->c->begin(),ncs*sizeof(type0));
    memcpy(y[1],mapp->c->begin(),ncs*sizeof(type0));
    memcpy(dy,mapp->c_d->begin(),ncs*sizeof(type0));
    for(int i=0;i<max_order+1;i++) t[i]=0.0;
    t[0]=0.0;
    t[1]=-del_t;
    q=1;
    init_phase=false;
}
/*--------------------------------------------
 store the vectors
 --------------------------------------------*/
inline void DMD_mbdf::store_vecs(type0 del_t)
{
    
    type0* tmp_y=y[max_order];
    for(int i=max_order;i>0;i--)
    {
        t[i]=t[i-1]-del_t;
        y[i]=y[i-1];
    }
    y[0]=tmp_y;
    memcpy(y[0],mapp->c->begin(),ncs*sizeof(type0));
    memcpy(dy,mapp->c_d->begin(),ncs*sizeof(type0));
}
/*--------------------------------------------
 init
 --------------------------------------------*/
inline void DMD_mbdf::interpolate(type0& del_t,int& q)
{
    type0 tmp0;
    
    type0 k0;
    type0 err_prefac0,err_prefac1;
    
    type0* c=mapp->c->begin();
    
    int idof;
    int err_chk_lcl;
    int err_chk=1;
    
    
    while(err_chk)
    {
        k0=0.0;
        
        
        for(int i=0;i<q+1;i++)
        {
            k0+=1.0/(1.0-t[i]/del_t);
            tmp0=1.0;
            for(int j=0;j<q+1;j++)
                if(i!=j)
                    tmp0*=(del_t-t[j])/(t[i]-t[j]);
            
            alpha_y[i]=tmp0;
        }
        
        for(int i=0;i<q+1;i++)
        {
            tmp0=k0-1.0/(1.0-t[i]/del_t);
            dalpha_y[i]=alpha_y[i]*tmp0/del_t;
        }
        
        tmp0=0.0;
        for(int i=0;i<q;i++)
            tmp0+=1.0/static_cast<type0>(i+1);
        beta=del_t/tmp0;
        
        
        
        
        err_chk_lcl=0;
        idof=0;
        while(idof<ncs && err_chk_lcl==0)
        {
            if(c[idof]>=0.0)
            {
                y_0[idof]=0.0;
                a[idof]=0.0;
                for(int j=0;j<q+1;j++)
                {
                    a[idof]+=beta*dalpha_y[j]*y[j][idof];
                    y_0[idof]+=alpha_y[j]*y[j][idof];
                }
                a[idof]-=y_0[idof];
                
                if(y_0[idof]<0.0 || y_0[idof]>1.0)
                {
                    err_chk_lcl=1;
                }
            }
            else
                y_0[idof]=c[idof];
            
            
            idof++;
        }
        
        MPI_Allreduce(&err_chk_lcl,&err_chk,1,MPI_INT,MPI_MAX,world);
        
        if(err_chk)
        {
            const_stps=0;
            init_phase=false;
            
            if(q>1)
            {
                q--;
            }
            else
            {
                memcpy(y[1],y[0],ncs*sizeof(type0));
                memcpy(y_0,y[0],ncs*sizeof(type0));
                for(int i=0;i<ncs;i++)
                    if(c[i]>=0.0)
                        a[i]=-y_0[i];
                err_chk=0;
                
            }
        }
        
        err_prefac0=(del_t)/(del_t-t[q]);
        err_prefac1=err_prefac0;
        for(int i=0;i<q;i++)
        {
            err_prefac1+=(del_t)/(del_t-t[i])
            -1.0/static_cast<type0>(i+1);
        }
        err_prefac=MAX(err_prefac0,fabs(err_prefac1));
        
        if(err_chk)
            intp_rej++;
    }
}
/*--------------------------------------------
 step addjustment after success
 --------------------------------------------*/
inline void DMD_mbdf::ord_dt(type0 err,type0 del_t,int q,type0& r,int& del_q)
{
    del_q=0;
    if(init_phase)
    {
        r=2.0;
        if(q<max_order) del_q=1;
        return;
    }
    
    
    type0 terkm2=0.0,terkm1=0.0,terk=0.0,terkp1=0.0;
    type0 est;
    
    bool terkm2_flag,terkm1_flag,terkp1_flag;
    
    terkp1_flag=false;
    terkm1_flag=false;
    terkm2_flag=false;
    
    if(q>2)
        terkm2_flag=true;
    if(q>1)
        terkm1_flag=true;
    
    if(const_stps==q+2 && q<max_order)
        terkp1_flag=true;
    
    
    if(terkm2_flag)
        terkm2=err_est(q-1,del_t);
    
    if(terkm1_flag)
        terkm1=err_est(q,del_t);
    
    terk=err*static_cast<type0>(q+1);
    
    if(terkp1_flag)
        terkp1=err_est(q+2,del_t);
    
    if(q>2)
    {
        if(MAX(terkm1,terkm2)<=terk)
        {
            est=terkm1/static_cast<type0>(q);
            del_q--;
        }
        else
        {
            if(terkp1<terk && terkp1_flag)
            {
                est=terkp1/static_cast<type0>(q+2);
                del_q++;
            }
            else
                est=err;
        }
    }
    else if(q==2)
    {
        if(terkm1<=0.5*terk)
        {
            est=terkm1/static_cast<type0>(q);
            del_q--;
        }
        else if(terkp1_flag && terk<terkm1)
        {
            if(terkp1<terk)
            {
                est=terkp1/static_cast<type0>(q+2);
                del_q++;
            }
            else
                est=err;
        }
        else
            est=err;
    }
    else
    {
        if(terkp1_flag && terkp1<0.5*terk)
        {
            est=terkp1/static_cast<type0>(q+2);
            del_q++;
        }
        else
            est=err;
    }
    
    r=pow(0.5/est,1.0/static_cast<type0>(q+1+del_q));
    
}
/*--------------------------------------------
 max step size
 --------------------------------------------*/
inline type0 DMD_mbdf::err_est(int q,type0 del_t)
{
    type0 tmp0,err_lcl,err,k0;
    type0* c=mapp->c->begin();
    
    k0=1.0;
    for(int i=0;i<q;i++)
        k0*=static_cast<type0>(i+1)/(1.0-t[i]/del_t);
    
    for(int i=0;i<q;i++)
    {
        tmp0=1.0;
        for(int j=0;j<q;j++)
            if(j!=i)
                tmp0*=(del_t-t[j])/(t[i]-t[j]);
        
        alph_err[i]=-tmp0;
    }
    
    err_lcl=0.0;
    for(int i=0;i<ncs;i++)
    {
        if(c[i]>=0.0)
        {
            tmp0=c[i];
            for(int j=0;j<q;j++)
                tmp0+=alph_err[j]*y[j][i];
            err_lcl+=tmp0*tmp0;
        }
    }
    MPI_Allreduce(&err_lcl,&err,1,MPI_TYPE0,MPI_SUM,world);
    err=sqrt(err/nc_dofs)/a_tol;
    err*=fabs(k0);
    return err;
}

