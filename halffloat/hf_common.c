/**
 * @file hf_common.c
 * @brief Implémentation des fonctions utilitaires pour Half-Float IEEE 754
 * 
 * Ce fichier contient les fonctions de base pour la manipulation des nombres
 * flottants de demi-précision: conversion, décomposition, composition,
 * détection des cas spéciaux et fonctions utilitaires pour l'arithmétique.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#include "hf_common.h"
#include <stddef.h>

//Prototypes des fonctions

//Conversion entre float et demi-flottant
uint16_t float_to_half(float f);
float half_to_float(uint16_t hf);

//Statut du demi-flottant
bool_t is_infinity(half_float hf);
bool_t is_nan(half_float hf);
bool_t is_zero(half_float hf);

//Décomposition et composition de demi-flottants
half_float decompose_half(uint16_t hf);
uint16_t compose_half(half_float hf);

//Fonctions internes
void align_mantissas(half_float *hf1, half_float *hf2);
void normalize_and_round(half_float *result);
void normalize_denormalized_mantissa(half_float *hf);

/**
 * @brief Convertit un float en demi-flottant (16 bits)
 *
 * @param f Le float à convertir
 * @return La valeur uint16_t correspondante au demi-flottant
 */
uint16_t float_to_half(float f) {
    union { float f; uint32_t u; } conv = {f};
    uint32_t bits = conv.u;
    uint32_t sign = (bits >> 31) & 0x1;
    int32_t exp = ((bits >> 23) & 0xFF) - 127; //exposant débiaisé float32
    uint32_t mant = bits & 0x7FFFFF; //mantisse float32 (23 bits)
    uint16_t result = (uint16_t)(sign << HF_SIGN_BITS);
    
    if(exp > HF_EXP_BIAS) {
        //Overflow -> inf ou NaN
        result |= mant != 0 ? HF_NAN : HF_INFINITY_POS;
    }
    else if(exp > HF_EXP_MIN) {
        //Valeur normalisée
        unsigned int exp_biased = exp + HF_EXP_BIAS;
        mant = mant + 0x1000; //Arrondi (bit à la position 12 du float)

        //Gestion débordement mantisse
        if(mant & 0x800000) {
            mant = 0;
            exp_biased++;
        }
        
        //Vérifier overflow après arrondi
        result |= (exp_biased > HF_MASK_EXP) ? HF_INFINITY_POS : (exp_biased << HF_MANT_BITS) | (mant >> 13);
    }
    else if(exp >= (HF_EXP_MIN - HF_MANT_BITS)) {
        //Valeur subnormale (exp entre -25 et -15)
        uint32_t shift = (uint32_t)(HF_EXP_SUBNORMAL - exp);
        mant = (mant | 0x800000) >> shift; //Ajouter bit implicite et décaler
        mant = (mant + 0x1000) >> 13; //Arrondi puis extraction des 10 bits
        result |= mant;
    }
    //else: Trop petit -> 0 (déjà initialisé avec juste le signe)
    
    return result;
}

/**
 * @brief Convertit un demi-flottant (16 bits) en float
 *
 * @param half La valeur uint16_t du demi-flottant à convertir
 * @return Le float correspondant
 */
float half_to_float(uint16_t hf) {
    union { float f; uint32_t u; } conv;
    uint32_t sign = (hf & HF_MASK_SIGN) << 16;
    uint32_t exp = (hf >> HF_MANT_BITS) & HF_MASK_EXP;
    uint32_t mant = hf & HF_MASK_MANT;
    uint32_t f_bits = sign;
    
    if(exp == HF_MASK_EXP) {
        //Infini ou NaN
        f_bits |= 0x7F800000U;

        //NaN: propager la mantisse avec bit de signalement
        if(mant != 0) f_bits |= (mant << 13) | 0x400000U;
    } else if(exp == 0) {
        if(mant != 0) {
            //Subnormal: normaliser pour float32
            int32_t shift = 0;
            while(!(mant & (1U << HF_MANT_BITS))) {
                mant <<= 1;
                shift++;
            }
            mant &= HF_MASK_MANT;
            exp = 113 - shift;
            f_bits |= (exp << 23) | (mant << 13);
        }
        //else: Zéro (déjà initialisé avec juste le signe)
    } else {
        //Normal: convertir l'exposant (half: bias=15, float: bias=127)
        //exp_float = exp_half + (127 - 15) = exp_half + 112
        f_bits |= ((exp + 112) << 23) | (mant << 13);
    }
    
    conv.u = f_bits;
    return conv.f;
}

/**
 * @brief Vérifie si un demi-flottant représente l'infini
 * 
 * @param hf Le demi-flottant à vérifier
 * @return true si le demi-flottant est infini, false sinon
 */
bool_t is_infinity(half_float hf) {
    return (hf.exp == HF_EXP_FULL) && (hf.mant == 0);
}

/**
 * @brief Vérifie si un demi-flottant représente NaN (Not a Number)
 * 
 * @param hf Le demi-flottant à vérifier
 * @return true si le demi-flottant est NaN, false sinon
 */
bool_t is_nan(half_float hf) {
    return (hf.exp == HF_EXP_FULL) && (hf.mant != 0);
}

/**
 * @brief Vérifie si un demi-flottant représente zéro
 * 
 * @param hf Le demi-flottant à vérifier
 * @return true si le demi-flottant est zéro, false sinon
 */
bool_t is_zero(half_float hf) {
    return (hf.exp != HF_EXP_FULL) && (hf.mant == 0);
}

/**
 * @brief Décompose un uint16_t en demi-flottant
 *
 * @param hf La valeur uint16_t à décomposer
 * @return La structure half_float correspondante
 */
half_float decompose_half(uint16_t hf) {
    half_float result;
    int exp = (hf >> HF_MANT_BITS) & HF_MASK_EXP;
   
    result.sign = hf & HF_MASK_SIGN;
    result.mant = (hf & HF_MASK_MANT) << HF_PRECISION_SHIFT;
   
    if(exp == 0) {
        //Subnormal: exposant = HF_EXP_MIN, mantisse sans bit implicite
        result.exp = HF_EXP_MIN;
    }
    else if(exp == HF_MASK_EXP) {
        //Infini ou NaN
        result.exp = HF_EXP_FULL;
    }
    else {
        //Nombre normalisé: débiaiser l'exposant et ajouter bit implicite
        result.exp = exp - HF_EXP_BIAS;
        result.mant |= HF_MANT_NORM_MIN;
    }
   
    return result;
}

/**
 * @brief Compose un demi-flottant en uint16_t
 *
 * @param hf La structure half_float à composer
 * @return La valeur uint16_t correspondante
 */
uint16_t compose_half(half_float hf) {
    uint16_t result = hf.sign;  //Déjà 0x0000 ou 0x8000
   
    //Gestion des cas spéciaux
    if(hf.exp == HF_EXP_FULL) {
        //Infini ou NaN
        result |= (hf.mant != 0 ? HF_NAN : HF_INFINITY_POS);
    }
    else if(hf.exp == HF_EXP_MIN) {
        //Subnormal: décalage +1 pour les subnormaux
        uint16_t mant_bits = (hf.mant >> (HF_PRECISION_SHIFT + 1)) & HF_MASK_MANT;
        result |= mant_bits;
    }
    else if(hf.mant != 0) {
        //Nombre normalisé: retirer le bit implicite
        uint16_t exp_bits = (hf.exp + HF_EXP_BIAS) & HF_MASK_EXP;
        uint16_t mant_bits = (hf.mant >> HF_PRECISION_SHIFT) & HF_MASK_MANT;
        result |= (exp_bits << HF_MANT_BITS) | mant_bits;
    }
    //Cas Zéro par défaut: signe préservé
   
    return result;
}

/**
 * @brief Aligne les mantisses de deux demi-flottants pour l'addition/soustraction
 *
 * Cette fonction aligne deux demi-flottants en ajustant leurs mantisses
 * pour qu'ils aient le même exposant. Le nombre avec le plus petit exposant
 * voit sa mantisse décalée à droite pour compenser la différence.
 *
 * @param hf1 Pointeur vers le premier demi-flottant (modifié, non NULL)
 * @param hf2 Pointeur vers le second demi-flottant (modifié, non NULL)
 */
void align_mantissas(half_float *hf1, half_float *hf2) {
    half_float *smaller = NULL;
    int exp_target = 0;
    
    if(hf1->exp > hf2->exp) {
        smaller = hf2;
        exp_target = hf1->exp;
    } else if(hf2->exp > hf1->exp) {
        smaller = hf1;
        exp_target = hf2->exp;
    }
    
    if(smaller != NULL) {
        int shift = exp_target - smaller->exp;
        if(shift > 31) shift = 31;
        
        if(shift > 0 && shift < 31) {
            uint32_t lost = (uint32_t)smaller->mant & ((1U << shift) - 1U);
            smaller->mant >>= shift; //Décaler la mantisse
            if(lost) smaller->mant |= 1; //Sticky bit pour préserver l'info perdue
        } else if(shift >= 31) {
            smaller->mant = (smaller->mant != 0) ? 1 : 0;
        }
        smaller->exp = exp_target; //Aligner les exposants
    }
}

/**
 * @brief Normalise et arrondit le résultat après addition
 * 
 * Cette fonction normalise la mantisse d'un demi-flottant pour s'assurer
 * qu'elle est dans la plage correcte et ajuste l'exposant en conséquence.
 * Elle effectue également un arrondi selon la méthode round-to-nearest, ties to even.
 *
 * @param result Pointeur vers le demi-flottant à normaliser et arrondir
 */
void normalize_and_round(half_float *result) {
    //NORMALISATION
    if(result->mant != 0) {
        //Positionner rapidement le bit le plus significatif 
        int shift = 24, margin;
        uint32_t temp = result->mant;
       
        //Trouver rapidement la position du MSB (dichotomie + affinage)
        if(temp > 0x00FFFFFFU) shift = 0;
        else if(temp > 0x0000FFFFU) shift = 8;
        else if(temp > 0x000000FFU) shift = 16;
        temp <<= shift;
        while((int32_t)temp > 0) {temp <<= 1; shift++;}
       
        //Calculer décalage pour placer MSB au bit 15
        shift -= HF_MANT_BITS + HF_PRECISION_SHIFT + 1; //16 = 10 (mantisse) + 5 (précision)

        //Limiter le décalage pour ne pas passer sous HF_EXP_MIN
        margin = result->exp - HF_EXP_MIN;
        if(shift > margin) shift = margin;
       
        //Application de la normalisation
        if(shift > 0) result->mant <<= shift;
        else if(shift < 0) result->mant >>= -shift;
        result->exp -= shift;
    }

    //ARRONDI ROUND-TO-NEAREST-EVEN
    if(result->mant & HF_GUARD_BIT) {
        uint32_t round_bits = result->mant & HF_ROUND_BIT_MASK;
        if(round_bits > HF_GUARD_BIT || (round_bits == HF_GUARD_BIT && (result->mant & (1U << HF_PRECISION_SHIFT)))) {
            result->mant += (1U << HF_PRECISION_SHIFT);
            if(result->mant >= HF_MANT_NORM_MAX) {
                result->mant >>= 1;
                result->exp++;
            }
        }
    }

    //GESTION DES CAS LIMITES
    if(result->exp > HF_EXP_BIAS) {
        //Overflow -> Infini
        result->exp = HF_EXP_FULL;
        result->mant = 0;
    }
    else if(result->exp < HF_EXP_MIN) {
        //Underflow: créer subnormal ou zéro
        int shift = HF_EXP_MIN - result->exp;
        result->mant = (shift < HF_MANT_BITS + HF_PRECISION_SHIFT + 1) ? (result->mant + (1U << (shift - 1))) >> shift : 0;
        result->exp = HF_EXP_MIN;
    }
    //Sinon exp == HF_EXP_MIN: subnormal déjà bien positionné, rien à faire

    //NETTOYAGE
    result->mant &= ~HF_ROUND_BIT_MASK;
}

/**
 * @brief Normalise une mantisse dénormalisée
 * 
 * Cette fonction normalise une mantisse dénormalisée en décalant la mantisse
 * vers la gauche et en décrémentant l'exposant jusqu'à ce que la mantisse
 * soit normalisée (bit implicite défini).
 *
 * @param hf Pointeur vers la structure half_float à normaliser
 */
void normalize_denormalized_mantissa(half_float *hf) {
    if(hf->exp == HF_EXP_MIN) {
        hf->exp++;
        while(hf->mant < HF_MANT_NORM_MIN) {
            hf->mant <<= 1;
            hf->exp--;
        }
    }
}
