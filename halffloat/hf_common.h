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

typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned int bool_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;

//Définitions du format fp16
#define HF_SIGN_BITS 15                                         //Position du bit de signe
#define HF_EXP_BITS 5                                           //Nombre de bits pour l'exposant
#define HF_MANT_BITS 10                                         //Nombre de bits pour la mantisse
#define HF_EXP_BIAS 15                                          //Biais pour l'exposant

//Constantes remarquables du format fp16
#define HF_MASK_SIGN        (1U << HF_SIGN_BITS)                //Masque pour extraire le bit de signe
#define HF_MASK_MANT        ((1U << HF_MANT_BITS) - 1)          //Masque pour extraire les bits de la mantisse
#define HF_MASK_EXP         ((1U << HF_EXP_BITS) - 1)           //Masque pour extraire les bits de l'exposant
#define HF_INFINITY_POS     (((1U << HF_EXP_BITS) - 1) << HF_MANT_BITS) //Valeur demi-flottante pour +Infini
#define HF_INFINITY_NEG     (HF_INFINITY_POS | HF_MASK_SIGN)    //Valeur demi-flottante pour -Infini
#define HF_NAN              (HF_INFINITY_POS | (1U << (HF_MANT_BITS - 1))) //Valeur demi-flottante pour NaN
#define HF_ZERO_POS         0                                   //Valeur demi-flottante pour +0
#define HF_ZERO_NEG         (HF_MASK_SIGN)                      //Valeur demi-flottante pour -0
#define HF_ONE_POS          (HF_EXP_BIAS << HF_MANT_BITS)       //Valeur demi-flottante pour +1.0
#define HF_ONE_NEG          (HF_ONE_POS | HF_MASK_SIGN)         //Valeur demi-flottante pour -1.0

//Quelques définitions pour la gestion interne
#define HF_PRECISION_SHIFT  5                                   //Décalage pour la précision
#define HF_EXP_FULL         (HF_EXP_BIAS + 1)                   //Pour indiquer si NaN ou Infini
#define HF_EXP_MIN          (-HF_EXP_BIAS)                      //Exposant minimal absolu
#define HF_EXP_SUBNORMAL    (-HF_EXP_BIAS + 1)                  //Exposant pour les subnormaux
#define HF_MANT_NORM_MIN    (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT))
#define HF_MANT_NORM_MAX    (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT + 1))
#define HF_GUARD_BIT        (1 << (HF_PRECISION_SHIFT - 1))     //bit du milieu pour l'arrondi
#define HF_ROUND_BIT_MASK   ((1 << HF_PRECISION_SHIFT) - 1)     //masque pour round+sticky

//Structure pour stocker les composants d'un demi-flottant
typedef struct {
    uint16_t sign;  //Bit de signe
    int exp;        //Exposant
    int32_t mant;   //Mantisse (avec un bit implicite)
} half_float;

//Prototypes des fonctions
uint16_t float_to_half(float f);
float half_to_float(uint16_t hf);

bool_t is_infinity(half_float hf);
bool_t is_nan(half_float hf);
bool_t is_zero(half_float hf);

half_float decompose_half(uint16_t hf);
uint16_t compose_half(half_float hf);

void align_mantissas(half_float *hf1, half_float *hf2);
void normalize_and_round(half_float *result);
void normalize_denormalized_mantissa(half_float *hf);

#endif //HF_COMMON_H
