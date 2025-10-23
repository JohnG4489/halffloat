/**
 * @file hf_lib.h
 * @brief Interface des fonctions mathématiques pour Half-Float IEEE 754
 * 
 * Ce fichier définit l'interface publique de la bibliothèque pour toutes les
 * opérations arithmétiques et mathématiques sur les nombres flottants de
 * demi-précision : arithmétique de base, fonctions transcendantes et
 * trigonométriques conformes à la norme IEEE 754.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#ifndef HF_LIB_H
#define HF_LIB_H
#include "hf_common.h"

//Fonctions de comparaison
uint16_t hf_cmp(uint16_t hf1, uint16_t hf2);
uint16_t hf_min(uint16_t hf1, uint16_t hf2);
uint16_t hf_max(uint16_t hf1, uint16_t hf2);

//Fonctions mathématiques de base
uint16_t hf_int(uint16_t hf);
uint16_t hf_abs(uint16_t hf);
uint16_t hf_neg(uint16_t hf);

//Opérations arithmétiques
uint16_t hf_add(uint16_t hf1, uint16_t hf2);
uint16_t hf_sub(uint16_t hf1, uint16_t hf2);
uint16_t hf_mul(uint16_t hf1, uint16_t hf2);
uint16_t hf_div(uint16_t hf1, uint16_t hf2);
uint16_t hf_inv(uint16_t hf);
uint16_t hf_sqrt(uint16_t hf);
uint16_t hf_rsqrt(uint16_t hf);

//Fonctions transcendantes
uint16_t hf_ln(uint16_t hf);
uint16_t hf_exp(uint16_t hf);
uint16_t hf_pow(uint16_t hfbase, uint16_t hfexp);

//Fonctions trigonométriques
uint16_t hf_sin(uint16_t hfangle);
uint16_t hf_cos(uint16_t hfangle);
uint16_t hf_tan(uint16_t hfangle);
uint16_t hf_asin(uint16_t hf);
uint16_t hf_acos(uint16_t hf);

#endif //HF_LIB_H
