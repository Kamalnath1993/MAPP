/*--------------------------------------------
 Created by Sina on 06/20/13.
 Copyright (c) 2013 MIT. All rights reserved.
 --------------------------------------------*/
#include <stdlib.h>
#include "md_nh.h"
#include "ff.h"
#include "random.h"
#include "atom_types.h"
#include "error.h"
#include "memory.h"
#include "write.h"
#include "timer.h"
#include "neighbor.h"
#include "thermo_dynamics.h"
using namespace MAPP_NS;
enum {NONE,TAU,XYZ,YZ,ZX,XY};
/*--------------------------------------------
 constructor
 --------------------------------------------*/
MD_nh::MD_nh(MAPP* mapp,int nargs,char** args)
: MD(mapp)
{

    if(atoms->dimension!=3)
        error->abort("dimension of the box should be 3 for md nh");
    
    //the defaults
    no_it_eta=1;
    no_ch_eta=3;
    t_tar=0;
    chk_create_vel=0;
    dt=0.0;

    
    int iarg=2;
    if(!strcmp(args[iarg],"nvt"))
    {
        chk_stress=NONE;
        iarg++;
    }
    else if(!strcmp(args[iarg],"ntaut"))
    {
        chk_stress=TAU;
        iarg++;
    }
    else if(!strcmp(args[iarg],"npt")&&((iarg+2)<=nargs))
    {
        
        iarg++;
        if(!strcmp(args[iarg],"xyz")
           || !strcmp(args[iarg],"xzy")
           || !strcmp(args[iarg],"yzx")
           || !strcmp(args[iarg],"yxz")
           || !strcmp(args[iarg],"zxy")
           || !strcmp(args[iarg],"zyx"))
        {
            chk_stress=XYZ;
            iarg++;
        }
        else if(!strcmp(args[iarg],"yz")
                || !strcmp(args[iarg],"zy"))
        {
            chk_stress=YZ;
            iarg++;
        }
        else if(!strcmp(args[iarg],"zx")
                || !strcmp(args[iarg],"xz"))
        {
            chk_stress=ZX;
            iarg++;
        }
        else if(!strcmp(args[iarg],"xy")
                || !strcmp(args[iarg],"yx"))
        {
            chk_stress=XY;
            iarg++;
        }
        else
            error->abort("unknown coupling style for npt in md nh: %s",args[iarg]);
        
    }
    else error->abort("unknown ensemble for md nh: %s",args[iarg]);
  
    

    // create and allocate the memory for
    // necessary

    if(chk_stress)
    {
        CREATE1D(chk_tau,6);
        for(int i=0;i<6;i++)
            chk_tau[i]=0;
        CREATE1D(v_per_atm,6);
        CREATE1D(tau_freq,6);
        CREATE1D(tau_tar,6);
        CREATE1D(omega_m,6);
        CREATE1D(omega_d,6);
        no_it_peta=1;
        no_ch_peta=3;
        
    }

    CREATE1D(ke_curr,6);

    
    
    while(iarg<nargs)
    {
        if(!strcmp(args[iarg],"temp")&&((iarg+2)<=nargs))
        {
            iarg++;
            if(nargs-iarg<2)
                error->abort("temp in md nh should at least have 2arguments");
                
            t_tar=atof(args[iarg]);
            if(t_tar<=0)
                error->abort("temp in md nh should be greater than 0.0");
            iarg++;
            if(atof(args[iarg])<=0)
                error->abort("temp frequency (arguement 2 after temp) in md nh should be greater than 0.0");
            t_freq=1.0/atof(args[iarg]);
            iarg++;
        }
        else if(!strcmp(args[iarg],"stress")&&((iarg+7)<=nargs))
        {
            iarg++;
            if(chk_stress==NONE)
                error->abort("stress in md nh is valid for ntaut or npt ensemble");
            if(chk_stress==TAU)
            {
                if(nargs-iarg<12)
                    error->abort("stress in md nh should at least have 12arguments");
                for(int i=0;i<6;i++)
                {
                    tau_tar[i]=-atof(args[iarg]);
                    iarg++;
                    
                    if (atof(args[iarg])<=0)
                        error->abort("stress frequency (arguement %d after stress) in md nh should be greater than 0.0",(i+1)*2);
                    tau_freq[i]=1.0/atof(args[iarg]);
                    
                    chk_tau[i]=1;
                    iarg++;
                }
            }
            else
            {
                if(nargs-iarg<2)
                    error->abort("stress in md nh should at least have 2arguments");
                if(chk_stress==XYZ)
                {
                    tau_tar[0]=tau_tar[1]=tau_tar[2]=-atof(args[iarg]);
                    chk_tau[0]=chk_tau[1]=chk_tau[2]=1;
                    iarg++;
                    if (atof(args[iarg])<=0)
                        error->abort("stress frequency (arguement 2 after ave) in md nh should be greater than 0.0");
                    tau_freq[0]=tau_freq[1]=tau_freq[2]=1.0/atof(args[iarg]);
                    iarg++;
                }
                else if(chk_stress==YZ)
                {
                    tau_tar[1]=tau_tar[2]=-atof(args[iarg]);
                    chk_tau[1]=chk_tau[2]=1;
                    iarg++;
                    if (atof(args[iarg])<=0)
                        error->abort("stress frequency (arguement 2 after stress) in md nh should be greater than 0.0");
                    tau_freq[1]=tau_freq[2]=1.0/atof(args[iarg]);
                    iarg++;
                }
                else if(chk_stress==ZX)
                {
                    tau_tar[0]=tau_tar[2]=-atof(args[iarg]);
                    chk_tau[0]=chk_tau[2]=1;
                    iarg++;
                    if (atof(args[iarg])<=0)
                        error->abort("stress frequency (arguement 2 after stress) in md nh should be greater than 0.0");
                    tau_freq[0]=tau_freq[2]=1.0/atof(args[iarg]);
                    iarg++;
                }
                else if(chk_stress==XY)
                {
                    tau_tar[0]=tau_tar[1]=-atof(args[iarg]);
                    chk_tau[0]=chk_tau[1]=1;
                    iarg++;
                    if (atof(args[iarg])<=0)
                        error->abort("stress frequency (arguement 2 after stress) in md nh should be greater than 0.0");
                    tau_freq[0]=tau_freq[1]=1.0/atof(args[iarg]);
                    iarg++;
                }
            }

        }
        else if(!strcmp(args[iarg],"eta_iter")&&((iarg+2)<=nargs))
        {
            iarg++;
            no_it_eta=atoi(args[iarg]);
            if (no_it_eta<1)
                error->abort("eta_iter in md nh should be greater than 0");
            iarg++;
        }
        else if(!strcmp(args[iarg],"peta_iter")&&((iarg+2)<=nargs))
        {
            if(chk_stress==NONE)
                error->abort("peta_iter in md nh is valid for npt ntaut ensemble");
            iarg++;
            no_it_peta=atoi(args[iarg]);
            if (no_it_peta<1)
                error->abort("peta_iter in md nh should be greater than 0");
            iarg++;
        }
        else if(!strcmp(args[iarg],"eta_chains")&&((iarg+2)<=nargs))
        {
            iarg++;
            no_ch_eta=atoi(args[iarg]);
            if (no_ch_eta<3)
                error->abort("eta_chains in md nh should be greater than 2");
            iarg++;
        }
        else if(!strcmp(args[iarg],"peta_chains")&&((iarg+2)<=nargs))
        {
            if(chk_stress==NONE)
                error->abort("peta_chains in md nh is valid for npt ntaut ensemble");
            iarg++;
            no_ch_peta=atoi(args[iarg]);
            if (no_ch_peta<3)
                error->abort("peta_chains in md nh should be greater than 2");
            iarg++;
        }
        else if(!strcmp(args[iarg],"create_vel")&&((iarg+2)<=nargs))
        {
            chk_create_vel=1;
            iarg++;
            seed=atoi(args[iarg]);
            iarg++;
        }
        else error->abort("unknown keyword for md nh: %s",args[iarg]);
    }
    
    // check wether all the relavant quantities are assigned or not
    // check wether the temperature is set
    if(t_tar==0)
        error->abort("temp was not set by md nh");
    // check wether the stresses are set
    if(chk_stress)
    {
        int temp=1;
        if(chk_stress==TAU)
        {
            for(int i=0;i<6;i++)
                temp*=chk_tau[i];
        }
        else if(chk_stress==XYZ)
        {
            temp=chk_tau[0]*chk_tau[1]*chk_tau[2];
            for(int i=0;i<6;i++)
                if(i!=0&&i!=1&&i!=2)
                    temp*=1-chk_tau[i];
            
        }
        else if(chk_stress==YZ)
        {
            temp=chk_tau[1]*chk_tau[2];
            for(int i=0;i<6;i++)
                if(i!=1&&i!=2)
                    temp*=1-chk_tau[i];
            
        }
        else if(chk_stress==ZX)
        {
            temp=chk_tau[0]*chk_tau[2];
            for(int i=0;i<6;i++)
                if(i!=0&&i!=2)
                    temp*=1-chk_tau[i];
            
        }
        else if(chk_stress==XY)
        {
            temp=chk_tau[0]*chk_tau[1];
            for(int i=0;i<6;i++)
                if(i!=0&&i!=1)
                    temp*=1-chk_tau[i];
            
        }
        
        if(temp==0)
        {
            if(chk_stress==TAU)
                error->abort("stress was not set by md nh");
            else
                error->abort("ave was not set by md nh");
        }
    }
        
    // allocate the Nose Hoover chains stuff
    
    if(chk_stress)
    {
        CREATE1D(peta_d,no_ch_peta);
        CREATE1D(peta_dd,no_ch_peta);
        CREATE1D(peta_m,no_ch_peta);
        for(int i=0;i<no_ch_peta;i++)
            peta_d[i]=0.0;
        CREATE1D(tmp_x,3);
        CREATE1D(tmp_fac,3);
        CREATE2D(M1,3,3);
        CREATE2D(M2,3,3);
    }
    
    CREATE1D(tmp_ke_curr,6);
    CREATE1D(eta_d,no_ch_eta);
    CREATE1D(eta_dd,no_ch_eta);
    CREATE1D(eta_m,no_ch_eta);
    for(int i=0;i<no_ch_eta;i++)
        eta_d[i]=0.0;

    for(int i=0;i<6;i++)
        ke_curr[i]=0.0;
    
    tau_freq_m=0.0;
    if(chk_stress)
        for(int i=0;i<6;i++)
            if(chk_tau[i])
                tau_freq_m=MAX(tau_freq_m,tau_freq[i]);
    
    CREATE1D(x_ave,3);
    CREATE1D(x_ave_tot,3);
    
    boltz=0.0;

}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
MD_nh::~MD_nh()
{
    
    
    if(chk_stress)
    {
        for(int i=0;i<3;i++)
        {
            delete [] M1[i];
            delete [] M2[i];
        }
        delete [] tmp_x;
        delete [] M1;
        delete [] M2;
        delete [] ke_curr;
        delete [] tmp_fac;
        delete [] chk_tau;
        delete [] v_per_atm;
        delete [] tau_tar;
        delete [] tau_freq;
        delete [] omega_m;
        delete [] omega_d;
    }
    
    delete [] tmp_ke_curr;
    delete [] eta_d;
    delete [] eta_dd;
    delete [] eta_m;
    
    if(chk_stress)
    {
        delete [] peta_d;
        delete [] peta_dd;
        delete [] peta_m;
    }
    
    delete [] x_ave_tot;
    delete [] x_ave;
    
}
/*--------------------------------------------
 setup before start of run
 --------------------------------------------*/
void MD_nh::init()
{
    if(boltz==0.0)
        error->abort("boltzmann should be set after md nh and before run");
    if(dt==0.0)
        error->abort("time_step should be set after md nh and before run");
    
    no_dof=static_cast<type0>(atoms->tot_natms*3-3);
    ke_tar=t_tar*boltz*no_dof;
    dt2=0.5*dt;
    dt4=0.25*dt;
    dt8=0.125*dt;

    
    

    if(mapp->x_d==NULL)
        mapp->x_d=new Vec<type0>(atoms,3);
    
    if(mapp->f==NULL)
        mapp->f=new Vec<type0>(atoms,3);
    
    
    x_dim=mapp->x->dim;
    x_d_dim=mapp->x_d->dim;
    f_dim=mapp->f->dim;
    
    /*
     0. set the atomic vectors communication
     we need force and type in addition to
     x vector; however, we do not need to
     include force since their pervious
     values are not important and are set to
     0; when a communication happens atoms
     allocates the f vector with the
     appropriate size
     */

    dof_xst=false;
    if(mapp->dof!=NULL)
    {
        dof_xst=true;
        byte* dof=mapp->dof->begin();
        dof_dim=mapp->dof->dim;
        
        int tmp_0=0;
        int tmp_1=0;
        int tmp_2=0;
        
        for(int i=0;i<atoms->natms;i++)
        {
            if(dof[dof_dim*i]==1) tmp_0++;
            if(dof[dof_dim*i+1]==1) tmp_1++;
            if(dof[dof_dim*i+2]==1) tmp_2++;
        }
        
        
        int tmp_all_0=0;
        int tmp_all_1=0;
        int tmp_all_2=0;
        MPI_Allreduce(&tmp_0,&tmp_all_0,1,MPI_INT,MPI_SUM,world);
        MPI_Allreduce(&tmp_1,&tmp_all_1,1,MPI_INT,MPI_SUM,world);
        MPI_Allreduce(&tmp_2,&tmp_all_2,1,MPI_INT,MPI_SUM,world);
        
        no_dof-=static_cast<type0>(tmp_all_0+tmp_all_1+tmp_all_2);
        
        if(chk_stress)
            omega_denom-=chk_tau[0]*tmp_all_0
            +chk_tau[1]*tmp_all_1+chk_tau[2]*tmp_all_2;
        
        
    }
    else
    {
        if(chk_stress)
        {
            omega_denom=chk_tau[0]*atoms->tot_natms
            +chk_tau[1]*atoms->tot_natms
            +chk_tau[2]*atoms->tot_natms;
        }
    }
    
    if(no_dof==0.0)
        error->abort("degrees of freedom shoud be greater than 0 for md nh");
    
    vecs_comm=new VecLst(atoms);
    vecs_comm->add_updt(mapp->type);
    vecs_comm->add_xchng(mapp->x_d);
    
    if(mapp->dof!=NULL)
    {
        vecs_comm->add_xchng(mapp->dof);
    }
    
    if(chk_stress)
        atoms->init(vecs_comm,true);
    else
        atoms->init(vecs_comm,false);


    if(chk_create_vel)
        create_vel(seed,t_tar);
    else
        init_vel(t_tar);
    
    zero_f();
    
    forcefield->force_calc_timer(1,nrgy_strss);
    
    thermo->update(stress_idx,6,&nrgy_strss[1]);
    thermo->update(pe_idx,nrgy_strss[0]);
    thermo->update(ke_idx,ke_cur);
    thermo->update(temp_idx,t_cur);

    thermo->init();
    

    if(write!=NULL)
        write->init();

    for(int i=0;i<no_ch_eta;i++)
        eta_m[i] = boltz * t_tar/(t_freq*t_freq);
    eta_m[0]*=no_dof;

    for(int i=0;i<no_ch_eta;i++)
        eta_d[i]=0.0;
    
    for(int i=1;i<no_ch_eta;i++)
        eta_dd[i] = (eta_m[i-1]*eta_d[i-1]*eta_d[i-1]-
                    boltz*t_tar)/eta_m[i];
    
    if(chk_stress)
    {
        for(int i=0;i<6;i++)
            v_per_atm[i]=-nrgy_strss[i+1];
       
        for(int i=0;i<no_ch_peta;i++)
            peta_m[i] = boltz * t_tar/(tau_freq_m*tau_freq_m);
        
        
        for(int i=0;i<no_ch_peta;i++)
            peta_d[i]=0.0;
        
        for(int i=1;i<no_ch_peta;i++)
            peta_dd[i] = (peta_m[i-1]*peta_d[i-1]*peta_d[i-1]-
                         boltz*t_tar)/peta_m[i];
        for(int i=0;i<6;i++)
            omega_d[i]=omega_m[i]=0.0;
            
         
    }
    
    

}
/*--------------------------------------------
 finalize after the run is complete
 --------------------------------------------*/
void MD_nh::fin()
{
    if(write!=NULL)
        write->fin();
    
    thermo->fin();
    atoms->fin();
    timer->print_stats();
    neighbor->print_stats();
    
    delete vecs_comm;
}
/*--------------------------------------------
 MDrun
 --------------------------------------------*/
void MD_nh::run(int no_stps)
{
    type0 vol=1.0;
    for(int idim=0;idim<3;idim++)
        vol*=atoms->H[idim][idim];
    if(chk_stress)
    {
        for(int i=0;i<no_stps;i++)
        {
            update_NH_tau(dt2);
            update_NH_T(dt2);
            update_omega_d(dt2);
            update_x_d_xpnd(dt2);
            update_x_d(dt2);
            update_x(dt);
            
            atoms->update(mapp->x);
            
            zero_f();
            thermo->thermo_print();
        
            if(write!=NULL)
                write->write();
            
            forcefield->force_calc_timer(1,nrgy_strss);
            
            
            for(int j=0;j<6;j++)
            {
                vol=1.0;
                for(int idim=0;idim<3;idim++)
                    vol*=atoms->H[idim][idim];
                v_per_atm[j]=-nrgy_strss[1+j];
            }

            update_x_d(dt2);
            update_x_d_xpnd(dt2);
            update_omega_d(dt2);
            update_NH_T(dt2);
            update_NH_tau(dt2);
            
            if(thermo->test_prev_step()|| i==no_stps-1)
            {
                for(int j=0;j<6;j++)
                    nrgy_strss[1+j]-=ke_curr[j]/vol;
                thermo->update(stress_idx,6,&nrgy_strss[1]);
                thermo->update(pe_idx,nrgy_strss[0]);
                thermo->update(ke_idx,ke_cur);
                thermo->update(temp_idx,t_cur);
                
            }
            step_no++;
        }
    }
    else
    {

        for(int i=0;i<no_stps;i++)
        {
            update_NH_T(dt2);
            update_x_d(dt2);
            update_x(dt);
            
            atoms->update(mapp->x);
            
            zero_f();
            thermo->thermo_print();
            
            if(write!=NULL)
                write->write();
            
            if(thermo->test_prev_step()|| i==no_stps-1)
                 forcefield->force_calc_timer(1,&nrgy_strss[0]);
            else
                forcefield->force_calc_timer(0,&nrgy_strss[0]);

            update_x_d(dt2);
            update_NH_T(dt2);
            
            if(thermo->test_prev_step()|| i==no_stps-1)
            {
                for(int j=0;j<6;j++)
                    nrgy_strss[1+j]-=ke_curr[j]/vol;
                thermo->update(stress_idx,6,&nrgy_strss[1]);
                thermo->update(pe_idx,nrgy_strss[0]);
                thermo->update(ke_idx,ke_cur);
                thermo->update(temp_idx,t_cur);
            }
            
            step_no++;
        }
    }
}
/*--------------------------------------------
 
 --------------------------------------------*/
void MD_nh::update_H(type0 dlt)
{
    type0 dlt2,dlt4,dlt8,exfac;
    type0 H0[3][3];
    type0 B0[3][3];
    dlt2=0.5*dlt;
    dlt4=0.25*dlt;
    dlt8=0.125*dlt;
    
    
    M3EQV(atoms->H,H0);
    
    for(int i=0;i<2;i++)
    {
        M3INV_TRI_LOWER(H0,B0);
        
        if (chk_tau[4])
        {
            exfac=exp(dlt8*omega_d[0]);
            H0[2][0]*=exfac;
            H0[2][0]+=dlt4*(omega_d[5]*H0[2][1]
                            +omega_d[4]*H0[2][2]);
            H0[2][0]*=exfac;
        }
        
        if (chk_tau[3])
        {
            exfac=exp(dlt4*omega_d[1]);
            H0[2][1]*=exfac;
            H0[2][1]+=dlt2*omega_d[3]*H0[2][2];
            H0[2][1]*=exfac;
        }
        
        if (chk_tau[5])
        {
            exfac=exp(dlt4*omega_d[0]);
            H0[1][0]*=exfac;
            H0[1][0]+=dlt2*omega_d[5]*H0[1][1];
            H0[1][0]*=exfac;
        }
        
        if (chk_tau[4])
        {
            exfac=exp(dlt8*omega_d[0]);
            H0[2][0]*=exfac;
            H0[2][0]+=dlt4*(omega_d[5]*H0[2][1]+omega_d[4]*H0[2][2]);
            H0[2][0]*=exfac;
        }
        
        if (chk_tau[0])
        {
            exfac=exp(dlt*omega_d[0]);
            H0[0][0]*=exfac;
        }
        
        if (chk_tau[1])
        {
            exfac=exp(dlt*omega_d[1]);
            H0[1][1]*=exfac;
            H0[1][0]*=exfac;
        }
        
        if (chk_tau[2])
        {
            exfac=exp(dlt*omega_d[2]);
            H0[2][2]*=exfac;
            H0[2][1]*=exfac;
            H0[2][0]*=exfac;
        }
        
        if (chk_tau[4])
        {
            exfac=exp(dlt8*omega_d[0]);
            H0[2][0]*=exfac;
            H0[2][0]+=dlt4*(omega_d[5]*H0[2][1]+omega_d[4]*H0[2][2]);
            H0[2][0]*=exfac;
        }
        
        if (chk_tau[5])
        {
            exfac=exp(dlt4*omega_d[0]);
            H0[1][0]*=exfac;
            H0[1][0]+=dlt2*omega_d[5]*H0[1][1];
            H0[1][0]*=exfac;
        }
        
        if (chk_tau[3])
        {
            exfac=exp(dlt4*omega_d[1]);
            H0[2][1]*=exfac;
            H0[2][1]+=dlt2*omega_d[3]*H0[2][2];
            H0[2][1]*=exfac;
        }
        
        if (chk_tau[4])
        {
            exfac=exp(dlt8*omega_d[0]);
            H0[2][0]*=exfac;
            H0[2][0]+=dlt4*(omega_d[5]*H0[2][1]+omega_d[4]*H0[2][2]);
            H0[2][0]*=exfac;
        }
        
        if (i==0) M3MUL_TRI_LOWER(B0,H0,M1);
        else if (i==1) M3MUL_TRI_LOWER(B0,H0,M2);
    }
    
    M3EQV(H0,atoms->H);
    M3INV_TRI_LOWER(atoms->H,atoms->B);
    
}
/*--------------------------------------------
 
 --------------------------------------------*/
void MD_nh::update_x(type0 dlt)
{
    type0* x=mapp->x->begin();
    type0* x_d=mapp->x_d->begin();
    
    byte* dof=NULL;
    if(dof_xst)
        dof=mapp->dof->begin();
    
    int natms=atoms->natms;
    int icomp,iicomp,iiicomp;
    x_ave[0]=x_ave[1]=x_ave[2]=0.0;
    x_ave_tot[0]=x_ave_tot[1]=x_ave_tot[2]=0.0;
    
    if (chk_stress)
    {
        update_H(0.5*dlt);
        for(int i=0;i<natms;i++)
        {
            icomp=x_dim*i;
            iicomp=x_d_dim*i;
            
            x_ave[0]-=x[icomp];
            x_ave[1]-=x[icomp+1];
            x_ave[2]-=x[icomp+2];
            
            tmp_x[0]=x[icomp]*M1[0][0]
            +x[icomp+1]*M1[1][0]
            +x[icomp+2]*M1[2][0];
            tmp_x[1]=x[icomp+1]*M1[1][1]
            +x[icomp+2]*M1[2][1];
            tmp_x[2]=x[icomp+2]*M1[2][2];
            
            tmp_x[0]+=x_d[iicomp]*dlt;
            tmp_x[1]+=x_d[iicomp+1]*dlt;
            tmp_x[2]+=x_d[iicomp+2]*dlt;
            
            if(dof_xst)
            {
                iiicomp=dof_dim*i;
                if(dof[iiicomp]==0)
                    x[icomp]=tmp_x[0]*M2[0][0]
                    +tmp_x[1]*M2[1][0]
                    +tmp_x[2]*M2[2][0];
                
                if(dof[iiicomp+1]==0)
                    x[icomp+1]=tmp_x[1]*M2[1][1]
                    +tmp_x[2]*M2[2][1];
                
                if(dof[iiicomp+2]==0)
                    x[icomp+2]=tmp_x[2]*M2[2][2];
                    
            }
            else
            {
                x[icomp]=tmp_x[0]*M2[0][0]
                +tmp_x[1]*M2[1][0]
                +tmp_x[2]*M2[2][0];
                x[icomp+1]=tmp_x[1]*M2[1][1]
                +tmp_x[2]*M2[2][1];
                x[icomp+2]=tmp_x[2]*M2[2][2];
            }
            
            x_ave[0]+=x[icomp];
            x_ave[1]+=x[icomp+1];
            x_ave[2]+=x[icomp+2];
        }
    }
    else
    {
        for(int i=0;i<natms;i++)
        {
            icomp=x_dim*i;
            iicomp=x_d_dim*i;
            x[icomp]+=x_d[iicomp]*dlt;
            x[icomp+1]+=x_d[iicomp+1]*dlt;
            x[icomp+2]+=x_d[iicomp+2]*dlt;
            x_ave[0]+=x_d[iicomp]*dlt;
            x_ave[1]+=x_d[iicomp+1]*dlt;
            x_ave[2]+=x_d[iicomp+2]*dlt;
        }
    }
    
    MPI_Allreduce(&x_ave[0],&x_ave_tot[0],3,MPI_TYPE0,MPI_SUM,world);
    x_ave_tot[0]*=1.0/(atoms->tot_natms);
    x_ave_tot[1]*=1.0/(atoms->tot_natms);
    x_ave_tot[2]*=1.0/(atoms->tot_natms);
    
    for(int i=0;i<natms;i++)
    {
        icomp=x_dim*i;
        x[icomp]-=x_ave_tot[0];
        x[icomp+1]-=x_ave_tot[1];
        x[icomp+2]-=x_ave_tot[2];
    }


}
/*--------------------------------------------
 
 --------------------------------------------*/
void MD_nh::update_x_d(type0 dlt)
{
    
    type0* x_d=mapp->x_d->begin();
    type0* f=mapp->f->begin();
    md_type* type=mapp->type->begin();
    
    byte* dof=NULL;
    if(dof_xst)
        dof=mapp->dof->begin();

    type0* mass=atom_types->mass;
    int natms=atoms->natms;
    int icomp,iicomp,iiicomp;
    
    for(int i=0;i<6;i++)
        tmp_ke_curr[i]=0.0;
    
    for(int i=0;i<natms;i++)
    {
        icomp=x_d_dim*i;
        iicomp=f_dim*i;
        
        x_d[icomp]+=f[iicomp]*dlt/mass[type[i]];
        x_d[icomp+1]+=f[iicomp+1]*dlt/mass[type[i]];
        x_d[icomp+2]+=f[iicomp+2]*dlt/mass[type[i]];
        
        if(dof_xst)
        {
            iiicomp=dof_dim*i;
            if(dof[iiicomp]==1)
                x_d[icomp]=0.0;
            if(dof[iiicomp+1]==1)
                x_d[icomp+1]=0.0;
            if(dof[iiicomp+2]==1)
                x_d[icomp+2]=0.0;
            
        }
        
        tmp_ke_curr[0]+=mass[type[i]]*x_d[icomp]*x_d[icomp];
        tmp_ke_curr[1]+=mass[type[i]]*x_d[icomp+1]*x_d[icomp+1];
        tmp_ke_curr[2]+=mass[type[i]]*x_d[icomp+2]*x_d[icomp+2];
        tmp_ke_curr[3]+=mass[type[i]]*x_d[icomp+1]*x_d[icomp+2];
        tmp_ke_curr[4]+=mass[type[i]]*x_d[icomp]*x_d[icomp+2];
        tmp_ke_curr[5]+=mass[type[i]]*x_d[icomp]*x_d[icomp+1];

    }
    
    for(int i=0;i<6;i++)
        ke_curr[i]=0.0;
    MPI_Allreduce(tmp_ke_curr,ke_curr,6,MPI_TYPE0,MPI_SUM,world);
    ke_cur=(ke_curr[0]+ke_curr[1]+ke_curr[2]);

    t_cur=ke_cur/(boltz*no_dof);

}
/*--------------------------------------------
 Nosé–Hoover thermostat chains 
 --------------------------------------------*/
void MD_nh::update_NH_T(type0 dlt)
{
    type0 dltm,dltm2,dltm4,exfac,velfac;
    int natoms=atoms->natms;

    
    for(int i=0;i<no_ch_eta;i++)
        eta_m[i]=boltz*t_tar/(t_freq*t_freq);
    eta_m[0]*=no_dof;
    
    dltm=dlt*(1.0/no_it_eta);
    dltm2=0.5*dltm;
    dltm4=0.25*dltm;
    velfac=1.0;
    
    eta_dd[0]=(ke_cur-ke_tar)/eta_m[0];
    for(int it=0;it<no_it_eta;it++)
    {
        exfac=1.0;
        for(int ich=no_ch_eta-1;ich>-1;ich--)
        {
            eta_d[ich]*=exfac;
            eta_d[ich]+=eta_dd[ich]*dltm2;
            eta_d[ich]*=exfac;
            exfac=exp(-dltm4*eta_d[ich]);
        }

        //rescale x_d dlt & claculate the new temperature
        exfac=exp(-dltm*eta_d[0]);
        velfac*=exfac;
        t_cur*=exfac*exfac;
        ke_cur*=exfac*exfac;
        ke_curr[0]*=exfac*exfac;
        ke_curr[1]*=exfac*exfac;
        ke_curr[2]*=exfac*exfac;
        ke_curr[3]*=exfac*exfac;
        ke_curr[4]*=exfac*exfac;
        ke_curr[5]*=exfac*exfac;
        
        exfac=exp(-dltm4*eta_d[1]);
        eta_d[0]*=exfac;
        eta_dd[0]=(ke_cur-ke_tar)/eta_m[0];
        eta_d[0]+=eta_dd[0]*dltm2;
        eta_d[0]*=exfac;
        
        for(int ich=1;ich<no_ch_eta;ich++)
        {
            if (ich==no_ch_eta-1) exfac=1.0;
            else exfac=exp(-dltm4*eta_d[ich+1]);
            
            eta_d[ich]*=exfac;
            eta_dd[ich]=(eta_m[ich-1]*eta_d[ich-1]*eta_d[ich-1]-boltz*t_tar)/eta_m[ich];
            eta_d[ich]+=eta_dd[ich]*dltm2;
            eta_d[ich]*=exfac;
        }
    }

    type0* x_d=mapp->x_d->begin();
    for(int i=0;i<natoms;i++)
        for(int j=0;j<3;j++)
            x_d[i*x_d_dim+j]*=velfac;
    
}
/*--------------------------------------------
 
 --------------------------------------------*/
void MD_nh::update_NH_tau(type0 dlt)
{
    type0 dltm,dltm2,dltm4,exfac,kec;
    int dof=0;
    
    for(int i=0;i<6;i++)
        if (chk_tau[i]) dof++;
    
    for(int i=0;i<6;i++)
        if(chk_tau[i])
            omega_m[i]=boltz*t_tar/(tau_freq[i]*tau_freq[i]);
    
    for(int i=0;i<no_ch_peta;i++)
        peta_m[i]=boltz*t_tar/(tau_freq_m*tau_freq_m);
    
    for(int i=1;i<no_ch_peta;i++)
        peta_dd[i]=(peta_m[i-1]*peta_d[i-1]*peta_d[i-1]-boltz*t_tar)/peta_m[i];
    kec=0.0;
    for(int i=0;i<6;i++)
        kec+=omega_m[i]*omega_d[i]*omega_d[i];
    peta_dd[0]=(kec-boltz*t_tar)/peta_m[0];
    
    dltm=dlt*(1.0/no_it_peta);
    dltm2=0.5*dltm;
    dltm4=0.25*dltm;
    
    for(int it=0;it<no_ch_peta;it++)
    {
        exfac=1.0;
        for(int ich=no_ch_peta-1;ich>-1;ich--)
        {
            peta_d[ich]*=exfac;
            peta_d[ich]+=peta_dd[ich]*dltm2;
            peta_d[ich]*=exfac;
            exfac=exp(-dltm4*peta_d[ich]);
        }
        
        exfac=exp(-dltm*peta_d[0]);
        for(int i=0;i<6;i++)
            omega_d[i]*=exfac;
        
        kec*=exfac*exfac;
        
        exfac=exp(-dltm4*peta_d[1]);
        peta_d[0]*=exfac;
        peta_dd[0]=(kec-dof*boltz*t_tar)/peta_m[0];
        peta_d[0]+=peta_dd[0]*dltm2;
        peta_d[0]*=exfac;
        
        for(int ich=1;ich<no_ch_peta;ich++)
        {
            if (ich==no_ch_peta-1) exfac=1.0;
            else exfac=exp(-dltm4*peta_d[ich+1]);
            
            peta_d[ich]*=exfac;
            peta_dd[ich]=(peta_m[ich-1]*peta_d[ich-1]*peta_d[ich-1]-boltz*t_tar)/peta_m[ich];
            peta_d[ich]+=peta_dd[ich]*dltm2;
            peta_d[ich]*=exfac;
        }
    }
    

}
/*--------------------------------------------
 
 --------------------------------------------*/
void MD_nh::update_omega_d(type0 dlt)
{
    //type0** H=atoms->H;
    MTK_1=0.0;
    for(int i=0;i<3;i++)
        if (chk_tau[i])
            MTK_1+=ke_curr[i];
    MTK_1/=static_cast<type0>(omega_denom);
    
    couple();
    for(int i=0;i<6;i++)
        if (chk_tau[i])
            omega_d[i]+=((v_per_atm[i]+ke_curr[i]
            -tau_tar[i])
            -MTK_1)*dlt/omega_m[i];
    
    MTK_2=0.0;
    for(int i=0;i<3;i++)
        if (chk_tau[i])
            MTK_2+=omega_d[i];

    MTK_2/=static_cast<type0>(omega_denom);
    
}
/*--------------------------------------------
 
 --------------------------------------------*/
void MD_nh::update_x_d_xpnd(type0 dlt)
{
    
    byte* dof=NULL;
    if(dof_xst)
        dof=mapp->dof->begin();
    
    
    type0* x_d=mapp->x_d->begin();
    md_type* type=mapp->type->begin();
    
    type0* mass=atom_types->mass;
    
    int natms=atoms->natms;
    int icomp,iicomp;
    
    tmp_fac[0]=exp(0.5*dlt*(omega_d[0]+MTK_2));
    tmp_fac[1]=exp(0.5*dlt*(omega_d[1]+MTK_2));
    tmp_fac[2]=exp(0.5*dlt*(omega_d[2]+MTK_2));
    
    for(int i=0;i<6;i++)
        tmp_ke_curr[i]=0.0;
    for(int i=0;i<natms;i++)
    {
        icomp=x_d_dim*i;
        x_d[icomp]*=tmp_fac[0];
        x_d[icomp+1]*=tmp_fac[1];
        x_d[icomp+2]*=tmp_fac[2];
        
        x_d[icomp]-=dlt*(x_d[icomp+1]*omega_d[5]
                       +x_d[icomp+2]*omega_d[4]);
        x_d[icomp+1]-=dlt*x_d[icomp+2]*omega_d[3];
        
        x_d[icomp]*=tmp_fac[0];
        x_d[icomp+1]*=tmp_fac[1];
        x_d[icomp+2]*=tmp_fac[2];
        
        if(dof_xst)
        {
            iicomp=dof_dim*i;
            if(dof[iicomp]==1) x_d[icomp]=0.0;
            if(dof[iicomp+1]==1) x_d[icomp+1]=0.0;
            if(dof[iicomp+2]==1) x_d[icomp+2]=0.0;
        }
        
        tmp_ke_curr[0]+=mass[type[i]]*x_d[icomp]*x_d[icomp];
        tmp_ke_curr[1]+=mass[type[i]]*x_d[icomp+1]*x_d[icomp+1];
        tmp_ke_curr[2]+=mass[type[i]]*x_d[icomp+2]*x_d[icomp+2];
        tmp_ke_curr[3]+=mass[type[i]]*x_d[icomp+1]*x_d[icomp+2];
        tmp_ke_curr[4]+=mass[type[i]]*x_d[icomp]*x_d[icomp+2];
        tmp_ke_curr[5]+=mass[type[i]]*x_d[icomp]*x_d[icomp+1];
    }
    
    for(int i=0;i<6;i++)
        ke_curr[i]=0.0;
    MPI_Allreduce(tmp_ke_curr,ke_curr,6,MPI_TYPE0,MPI_SUM,world);
    ke_cur=(ke_curr[0]+ke_curr[1]+ke_curr[2]);
    t_cur=ke_cur/(boltz*no_dof);
}
/*--------------------------------------------
 zero acceleration
 --------------------------------------------*/
void MD_nh::zero_f()
{
    type0* f=mapp->f->begin();
    for(int i=0;i<atoms->natms*f_dim;i++)
        f[i]=0.0;
}
/*--------------------------------------------
 create initial velocity 
 --------------------------------------------*/
void MD_nh::create_vel(int seed,type0 temperature)
{
    byte* dof=NULL;
    if(dof_xst)
        dof=mapp->dof->begin();
    
    type0* x_d=mapp->x_d->begin();
    md_type* type=mapp->type->begin();
    type0* mass=atom_types->mass;
    
    int natms=atoms->natms;
    int icomp,iicomp;
    type0* temp;
    CREATE1D(temp,6);
    for(int i=0;i<6;i++)
        temp[i]=0.0;
    
    class Random* random=new Random(mapp,seed);
    for(int i=0;i<natms;i++)
    {
        icomp=x_d_dim*i;
        for(int j=0;j<3;j++)
            x_d[icomp+j]=random->gaussian()/(sqrt(mass[type[i]]));
        if(dof_xst)
        {
            iicomp=dof_dim*i;
            if(dof[iicomp]==1) x_d[icomp]=0.0;
            if(dof[iicomp+1]==1) x_d[icomp+1]=0.0;
            if(dof[iicomp+2]==1) x_d[icomp+2]=0.0;
        }

        temp[0]+=mass[type[i]]*x_d[icomp]*x_d[icomp];
        temp[1]+=mass[type[i]]*x_d[icomp+1]*x_d[icomp+1];
        temp[2]+=mass[type[i]]*x_d[icomp+2]*x_d[icomp+2];
        temp[3]+=mass[type[i]]*x_d[icomp+1]*x_d[icomp+2];
        temp[4]+=mass[type[i]]*x_d[icomp]*x_d[icomp+2];
        temp[5]+=mass[type[i]]*x_d[icomp]*x_d[icomp+1];
    }
    delete random;
    
    for(int i=0;i<6;i++)
        ke_curr[i]=0.0;

    
    MPI_Allreduce(&temp[0],&ke_curr[0],6,MPI_TYPE0,MPI_SUM,world);
    
    
    ke_cur=(ke_curr[0]+ke_curr[1]+ke_curr[2]);
    t_cur=ke_cur/(boltz*no_dof);
    
    
    type0 ke_des=(boltz*no_dof)*temperature;
    type0 factor=sqrt(ke_des/ke_cur);
    type0 facsq=ke_des/ke_cur;
    for(int i=0;i<natms;i++)
        for(int j=0;j<3;j++)
            x_d[i*x_d_dim+j]*=factor;
    
    for(int i=0;i<6;i++)
        ke_curr[i]*=facsq;

    ke_cur*=facsq;
    t_cur*=facsq;
    delete [] temp;

}
/*--------------------------------------------
 create initial velocity
 --------------------------------------------*/
void MD_nh::init_vel(type0 temperature)
{

    type0* x_d=mapp->x_d->begin();
    md_type* type=mapp->type->begin();
    type0* mass=atom_types->mass;
    
    int natms=atoms->natms;
    int icomp;
    type0* temp;
    CREATE1D(temp,6);
    
    for(int i=0;i<6;i++)
        temp[i]=0.0;
    
    for(int i=0;i<natms;i++)
    {
        icomp=x_d_dim*i;
        temp[0]+=mass[type[i]]*x_d[icomp]*x_d[icomp];
        temp[1]+=mass[type[i]]*x_d[icomp+1]*x_d[icomp+1];
        temp[2]+=mass[type[i]]*x_d[icomp+2]*x_d[icomp+2];
        temp[3]+=mass[type[i]]*x_d[icomp+1]*x_d[icomp+2];
        temp[4]+=mass[type[i]]*x_d[icomp]*x_d[icomp+2];
        temp[5]+=mass[type[i]]*x_d[icomp]*x_d[icomp+1];
    }
    
    
    for(int i=0;i<6;i++)
        ke_curr[i]=0.0;
    MPI_Allreduce(temp,ke_curr,6,MPI_TYPE0,MPI_SUM,world);
    
    ke_cur=(ke_curr[0]+ke_curr[1]+ke_curr[2]);
    t_cur=ke_cur/(boltz*no_dof);
    
    if(ke_cur==0.0)
        error->abort("kinetic energy of the system should be "
        "greater than 0.0 for md nh, please assign velocities "
        "or use create_vel keyword");
    
    /*
    type0 ke_des=(boltz*no_dof)*temperature;
    type0 factor=sqrt(ke_des/ke_cur);
    type0 facsq=factor*factor;
    
    for(int i=0;i<natms;i++)
        for(int j=0;j<3;j++)
            x_d[i*x_d_dim+j]*=factor;
    
    for(int i=0;i<6;i++)
        ke_curr[i]*=facsq;
    
    ke_cur*=facsq;
    t_cur*=facsq;
    */
    delete [] temp;
}
/*--------------------------------------------
 create initial velocity
 --------------------------------------------*/
void MD_nh::couple()
{
    type0 tmp;
    if(chk_stress==XYZ)
    {
        tmp=v_per_atm[0]+v_per_atm[1]+v_per_atm[2];
        tmp=tmp/3.0;
        v_per_atm[0]=v_per_atm[1]=v_per_atm[2]=tmp;
        
        tmp=ke_curr[0]+ke_curr[1]+ke_curr[2];
        tmp=tmp/3.0;
        ke_curr[0]=ke_curr[1]=ke_curr[2]=tmp;
    }
    else if(chk_stress==YZ)
    {
        tmp=v_per_atm[1]+v_per_atm[2];
        tmp=tmp/2.0;
        v_per_atm[1]=v_per_atm[2]=tmp;

        tmp=ke_curr[1]+ke_curr[2];
        tmp=tmp/2.0;
        ke_curr[1]=ke_curr[2]=tmp;
    }
    else if(chk_stress==ZX)
    {
        tmp=v_per_atm[2]+v_per_atm[0];
        tmp=tmp/2.0;
        v_per_atm[2]=v_per_atm[0]=tmp;
        
        tmp=ke_curr[2]+ke_curr[0];
        tmp=tmp/2.0;
        ke_curr[2]=ke_curr[0]=tmp;

    }
    else if(chk_stress==XY)
    {
        tmp=v_per_atm[0]+v_per_atm[1];
        tmp=tmp/2.0;
        v_per_atm[0]=v_per_atm[1]=tmp;
        
        tmp=ke_curr[0]+ke_curr[1];
        tmp=tmp/2.0;
        ke_curr[0]=ke_curr[1]=tmp;

    }
}

