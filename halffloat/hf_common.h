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

#include <math.h>

typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned int bool_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;

//Modes d'arrondi IEEE 754
typedef enum {
    HF_ROUND_NEAREST_EVEN = 0,    //Round to nearest, ties to even (par défaut)
    HF_ROUND_NEAREST_UP = 1,      //Round to nearest, ties away from zero
    HF_ROUND_TOWARD_ZERO = 2,     //Round toward zero (troncature)
    HF_ROUND_TOWARD_POS_INF = 3,  //Round toward +inf (plafond)
    HF_ROUND_TOWARD_NEG_INF = 4   //Round toward -inf (plancher)
} hf_rounding_mode;

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
#define HF_MANT_SHIFT       (HF_MANT_BITS + HF_PRECISION_SHIFT) //Décalage total mantisse (15)
#define HF_EXP_FULL         (HF_EXP_BIAS + 1)                   //Pour indiquer si NaN ou Infini
#define HF_EXP_MIN          (-HF_EXP_BIAS)                      //Exposant minimal absolu
#define HF_EXP_SUBNORMAL    (-HF_EXP_BIAS + 1)                  //Exposant pour les subnormaux
#define HF_MANT_NORM_MIN    (1 << HF_MANT_SHIFT)
#define HF_MANT_NORM_MAX    (1 << (HF_MANT_SHIFT + 1))
#define HF_GUARD_BIT        (1 << (HF_PRECISION_SHIFT - 1))     //bit du milieu pour l'arrondi
#define HF_ROUND_BIT_MASK   ((1 << HF_PRECISION_SHIFT) - 1)     //masque pour round+sticky

//Constantes pour les calculs en virgule fixe
#define Q15_SHIFT 15                                            //Décalage pour format Q15 (virgule fixe 15 bits fractionnaires)
#define Q15_ONE (1 << Q15_SHIFT)                                //Valeur 1.0 en format Q15 (32768)
#define PI_Q15 (int)(M_PI * 32768.0 + 0.5)                      //Valeur de pi en Q15
#define PI_1_2_Q15 (int)(M_PI / 2.0 * 32768.0 + 0.5)            //Valeur de pi/2 en Q15
#define PI_1_4_Q15 (int)(M_PI / 4.0 * 32768.0 + 0.5)            //Valeur de pi/4 en Q15
#define PI_3_4_Q15 (int)(3.0 * M_PI / 4.0 * 32768.0 + 0.5)      //Valeur de 3pi/4 en Q15

//Structure pour stocker les composants d'un demi-flottant
typedef struct {
    uint16_t sign;  //Bit de signe
    int exp;        //Exposant
    int32_t mant;   //Mantisse (avec un bit implicite)
} half_float;

//Conversion entre float et demi-flottant
uint16_t float_to_half(float f);
float half_to_float(uint16_t hf);

//Statut du demi-flottant
bool_t is_infinity(const half_float *hf);
bool_t is_nan(const half_float *hf);
bool_t is_zero(const half_float *hf);

//Décomposition et composition de demi-flottants
half_float decompose_half(uint16_t hf);
uint16_t compose_half(const half_float *hf);

//Fonctions pour gérer les mantisses et exposants
void align_mantissas(half_float *hf1, half_float *hf2);
void normalize_and_round(half_float *result);
void normalize_denormalized_mantissa(half_float *hf);

//Gestion du mode d'arrondi global
void hf_set_rounding_mode(hf_rounding_mode mode);
hf_rounding_mode hf_get_rounding_mode(void);

#endif //HF_COMMON_H
