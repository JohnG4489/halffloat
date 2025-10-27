/**
 * @file hf_lib.c
 * @brief Implémentation des fonctions mathématiques pour Half-Float IEEE 754
 * 
 * Ce fichier implémente toutes les opérations arithmétiques et mathématiques
 * pour les nombres flottants de demi-précision: addition, multiplication,
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

#define ROL32(x, n) ((x<<n) | (x>>(32-n)))

//Fonctions de comparaison
int hf_cmp(uint16_t hf1, uint16_t hf2);
uint16_t hf_min(uint16_t hf1, uint16_t hf2);
uint16_t hf_max(uint16_t hf1, uint16_t hf2);

//Fonctions mathématiques de base
uint16_t hf_int(uint16_t hf);
uint16_t hf_abs(uint16_t hf);
uint16_t hf_neg(uint16_t hf);

//Opérations arithmétiques
uint16_t hf_add(uint16_t hf1, uint16_t hf2);
uint16_t hf_sub(uint16_t hf1, uint16_t hf2);
uint16_t hf_mul(uint16_t hf1, uint16_t hf2);
uint16_t hf_div(uint16_t hf1, uint16_t hf2);
uint16_t hf_inv(uint16_t hf);
uint16_t hf_sqrt(uint16_t hf);
uint16_t hf_rsqrt(uint16_t hf);

//Fonctions transcendantes
uint16_t hf_ln(uint16_t hf);
uint16_t hf_exp(uint16_t hf);
uint16_t hf_pow(uint16_t hfbase, uint16_t hfexp);

//Fonctions trigonométriques
uint16_t hf_sin(uint16_t hfangle);
uint16_t hf_cos(uint16_t hfangle);
uint16_t hf_tan(uint16_t hfangle);
uint16_t hf_asin(uint16_t hf);
uint16_t hf_acos(uint16_t hf);

//Déclarations des fonctions internes statiques
static uint16_t reduce_radian_uword(uint32_t angle_rad_fixed, int fact);
static uint16_t sinus_shiftable(uint16_t hfangle, uint16_t shift);
static uint16_t asinus_shiftable(uint16_t hf, uint32_t shift);
static uint32_t square_root(uint32_t value);
static int compare_half(const half_float *input1, const half_float *input2);
static int check_int_half(const half_float *hf);

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
 *  - NaN (quiet ou signaling) -> retourne qNaN (aucune exception levée)
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
    int cmp = compare_half(&input1, &input2);

    if(cmp == -2) {
        //Propagation silencieuse du NaN
        result = HF_NAN;
    }
    else if(cmp == 0 && is_zero(&input1) && is_zero(&input2)) {
        //Cas +0 / -0 : on renvoie toujours -0
        result = (input1.sign || input2.sign) ? HF_ZERO_NEG : HF_ZERO_POS;
    }
    else if(cmp > 0) {
        //hf1 > hf2, donc min = hf2
        result = hf2;
    }

    return result;
}

/**
 * @brief Renvoie le maximum de deux demi-flottants (IEEE 754 - half precision)
 *
 * Compare deux demi-flottants et renvoie le plus grand au format IEEE 754.
 *
 * Cas particuliers (IEEE 754):
 *  - NaN (quiet ou signaling) -> retourne qNaN (aucune exception levée)
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
    int cmp = compare_half(&input1, &input2);

    if(cmp == -2) {
        //Propagation silencieuse du NaN
        result = HF_NAN;
    }
    else if(cmp == 0 && is_zero(&input1) && is_zero(&input2)) {
        //Cas +0 / -0 : on renvoie toujours +0
        result = (input1.sign && input2.sign) ? HF_ZERO_NEG : HF_ZERO_POS;
    }
    else if(cmp < 0) {
        //hf1 < hf2, donc max = hf2
        result = hf2;
    }

    return result;
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
    return hf & ~HF_MASK_SIGN;
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
    return hf ^ HF_MASK_SIGN;
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

    result.sign = HF_ZERO_POS;
    result.exp = HF_EXP_FULL;
    result.mant = 1;

    //Gestion unifiée des NaN - propager le premier NaN rencontré
    if(is_nan(&input1) || is_nan(&input2)) {
        result.sign = is_nan(&input1) ? input1.sign : input2.sign;
    } else if(is_infinity(&input1) || is_infinity(&input2)) {
        //Gestion des cas infinis
        if(is_infinity(&input1) && is_infinity(&input2)) {
            //Les deux sont infinis
            if(input1.sign != input2.sign) {
                //Infini positif + Infini négatif = NaN négatif
                result.sign = HF_ZERO_NEG;
            } else {
                //Infini + Infini de même signe = Infini
                result = input1;
            }
        } else {
            //Un seul est infini - le résultat est cet infini
            result = is_infinity(&input1) ? input1 : input2;
        }
    } else if(is_zero(&input1) && is_zero(&input2)) {
        //Cas spécial -0 + -0 = -0
        result.sign = (input1.sign && input2.sign) ? HF_ZERO_NEG : HF_ZERO_POS;
        result.exp = 0;
        result.mant = 0;
    } else {
        //Addition normale
        int32_t sum;
        
        //Aligner les mantisses
        align_mantissas(&input1, &input2);
        
        //Calcul avec mantisses signées combiné - pas de variables intermédiaires
        sum = (input1.sign ? -input1.mant : input1.mant);
        sum += (input2.sign ? -input2.mant : input2.mant);

        //Déterminer signe et valeur absolue du résultat avec assignation directe
        result.exp = input1.exp;
        result.mant = sum;
        if(sum < 0) {
            result.mant = -sum;
            result.sign = HF_ZERO_NEG;
        }

        normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Soustrait deux demi-flottants
 *
 * @param hf1 Premier demi-flottant
 * @param hf2 Second demi-flottant (sera soustrait)
 * @return Le résultat de la soustraction sous forme de demi-flottant
 */
uint16_t hf_sub(uint16_t hf1, uint16_t hf2) {
    //La soustraction est équivalente à l'addition du négatif du second opérande
    return hf_add(hf1, hf2 ^ HF_ZERO_NEG);
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
    
    //Initialisation par défaut (NaN positif - cas le plus fréquent pour les cas spéciaux)
    result.sign = HF_ZERO_POS;
    result.exp = HF_EXP_FULL;
    result.mant = 1;
    
    //Gestion unifiée des NaN - propager le premier NaN rencontré
    if(is_nan(&input1) || is_nan(&input2)) {
        result.sign = is_nan(&input1) ? input1.sign : input2.sign;
    } else if((is_infinity(&input1) && is_zero(&input2)) ||
              (is_infinity(&input2) && is_zero(&input1))) {
        //Inf * 0 = NaN négatif selon la convention de la référence
        result.sign = HF_ZERO_NEG;
    } else {
        //Signe du résultat
        result.sign = input1.sign ^ input2.sign;
        result.mant = 0;

        if(is_zero(&input1) || is_zero(&input2)) {
            //Résultat = zéro
            result.exp = -HF_EXP_BIAS;
        } else if(!is_infinity(&input1) && !is_infinity(&input2)) {
            //Multiplication normale
            uint32_t mult_result = (uint32_t)(input1.mant * input2.mant);
            
            result.exp = input1.exp + input2.exp;
            result.mant = (int32_t)(mult_result >> (HF_MANT_BITS + HF_PRECISION_SHIFT));
            
            normalize_and_round(&result);
        }
        //Par défaut: Résultat = infini
    }

    return compose_half(&result);
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
    
    //Initialisation par défaut: NaN positif (cas le plus fréquent pour les cas spéciaux)
    result.sign = input1.sign ^ input2.sign;
    result.exp = HF_EXP_FULL;
    result.mant = 1;
    
    //Gestion unifiée des NaN - propager le premier NaN rencontré
    if(is_nan(&input1) || is_nan(&input2)) {
        result.sign = is_nan(&input1) ? input1.sign : input2.sign;
    } else if(is_infinity(&input1) && is_infinity(&input2)) {
        //Inf / Inf = NaN négatif
        result.sign = HF_ZERO_NEG;
    } else if(is_zero(&input1) && is_zero(&input2)) {
        //0 / 0 = NaN (sign et autres déjà ok)
    } else if(is_infinity(&input1) || is_zero(&input2)) {
        //Inf / valeur finie = Inf ou Fini / 0 = Inf
        result.mant = 0;
    } else if(is_infinity(&input2) || is_zero(&input1)) {
        //Fini / Inf = 0 ou 0 / Fini = 0
        result.exp = -HF_EXP_BIAS;
        result.mant = 0;
    } else {
        //Division arithmétique normale
        uint32_t dividend = input1.mant << (HF_MANT_BITS + HF_PRECISION_SHIFT);
        result.exp = input1.exp - input2.exp;
        result.mant = dividend / input2.mant;
        if(dividend % input2.mant) result.mant |= 1;
        normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Inverse un demi-flottant (calcule 1/x)
 *
 * @param hf Le demi-flottant à inverser
 * @return L'inverse du demi-flottant (1/x)
 */
uint16_t hf_inv(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);
    
    //Initialisation par défaut: préserve le signe et prépare pour les cas spéciaux
    result.sign = input.sign;
    result.exp = HF_EXP_FULL;
    result.mant = 1;
    
    //Gestion des cas spéciaux
    if(is_infinity(&input)) {
        //1/Inf = 0 (avec le signe)
        result.exp = -HF_EXP_BIAS;
        result.mant = 0;
    } else if(is_zero(&input)) {
        //1/0 = Inf (avec le signe)
        result.mant = 0;
    } else if(!is_nan(&input)) {
        //Créer 1.0 avec bit implicite: mantisse = 1.0 en format étendu
        //input.mant contient déjà le bit implicite, donc dividend doit aussi
        uint32_t dividend = (1U << (HF_MANT_BITS + HF_PRECISION_SHIFT)) << (HF_MANT_BITS + HF_PRECISION_SHIFT);
        
        //Pour 1/x avec exposant débiaisé: exp_result = -exp_input
        result.exp = -input.exp;

        //Division arithmétique: 1.0 / input
        result.mant = dividend / input.mant;
        
        normalize_and_round(&result);
    }
    //Gestion du NaN: valeurs déjà bonnes par défaut

    return compose_half(&result);
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
    half_float input  = decompose_half(hf);
    
    //Initialisation par défaut: NaN positif (gère NaN et -x automatiquement)
    result.sign = HF_ZERO_POS;
    result.exp  = HF_EXP_FULL;
    result.mant = 1;
    
    //Cas spéciaux IEEE 754
    if(is_zero(&input)) {
        //sqrt(+/-0) -> +/-0
        result.sign = input.sign;
        result.exp  = input.exp;
        result.mant = 0;
    }
    else if(is_infinity(&input) && !input.sign) {
        //sqrt(+inf) -> +inf
        result.mant = 0;  //exp déjà FULL, sign déjà positif
    }
    else if(!input.sign && !is_infinity(&input) && !is_nan(&input)) {
        //Calcul arithmétique normal (x > 0 fini, non-zéro déjà exclu)
        uint32_t root = 0;
        uint32_t value = (uint32_t)(input.mant << 15);
        
        //Exposant impair: ajustement de l'exposant à pair + mantisse
        if(input.exp & 1) {
            value <<= 1;
            input.exp--;
        }
        
        //Calcul de la racine carrée
        root = square_root(value);

        if(root > 0) {
            result.exp  = input.exp / 2;
            result.mant = (int32_t)root;
            normalize_and_round(&result);
        }
    }
    //NaN et -x (incluant -inf) -> NaN: déjà correct par l'initialisation

    return compose_half(&result);
}

/**
 * @brief Calcule la racine carrée inverse d'un demi-flottant (1/sqrt(x))
 *
 * Cette fonction calcule directement l'inverse de la racine carrée d'un nombre
 * représenté en demi-flottant (half-float). Elle utilise une approche optimisée
 * avec gestion simplifiée des cas IEEE 754.
 *
 * @param hf Le demi-flottant dont on veut calculer 1/sqrt(x)
 * @return Le résultat de 1/sqrt(x) sous forme de demi-flottant
 */
uint16_t hf_rsqrt(uint16_t hf) {
    half_float result;
    half_float input  = decompose_half(hf);

    //Initialisation par défaut: +inf (gère automatiquement +/-0)
    result.sign = HF_ZERO_POS;
    result.exp  = HF_EXP_FULL;
    result.mant = 0;
   
    //Cas spéciaux IEEE 754
    if(is_nan(&input) || (input.sign && !is_zero(&input))) {
        //NaN ou x<0 (hors -0, incluant -inf) -> NaN
        result.mant = 1;
    }
    else if(is_infinity(&input)) {
        //rsqrt(+inf) -> +0
        result.exp  = 0;
    }
    else if(!is_zero(&input)) {
        //Calcul arithmétique normal: 1/sqrt(x)
        uint32_t root = 0;
        uint32_t value = (uint32_t)(input.mant << 15);
       
        //Exposant impair: ajustement de l'exposant à pair + mantisse
        if(input.exp & 1) {
            value <<= 1;
            input.exp--;
        }

        //Calcul de la racine carrée
        root = square_root(value);
              
        if(root > 0) {
            uint32_t one = 1U << 31;
            result.mant = (int32_t)(one / root);
            result.exp  = -(input.exp / 2) - 1;
            normalize_and_round(&result);
        }
    }
    //rsqrt(+/-0) -> +inf: déjà correct par l'initialisation

    return compose_half(&result);
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
        const int32_t THRESHOLD_MANT = (3 * (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT))) >> 1;
        int overflow = (input.exp > 3) || (input.exp == 3 && input.mant >= THRESHOLD_MANT);
        
        if(overflow) {
            //Overflow positif -> +inf, underflow négatif -> 0
            if(!input.sign) result.exp = HF_EXP_FULL; //exp(+large) = +inf
            //mant et exp déjà corrects pour exp(-large) = 0
        } else {
            int32_t x_fixed = input.sign ? -input.mant : input.mant;
            int32_t k_exp, r_fixed, frac;
            int index;
            
            //Ajustement selon l'exposant pour obtenir la vraie valeur
            x_fixed = (input.exp >= 0) ? (x_fixed << input.exp) : (x_fixed >> -input.exp);
            
            //Réduction à [0, ln(2)]: k = floor(x/ln(2)), r = x - k*ln(2)
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
    result.mant = 1 << (HF_MANT_BITS + HF_PRECISION_SHIFT);

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
                int result_negative = 0;
                int32_t ln_base_fixed, exp_fixed, exp_ln_fixed, k_exp, r_fixed;
                int idx_ln, idx_exp, frac;

                result.sign = HF_ZERO_POS;
                if(inputbase.sign && exp_int_part >= 0) result.sign = (exp_int_part & 1) ? HF_ZERO_NEG : HF_ZERO_POS;

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

                //CALCUL exp * ln(|base|)
                exp_fixed   = (inputexp.exp >= 0) ? (inputexp.mant << inputexp.exp) : (inputexp.mant >> -inputexp.exp);
        
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
                k_exp  = exp_ln_fixed / LNI_2;
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

                result.exp  = k_exp;

                //Inversion si exposant négatif (exp_ln_fixed < 0 à l'origine)
                if(result_negative && result.mant != 0) {
                    result.mant = (int32_t)((1U << 30) / result.mant);
                    result.exp  = -result.exp;
                }

                normalize_and_round(&result);
            }
        }
    }

    return compose_half(&result);
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
 * - Table LOW Q13 [0°, 75°]: Précision maximale (step=0.0002441)
 * - Table HIGH Q6 [75°, 90°]: Plage maximale (max=1024, saturation 0.4%)
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

    result.sign = HF_ZERO_POS;
    result.mant = 0;

    if(is_nan(&angle_hf) || is_infinity(&angle_hf)) {
        if(is_nan(&angle_hf)) {
            //Propager le signe original pour les NaN
            result.sign = angle_hf.sign;
        } else {
            //Pour les infinis, retourner NaN négatif selon la convention de la bibliothèque
            result.sign = HF_ZERO_NEG; //NaN négatif pour tan(+/-inf)
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
            result.sign = result.mant < 0 ? HF_ZERO_NEG : HF_ZERO_POS;
            result.mant = 0;
            result.exp = HF_EXP_FULL;
        } else if(result.mant < 0) {
            //Gestion du signe - conversion en valeur absolue
            result.sign = HF_ZERO_NEG;
            result.mant = -result.mant;
        }
        //result.sign reste à 0 par défaut si result.mant >= 0

        //Normalisation et arrondi du résultat final
        normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Calcule l'arc sinus d'un demi-flottant
 *
 * @param hf La valeur dont on cherche l'arc sinus (doit être dans [-1, 1])
 * @return L'angle en radians dans [-pi/2, pi/2]
 */

uint16_t hf_asin(uint16_t hf) {
    return asinus_shiftable(hf, 0UL);
}

/**
 * @brief Calcule l'arc cosinus d'un demi-flottant
 * 
 * @param hf La valeur dont on cherche l'arc cosinus (doit être dans [-1, 1])
 * @return L'angle en radians dans [0, pi]
 */
uint16_t hf_acos(uint16_t hf) {
    return asinus_shiftable(hf, ACOS_SHIFT);
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
 * @param fact Facteur de réduction: 0 pour réduction à 2pi, 1 pour réduction à pi
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

    //Initialisation par défaut: calcul trigonométrique normal
    result.sign = HF_ZERO_POS;
    result.exp = 0;

    //Gestion unifiée des cas spéciaux
    if(is_nan(&angle_hf)) {
        //Propager le signe original pour les NaN
        result.sign = angle_hf.sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(&angle_hf)) {
        //Pour les infinis, retourner NaN négatif selon la convention
        result.sign = HF_ZERO_NEG;
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
        if(norm & HF_MASK_SIGN) result.mant = -result.mant;
        if(result.mant < 0) {
            result.sign = HF_ZERO_NEG;
            result.mant = -result.mant;
        }

        //Normalisation et arrondi du résultat final
        normalize_and_round(&result);
    }

    return compose_half(&result);
}


/**
 * @brief Calcule l'arc sinus ou l'arc cosinus avec ajustement de phase
 *
 * Cette fonction calcule asin(x) ou acos(x) en utilisant la même table.
 * Pour acos: acos(x) = pi/2 - asin(x) pour x >= 0, pi/2 + asin(|x|) pour x < 0
 *
 * @param hf La valeur en format demi-précision (doit être dans [-1, 1])
 * @param shift Décalage à appliquer (0 pour asin, pi/2 en Q15 pour acos)
 * @return L'angle en radians (asin: [-pi/2, pi/2], acos: [0, pi])
 */
static uint16_t asinus_shiftable(uint16_t hf, uint32_t shift) {
    half_float result = {0, 0, 0};
    half_float input = decompose_half(hf);
   
    //Cas spéciaux: NaN
    if(is_nan(&input)) {
        result.sign = input.sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    }
    //Cas spéciaux: Infinity (hors domaine)
    else if(is_infinity(&input)) {
        result.sign = HF_ZERO_NEG;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    }
    //Calcul normal
    else {
        int idx0, idx1, frac;

        //Normaliser l'entrée
        int32_t norm = input.mant;
        norm = input.exp >= 0 ? norm << input.exp : norm >> -input.exp;
       
        //Vérifier le domaine: |x| <= 1.0
        if(norm > (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT))) {
            result.sign = HF_ZERO_NEG;
            result.exp = HF_EXP_FULL;
            result.mant = 1;
        }
        //Calculer via la table asin
        else {
            const int bits = HF_MANT_BITS + HF_PRECISION_SHIFT - ASIN_TABLE_BITS;

            idx0 = norm >> bits;
            idx1 = idx0 + 1;
            if(idx1 > ASIN_TABLE_SIZE) idx1 = ASIN_TABLE_SIZE;
           
            frac = norm & ((1 << (bits)) - 1);
           
            //Interpolation linéaire
            result.mant = (int32_t)asin_table[idx0];
            result.mant += (((int32_t)asin_table[idx1] - result.mant) * frac + (1 << ((bits) - 1))) >> (bits);
           
            //Appliquer le shift pour acos, ou le signe pour asin
            if(shift) {
                result.mant = input.sign ? shift + result.mant : shift - result.mant;
            } else {
                result.sign = input.sign;
            }
           
            normalize_and_round(&result);
        }
    }

    return compose_half(&result);
}

/**
 * @brief Calcule la racine carrée entière d'un entier non signé 32 bits
 * 
 * Cette fonction utilise un algorithme de décalage pour calculer
 * la racine carrée entière d'un entier non signé 32 bits.
 * 
 * @param value L'entier non signé dont on veut calculer la racine carrée
 * @return La racine carrée entière de value
 */
static uint32_t square_root(uint32_t value) {
    uint32_t root = 0;
    uint32_t rest = 0;
    int loop = 16;

    //Algorithme de racine carrée par décalage
    while(--loop >= 0) {
        uint32_t cond;
        value = ROL32(value, 2);           //Fait remonter les 2 bits suivants
        rest  = (rest << 2) + (value & 3); //Ajoute ces 2 bits au reste
        root  = (root << 2) + 1;           //Combine doublage et calcul du diviseur d’essai
        cond  = (rest >= root);            //0 ou 1 selon comparaison
        rest -= root & -cond;              //Retire trial si cond==1
        root  = (root >> 1) + cond;        //Met à jour la racine
    }

    return root;
}

/**
 * @brief Compare deux demi-flottants décomposés
 *
 * Compare deux valeurs demi-précision sous forme décomposée (sign, exp, mant)
 * et renvoie un code de comparaison générique utilisable par hf_cmp, hf_min et hf_max.
 *
 * Cette fonction ne manipule pas les valeurs encodées en binaire IEEE,
 * mais les structures internes `half_float` (signe, exposant, mantisse).
 *
 * @param input1 Pointeur vers le premier demi-flottant décomposé (const)
 * @param input2 Pointeur vers le second demi-flottant décomposé (const)
 * @return Code de comparaison :
 *         -2 si NaN détecté (comparaison impossible)
 *         -1 si *input1* < *input2*
 *          0 si *input1* == *input2*
 *         +1 si *input1* > *input2*
 */
static int compare_half(const half_float *input1, const half_float *input2) {
    int cmp = 0;

    if(is_nan(input1) || is_nan(input2)) {
        //Détection des NaN : toute comparaison devient indéfinie
        cmp = -2;
    }
    else if(input1->sign != input2->sign) {
        //Comparaison par signe (négatif < positif)
        cmp = input1->sign ? -1 : 1;
    }
    else if(input1->exp != input2->exp) {
        //Comparaison par exposant
        cmp = ((input1->exp < input2->exp) ^ input1->sign) ? -1 : 1;
    }
    else if(input1->mant != input2->mant) {
        //Comparaison par mantisse
        cmp = ((input1->mant < input2->mant) ^ input1->sign) ? -1 : 1;
    }

    //Sinon cmp reste 0 (égalité parfaite)
    return cmp;
}

/**
 * @brief Vérifie si un demi-flottant représente un entier exact.
 *
 * Retourne :
 *  - valeur entière absolue si le demi-flottant est un entier exact
 *  - -1 si non-entier, NaN, +/-Inf ou subnormal
 */
static int check_int_half(const half_float *hf) {
    int result = -1;

    //Rejeter NaN, Inf et subnormaux (exp == -bias et mant != 0)
    if (!is_nan(hf) && !is_infinity(hf)) {
        if (hf->mant == 0 && hf->exp == -HF_EXP_BIAS) {
            //Cas spécial +/-0
            result = 0;
        } else if (!(hf->exp == -HF_EXP_BIAS && hf->mant != 0)) {
            //Calculer combien de bits fractionnaires existent
            int shift = (HF_MANT_BITS + HF_PRECISION_SHIFT) - hf->exp;

            if (shift <= 0) {
                //Exposant trop grand : valeur énorme, forcément un entier pair
                result = 0;
            } else if (shift <= (HF_MANT_BITS + HF_PRECISION_SHIFT)) {
                //Vérifier si les bits fractionnaires sont à 0
                uint32_t mant = hf->mant;

                if (((mant >> shift) << shift) == mant) {
                    result = (int)(mant >> shift);
                }
            }
        }
    }

    return result;
}
