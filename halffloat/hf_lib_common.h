/**
 * @file hf_lib_common.h
 * @brief Utilitaires communs pour la bibliothèque Half-Float
 * 
 * Module contenant les fonctions utilitaires partagées entre
 * tous les modules de la bibliothèque Half-Float.
 *
 * @author Seg  
 * @date Novembre 2025
 * @version 1.0
 */

#ifndef HF_LIB_COMMON_H
#define HF_LIB_COMMON_H

#include "hf_common.h"
#include "hf_precalc.h"

/* Utilitaires internes partagés */
uint16_t reduce_radian_uword(uint32_t angle_rad_fixed, int fact);
int compare_half(const half_float *input1, const half_float *input2);
int check_int_half(const half_float *hf);

//Prototypes des fonctions utilitaires spécialisées
uint16_t table_interpolate(const uint16_t *table, int size, uint32_t index, int frac_bits);
void exp_fixed(int32_t x_fixed, half_float *result);

#endif //HF_LIB_COMMON_H
