/**
 * @file hf_common.h
 * @brief Définitions communes pour la bibliothèque Half-Float IEEE 754
 * 
 * Cette bibliothèque implémente les opérations arithmétiques et mathématiques
 * pour les nombres flottants de demi-précision (16 bits) selon la norme IEEE 754.
 * 
 * Format Half-Float (IEEE 754 binary16) :
 * - 1 bit de signe
 * - 5 bits d'exposant (biais = 15) 
 * - 10 bits de mantisse (+ 1 bit implicite)
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#ifndef HF_COMMON_H
#define HF_COMMON_H

#define HF_MASK_SIGN 0x8000    //Masque pour extraire le bit de signe
#define HF_MASK_MANT 0x3FF     //Masque pour extraire les bits de la mantisse
#define HF_MASK_EXP 0x1F       //Masque pour extraire les bits de l'exposant
#define HF_INFINITY_POS 0x7C00 //Valeur demi-flottante pour +Infini
#define HF_INFINITY_NEG 0xFC00 //Valeur demi-flottante pour -Infini
#define HF_NAN 0x7E00          //Valeur demi-flottante pour NaN
#define HF_ZERO_POS 0x0000     //Valeur demi-flottante pour +0
#define HF_ZERO_NEG 0x8000     //Valeur demi-flottante pour -0
#define HF_EXP_BIAS 15         //Biais pour l'exposant
#define HF_MANT_BITS 10        //Nombre de bits pour la mantisse
#define HF_EXP_BITS 5          //Nombre de bits pour l'exposant
#define HF_PRECISION_SHIFT 5   //Décalage pour la précision
#define HF_SIGN_BITS 15        //Nombre de bits pour le signe
#define HF_EXP_FULL (HF_EXP_BIAS + 1) //Pour indiquer si NaN ou Infini

typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned int bool_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;

//Structure pour stocker les composants d'un demi-flottant
typedef struct {
    uint16_t sign;  //Bit de signe
    int exp;        //Exposant
    int32_t mant;   //Mantisse (avec un bit implicite)
} half_float;

//Prototypes des fonctions
uint16_t float_to_half(float f);
float half_to_float(uint16_t half);

bool_t is_infinity(half_float hf);
bool_t is_nan(half_float hf);
bool_t is_zero(half_float hf);

half_float decompose_half(uint16_t half);
uint16_t compose_half(half_float hf);

void align_mantissas(half_float *a, half_float *b);
void normalize_and_round(half_float *result);

#endif //HF_COMMON_H
