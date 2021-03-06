/*!
 * \file operators.hpp
 * \brief Header file for oper class
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
#pragma once

#include "global.hpp"
#include "mesh.hpp"
#include "polynomials.hpp"

class oper
{
public:
  //! Overall setup function for one element type & polynomial order
  void setup_operators(int eType, int order);

  //! Setup operator for extrapolation from solution points to flux points
  void setup_extrapolate_spts_fpts(vector<point> loc_spts, vector<point> loc_fpts, int eType, int order);

  //! Setup operator for calculation of gradient at the solution points
  void setup_grad_spts(vector<point> loc_spts, int eType, int order);

  /*! Setup operator to calculate divergence of correction function at solution points
   *  based upon the normal flux correction at the flux points */
  void setup_correction(vector<point> loc_spts, vector<point> loc_fpts, int eType, int order);

  // Create a map<int,double*> (?) to get access to the correct operator
  // i.e. somthing like: div_flux_spts_tri = oper.get_oper_div[TRI]

  map<int,matrix<double>*> get_oper_div_spts;
  map<int,matrix<double>*> get_oper_grad_spts;
  map<int,matrix<double>*> get_oper_spts_fpts;
  map<int,matrix<double>*> get_oper_correct;

  matrix<double> opp_spts_to_fpts;
  matrix<double> opp_grad_spts;
  matrix<double> opp_div_spts;
  matrix<double> opp_correction;

private:
  mesh *Mesh;
  int nDims;
}
