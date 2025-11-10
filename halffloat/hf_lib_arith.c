/**
 * @file hf_lib_arith.c
 * @brief Implémentation des opérations arithmétiques pour Half-Float
 * 
 * Module contenant l'implémentation de toutes les opérations arithmétiques
 * de base, les fonctions de racines et les opérations avancées.
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#include "hf_lib_arith.h"

//Definition de la macro ROL32
#define ROL32(x, n) ((x<<n) | (x>>(32-n)))

//Déclaration des helpers statiques
static uint32_t square_root(uint32_t value);


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
    //On utilise l'opération XOR avec HF_MASK_SIGN pour inverser uniquement le bit de signe
    return hf ^ HF_MASK_SIGN;
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
            result.mant = (int32_t)(mult_result >> HF_MANT_SHIFT);
            
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
    } else if(is_infinity(&input1) || is_zero(&input2)) {
        //Inf / valeur finie = Inf ou Fini / 0 = Inf
        result.mant = 0;
    } else if(is_infinity(&input2) || is_zero(&input1)) {
        //Fini / Inf = 0 ou 0 / Fini = 0
        result.exp = -HF_EXP_BIAS;
        result.mant = 0;
    } else if(!(is_zero(&input1) && is_zero(&input2))) {
        //Division arithmétique normale
        uint32_t dividend = input1.mant << HF_MANT_SHIFT;
        
        result.exp = input1.exp - input2.exp;
        result.mant = dividend / input2.mant;
        if(dividend % input2.mant) result.mant |= 1;

        normalize_and_round(&result);
    }
    //Gestion du NaN: valeurs déjà bonnes par défaut

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
        uint32_t dividend = HF_MANT_NORM_MIN << HF_MANT_SHIFT;
        
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
        uint32_t value = 0;

        //Si l'entrée est subnormale, la mantisse doit d'abord être normalisée
        //pour que l'algorithme de racine carrée voie le bit implicite.
        normalize_denormalized_mantissa(&input);
        value = (uint32_t)(input.mant << 15);
        
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
        uint32_t value = 0;

        //Normaliser les subnormales avant le calcul pour éviter des
        //écarts importants sur les très petites valeurs.
        normalize_denormalized_mantissa(&input);
        value = (uint32_t)(input.mant << 15);
       
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
 * @brief Calcule la racine cubique d'un demi-flottant
 *
 * @param hf Le demi-flottant dont on veut calculer la racine cubique
 * @return La racine cubique de hf
 */
uint16_t hf_cbrt(uint16_t hf) {
    (void)hf;
    return HF_NAN;
}

/**
 * @brief Multiplication-addition fusionnée (FMA)
 *
 * Calcule (hfa * hfb) + hfc avec une seule opération d'arrondi.
 *
 * @param hfa Premier facteur
 * @param hfb Second facteur
 * @param hfc Valeur à ajouter
 * @return Le résultat de (hfa * hfb) + hfc
 */
uint16_t hf_fma(uint16_t hfa, uint16_t hfb, uint16_t hfc) {
    (void)hfa; (void)hfb; (void)hfc;
    return HF_NAN;
}

/**
 * @brief Calcule l'hypoténuse (sqrt(x^2 + y^2))
 *
 * @param hfx Coordonnée X
 * @param hfy Coordonnée Y
 * @return La longueur de l'hypoténuse
 */
uint16_t hf_hypot(uint16_t hfx, uint16_t hfy) {
    (void)hfx; (void)hfy;
    return HF_NAN;
}

//Opérations modulo - stubs pour l'instant
/**
 * @brief Calcule le reste de la division flottante
 *
 * @param hfx Dividende
 * @param hfy Diviseur
 * @return Le reste de hfx / hfy
 */
uint16_t hf_fmod(uint16_t hfx, uint16_t hfy) {
    (void)hfx; (void)hfy;
    return HF_NAN;
}

/**
 * @brief Calcule le reste de la division avec arrondi
 *
 * @param hfx Dividende
 * @param hfy Diviseur
 * @return Le reste arrondi de hfx / hfy
 */
uint16_t hf_remainder(uint16_t hfx, uint16_t hfy) {
    (void)hfx; (void)hfy;
    return HF_NAN;
}

/**
 * @brief Calcule le reste et le quotient de la division
 *
 * @param hfx Dividende
 * @param hfy Diviseur
 * @param quo Pointeur vers le quotient (optionnel)
 * @return Le reste de hfx / hfy
 */
uint16_t hf_remquo(uint16_t hfx, uint16_t hfy, int *quo) {
    (void)hfx; (void)hfy; (void)quo;
    return HF_NAN;
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
        root  = (root << 2) + 1;           //Combine doublage et calcul du diviseur d'essai
        cond  = (rest >= root);            //0 ou 1 selon comparaison
        rest -= root & -cond;              //Retire trial si cond==1
        root  = (root >> 1) + cond;        //Met à jour la racine
    }

    return root;
}
