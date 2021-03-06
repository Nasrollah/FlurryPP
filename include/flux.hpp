/*!
 * \file flux.hpp
 * \brief Header file flux-calculation functions
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
#include "input.hpp"
#include "matrix.hpp"

/*! Calculate the inviscid portion of the Euler or Navier-Stokes flux vector at a point */
void inviscidFlux(vector<double> &U, matrix<double> &F, input *params);

/*! Calculate the viscous portion of the Navier-Stokes flux vector at a point */
void viscousFlux(vector<double> &U, matrix<double> &gradU, matrix<double> &Fvis, input *params);

/*! Calculate the common inviscid flux at a point using Roe's method */
void roeFlux(vector<double> &uL, vector<double> &uR, array<double,3> &norm, vector<double> &Fn, input *params);

/*! Calculate the common inviscid flux at a point using the Rusanov scalar-diffusion method */
void rusanovFlux(vector<double> &UL, vector<double> &UR, matrix<double> &FL, matrix<double> &FR, array<double,3> &norm, vector<double> &Fn, input *params);

/*! Calculate the common viscous flux at a point using the LDG penalty method */
void ldgFlux(vector<double> &uL, vector<double> &uL, matrix<double> &gradU_L, matrix<double> &gradU_R, vector<double> &Fn, input *params);
