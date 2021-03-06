#include "Throat_Solver.h"

#include <cmath>
#include <list>
#include <iostream>
#include <array>
#include<vector>

#include "Throat.h"
#include "Throat.h"


Throat_Solver::Throat_Solver()
{
}
Throat_Solver::Throat_Solver(Gas_Model* GM, Throat* TH)
{
    double a0 = GM->get_a0();
    double gamma = GM->get_gamma();
    double rad_th = TH->get_upstream_rth();
    double yt = TH->get_height();
    int delta = TH->get_delta();

    this->GM = GM;
    this->TH = TH;

    ///prepare parameters ### switch to Throat_Solver constructor?
    this->astar = a0*sqrt(2/(gamma+1));
    this->alpha = sqrt((1+delta)/((gamma+1)*rad_th*yt));
    this->eps = -(0.5*yt/(3+delta))*sqrt((gamma+1)*(1+delta)/(rad_th/yt));
    //this->xbar=x+eps;
}
//Throat_Solver::~Throat_Solver()
//{
//    //dtor
//}

//std::list<Point> Throat_Solver::Compute_IDL(Gas_Model* GM, Throat* TH,int npts)
//std::list<Point> Throat_Solver::Compute_IDL(int npts)
//std::array<Point> Throat_Solver::Compute_IDL(int npts)
std::vector<Point> Throat_Solver::Compute_IDL(int npts)
{/// This function computes the initial data line based on only the number of points and the gas and throat models

//    double a0 = this->GM->get_a0();
//    double rad_th = this->TH->get_upstream_rth();

    ///extract variables
    double gamma = this->GM->get_gamma();
    double yt = this->TH->get_height();
    int delta = this->TH->get_delta();
    double astar = this->get_astar();
    double alpha = this->get_alpha();
    double eps = this->get_eps();

    ///instantiate array
    std::vector<Point> idl(npts);// length will be [npts]

    ///determine spacing
    double h = (yt-0)/(npts-1); //(b - a)/(n-1)
    double h2 = (0.5*3.14159265)/(npts-1); //linspace(0,pi/2,npts)
//    double h = (0 - yt)/(npts-1); //(b - a)/(n-1)
//    double h2 = (0.5*3.14159265)/(npts-1); //linspace(0,pi/2,npts)

    ///Create spacing vectors
    std::vector<double> y_v(npts);
    std::vector<double> y_lin(npts);
    std::vector<double> y_t2(npts);
    std::vector<double> y_smooth(npts);

    /// start lines at centerline
    y_lin[0] = 0;
    y_smooth[0] = 0;
    y_t2[0] = 0;
//    y_lin[0] = 1;
//    y_smooth[0] = 1;
//    y_t2[0] =0;

    for(int i = 0;i<npts-1;i++)
    {
//        y_lin[i+1] = y_lin[i]+h;
//        y_t2[i+1] = y_t2[i]+h2;
        y_lin[i+1] = y_lin[i]+h;
        y_t2[i+1] = y_t2[i]+h2;
    }

    /// Curve smoothing
    if(TH->get_ysmooth())
    {
        for(int i = 0; i < npts; i++)
        {
            y_smooth[i] = 0.25*y_lin[i] + yt*0.75*sin(y_t2[i]);
        }
        //y_v = y_smooth;
        std::copy(y_smooth.begin(), y_smooth.end(), y_v.begin());
    }else{
        //y_v = y_lin;
        std::copy(y_lin.begin(), y_lin.end(), y_v.begin());
    }

    double y = y_v[0]; // start at centerline
    double x;
    double u;
    double v = 0; // constant

    ///calculate idl pts
    for(int i=1;i<=npts;i++)
    {
        x = (0.5*alpha*(gamma+1)*pow(yt,2)/(3+delta))*(1-pow((y/yt),2)); // compute x

        /// ITS DIFERENT IN MATLAB CUZ eps IS CALCULATED DIFFERENTLY
        //u = astar*(1+alpha*((x-eps)+(0.5*alpha*(gamma+1)*pow(y,2))/(1+delta))); // compute u
        u = astar*(1+alpha*((x+eps)+(0.5*alpha*(gamma+1)*pow(y,2))/(1+delta))); // compute u
        //idl.emplace_front(x,y,u,v); // create point on list, v is constant

//        idl[i-1] = Point(x,y,u,v);
        idl[npts-i] = Point(x,y,u,v);
        y = y_v[i]; // update y value, depends on smoothing

    }

    return idl;
} /// end of COMPUTE_IDL


////std::list<Point> Compute_THROAT(Gas_Model* GM,Throat* TH,std::list<Point>* idl)
//std::list<std::list<Point>> Throat_Solver::Compute_THROAT(std::list<Point>* idl)
////std::list<std::list<Point>> Throat_Solver::Compute_THROAT(std::array<Point>* idl)
//{///this function computes all throat points,
// ///returns a list of lists of Points, including idl pts
//    int n = idl->size();
//
//
//    std::list<std::list<Point>> throat_pts;
//
//    for(std::list<Point>::iterator it = idl->begin(); it != idl->end(); ++it) /// change to iterator of idl
//    {
////        double x = idl(in).x
//        double x = it->get_x();
//        for(std::list<Point>::iterator il = it; il != idl->end(); ++il)//while(il<n)
//        {
////            y = idl(il).y
//            double y = it->get_y();
//            Point pt = this->Transonic_Velocity(x,y);
//        }
//
//    }
//
//    return throat_pts;
////    return void;
//} /// end of COMPUTE THROAT

// private func
Point Throat_Solver::Transonic_Velocity(double x, double y)
{ /// This func solves the u,v velocities at x,y using the sauer transonic method
    //return Point(x,y);/// debug version!!!

    ///extract variables

    double gamma = this->GM->get_gamma();
    int delta = this->TH->get_delta();

    double astar = this->get_astar();
    double alpha = this->get_alpha();
    double eps = this->get_eps();

//    std::cout << "astar: " <<astar << std::endl;
//    std::cout << "alpha: " <<alpha << std::endl;
//    std::cout << "eps: " <<eps<< std::endl;
    double xbar = x+eps;
//    std::cout << "xbar: " << xbar << std::endl;
    /// THIS ONE IS NOT
    ///u=astar*(1+alpha*xbar+(gamma+1)*(alpha*y).^2/(2*(1+delta)));
    double tmp1 = alpha*xbar;
    double tmp2 = pow((alpha*y),2);
    double tmp3 = 2*(1+delta);
    double u = astar*(1+tmp1+(gamma+1)*tmp2/tmp3);
    //double u = astar*(1+(alpha*xbar)+(gamma+1)*pow((alpha*y),2))/(2*(1+delta));

  //v=astar*((gamma+1)*alpha^2*xbar.*y/(1+delta)...
   //+(gamma+1)^2*alpha^3*y.^3/(2*(1+delta)*(3+delta)));
   ///THIS ONE IS GOOD I THINK
    double v = astar*((gamma+1)*pow(alpha,2)*xbar*y/(1+delta))
              +astar*pow((gamma+1),2)*pow(alpha,3)*pow(y,3)/(2*(1+delta)*(3+delta));

   return Point(x,y,u,v);
}

double Throat_Solver::get_astar(){return this->astar;}
double Throat_Solver::get_alpha(){return this->alpha;}
double Throat_Solver::get_eps(){return this->eps;}


