/**
 * @file hf_lib_round.h
 * @brief Module d'arrondis et manipulation pour Half-Float
 * 
 * Module contenant les fonctions d'arrondis, de manipulation
 * et les opérations sur les bits pour les nombres half-float.
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#ifndef HF_LIB_ROUND_H
#define HF_LIB_ROUND_H

#include "hf_lib_common.h"

/* Fonctions d'arrondis */
uint16_t hf_ceil(uint16_t hf);
uint16_t hf_floor(uint16_t hf);
uint16_t hf_round(uint16_t hf);
uint16_t hf_trunc(uint16_t hf);
uint16_t hf_int(uint16_t hf);
uint16_t hf_nearbyint(uint16_t hf);
uint16_t hf_rint(uint16_t hf);

#endif /* HF_LIB_ROUND_H */
