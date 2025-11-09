/**
 * @file hf_lib_round.c
 * @brief Implémentation du module d'arrondis pour Half-Float
 * 
 * Module contenant l'implémentation des fonctions d'arrondis et de manipulation
 * pour le format Half-Float IEEE 754.
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#include "hf_lib_round.h"

/**
 * @brief Arrondi vers l'entier superieur (plafond)
 * @param hf Le demi-flottant a arrondir vers le haut
 * @return Le plafond de hf
 */
uint16_t hf_ceil(uint16_t hf) {
    half_float result = decompose_half(hf);
    
    //Traitement uniquement pour les nombres non speciaux
    if(!is_nan(&result) && !is_infinity(&result) && !is_zero(&result)) {
        //Si l'exposant est negatif
        if(result.exp < 0) {
            //Pour les nombres positifs < 1, ceil = 1
            //Pour les nombres negatifs > -1, ceil = -0
            if(!result.sign) {
                result.mant = HF_MANT_NORM_MIN;
                result.exp = 0;
            } else {
                result.sign = HF_ZERO_NEG;
                result.mant = 0;
                result.exp = HF_EXP_MIN;
            }
        }
        //Si l'exposant est >= 0, verifier s'il y a une partie fractionnaire
        else if(result.exp < HF_MANT_BITS) {
            int frac_bits = HF_MANT_BITS - result.exp;
            uint32_t mask = (1U << (frac_bits + HF_PRECISION_SHIFT)) - 1U;
            uint32_t frac_part = result.mant & mask;
            
            if(frac_part != 0) {
                //Il y a une partie fractionnaire
                result.mant = result.mant & ~mask; //Tronquer
                
                //Si positif, ajouter 1 a la partie entiere
                if(!result.sign) {
                    result.mant += (1U << (frac_bits + HF_PRECISION_SHIFT));
                }
            }
            normalize_and_round(&result);
        }
    }

    return compose_half(&result);
}

/**
 * @brief Arrondi vers l'entier inferieur (plancher)
 * @param hf Le demi-flottant a arrondir vers le bas
 * @return Le plancher de hf
 */
uint16_t hf_floor(uint16_t hf) {
    half_float result = decompose_half(hf);
    
    //Traitement uniquement pour les nombres non speciaux
    if(!is_nan(&result) && !is_infinity(&result) && !is_zero(&result)) {
        //Si l'exposant est negatif
        if(result.exp < 0) {
            //Pour les nombres positifs < 1, floor = 0
            //Pour les nombres negatifs > -1, floor = -1
            if(!result.sign) {
                result.mant = 0;
                result.exp = HF_EXP_MIN;
            } else {
                result.mant = HF_MANT_NORM_MIN;
                result.exp = 0;
            }
        }
        //Si l'exposant est >= 0, verifier s'il y a une partie fractionnaire
        else if(result.exp < HF_MANT_BITS) {
            int frac_bits = HF_MANT_BITS - result.exp;
            uint32_t mask = (1U << (frac_bits + HF_PRECISION_SHIFT)) - 1U;
            uint32_t frac_part = result.mant & mask;
            
            if(frac_part != 0) {
                //Il y a une partie fractionnaire
                result.mant = result.mant & ~mask; //Tronquer
                
                //Si negatif, soustraire 1 de la partie entiere
                if(result.sign) {
                    result.mant += (1U << (frac_bits + HF_PRECISION_SHIFT));
                }
            }
            normalize_and_round(&result);
        }
    }

    return compose_half(&result);
}

/**
 * @brief Arrondi vers l'entier le plus proche
 * @param hf Le demi-flottant a arrondir
 * @return L'entier le plus proche de hf
 */
uint16_t hf_round(uint16_t hf) {
    half_float result = decompose_half(hf);
    
    //Traitement uniquement pour les nombres non speciaux
    if(!is_nan(&result) && !is_infinity(&result) && !is_zero(&result)) {
        //Si l'exposant est negatif
        if(result.exp < 0) {
            uint32_t half_threshold = 1U << (HF_MANT_SHIFT - 1);
            
            if((int32_t)result.mant < (int32_t)half_threshold) {
                //|x| < 0.5 -> 0
                result.mant = 0;
                result.exp = HF_EXP_MIN;
                result.sign = HF_ZERO_POS;
            } else {
                //|x| >= 0.5 -> +/-1
                result.mant = HF_MANT_NORM_MIN;
                result.exp = 0;
            }
        }
        //Si l'exposant est >= 0, verifier s'il y a une partie fractionnaire
        else if(result.exp < HF_MANT_BITS) {
            int frac_bits = HF_MANT_BITS - result.exp;
            uint32_t mask = (1U << (frac_bits + HF_PRECISION_SHIFT)) - 1U;
            uint32_t frac_part = result.mant & mask;
            
            if(frac_part != 0) {
                uint32_t half_bit = 1U << (frac_bits + HF_PRECISION_SHIFT - 1);
                uint32_t int_part = result.mant & ~mask;
                
                //Tronquer d'abord
                result.mant = int_part;
                
                //Verifier si on doit arrondir vers le haut
                if(frac_part > half_bit || 
                   (frac_part == half_bit && (int_part & (1U << (frac_bits + HF_PRECISION_SHIFT))))) {
                    //Arrondir vers le haut
                    result.mant += (1U << (frac_bits + HF_PRECISION_SHIFT));
                }
            }

            normalize_and_round(&result);
        }
    }

    return compose_half(&result);
}

/**
 * @brief Troncature d'un demi-flottant (arrondi vers zero)
 * @param hf Le demi-flottant a tronquer
 * @return Le resultat tronque vers zero
 */
uint16_t hf_trunc(uint16_t hf) {
    half_float result = decompose_half(hf);
    
    //Traitement uniquement pour les nombres non spéciaux
    if(!is_nan(&result) && !is_infinity(&result) && !is_zero(&result)) {
        //Si l'exposant est négatif, la partie entière est 0
        if(result.exp < 0) {
            result.mant = 0;
            result.exp = HF_EXP_MIN;
        }
        //Si l'exposant est >= 0, tronquer les bits fractionnaires
        else if(result.exp < HF_MANT_BITS) {
            //Nombre de bits fractionnaires à éliminer
            int frac_bits = HF_MANT_BITS - result.exp;
            
            //Masque pour garder seulement les bits entiers
            uint32_t mask = ~((1U << (frac_bits + HF_PRECISION_SHIFT)) - 1U);
            result.mant = result.mant & mask;
            normalize_and_round(&result);
        }
        //Si exp >= 10, le nombre est déjà entier (pas de bits fractionnaires)
    }

    return compose_half(&result);
}

/**
 * @brief Récupère la partie entière d'un demi-flottant
 * 
 * Cette fonction extrait la partie entière d'un nombre représenté en demi-flottant (half-float).
 * Elle gère les cas spéciaux tels que NaN, l'infini, et zéro, et ajuste l'exposant en conséquence.
 *
 * @param hf Le demi-flottant dont on veut récupérer la partie entière
 * @return La partie entière sous forme de demi-flottant
 */
uint16_t hf_int(uint16_t hf) {
    return hf_trunc(hf);
}
