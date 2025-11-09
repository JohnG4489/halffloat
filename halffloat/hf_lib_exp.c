/**
 * @file hf_lib_exp.c
 * @brief Implémentation des fonctions exponentielles et logarithmiques pour Half-Float
 * 
 * Module contenant l'implémentation des fonctions exponentielles et logarithmiques
 * pour le format Half-Float IEEE 754.
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#include "hf_lib_exp.h"
#include "hf_lib_common.h"

/**
 * @brief Calcule le logarithme naturel d'un demi-flottant
 * 
 * Cette fonction calcule le logarithme naturel d'un nombre représenté en demi-flottant (half-float).
 * Elle gère les cas spéciaux tels que NaN, l'infini, zéro et les nombres négatifs.
 * Le résultat est normalisé et arrondi avant d'être retourné sous forme de demi-flottant.
 *
 * @param hf Le demi-flottant dont on veut calculer le logarithme naturel
 * @return Le résultat du logarithme naturel sous forme de demi-flottant
 */
uint16_t hf_ln(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    //Initialisation par défaut - évite le branchement else
    result.sign = HF_ZERO_POS;
    result.exp = HF_EXP_FULL;  //Utilisé par les cas spéciaux (inf, NaN)
    result.mant = 0;

    //Gestion des cas spéciaux
    if(is_zero(&input)) {
        //ln(0) = -inf, exp et mant déjà corrects
        result.sign = HF_ZERO_NEG;
    } else if(is_nan(&input) || input.sign) {
        //ln(NaN) = NaN, ln(négatif) = NaN, exp déjà correct
        result.sign = input.sign;
        result.mant = 1;
    } else if(is_infinity(&input)) {
        //ln(+inf) = +inf
        result = input;
    } else {
        //Calcul normal: ln(x) = exp*ln(2) + ln_table[mantisse]
        int idx;
        
        //Normaliser les nombres dénormalisés pour avoir le bit implicite et un exposant valide
        normalize_denormalized_mantissa(&input);

        idx = (input.mant >> HF_PRECISION_SHIFT) & HF_MASK_MANT;
        result.mant = input.exp * LNI_2 + ln_table[idx];
        result.exp = 0;
        
        //Gestion du signe du résultat (réassignation conditionnelle)
        if(result.mant < 0) {
            result.mant = -result.mant;
            result.sign = HF_ZERO_NEG;
        }
        
        normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Calcule l'exponentielle d'un demi-flottant (e^x)
 * 
 * Cette fonction calcule l'exponentielle (e^x) d'un nombre représenté
 * en demi-flottant. Elle utilise une table de recherche précalculée et
 * une interpolation linéaire pour améliorer la précision.
 * 
 * @param hf Le demi-flottant dont on veut calculer l'exponentielle
 * @return L'exponentielle du demi-flottant en entree
 *         - exp(-inf) = 0
 *         - exp(+inf) = +inf
 *         - exp(NaN) = NaN
 *         - exp(x) = +inf si x > ~12 (pour eviter l'overflow)
 */
uint16_t hf_exp(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    //Initialisation par défaut
    result.sign = HF_ZERO_POS; //L'exponentielle est toujours positive (sauf NaN)
    result.exp = 0;
    result.mant = 0;

    //Gestion des cas spéciaux
    if(is_nan(&input)) {
        //Propager le NaN avec son signe original
        result.sign = input.sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(&input)) {
        if(!input.sign) result.exp = HF_EXP_FULL; //exp(+inf) = +inf
        //exp(-inf) = 0, valeurs déjà correctes par défaut
    } else {
        //Test d'overflow/underflow unifié: |x| > ~12
        const int32_t THRESHOLD_MANT = (3 * HF_MANT_NORM_MIN) >> 1;
        int overflow = (input.exp > 3) || (input.exp == 3 && input.mant >= THRESHOLD_MANT);
        
        if(overflow) {
            //Overflow positif -> +inf, underflow négatif -> 0
            if(!input.sign) result.exp = HF_EXP_FULL; //exp(+large) = +inf
            //mant et exp déjà corrects pour exp(-large) = 0
        } else {
            int32_t x_fixed = input.sign ? -input.mant : input.mant;
            
            //Ajustement selon l'exposant pour obtenir la vraie valeur
            x_fixed = (input.exp >= 0) ? (x_fixed << input.exp) : (x_fixed >> -input.exp);
            
            //Calcul de e^x via exp_fixed
            exp_fixed(x_fixed, &result);

            normalize_and_round(&result);
        }
    }

    return compose_half(&result);
}

/**
 * @brief Calcule la puissance d'un demi-flottant
 * 
 * Cette fonction calcule hfbase^hfexp où hfbase et hfexp sont des demi-flottants (half-float).
 * Elle utilise la formule hfbase^hfexp = e^(hfexp * ln(hfbase)) pour effectuer le calcul.
 *
 * Conforme IEEE 754 et std::pow pour les cas spéciaux :
 *  - x^0 = 1 (même si x = NaN), 1^y = 1, (-1)^+/-inf = 1
 *  - (-1)^entier : signe selon parité
 *  - base négative avec exposant non-entier -> NaN
 *  - 0^x : x>0 -> 0 ; x<0 -> +inf ; signe -0 préservé pour exposants entiers impairs
 *  - (+/-inf)^x : x>0 -> +/-inf ; x<0 -> +/-0 ; signe selon parité si base = -inf et x entier
 *  - x^(+/-inf) : |x|>1 -> (+/-inf)-> +inf / (-inf)-> 0 ; |x|<1 -> (+/-inf)-> 0 / (-inf)-> +inf ; |x|=1 -> 1
 * 
 * @param hfbase Le demi-flottant représentant la base
 * @param hfexp Le demi-flottant représentant l'exposant
 * @return Le résultat de hfbase^hfexp sous forme de demi-flottant
 */
uint16_t hf_pow(uint16_t hfbase, uint16_t hfexp) {
    half_float result;
    half_float inputbase = decompose_half(hfbase);
    half_float inputexp  = decompose_half(hfexp);

    //Initialisation par défaut : 1.0
    result.sign = HF_ZERO_POS;
    result.exp  = 0;
    result.mant = HF_MANT_NORM_MIN;

    //Rien à faire si x^0 : on garde 1.0
    if(!is_zero(&inputexp)) {
        uint16_t abs_base_bits = (uint16_t)(hfbase & ~HF_MASK_SIGN);
        int exp_int_part  = check_int_half(&inputexp);

        //|base| == 1 : tous les cas spéciaux (+/-1, +/-inf, NaN)
        if(abs_base_bits == HF_ONE_POS) {
            //1^+/-inf = 1, (-1)^+/-inf = 1, 1^NaN = 1, (-1)^NaN = 1 (conforme std::pow)
            int is_exp_inf_or_nan = is_infinity(&inputexp) || is_nan(&inputexp);
            if(!is_exp_inf_or_nan) {
                if(inputbase.sign) {
                    //(-1)^y
                    if(exp_int_part < 0) {
                        //(-1)^non-entier = NaN
                        result.exp  = HF_EXP_FULL;
                        result.mant = 1;
                    } else {
                        //(-1)^entier : signe selon parité, magnitude = 1
                        result.sign = (exp_int_part & 1) ? HF_ZERO_NEG : HF_ZERO_POS;
                    }
                }
                //(+1)^y = 1 : déjà initialisé
            }
            //Cas +/-inf ou NaN : garder 1.0
        }
        else {
            //NaN propagé si l'un est NaN (hors cas |base|==1 géré ci-dessus)
            if(is_nan(&inputbase) || is_nan(&inputexp)) {
                result.exp  = HF_EXP_FULL;
                result.mant = 1;
            }
            //0^x (x != 0)
            else if(is_zero(&inputbase)) {
                //x<0 -> +inf ; x>0 -> 0 ; signe -0 si base négative et exposant entier impair
                result.exp  = inputexp.sign ? HF_EXP_FULL : -HF_EXP_BIAS;
                result.mant = 0;
                result.sign = (inputbase.sign && exp_int_part >= 0 && (exp_int_part & 1)) ? HF_ZERO_NEG : HF_ZERO_POS;
            }
            //(+/-Inf)^x (x != 0)
            else if(is_infinity(&inputbase)) {
                //x<0 -> +/-0 ; x>0 -> +/-inf ; signe si base = -inf et exposant entier impair
                result.exp  = inputexp.sign ? -HF_EXP_BIAS : HF_EXP_FULL;
                result.mant = 0;
                result.sign = (inputbase.sign && exp_int_part >= 0 && (exp_int_part & 1)) ? HF_ZERO_NEG : HF_ZERO_POS;
            }
            //x^(+/-Inf) (|x| != 1)
            else if(is_infinity(&inputexp)) {
                if(abs_base_bits > HF_ONE_POS) {
                    //|x| > 1 : +inf si +inf ; 0 si -inf
                    result.exp = inputexp.sign ? -HF_EXP_BIAS : HF_EXP_FULL;
                } else if(abs_base_bits < HF_ONE_POS) {
                    //|x| < 1 : 0 si +inf ; +inf si -inf
                    result.exp = inputexp.sign ? HF_EXP_FULL : -HF_EXP_BIAS;
                }
                result.mant = 0;
            }
            //x^1 = x
            else if(inputexp.exp == 0 && inputexp.mant == result.mant && inputexp.sign == 0) {
        		//x^1 = x (cas spécial optimisé)
                result = inputbase;
            }
            //Base négative avec exposant non-entier -> NaN
            else if(inputbase.sign && exp_int_part < 0) {
                result.exp  = HF_EXP_FULL;
                result.mant = 1;
            }
            else {
                //Calcul général via ln/exp sur |base|, puis ajuste le signe si base < 0 et exposant entier impair
                int32_t ln_base_fixed, exp_fixed_val, exp_ln_fixed;
                int idx_ln, frac;

                result.sign = HF_ZERO_POS;
                if(inputbase.sign && exp_int_part >= 0) result.sign = (exp_int_part & 1) ? HF_ZERO_NEG : HF_ZERO_POS;

                //Normaliser la base pour ln
                normalize_denormalized_mantissa(&inputbase);

                //CALCUL DIRECT ln(base)
                idx_ln = (inputbase.mant >> HF_PRECISION_SHIFT) & HF_MASK_MANT;
                ln_base_fixed = inputbase.exp * LNI_2 + ln_table[idx_ln];
                if(idx_ln < LN_TABLE_SIZE - 1) {
                    frac = inputbase.mant & HF_ROUND_BIT_MASK;
                    ln_base_fixed += ((ln_table[idx_ln + 1] - ln_table[idx_ln]) * frac) >> HF_PRECISION_SHIFT;
                }

                //CALCUL exp * ln(|base|)
                exp_fixed_val = (inputexp.exp >= 0) ? (inputexp.mant << inputexp.exp) : (inputexp.mant >> -inputexp.exp);
                exp_ln_fixed = (int32_t)(((int64_t)exp_fixed_val * ln_base_fixed) >> 15);

                //Signe appliqué directement
                if(inputexp.sign) exp_ln_fixed = -exp_ln_fixed;

                //Utilisation directe de exp_fixed()
                exp_fixed(exp_ln_fixed, &result);
                
                normalize_and_round(&result);
            }
        }
    }

    return compose_half(&result);
}

/* Stubs pour les autres fonctions */
/**
 * @brief Calcule le logarithme en base 2 d'un demi-flottant
 *
 * @param a Le demi-flottant dont on veut calculer le logarithme en base 2
 * @return Le logarithme en base 2 de a
 */
uint16_t hf_log2(uint16_t a) {
    (void)a; return HF_NAN;
}

/**
 * @brief Calcule le logarithme en base 10 d'un demi-flottant
 *
 * @param a Le demi-flottant dont on veut calculer le logarithme en base 10
 * @return Le logarithme en base 10 de a
 */
uint16_t hf_log10(uint16_t a) {
    (void)a; return HF_NAN;
}

/**
 * @brief Calcule 2^x pour un demi-flottant x
 *
 * @param a L'exposant
 * @return 2^a
 */
uint16_t hf_exp2(uint16_t a) {
    (void)a; return HF_NAN;
}

/**
 * @brief Calcule 10^x pour un demi-flottant x
 *
 * @param a L'exposant
 * @return 10^a
 */
uint16_t hf_exp10(uint16_t a) {
    (void)a; return HF_NAN;
}

/**
 * @brief Calcule e^x - 1 pour un demi-flottant x
 *
 * @param a L'exposant
 * @return e^a - 1
 */
uint16_t hf_expm1(uint16_t a) {
    (void)a; return HF_NAN;
}

/**
 * @brief Calcule ln(1 + x) pour un demi-flottant x
 *
 * @param a La valeur à ajouter à 1
 * @return ln(1 + a)
 */
uint16_t hf_log1p(uint16_t a) {
    (void)a; return HF_NAN;
}
