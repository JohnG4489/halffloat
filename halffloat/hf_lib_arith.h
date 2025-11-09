/**
 * @file hf_lib_arith.h
 * @brief Module arithmétique pour la bibliothèque Half-Float
 * 
 * Module contenant toutes les opérations arithmétiques de base,
 * les racines et les opérations avancées.
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#ifndef HF_LIB_ARITH_H
#define HF_LIB_ARITH_H

#include "hf_lib_common.h"

//Opérations unaires
uint16_t hf_neg(uint16_t hf);
uint16_t hf_abs(uint16_t hf);

//Opérations arithmétiques de base
uint16_t hf_add(uint16_t hf1, uint16_t hf2);
uint16_t hf_sub(uint16_t hf1, uint16_t hf2);
uint16_t hf_mul(uint16_t hf1, uint16_t hf2);
uint16_t hf_div(uint16_t hf1, uint16_t hf2);
uint16_t hf_inv(uint16_t hf);

//Fonctions de racines
uint16_t hf_sqrt(uint16_t hf);
uint16_t hf_rsqrt(uint16_t hf);
uint16_t hf_cbrt(uint16_t hf);

//Opérations avancées
uint16_t hf_fma(uint16_t hfa, uint16_t hfb, uint16_t hfc);  //a*b+c
uint16_t hf_hypot(uint16_t hfx, uint16_t hfy);              //sqrt(x^2+y^2)

//Opérations modulo
uint16_t hf_fmod(uint16_t hfx, uint16_t hfy);              //x mod y
uint16_t hf_remainder(uint16_t hfx, uint16_t hfy);         //IEEE remainder
uint16_t hf_remquo(uint16_t hfx, uint16_t hfy, int *quo);  //remainder + quotient

#endif //HF_LIB_ARITH_H
