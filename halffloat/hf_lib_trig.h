/**
 * @file hf_lib_trig.h
 * @brief Module trigonométrique pour la bibliothèque Half-Float
 * 
 * Module contenant toutes les fonctions trigonométriques (sin, cos, tan, asin, acos, atan, atan2)
 * et les fonctions hyperboliques (sinh, cosh, tanh, asinh, acosh, atanh).
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#ifndef HF_LIB_TRIG_H
#define HF_LIB_TRIG_H

#include "hf_common.h"

//Fonctions trigonométriques
uint16_t hf_sin(uint16_t hfangle);           //Sinus
uint16_t hf_cos(uint16_t hfangle);           //Cosinus
uint16_t hf_tan(uint16_t hfangle);           //Tangente
uint16_t hf_asin(uint16_t hf);               //Arc sinus
uint16_t hf_acos(uint16_t hf);               //Arc cosinus
uint16_t hf_atan(uint16_t hf);               //Arc tangente
uint16_t hf_atan2(uint16_t hfy, uint16_t hfx); //Arc tangente à 2 arguments

//Fonctions hyperboliques
uint16_t hf_sinh(uint16_t hf);               //Sinus hyperbolique
uint16_t hf_cosh(uint16_t hf);               //Cosinus hyperbolique
uint16_t hf_tanh(uint16_t hf);               //Tangente hyperbolique
uint16_t hf_asinh(uint16_t hf);              //Arc sinus hyperbolique
uint16_t hf_acosh(uint16_t hf);              //Arc cosinus hyperbolique
uint16_t hf_atanh(uint16_t hf);              //Arc tangente hyperbolique

#endif //HF_LIB_TRIG_H
