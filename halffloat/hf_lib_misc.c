/**
 * @file hf_lib_misc.c
 * @brief Implémentation des fonctions utilitaires pour Half-Float
 * 
 * Ce fichier implémente des fonctions utilitaires diverses pour les nombres
 * flottants de demi-précision (comparaison, min, max, etc.).
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#include "hf_lib_misc.h"
#include "hf_lib_common.h"

/**
 * @brief Compare deux demi-flottants (IEEE 754 - half precision)
 *
 * Compare deux valeurs au format demi-précision IEEE et renvoie un code entier :
 *   - +1  si hf1 > hf2
 *   - -1  si hf1 < hf2
 *   -  0  si hf1 == hf2
 *   - -2  si comparaison impossible (NaN détecté)
 *
 * Comportement conforme IEEE 754 :
 *  - NaN (quiet ou signaling) -> retourne -2 (aucune exception levée)
 *  - +0 et -0 sont considérés égaux
 *  - +/-Inf et subnormaux respectent l'ordre total IEEE
 *  - Aucun signal d'exception matériel (implémentation logicielle pure)
 *
 * @param hf1 Premier demi-flottant encodé sur 16 bits
 * @param hf2 Second demi-flottant encodé sur 16 bits
 * @return Code de comparaison (-2, -1, 0, +1)
 */
int hf_cmp(uint16_t hf1, uint16_t hf2) {
    half_float input1 = decompose_half(hf1);
    half_float input2 = decompose_half(hf2);
    return compare_half(&input1, &input2);
}

/**
 * @brief Renvoie le minimum de deux demi-flottants (IEEE 754 - half precision)
 *
 * Compare deux demi-flottants et renvoie le plus petit au format IEEE 754.
 *
 * Cas particuliers (IEEE 754):
 *  - Si un seul opérande est NaN, retourne l'autre opérande
 *  - Si les deux sont NaN, retourne NaN
 *  - min(+0,-0) = -0
 *  - min(+Inf, valeur) = valeur
 *  - min(-Inf, valeur) = -Inf
 *
 * @param hf1 Premier demi-flottant encodé
 * @param hf2 Second demi-flottant encodé
 * @return Le minimum au format demi-flottant IEEE 754
 */
uint16_t hf_min(uint16_t hf1, uint16_t hf2) {
    uint16_t result = hf1;
    half_float input1 = decompose_half(hf1);
    half_float input2 = decompose_half(hf2);
    
    //Si les deux sont NaN, retourner NaN
    if(is_nan(&input1) && is_nan(&input2)) {
        result = HF_NAN;
    }
    //Si seul input1 est NaN, retourner input2
    else if(is_nan(&input1)) {
        result = hf2;
    }
    //Si seul input2 est NaN, retourner input1 (déjà dans result)
    else if(!is_nan(&input2)) {
        //Aucun n'est NaN, comparaison normale
        int cmp = compare_half(&input1, &input2);
        
        if(cmp == 0 && is_zero(&input1) && is_zero(&input2)) {
            //Cas +0 / -0 : on renvoie toujours -0
            result = (input1.sign || input2.sign) ? HF_ZERO_NEG : HF_ZERO_POS;
        }
        else if(cmp > 0) {
            //hf1 > hf2, donc min = hf2
            result = hf2;
        }
        //Sinon hf1 <= hf2, result contient déjà hf1
    }
    //Sinon seul input2 est NaN, result contient déjà hf1
    
    return result;
}

/**
 * @brief Renvoie le maximum de deux demi-flottants (IEEE 754 - half precision)
 *
 * Compare deux demi-flottants et renvoie le plus grand au format IEEE 754.
 *
 * Cas particuliers (IEEE 754):
 *  - Si un seul opérande est NaN, retourne l'autre opérande
 *  - Si les deux sont NaN, retourne NaN
 *  - max(+0,-0) = +0
 *  - max(+Inf, valeur) = +Inf
 *  - max(-Inf, valeur) = valeur
 *
 * @param hf1 Premier demi-flottant encodé
 * @param hf2 Second demi-flottant encodé
 * @return Le maximum au format demi-flottant IEEE 754
 */
uint16_t hf_max(uint16_t hf1, uint16_t hf2) {
    uint16_t result = hf1;
    half_float input1 = decompose_half(hf1);
    half_float input2 = decompose_half(hf2);
    
    //Si les deux sont NaN, retourner NaN
    if(is_nan(&input1) && is_nan(&input2)) {
        result = HF_NAN;
    }
    //Si seul input1 est NaN, retourner input2
    else if(is_nan(&input1)) {
        result = hf2;
    }
    //Si seul input2 est NaN, retourner input1 (déjà dans result)
    else if(!is_nan(&input2)) {
        //Aucun n'est NaN, comparaison normale
        int cmp = compare_half(&input1, &input2);
        
        if(cmp == 0 && is_zero(&input1) && is_zero(&input2)) {
            //Cas +0 / -0 : on renvoie toujours +0
            result = (input1.sign && input2.sign) ? HF_ZERO_NEG : HF_ZERO_POS;
        }
        else if(cmp < 0) {
            //hf1 < hf2, donc max = hf2
            result = hf2;
        }
        //Sinon hf1 >= hf2, result contient déjà hf1
    }
    //Sinon seul input2 est NaN, result contient déjà hf1
    
    return result;
}

/**
 * @brief Sépare la partie entière et fractionnaire
 *
 * @param hf Le demi-flottant à décomposer
 * @param intpart Pointeur vers la partie entière (optionnel)
 * @return La partie fractionnaire
 */
uint16_t hf_modf(uint16_t hf, uint16_t *intpart) {
    (void)hf; if(intpart) *intpart = HF_NAN; return HF_NAN;
}

/**
 * @brief Décompose en mantisse et exposant
 *
 * @param hf Le demi-flottant à décomposer
 * @param exp Pointeur vers l'exposant (optionnel)
 * @return La mantisse normalisée
 */
uint16_t hf_frexp(uint16_t hf, int *exp) {
    (void)hf; if(exp) *exp = 0; return HF_NAN;
}

/**
 * @brief Multiplie par une puissance de 2
 *
 * @param hf Le demi-flottant
 * @param exp L'exposant de 2 à multiplier
 * @return hf * 2^exp
 */
uint16_t hf_ldexp(uint16_t hf, int exp) {
    (void)hf; (void)exp; return HF_NAN;
}

/**
 * @brief Met à l'échelle par une puissance de FLT_RADIX (2)
 *
 * @param hf Le demi-flottant
 * @param n L'exposant
 * @return hf * FLT_RADIX^n
 */
uint16_t hf_scalbn(uint16_t hf, int n) {
    (void)hf; (void)n; return HF_NAN;
}

/**
 * @brief Extrait l'exposant (en flottant)
 *
 * @param hf Le demi-flottant
 * @return L'exposant sous forme flottante
 */
uint16_t hf_logb(uint16_t hf) {
    (void)hf; return HF_NAN;
}

/**
 * @brief Extrait l'exposant (en entier)
 *
 * @param hf Le demi-flottant
 * @return L'exposant sous forme entière
 */
int hf_ilogb(uint16_t hf) {
    (void)hf; return -1;
}

/**
 * @brief Copie le signe d'un nombre vers un autre
 *
 * @param mag La magnitude
 * @param sign La source du signe
 * @return mag avec le signe de sign
 */
uint16_t hf_copysign(uint16_t mag, uint16_t sign) {
    (void)mag; (void)sign; return HF_NAN;
}

/**
 * @brief Valeur suivante vers une direction donnée
 *
 * @param from La valeur de départ
 * @param to La direction cible
 * @return La valeur suivante vers to
 */
uint16_t hf_nextafter(uint16_t from, uint16_t to) {
    (void)from; (void)to; return HF_NAN;
}

/**
 * @brief Valeur suivante vers une direction donnée (long double)
 *
 * @param from La valeur de départ
 * @param to La direction cible (long double)
 * @return La valeur suivante vers to
 */
uint16_t hf_nexttoward(uint16_t from, long double to) {
    (void)from; (void)to; return HF_NAN;
}
