#ifndef __MAPP__ls__
#define __MAPP__ls__
namespace MAPP_NS
{
    enum
    {
        LS_S,
        LS_F_DOWNHILL,
        LS_F_GRAD0,
        LS_MIN_ALPHA,
        
        MIN_S_TOLERANCE,
        MIN_F_MAX_ITER,
        
        B_S,
        B_F_MAX_ALPHA,
        B_F_DOWNHILL
    };
    
    template<class Func>
    class LineSearch
    {
    private:
    protected:
        
        type0 epsilon;
        type0 epsilon_3_4;
        
        type0 max_dx;
        type0 golden;
        type0 prev_val,h_norm;
        int bracket(type0,type0,type0&,type0&,type0&,type0&,type0&,type0&);
        Func* func;
    public:

        LineSearch(MAPP*);
        virtual ~LineSearch();
        virtual int line_min(type0&,type0&,int)=0;
        virtual void init(Func*);
        void test(type0,type0,type0);
    };

}
#include <limits>
using namespace MAPP_NS;
/*--------------------------------------------
 constructor
 --------------------------------------------*/
template<class Func>
LineSearch<Func>::LineSearch(MAPP* mapp)
{
    max_dx=1.0;
    prev_val=0.0;
    golden=0.5+0.5*sqrt(5.0);
    epsilon=std::numeric_limits<type0>::epsilon();
    epsilon_3_4=pow(epsilon,0.75);
    
    prev_val=-1.0;
}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
template<class Func>
LineSearch<Func>::~LineSearch()
{
    
}
/*--------------------------------------------
 destructor
 --------------------------------------------*/
template<class Func>
void LineSearch<Func>::init(Func* func_)
{
    func=func_;
}
/*--------------------------------------------
 bracketing routine
 --------------------------------------------*/
template<class Func>
int LineSearch<Func>::bracket(type0 dfa,type0 max_a,type0& a,
type0& b,type0& c,type0& fa,type0& fb,type0& fc)
{
    type0 u,fu,r,q,ulim;
    int uphill_iter=2,iter;
    
    if(dfa<-epsilon_3_4)
    {
        b=MIN(max_a,MAX(sqrt(epsilon),2.0*sqrt(epsilon)/dfa));
        b=1.0e-4*max_a;
    }
    else
        b=1.0e-2*max_a;
    
    r=u=b;
    b=0.0;
    fb=fa;
    iter=uphill_iter;
    
    while(fb>=fa && u<max_a && iter)
    {
        fu=func->F(u);
        if(fa<=fu)
            iter--;
        else
            iter=uphill_iter;
        b=u;
        fb=fu;
        u*=2.0;
    }
    //test(fa,dfa,max_a);
    
    if(u>max_a && fb>=fa)
    {
        
        //test(fa,dfa,max_a);
        return B_F_MAX_ALPHA;
    }
    
    if(fb>fa)
    {
        //printf("bbb %e \n",-2.0/(1.0+dfa/sqrt(epsilon)));
        //test(fa,dfa,max_a);
        return B_F_DOWNHILL;
    }
    
    fc=fb;
    while (fb>=fc)
    {
        
        c=b+golden*(b-a);
        if(c>=max_a)
        {
            c=max_a;
            fc=func->F(c);
            return B_S;
        }
        
        fc=func->F(c);
        
        if(fc>fb)
            continue;
        
        ulim=MIN(b+(golden+2.0)*(b-a),max_a);
        ulim=max_a;
        
        r=(b-a)*(fb-fc);
        q=(b-c)*(fb-fa);
        
        u=0.5*b+(c*q-a*r)/(2.0*(q-r));
        
        if(b<u && u<c)
        {
            fu=func->F(u);
            if(fu<fc)
            {
                a=b;
                b=u;
                fa=fb;
                fb=fu;
                return B_S;
            }
            else if (fu>fb)
            {
                c=u;
                fc=fu;
                return B_S;
            }
            
            a=b;
            b=c;
            
            fa=fb;
            fb=fc;
        }
        else if(u>c)
        {
            u=MIN(u,ulim);
            fu=func->F(u);
            
            a=b;
            b=c;
            c=u;
            
            fa=fb;
            fb=fc;
            fc=fu;
            
            if(fu>fc)
            {
                return B_S;
            }
        }
        else
        {
            a=b;
            b=c;
            
            fa=fb;
            fb=fc;
        }
    }
    
    return B_S;
}
/*--------------------------------------------
 reset to initial position
 --------------------------------------------*/
template<class Func>
void LineSearch<Func>::test(type0 fa,type0 dfa,type0 max_a)
{
    int no=100;
    type0 frac=1.0e-2*max_a;
    type0 dfu,fu,u=0.0;
    
    printf("dfa %e\n",dfa);
    printf("u fu f_x u*dfa\n");

    type0 sum=0.0;
    for(int i=0;i<no;i++)
    {
        fu=func->dF(u,dfu);
        printf("%22.20lf %22.20lf %22.20lf %22.20lf \n",u,fu-fa,sum,u*dfa);
        sum+=frac*dfu;
        u+=frac;
    }
    
}



#endif 
