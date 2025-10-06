/**
 * @file hf_lib.c
 * @brief Implémentation des fonctions mathématiques pour Half-Float IEEE 754
 * 
 * Ce fichier implémente toutes les opérations arithmétiques et mathématiques
 * pour les nombres flottants de demi-précision : addition, multiplication,
 * division, racine carrée, fonctions transcendantes (exp, ln, pow) et
 * trigonométriques (sin, cos, tan) avec conformité IEEE 754 complète.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#include "hf_common.h"
#include "hf_lib.h"
#include "hf_precalc.h"

//Fonctions mathématiques de base
uint16_t hf_int(uint16_t hf);
uint16_t hf_abs(uint16_t hf);
uint16_t hf_neg(uint16_t hf);

//Opérations arithmétiques
uint16_t hf_add(uint16_t hf1, uint16_t hf2);
uint16_t hf_mul(uint16_t hf1, uint16_t hf2);
uint16_t hf_div(uint16_t hf1, uint16_t hf2);
uint16_t hf_sqrt(uint16_t hf);

//Fonctions transcendantes
uint16_t hf_ln(uint16_t hf);
uint16_t hf_exp(uint16_t hf);
uint16_t hf_pow(uint16_t hfbase, uint16_t hfexp);

//Fonctions trigonométriques
uint16_t hf_sin(uint16_t hfangle);
uint16_t hf_cos(uint16_t hfangle);
uint16_t hf_tan(uint16_t hfangle);

//Déclarations des fonctions internes statiques
static uint16_t reduce_radian_uword(uint32_t angle_rad_fixed, int fact);
static uint16_t sinus_shiftable(uint16_t hfangle, uint16_t shift);
static void normalize_denormalized_mantissa(half_float *hf);

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
    half_float result = decompose_half(hf);

    //Traitement uniquement pour les nombres non spéciaux
    if(!is_nan(result) && !is_infinity(result) && !is_zero(result)) {
        //Normaliser la mantisse avec déclaration+affectation directe
        uint32_t value = result.exp >= 0 ? (uint32_t)(result.mant << result.exp) : (uint32_t)(result.mant >> -result.exp);

        //Convertir la valeur entière avec masquage des bits fractionnaires
        result.mant = (int32_t)(value & ~((1U << (HF_MANT_BITS + HF_PRECISION_SHIFT)) - 1U));
        result.exp = 0;
        normalize_and_round(&result);
    }

    return compose_half(result);
}

/**
 * @brief Calcule la valeur absolue d'un demi-flottant
 *
 * Cette fonction retourne la valeur absolue d'un nombre en format demi-flottant.
 * Elle ne modifie que le bit de signe, laissant l'exposant et la mantisse inchangés.
 *
 * @param hf Le demi-flottant dont on veut calculer la valeur absolue
 * @return La valeur absolue de hf sous forme de demi-flottant
 */
uint16_t hf_abs(uint16_t hf) {
    //La valeur absolue est obtenue en mettant le bit de signe à 0
    //On utilise un masque pour conserver tous les bits sauf le bit de signe
    return hf & (uint16_t)~(1U << HF_SIGN_BITS);
}

/**
 * @brief Calcule l'opposé (négation) d'un demi-flottant
 *
 * Cette fonction retourne l'opposé d'un nombre en format demi-flottant.
 * Elle inverse simplement le bit de signe, laissant l'exposant et la mantisse inchangés.
 *
 * @param hf Le demi-flottant dont on veut calculer l'opposé
 * @return L'opposé de hf sous forme de demi-flottant
 */
uint16_t hf_neg(uint16_t hf) {
    //L'opposé est obtenu en inversant le bit de signe
    //On utilise l'opération XOR avec 0x8000 pour inverser uniquement le bit de signe
    return hf ^ (1 << HF_SIGN_BITS);
}

/**
 * @brief Additionne deux demi-flottants
 * 
 * @param hf1 Premier demi-flottant
 * @param hf2 Second demi-flottant
 * @return Le résultat de l'addition sous forme de demi-flottant
 */
uint16_t hf_add(uint16_t hf1, uint16_t hf2) {
    half_float result;
    half_float input1 = decompose_half(hf1);
    half_float input2 = decompose_half(hf2);

    //Gestion unifiée des NaN - propager le premier NaN rencontré
    if(is_nan(input1) || is_nan(input2)) {
        half_float *nan_source = is_nan(input1) ? &input1 : &input2;
        result.sign = nan_source->sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(input1) || is_infinity(input2)) {
        //Gestion des cas infinis
        if(is_infinity(input1) && is_infinity(input2)) {
            //Les deux sont infinis
            if(input1.sign != input2.sign) {
                //Infini positif + Infini négatif = NaN négatif
                result.sign = HF_ZERO_NEG;
                result.exp = HF_EXP_FULL;
                result.mant = 1;
            } else {
                //Infini + Infini de même signe = Infini
                result = input1;
            }
        } else {
            //Un seul est infini - le résultat est cet infini
            result = is_infinity(input1) ? input1 : input2;
        }
    } else {
        //OPTIMISATION : Déclarations avec affectations directes pour réduire les opérations
        int32_t sum;
        
        //Addition normale - aligner les mantisses
        align_mantissas(&input1, &input2);
        
        //Calcul avec mantisses signées combiné - pas de variables intermédiaires
        sum = (input1.sign ? -input1.mant : input1.mant);
        sum += (input2.sign ? -input2.mant : input2.mant);

        //Déterminer signe et valeur absolue du résultat avec assignation directe
        result.exp = input1.exp;
        if(sum < 0) {
            result.mant = -sum;
            result.sign = 1 << HF_SIGN_BITS;
        } else {
            result.mant = sum;
            result.sign = 0;
        }
        
        normalize_and_round(&result);
    }

    return compose_half(result);
}

/**
 * @brief Multiplie deux demi-flottants
 * 
 * @param hf1 Premier demi-flottant
 * @param hf2 Second demi-flottant
 * @return Le résultat de la multiplication sous forme de demi-flottant
 */
uint16_t hf_mul(uint16_t hf1, uint16_t hf2) {
    half_float result;
    half_float input1 = decompose_half(hf1);
    half_float input2 = decompose_half(hf2);

    //Gestion unifiée des NaN - propager le premier NaN rencontré
    if(is_nan(input1) || is_nan(input2)) {
        half_float *nan_source = is_nan(input1) ? &input1 : &input2;
        result.sign = nan_source->sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if((is_infinity(input1) && is_zero(input2)) ||
              (is_infinity(input2) && is_zero(input1))) {
        //Inf * 0 = NaN négatif selon la convention de la référence
        result.sign = HF_ZERO_NEG;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else {
        //Signe du résultat
        result.sign = input1.sign ^ input2.sign;
        
        if(is_zero(input1) || is_zero(input2)) {
            //Résultat = zéro
            result.exp = -HF_EXP_BIAS;
            result.mant = 0;
        } else if(is_infinity(input1) || is_infinity(input2)) {
            //Résultat = infini
            result.exp = HF_EXP_FULL;
            result.mant = 0;
        } else {
            //Multiplication normale
            uint32_t mult_result = (uint32_t)(input1.mant * input2.mant);
            
            result.exp = input1.exp + input2.exp;
            result.mant = (int32_t)(mult_result >> (HF_MANT_BITS + HF_PRECISION_SHIFT));
            
            normalize_and_round(&result);
        }
    }

    //Composition du résultat final
    return compose_half(result);
}

/**
 * @brief Divise deux demi-flottants
 * 
 * @param hf1 Premier demi-flottant (dividende)
 * @param hf2 Second demi-flottant (diviseur)
 * @return Le résultat de la division sous forme de demi-flottant
 */
uint16_t hf_div(uint16_t hf1, uint16_t hf2) {
    half_float result;
    half_float input1 = decompose_half(hf1);
    half_float input2 = decompose_half(hf2);
    
    //Initialisation par défaut : division normale
    result.sign = input1.sign ^ input2.sign;
    result.exp = input1.exp - input2.exp;
    
    //Gestion unifiée des NaN - propager le premier NaN rencontré
    if(is_nan(input1) || is_nan(input2)) {
        half_float *nan_source = is_nan(input1) ? &input1 : &input2;
        result.sign = nan_source->sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(input1) && is_infinity(input2)) {
        //Inf / Inf = NaN négatif
        result.sign = HF_ZERO_NEG;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_zero(input1) && is_zero(input2)) {
        //0 / 0 = NaN
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(input1)) {
        //Inf / valeur finie = Inf
        result.exp = HF_EXP_FULL;
        result.mant = 0;
    } else if(is_infinity(input2) || is_zero(input1)) {
        //Fini / Inf = 0 ou 0 / Fini = 0
        result.exp = -HF_EXP_BIAS;
        result.mant = 0;
    } else if(is_zero(input2)) {
        //Fini / 0 = Inf
        result.exp = HF_EXP_FULL;
        result.mant = 0;
    } else {
        //Division arithmétique normale avec déclaration+affectation directe
        uint32_t dividend = input1.mant << (HF_MANT_BITS + HF_PRECISION_SHIFT);

        result.mant = dividend / input2.mant;
        if(dividend % input2.mant) {
            result.mant |= 1;
        }

        normalize_and_round(&result);
    }
    
    return compose_half(result);
}

/**
 * @brief Calcule la racine carrée d'un demi-flottant
 * 
 * Cette fonction utilise un algorithme de décalage optimisé pour calculer
 * la racine carrée d'un nombre représenté en demi-flottant (half-float).
 * Elle gère les cas spéciaux tels que NaN, l'infini, et zéro, et ajuste l'exposant
 * en conséquence. Le résultat est normalisé et arrondi avant d'être retourné.
 *
 * @param hf Le demi-flottant dont on veut calculer la racine carrée
 * @return Le résultat de la racine carrée sous forme de demi-flottant
 */
uint16_t hf_sqrt(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);
    
    //Initialisation par défaut : calcul arithmétique normal
    result.sign = HF_ZERO_POS;  //sqrt(x) >= 0 pour x >= 0
    result.exp = 0;
    
    //Gestion des cas spéciaux - priorité absolue
    if(is_zero(input)) {
        result = input;  //Préserver sqrt(-0) = -0
    } else if(is_nan(input) || input.sign) {
        //NaN ou nombres négatifs = NaN
        result.exp = HF_EXP_FULL;
        result.mant = 1;
        result.sign = input.sign;
    } else if(is_infinity(input)) {
        //sqrt(+inf) = +inf
        result.exp = HF_EXP_FULL;
        result.mant = 0;
    } else {
        //Calcul arithmétique par algorithme de décalage avec déclarations+affectations directes
        int loop = 16;
        uint32_t sqrt = 0;
        uint32_t quot = 0;
        uint32_t value = (uint32_t)(input.mant << 1);
        
        //Normaliser la mantisse pour avoir un exposant effectif de 0
        value = input.exp >= 0 ? value << input.exp : value >> -input.exp;
        
        //Algorithme de racine carrée par décalage
        while(--loop >= 0) {
            quot = (quot << 2) + ((value >> 30) & 3);
            sqrt <<= 1;
            value <<= 2;

            if(quot >= sqrt + 1) {
                quot -= ++sqrt;
                sqrt++;
            }
        }

        //Ajuster le résultat avec décalage optimisé
        result.mant = (int32_t)(sqrt << 6);

        normalize_and_round(&result);
    }
    
    return compose_half(result);
}

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
    result.sign = 0;

    //Gestion des cas spéciaux
    if(is_zero(input)) {
        //ln(0) = -inf
        result.sign = HF_ZERO_NEG;
        result.exp = HF_EXP_FULL;
        result.mant = 0;
    } else if(is_nan(input) || input.sign) {
        //ln(NaN) = NaN, ln(négatif) = NaN
        result.sign = input.sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(input)) {
        //ln(+inf) = +inf
        result = input;
    } else {
        //Calcul normal : ln(x) = exp*ln(2) + ln_table[mantisse]
        int idx;
        
        //Normaliser les nombres dénormalisés pour avoir le bit implicite et un exposant valide
        normalize_denormalized_mantissa(&input);

        idx = (input.mant >> HF_PRECISION_SHIFT) & 0x3ff;
        result.mant = input.exp * LNI_2 + ln_table[idx];
        result.exp = 0;
        
        //Gestion du signe du résultat (réassignation conditionnelle)
        if(result.mant < 0) {
            result.mant = -result.mant;
            result.sign = HF_ZERO_NEG;
        }
        
        normalize_and_round(&result);
    }

    return compose_half(result);
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
    
    //Gestion des cas spéciaux
    if(is_nan(input)) {
        //Propager le NaN avec son signe original
        result.sign = input.sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else {
        result.sign = 0; //L'exponentielle est toujours positive pour les cas non-NaN
        
        if(is_infinity(input)) {
            result.mant = 0;
            result.exp = input.sign ? 0 : HF_EXP_FULL; //0 ou +inf
        } else {
            //Test d'overflow/underflow unifié: |x| > ~12
            const int32_t THRESHOLD_MANT = (3 * (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT))) >> 1;
            int overflow = (input.exp > 3) || (input.exp == 3 && input.mant >= THRESHOLD_MANT);
            
            if(overflow) {
                //Overflow positif -> +inf, underflow négatif -> 0
                result.exp = input.sign ? 0 : HF_EXP_FULL;
                result.mant = 0;
            } else {
                int32_t x_fixed = input.sign ? -input.mant : input.mant;
                int32_t k_exp, r_fixed, frac;
                int index;
                
                //Ajustement selon l'exposant pour obtenir la vraie valeur
                x_fixed = (input.exp >= 0) ? (x_fixed << input.exp) : (x_fixed >> -input.exp);
                
                //Réduction à [0, ln(2)] : k = floor(x/ln(2)), r = x - k*ln(2)
                k_exp = x_fixed / LNI_2;
                r_fixed = x_fixed - k_exp * LNI_2;
                
                if(r_fixed < 0) {
                    k_exp--;
                    r_fixed += LNI_2;
                }
                
                //Index dans la table avec protection
                index = (r_fixed * EXP_TABLE_SIZE) / LNI_2;
                if(index >= EXP_TABLE_SIZE) index = EXP_TABLE_SIZE - 1;
                
                //Interpolation linéaire
                result.mant = exp_table[index];
                if(index < EXP_TABLE_SIZE - 1) {
                    frac = ((r_fixed * EXP_TABLE_SIZE) % LNI_2) << 8;
                    result.mant += ((exp_table[index + 1] - result.mant) * frac / LNI_2) >> 8;
                }
                
                result.exp = k_exp;
                normalize_and_round(&result);
            }
        }
    }

    return compose_half(result);
}

/**
 * @brief Calcule la puissance d'un demi-flottant
 * 
 * Cette fonction calcule hfbase^hfexp où hfbase et hfexp sont des demi-flottants (half-float).
 * Elle utilise la formule hfbase^hfexp = e^(hfexp * ln(hfbase)) pour effectuer le calcul.
 *
 * @param hfbase Le demi-flottant représentant la base
 * @param hfexp Le demi-flottant représentant l'exposant
 * @return Le résultat de hfbase^hfexp sous forme de demi-flottant
 */
uint16_t hf_pow(uint16_t hfbase, uint16_t hfexp) {
    half_float result;
    half_float inputbase = decompose_half(hfbase);
    half_float inputexp = decompose_half(hfexp);
    
    //Initialisation par défaut : résultat = 1.0 (cas IEEE 754 majoritaires)
    result.sign = 0;
    result.exp = 0;
    result.mant = 1 << (HF_MANT_BITS + HF_PRECISION_SHIFT);
    
    //Cas spéciaux IEEE 754 : base = 1.0 ou exposant = 0
    if((inputbase.exp == 0 && inputbase.mant == (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT)) && inputbase.sign == 0) ||
       is_zero(inputexp)) {
        //1^(n'importe quoi) = 1 ou N'importe quoi^0 = 1 - garder l'initialisation
    } else if(is_nan(inputbase) || is_nan(inputexp)) {
        //Gestion unifiée des NaN - propager le premier NaN rencontré
        half_float *nan_source = is_nan(inputbase) ? &inputbase : &inputexp;
        result.sign = nan_source->sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_zero(inputbase)) {
        //Gestion 0^x selon signe de x
        if(inputexp.sign == 0) {
            result = inputbase; //0^(positif) = 0
        } else {
            result.exp = HF_EXP_FULL;
            result.mant = 0; //0^(négatif) = inf
        }
    } else if(is_infinity(inputbase) || is_infinity(inputexp)) {
        if(is_infinity(inputbase)) {
            //Cas (+/-inf)^x
            if(inputexp.sign == 0) {
                //+/-inf^(positif) 
                if(inputbase.sign == 0) {
                    result = inputbase; //+inf^(positif) = +inf
                } else {
                    //(-inf)^(positif) - vérifier parité si entier
                    if(inputexp.exp >= 0) {
                        int32_t int_part = inputexp.mant >> (HF_MANT_BITS + HF_PRECISION_SHIFT - inputexp.exp);
                        result.sign = (int_part & 1) ? inputbase.sign : 0;
                    }
                    result.exp = HF_EXP_FULL;
                    result.mant = 0;
                }
            } else {
                //+/-inf^(négatif) = 0 avec signe selon parité
                result.exp = -HF_EXP_BIAS;
                result.mant = 0;
                if(inputbase.sign && inputexp.exp >= 0) {
                    int32_t int_part = inputexp.mant >> (HF_MANT_BITS + HF_PRECISION_SHIFT - inputexp.exp);
                    result.sign = (int_part & 1) ? 0x8000 : 0;
                }
            }
        } else {
            //Cas x^(+/-inf) selon IEEE 754
            uint16_t abs_base = inputbase.sign ? (hfbase ^ (1 << HF_SIGN_BITS)) : hfbase;
            uint16_t one_hf = (0 << HF_SIGN_BITS) | ((HF_EXP_BIAS) << HF_MANT_BITS) | 0; //1.0 en half-float
            
            if(abs_base > one_hf) {
                //|x| > 1 : exp=+inf -> +inf, exp=-inf -> 0
                result.exp = inputexp.sign ? -HF_EXP_BIAS : HF_EXP_FULL;
                result.mant = 0;
            } else if(abs_base < one_hf) {
                //|x| < 1 : exp=+inf -> 0, exp=-inf -> +inf
                result.exp = inputexp.sign ? HF_EXP_FULL : -HF_EXP_BIAS;
                result.mant = 0;
            }
            //|x| = 1 : garder l'initialisation result = 1
        }
    } else if(inputexp.exp == 0 && inputexp.mant == (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT)) && inputexp.sign == 0) {
        //x^1 = x (cas spécial optimisé)
        result = inputbase;
    } else if(inputbase.sign && (inputexp.exp != -HF_EXP_BIAS || inputexp.mant != 0)) {
        //Base négative avec exposant non-zéro = NaN
        result.exp = HF_EXP_FULL;
        result.mant = 1;
        result.sign = inputbase.sign;
    } else {
        //OPTIMISATION : Algorithme direct inspiré de hf_ln et hf_exp
        //Déclarations avec affectations directes pour réduire les opérations
        int result_negative = 0;
        int32_t ln_base_fixed, exp_fixed, exp_ln_fixed, k_exp, r_fixed;
        int idx_ln, idx_exp, frac;
        
        //Normaliser la base pour ln
        normalize_denormalized_mantissa(&inputbase);
        
        //CALCUL DIRECT ln(base) avec déclaration+affectation optimisée
        idx_ln = (inputbase.mant >> HF_PRECISION_SHIFT) & 0x3ff;
        ln_base_fixed = inputbase.exp * LNI_2 + ln_table[idx_ln];
        
        //Interpolation ln avec calcul de frac combiné
        if(idx_ln < LN_TABLE_SIZE - 1) {
            frac = inputbase.mant & ((1 << HF_PRECISION_SHIFT) - 1);
            ln_base_fixed += ((ln_table[idx_ln + 1] - ln_table[idx_ln]) * frac) >> HF_PRECISION_SHIFT;
        }
        
        //CALCUL exp * ln(base) avec ajustement exposant intégré
        exp_fixed = (inputexp.exp >= 0) ? (inputexp.mant << inputexp.exp) : (inputexp.mant >> -inputexp.exp);
        
        //Multiplication Q15*Q15->Q15 avec signe appliqué directement
        exp_ln_fixed = (int32_t)(((int64_t)exp_fixed * ln_base_fixed) >> 15);
        if(inputexp.sign) {
            exp_ln_fixed = -exp_ln_fixed;
        }
        
        //Gestion signe avec valeur absolue combinée
        if(exp_ln_fixed < 0) {
            result_negative = 1;
            exp_ln_fixed = -exp_ln_fixed;
        }
        
        //CALCUL exp() avec réduction modulo optimisée
        k_exp = exp_ln_fixed / LNI_2;
        r_fixed = exp_ln_fixed - k_exp * LNI_2;
        
        if(r_fixed < 0) {
            k_exp--;
            r_fixed += LNI_2;
        }
        
        //Index exp_table avec protection simplifiée
        idx_exp = (r_fixed * EXP_TABLE_SIZE) / LNI_2;
        if(idx_exp >= EXP_TABLE_SIZE) idx_exp = EXP_TABLE_SIZE - 1;
        
        //Interpolation exp avec résultat direct
        result.mant = exp_table[idx_exp];
        if(idx_exp < EXP_TABLE_SIZE - 1) {
            frac = ((r_fixed * EXP_TABLE_SIZE) % LNI_2) << 8;
            result.mant += ((exp_table[idx_exp + 1] - result.mant) * frac / LNI_2) >> 8;
        }
        
        result.exp = k_exp;
        result.sign = 0;
        
        //Inversion si exposant négatif avec constante optimisée
        if(result_negative && result.mant != 0) {
            result.mant = (int32_t)((1U << 30) / result.mant);
            result.exp = -result.exp;
        }
        
        normalize_and_round(&result);
    }
    
    return compose_half(result);
}

/**
 * @brief Calcule le sinus d'un angle exprimé en demi-précision.
 * 
 * Cette fonction est un wrapper autour de sinus_shiftable, spécialisé pour
 * le calcul du sinus. Elle utilise une table de recherche et une interpolation
 * linéaire pour calculer le sinus d'un angle donné en radians, représenté
 * sous forme de demi-précision. Les cas spéciaux tels que NaN et l'infini
 * sont gérés par la fonction sous-jacente.
 *
 * @param hfangle L'angle en radians, représenté en format demi-précision.
 * @return Le sinus de l'angle en format demi-précision.
 * 
 * @see sinus_shiftable
 */
uint16_t hf_sin(uint16_t hfangle) {
    return sinus_shiftable(hfangle, 0);
}

/**
 * @brief Calcule le cosinus d'un angle exprimé en demi-précision.
 * 
 * Cette fonction est un wrapper autour de sinus_shiftable, spécialisé pour
 * le calcul du cosinus. Elle applique un décalage de phase de pi/2 (16384 en
 * représentation fixe) à l'angle d'entrée pour transformer le sinus en cosinus.
 * La fonction utilise une table de recherche et une interpolation linéaire
 * pour le calcul. Les cas spéciaux tels que NaN et l'infini sont gérés par
 * la fonction sous-jacente.
 *
 * @param hfangle L'angle en radians, représenté en format demi-précision.
 * @return Le cosinus de l'angle en format demi-précision.
 * 
 * @see sinus_shiftable
 */
uint16_t hf_cos(uint16_t hfangle) {
    return sinus_shiftable(hfangle, 16384);
}

/**
 * @brief Calcule la tangente d'un angle en demi-précision.
 *
 * Cette fonction utilise un système dual-table optimisé avec formats Q adaptés :
 * - Table LOW Q13 [0°, 75°] : Précision maximale (step=0.0002441)
 * - Table HIGH Q6 [75°, 90°] : Plage maximale (max=1024, saturation 0.4%)
 * 
 * Économie mémoire: 75% vs table unique (1028 vs 4100 bytes)
 * Amélioration précision: 1.47x vs approche Q11/Q8
 *
 * @param hfangle L'angle en radians, représenté en format demi-précision.
 * @return La valeur de la tangente de l'angle en format demi-précision.
 */
uint16_t hf_tan(uint16_t hfangle) {
    half_float result;
    half_float angle_hf = decompose_half(hfangle);

    result.sign = 0;
    result.mant = 0;

    if(is_nan(angle_hf) || is_infinity(angle_hf)) {
        if(is_nan(angle_hf)) {
            //Propager le signe original pour les NaN
            result.sign = angle_hf.sign;
        } else {
            //Pour les infinis, retourner NaN négatif selon la convention de la bibliothèque
            result.sign = 0x8000; //NaN négatif pour tan(+/-inf)
        }
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else {
        const int32_t SWITCH_NORM_75DEG = 27306;  //65536 * (5pi/12) / pi
        const uint16_t *table_ptr = tan_table_low;
        int32_t qshift = 2;
        int32_t norm, input_norm, range_norm, frac_num, frac;
        int idx0, idx1;

        result.exp = 0;

        //Normalisation de la mantisse pour obtenir une représentation fixe de l'angle
        angle_hf.mant = angle_hf.exp >= 0 ? angle_hf.mant << angle_hf.exp : angle_hf.mant >> -angle_hf.exp;

        //Réduction de l'angle à l'intervalle [0, 65535] puis [0, pi/2]
        norm = (int32_t)reduce_radian_uword((uint32_t)angle_hf.mant, 1);
        if(angle_hf.sign) norm = 65536 - norm;
        input_norm = norm > 32768 ? 65536 - norm : norm;
        
        //Selection paramètres et écrasement si HIGH table
        range_norm = SWITCH_NORM_75DEG;
        if(input_norm > SWITCH_NORM_75DEG) {
            //Table HIGH Q6 [75°, 90°] - écraser les paramètres
            input_norm -= SWITCH_NORM_75DEG;
            range_norm = 32768 - SWITCH_NORM_75DEG;
            qshift = 9;  //Q6->Q15
            table_ptr = tan_table_high;
        }
        
        //Calculs unifiés pour les deux tables
        idx0 = (input_norm * TAN_DUAL_TABLE_SIZE) / range_norm;
        idx1 = (idx0 < TAN_DUAL_TABLE_SIZE) ? idx0 + 1 : TAN_DUAL_TABLE_SIZE;
        
        frac_num = (input_norm * TAN_DUAL_TABLE_SIZE) % range_norm;
        frac = (frac_num << 7) / range_norm;
        
        result.mant = (int32_t)table_ptr[idx0] << qshift;
        if(idx0 != idx1 && idx1 <= TAN_DUAL_TABLE_SIZE) {
            int32_t delta_val = ((int32_t)table_ptr[idx1] << qshift) - result.mant;
            result.mant += (delta_val * frac) >> 7;
        }
        
        //Application du signe selon le quadrant (test direct)
        if(norm > 32768) result.mant = -result.mant;
        
        //Gestion overflow et signe final
        if(result.mant > (1 << 26) || result.mant < -(1 << 26)) {
            result.sign = result.mant < 0 ? 0x8000 : 0;
            result.mant = 0;
            result.exp = HF_EXP_FULL;
        } else if(result.mant < 0) {
            //Gestion du signe - conversion en valeur absolue
            result.sign = 0x8000;
            result.mant = -result.mant;
        }
        //result.sign reste à 0 par défaut si result.mant >= 0

        //Normalisation et arrondi du résultat final
        normalize_and_round(&result);
    }

    return compose_half(result);
}

/**
 * @brief Réduit un angle en radians (format virgule fixe) à un entier non signé 16 bits
 * 
 * Cette fonction prend un angle en radians représenté en format virgule fixe (1.31)
 * et le réduit à un entier non signé 16 bits. Le résultat représente l'angle
 * dans l'intervalle [0, 2pi) ou [0, pi) selon la valeur de 'fact', tout en
 * maintenant une précision constante de 1/65536.
 *
 * @param angle_rad_fixed Angle en radians en format virgule fixe (1.31)
 * @param fact Facteur de réduction : 0 pour réduction à 2pi, 1 pour réduction à pi
 * @return Angle réduit sur 16 bits, représentant :
 *         - [0, 2pi] avec une précision de 1/65536 si fact = 0
 *         - [0, pi] avec une précision de 1/65536 si fact = 1
 *         Dans les deux cas, la valeur de retour couvre l'intégralité des 65536 valeurs possibles.
 */
static uint16_t reduce_radian_uword(uint32_t angle_rad_fixed, int fact) {
    uint64_t fixed_two_pi = 26986075409ULL >> fact;
    uint64_t fixed_two_pi_inv = 683565276ULL << fact;
    
    uint64_t angle = ((uint64_t)angle_rad_fixed) << 17;
    uint64_t cnt = angle / fixed_two_pi;
    uint64_t red = angle - cnt * fixed_two_pi;
    uint64_t res = red * fixed_two_pi_inv;
    res >>= 32 + 16;

    return (uint16_t)res;
}

/**
 * @brief Calcule le sinus ou le cosinus d'un angle en demi-précision avec décalage de phase.
 *
 * Cette fonction calcule le sinus d'un angle donné, avec la possibilité d'appliquer
 * un décalage de phase. Cela permet de calculer également le cosinus (avec un décalage
 * de pi/2) ou d'autres fonctions trigonométriques décalées. La fonction utilise une
 * table de recherche et une interpolation linéaire pour améliorer la précision.
 *
 * @param hfangle L'angle en radians, représenté en format demi-précision.
 * @param shift Le décalage de phase à appliquer (0 pour sinus, 16384 pour cosinus).
 * @return La valeur du sinus (ou cosinus) de l'angle en format demi-précision.
 */
static uint16_t sinus_shiftable(uint16_t hfangle, uint16_t shift) {
    half_float result;
    half_float angle_hf = decompose_half(hfangle);

    //Initialisation par défaut : calcul trigonométrique normal
    result.sign = 0;
    result.exp = 0;

    //Gestion unifiée des cas spéciaux
    if(is_nan(angle_hf)) {
        //Propager le signe original pour les NaN
        result.sign = angle_hf.sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(angle_hf)) {
        //Pour les infinis, retourner NaN négatif selon la convention
        result.sign = 0x8000;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else {
        //Calcul trigonométrique par table et interpolation
        int32_t norm = angle_hf.mant;
        int idx0, idx1;

        //Normalisation de la mantisse pour obtenir une représentation fixe de l'angle
        norm = angle_hf.exp >= 0 ? norm << angle_hf.exp : norm >> -angle_hf.exp;

        //Réduction de l'angle à l'intervalle [0, 65535] et application du décalage
        norm = (int32_t)reduce_radian_uword((uint32_t)norm, 0);
        if(angle_hf.sign) norm = 65536 - norm;
        norm = (norm + shift) & 0xffff;
        
        //Calcul des indices pour l'interpolation dans la table de sinus
        idx0 = (norm & 0x3fff) >> 4;
        idx1 = idx0 + 1;

        //Ajustement des indices pour les quadrants descendants (1 et 3)
        if(norm & 0x4000) {
            idx0 = SIN_TABLE_SIZE - idx0;
            idx1 = idx0 - 1;
            if(idx1 < 0) idx1 = 0;
        }
        if(idx0 > SIN_TABLE_SIZE) idx0 = SIN_TABLE_SIZE;
        if(idx1 > SIN_TABLE_SIZE) idx1 = SIN_TABLE_SIZE;

        //Interpolation linéaire entre les valeurs de la table
        result.mant = (int32_t)sin_table[idx0];
        result.mant += (((int32_t)sin_table[idx1] - result.mant) * (norm & 0xf) + 0x8) >> 4;

        //Gestion du signe pour les quadrants négatifs (2 et 3)
        if(norm & 0x8000) result.mant = -result.mant;
        if(result.mant < 0) {
            result.sign = 0x8000;
            result.mant = -result.mant;
        }

        //Normalisation et arrondi du résultat final
        normalize_and_round(&result);
    }

    return compose_half(result);
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
static void normalize_denormalized_mantissa(half_float *hf) {
    if(hf->exp == -HF_EXP_BIAS) {
        hf->exp++;
        while(hf->mant < (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT))) {
            hf->mant <<= 1;
            hf->exp--;
        }
    }
}
