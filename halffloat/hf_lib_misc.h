/**
 * @file hf_lib_misc.h
 * @brief Module utilitaire pour Half-Float
 * 
 * Module contenant les fonctions utilitaires diverses pour les nombres
 * flottants de demi-précision (comparaison, min, max, etc.).
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#ifndef HF_LIB_MISC_H
#define HF_LIB_MISC_H

#include "hf_lib_common.h"

//Fonctions de comparaison et sélection
int hf_cmp(uint16_t hf1, uint16_t hf2);
uint16_t hf_min(uint16_t hf1, uint16_t hf2);
uint16_t hf_max(uint16_t hf1, uint16_t hf2);

//Manipulation de mantisse et exposant
uint16_t hf_modf(uint16_t hf, uint16_t *intpart);
uint16_t hf_frexp(uint16_t hf, int *exp);
uint16_t hf_ldexp(uint16_t hf, int exp);
uint16_t hf_scalbn(uint16_t hf, int n);

//Fonctions d'information
uint16_t hf_logb(uint16_t hf);
int hf_ilogb(uint16_t hf);

//Opérations sur les bits
uint16_t hf_copysign(uint16_t mag, uint16_t sign);
uint16_t hf_nextafter(uint16_t from, uint16_t to);
uint16_t hf_nexttoward(uint16_t from, long double to);

#endif //HF_LIB_MISC_H
