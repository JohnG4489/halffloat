/**
 * @file hf_lib_exp.h
 * @brief Module exponentiel et logarithmique pour Half-Float
 * 
 * Module contenant les fonctions exponentielles et logarithmiques
 * pour le format Half-Float IEEE 754.
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#ifndef HF_LIB_EXP_H
#define HF_LIB_EXP_H

#include "hf_common.h"

//Fonctions exponentielles et logarithmiques
uint16_t hf_ln(uint16_t a);           //Logarithme népérien
uint16_t hf_log2(uint16_t a);         //Logarithme base 2
uint16_t hf_log10(uint16_t a);        //Logarithme base 10
uint16_t hf_exp(uint16_t a);          //Exponentielle
uint16_t hf_exp2(uint16_t a);         //Exponentielle base 2
uint16_t hf_exp10(uint16_t a);        //Exponentielle base 10
uint16_t hf_pow(uint16_t a, uint16_t b); //Puissance a^b
uint16_t hf_expm1(uint16_t a);        //exp(a) - 1
uint16_t hf_log1p(uint16_t a);        //ln(1 + a)

#endif //HF_LIB_EXP_H
