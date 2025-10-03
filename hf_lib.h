#ifndef HF_LIB_H
#define HF_LIB_H
#include <stdint.h>
#include "hf_common.h"

// Fonctions mathématiques de base
uint16_t hf_int(uint16_t hf);
uint16_t hf_abs(uint16_t hf);
uint16_t hf_neg(uint16_t hf);

// Opérations arithmétiques
uint16_t hf_add(uint16_t float1, uint16_t float2);
uint16_t hf_mul(uint16_t hf1, uint16_t hf2);
uint16_t hf_div(uint16_t hf1, uint16_t hf2);
uint16_t hf_sqrt(uint16_t hf);

// Fonctions transcendantes
uint16_t hf_ln(uint16_t hf);
uint16_t hf_exp(uint16_t hf);
uint16_t hf_pow(uint16_t base, uint16_t exp);

// Fonctions trigonométriques
uint16_t hf_sin(uint16_t angle);
uint16_t hf_cos(uint16_t angle);
uint16_t hf_tan(uint16_t angle);

// Fonctions internes (optionnelles)
uint16_t reduce_radian_uword(uint32_t angle_rad_fixed, int fact);
uint16_t sinus_shiftable(uint16_t angle, uint16_t shift);

#endif // HF_LIB_H
// Toutes les fonctions et commentaires sont maintenant en UTF-8, corrigeant les caractères corrompus.
