/*!
 * \file flux.cpp
 * \brief Functions for calculation of fluxes
 *
 * Includes functions for Euler and Navier-Stokes fluxes, as well as common flux
 * / Riemann solve routines
 *
 * \author - Jacob Crabill
 *           Aerospace Computing Laboratory (ACL)
 *           Aero/Astro Department. Stanford University
 *
 * \version 0.0.1
 *
 * Flux Reconstruction in C++ (Flurry++) Code
 * Copyright (C) 2014 Jacob Crabill.
 *
 */

#include "../include/flux.hpp"

void inviscidFlux(vector<double> &U, matrix<double> &F, input *params)
{
  double rho, u, v, p;
  rho = U[0];
  u = U[1]/rho;
  v = U[2]/rho;
  p = (params->gamma-1.0)*(U[3]-(0.5*rho*((u*u)+(v*v))));

  /* --- Assuming F has already been sized properly... --- */
  F[0][0] =  U[1];       F[1][0] =  U[2];
  F[0][1] =  U[1]*u;     F[1][1] =  U[1]*v;
  F[0][2] =  U[2]*u;     F[1][2] =  U[2]*v;
  F[0][3] = (U[3]+p)*u;  F[1][3] = (U[3]+p)*v;
}


void viscousFlux(vector<double> &U, matrix<double> &gradU, matrix<double> &Fvis, input *params)
{
  double rho, u, v, E, e;

  double dRho_dx, dRhoU_dx, dRhoV_dx, dE_dx;
  double dRho_dy, dRhoU_dy, dRhoV_dy, dE_dy;

  double du_dx, du_dy, dv_dx, dv_dy, dK_dx, dK_dy, de_dx, de_dy;
  double diag, tauxx, tauxy, tauyy;
  double rt_ratio;

  double mu, mu_t, nu_tilde;
  double p,T,R;
  double inv_Re_c, Mach_c;
  double T_gas_non, S_gas_non;

  /* --- Calculate Primitives --- */
  rho = U[0];
  u   = U[1]/rho;
  v   = U[2]/rho;
  e   = U[3]/rho - 0.5*(u*u+v*v);

  /* --- Get Gradients --- */
  dRho_dx	 = gradU[0][0];
  dRhoU_dx = gradU[0][1];
  dRhoV_dx = gradU[0][2];
  dE_dx	   = gradU[0][3];

  dRho_dy	 = gradU[1][0];
  dRhoU_dy = gradU[1][1];
  dRhoV_dy = gradU[1][2];
  dE_dy	   = gradU[1][3];

  /* --- Calculate Viscosity --- */
  rt_ratio = (run_input.gamma-1.0)*inte/(run_input.rt_inf);
  mu = (run_input.mu_inf)*pow(rt_ratio,1.5)*(1.+(run_input.c_sth))/(rt_ratio+(run_input.c_sth));
  mu = mu + run_input.fix_vis*(run_input.mu_inf - mu);

  /* --- Calculate Gradients --- */
  du_dx = (dRhoU_dx-dRho_dx*u)/rho;
  du_dy = (dRhoU_dy-dRho_dy*u)/rho;

  dv_dx = (dRhoV_dx-dRho_dx*v)/rho;
  dv_dy = (dRhoV_dy-dRho_dy*v)/rho;

  dK_dx = 0.5*(u*u+v*v)*dRho_dx+rho*(u*du_dx+v*dv_dx);
  dK_dy = 0.5*(u*u+v*v)*dRho_dy+rho*(u*du_dy+v*dv_dy);

  de_dx = (dE_dx-dK_dx-dRho_dx*e)/rho;
  de_dy = (dE_dy-dK_dy-dRho_dy*e)/rho;

  diag = (du_dx + dv_dy)/3.0;

  tauxx = 2.0*(mu+mu_t)*(du_dx-diag);
  tauxy = (mu+mu_t)*(du_dy + dv_dx);
  tauyy = 2.0*(mu+mu_t)*(dv_dy-diag);

  /* --- Calculate Viscous Flux --- */
  Fvis[0][0] =  0.0;
  Fvis[0][1] = -tauxx;
  Fvis[0][2] = -tauxy;
  Fvis[0][3] = -(u*tauxx+v*tauxy+(mu/params->prandtl)*(params->gamma)*de_dx);

  Fvis[1][0] =  0.0;
  Fvis[1][1] = -tauxy;
  Fvis[1][2] = -tauyy;
  Fvis[1][3] = -(u*tauxy+v*tauyy+(mu/params->prandtl)*(params->gamma)*de_dy);
}


void rusanovFlux(vector<double> &UL, vector<double> &UR, matrix<double> &FL, matrix<double> &FR, array<double,3> &norm, vector<double> &Fn, input *params)
{
  int i, j;
  double rhoL, uL, vL, wL, pL, vnL=0.0;
  double rhoR, uR, vR, wR, pR, vnR=0.0;
  double csqL, csqR, eigL, eigR, eig;

  vector<double> FnL(params->nFields,0.0);
  vector<double> FnR(params->nFields,0.0);

  // Get primitive variables
  rhoL = UL[0];     rhoR = UR[0];
  uL = UL[1]/rhoL;  uR = UR[1]/rhoR;
  vL = UL[2]/rhoL;  vR = UR[2]/rhoR;

  // Calculate pressure
  if (params->nDims==2) {
    pL = (params->gamma-1.0)*(UL[3]-rhoL*(uL*uL+vL*vL));
    pR = (params->gamma-1.0)*(UR[3]-rhoR*(uR*uR+vR*vR));
  }else if (params->nDims==3) {
    WL = UL[3]/U[0];   WR = UR[3]/UR[0];
    pL = (params->gamma-1.0)*(UL[3]-rhoL*(uL*uL+vL*vL+wL*wL));
    pR = (params->gamma-1.0)*(UR[3]-rhoR*(uR*uR+vR*vR+wR*wR));
  }

  // Get normal fluxes, normal velocities
  for (j=0; j<params->nDims; j++) {
    vnL += norm[j]*UL[j+1]/rhoL;
    vnR += norm[j]*UR[j+1]/rhoR;
    for (i=0; i<params->nFields; i++) {
      FnL[i] += norm[j]*FL[j][i];
      FnR[i] += norm[j]*FR[j][i];
    }
  }

  // Get maximum eigenvalue for diffusion coefficient
  csqL = max(params->gamma*pL/rhoL,0.0);
  csqR = max(params->gamma*pR/rhoR,0.0);
  eigL = fabs(vnL) + sqrt(csqL);
  eigR = fabs(vnR) + sqrt(csqR);
  eig = max(eigL,eigR);

  // Calculate Rusanov flux
  for (i=0; i<params->nFields; i++) {
    Fn[i] = 0.5*(FnL[i]+FnR[i] - eig*(UR[i]-UL[i]));
  }
}


void ldgFlux(vector<double> &uL, vector<double> &uR, matrix<double> &gradU_L, matrix<double> &gradU_R, vector<double> &Fn, input *params)
{
  FatalError("LDG flux not implemented just yet.  Go to flux.cpp and do it!!");
}


void roeFlux(vector<double> &uL, vector<double> &uR, array<double,3> &norm, vector<double> &Fn, input *params)
{
  FatalError("Roe flux not implemented just yet.  Go to flux.cpp and do it!!");
}
