#include "hf_common.h"
#include "hf_lib.h"
#include "hf_precalc.h"

// Fonctions mathématiques de base
uint16_t hf_int(uint16_t hf);
uint16_t hf_abs(uint16_t hf);
uint16_t hf_neg(uint16_t hf);

// Opérations arithmétiques
uint16_t hf_add(uint16_t float1, uint16_t float2);
uint16_t hf_mul(uint16_t hf1, uint16_t hf2);
uint16_t hf_div(uint16_t hf1, uint16_t hf2);
uint16_t hf_sqrt(uint16_t hf);

// Fonctions transcendantes
uint16_t hf_ln(uint16_t hf);
uint16_t hf_exp(uint16_t hf);
uint16_t hf_pow(uint16_t base, uint16_t exp);

// Fonctions trigonométriques
uint16_t hf_sin(uint16_t angle);
uint16_t hf_cos(uint16_t angle);
uint16_t hf_tan(uint16_t angle);


/**
 * @brief Récupère la partie entière d'un demi-flottant
 * 
 * Cette fonction extrait la partie entiére d'un nombre représenté en demi-flottant (half-float).
 * Elle gère les cas spéciaux tels que NaN, l'infini, et zéro, et ajuste l'exposant en conséquence.
 *
 * @param hf Le demi-flottant dont on veut récupérer la partie entiére
 * @return La partie entiére sous forme de demi-flottant
 */
uint16_t hf_int(uint16_t hf) {
    half_float result = decompose_half(hf);

    //Traitement uniquement pour les nombres non spéciaux
    if (!is_nan(result) && !is_infinity(result) && !is_zero(result)) {
        //Normaliser la mantisse pour avoir un exposant effectif de 0
        uint32_t value = result.exp >= 0 ? result.mant << result.exp : result.mant >> -result.exp;

        //Convertir la valeur entiére en demi-flottant
    result.mant = value & ~(1U << (HF_MANT_BITS + HF_PRECISION_SHIFT - 1));
        result.exp = 0;

        //Normaliser et arrondir le résultat
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
    // La valeur absolue est obtenue en mettant le bit de signe à 0
    // On utilise un masque pour conserver tous les bits sauf le bit de signe
    return hf & ~(1 << HF_SIGN_BITS);
}

/**
 * @brief Calcule l'opposé (négation) d'un demi-flottant
 *
 * Cette fonction retourne l'opposè d'un nombre en format demi-flottant.
 * Elle inverse simplement le bit de signe, laissant l'exposant et la mantisse inchangés.
 *
 * @param hf Le demi-flottant dont on veut calculer l'opposè
 * @return L'opposè de hf sous forme de demi-flottant
 */
uint16_t hf_neg(uint16_t hf) {
    // L'opposè est obtenu en inversant le bit de signe
    // On utilise l'opération XOR avec 0x8000 pour inverser uniquement le bit de signe
    return hf ^ (1 << HF_SIGN_BITS);
}

/**
 * @brief Additionne deux demi-flottants
 * 
 * @param float1 Premier demi-flottant
 * @param float2 Second demi-flottant
 * @return Le résultat de l'addition sous forme de demi-flottant
 */
uint16_t hf_add(uint16_t float1, uint16_t float2) {
    half_float result;
    half_float half1 = decompose_half(float1);
    half_float half2 = decompose_half(float2);

    // Initialisation partielle de result
    result.sign = 0;

    // Gestion des cas spéciaux
    if (is_nan(half1) || is_nan(half2)) {
        // Si l'un des opérandes est NaN
        result.exp = HF_EXP_FULL;
        result.mant = 1;  // NaN
    } else if (is_infinity(half1) && is_infinity(half2)) {
        if (half1.sign != half2.sign) {
            // Infini positif + Infini négatif = NaN
            result.exp = HF_EXP_FULL;
            result.mant = 1;  // NaN
        } else {
            // Infini + Infini = Infini
            result = half1;  // Ou half2, les deux étant égaux dans ce cas
        }
    } else if (is_infinity(half1)) {
        // Si half1 est infini
        result = half1;
    } else if (is_infinity(half2)) {
        // Si half2 est infini
        result = half2;
    } else {
        // Aligner les mantisses pour l'addition
        align_mantissas(&half1, &half2);

        // Déterminer l'exposant du résultat
        result.exp = half1.exp;

        // Convertir les mantisses en valeurs signées puis additionner
        if (half1.sign) half1.mant = -half1.mant;
        if (half2.sign) half2.mant = -half2.mant;
        result.mant = half1.mant + half2.mant;

        // Déterminer le signe du résultat
        if (result.mant < 0) {
            result.mant = -result.mant;
            result.sign = 1 << HF_SIGN_BITS;
        }

        // Normaliser et arrondir le résultat
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
    half_float a = decompose_half(hf1);
    half_float b = decompose_half(hf2);

    // Calcul du signe du résultat
    result.sign = a.sign ^ b.sign;

    // Gestion des cas spéciaux
    if (is_nan(a) || is_nan(b)) {
        // Si l'un des opérandes est NaN
        result.exp = HF_EXP_FULL;
        result.mant = 1; // NaN
    } else if ((is_infinity(a) && is_zero(b)) ||
               (is_infinity(b) && is_zero(a))) {
        // Inf * 0 = NaN
        result.exp = HF_EXP_FULL;
        result.mant = 1; // NaN
    } else if (is_zero(a) || is_zero(b)) {
        // Si l'un des opérandes est zéro
        result.exp = -HF_EXP_BIAS;
        result.mant = 0;
    } else if (is_infinity(a) || is_infinity(b)) {
        // Si l'un des opérandes est infini
        result.exp = HF_EXP_FULL;
        result.mant = 0; // Infini
    } else {
        // Multiplication normale
        result.exp = a.exp + b.exp;
        result.mant = (uint32_t)(a.mant * b.mant) >> (HF_MANT_BITS + HF_PRECISION_SHIFT);

        // Normalisation et arrondi
        normalize_and_round(&result);
    }

    // Composition du résultat final
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
    half_float a = decompose_half(hf1);
    half_float b = decompose_half(hf2);

    // Calcul du signe du résultat
    result.sign = a.sign ^ b.sign;

    // Gestion des cas spéciaux
    if (is_nan(a) || is_nan(b)) {
        // Si l'un des opérandes est NaN
        result.exp = HF_EXP_FULL;
        result.mant = 1; // NaN
    } else if (is_infinity(a)) {
        // Si le dividende est infini
        if (is_infinity(b)) {
            // Inf / Inf = NaN
            result.exp = HF_EXP_FULL;
            result.mant = 1; // NaN
        } else {
            // Inf / valeur non infinie = Inf
            result.exp = HF_EXP_FULL;
            result.mant = 0; // Infini
        }
    } else if (is_infinity(b)) {
        // Si le diviseur est infini
        result.exp = HF_EXP_BIAS;
        result.mant = 0; // 0
    } else if (is_zero(a)) {
       // Si le dividende est zéro
        if (is_zero(b)) {
            // 0 / 0 = NaN
            result.exp = HF_EXP_FULL;
            result.mant = 1; // NaN
        } else {
            // 0 / non-zéro = 0
            result.exp = -HF_EXP_BIAS;
            result.mant = 0; // 0
        }
    } else if (is_zero(b)) {
        // Si le diviseur est zéro
        result.exp = HF_EXP_FULL;
        result.mant = 0; // Infini
    } else {
        // Division normale
        result.exp = a.exp - b.exp;
        result.mant = (a.mant << (HF_MANT_BITS + HF_PRECISION_SHIFT)) / b.mant;

        // Normalisation et arrondi
        normalize_and_round(&result);
    }

    // Composition du résultat final
    uint16_t t= compose_half(result);
    return t;
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

    result.sign = 0;  //La racine carrée est toujours positive

    //Gestion des cas spéciaux
    if(is_zero(input)) {
        result=input;
    } else if (is_nan(input) || input.sign) {
        result.exp = HF_EXP_FULL;
        result.mant = 1;  //NaN
    } else if (is_infinity(input)) {
        result.exp = HF_EXP_FULL;
        result.mant = 0;  //Infini
    } else {
        uint32_t sqrt = 0;
        uint32_t quot = 0;
        int32_t loop = 16;  //Nombre d'itérations pour un nombre non signé 32 bits
        uint32_t value = input.mant << 1;  //On décale la mantisse pour la positionner entiérement sur les bits de poids forts

        //Normaliser la mantisse pour avoir un exposant effectif de 0
        value = input.exp>=0 ? value<<input.exp : value>>-input.exp;

        //Algorithme de racine carrée par décalage
        while (--loop >= 0) {
            quot = (quot << 2) + ((value >> 30) & 3);
            sqrt <<= 1;
            value <<= 2;

            if (quot >= sqrt + 1) {
                quot -= ++sqrt;
                sqrt++;
            }
        }

        //Ajuster le résultat
        result.exp = 0;
        result.mant = sqrt << 6; //Equivaux à 1 décalage à droite + 8 décalage à gauche moins 1)

        //Normalisation et arrondi
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
    
    result.sign = 0;

    //Gestion des cas 
    if(is_zero(input)) {
        result.exp = HF_EXP_FULL; //-inf
        result.mant = 0;
        result.sign = 0x8000;
    } else if (is_nan(input) || input.sign) {
        result.exp = HF_EXP_FULL; //NaN
        result.mant = 1;
    } else if (is_infinity(input)) {
        result = input;
    } else {
        int idx;
        
        //Si la mantisse est déréglée, on la normalise
        if (input.exp == -HF_EXP_BIAS) {
            input.exp++;
            while(input.mant < (1<<(HF_MANT_BITS+HF_PRECISION_SHIFT))) {
                input.mant<<=1;
                input.exp--;
            }
        }

        //Calcul de ln(x) = ln(2^exp * (1 + mant/1024)) = exp*ln(2) + ln(1 + mant/(sizeof(log2_table)/sizeof(uint16_t))
        idx=(input.mant >> HF_PRECISION_SHIFT) & 0x3ff;
        result.mant = (input.exp * LNI_2 + ln_table[idx]) >> 1;
        result.exp=0;

        //Définition du signe
        if(result.mant<0) {
            result.mant=-result.mant;
            result.sign=0x8000;
        }

        //Normalisation et arrondi
        normalize_and_round(&result);
    }

    return compose_half(result);
}

/**
 * @brief Calcule l'exponentielle d'un demi-flottant
 * 
 * @param hf Le demi-flottant dont on veut calculer l'exponentielle
 * @return L'exponentielle du demi-flottant en entrée
 */
uint16_t hf_exp(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);
    
    result.sign = 0; //L'exponentielle est toujours positive

    //Gestion des cas spéciaux
    if (is_nan(input)) {
        result = input;
    } else if(is_infinity(input)) {
        result.mant = 0;
        result.exp = input.sign?HF_EXP_BIAS:HF_EXP_FULL; //0 ou +inf
    } else if (input.exp > 3) {
        //Si valeur > 2^3, alors +inf
        result.exp = HF_EXP_FULL;
        result.mant = 0; //Infini
    } else {
        int32_t k_exp;
        int32_t r_frac;
        int32_t fact;
        int index;

        //Normalisation de la mantisse pour avoir un exposant effectif de 0, des bits de précision et le signe
        result.mant = input.sign != 0 ? -input.mant : input.mant;
        result.mant <<= 9;
        result.mant = input.exp >= 0 ? result.mant << input.exp : result.mant >> -input.exp;

        //Calcul de l'exposant et élimination de la partie fractionnaire
        k_exp = result.mant / LNI_2;
        k_exp -= k_exp & ((1 << EXP_PRECISION_SHIFT) - 1); //Équivalent à floor(k_exp)
        r_frac = (result.mant - k_exp * LNI_2) * 128 / LNI_2;
    
        //Index dans la table et calcul du facteur d'interpolation
        index = (r_frac << EXP_TABLE_SIZE_SHIFT) >> EXP_TABLE_PRECISION;
        fact = (r_frac << EXP_TABLE_SIZE_SHIFT) - (index << EXP_TABLE_PRECISION);
    
        //Interpolation pour obtenir la mantisse
        result.mant = exp_table[index];
        result.mant += ((exp_table[index + 1] - result.mant) * fact) >> EXP_TABLE_PRECISION;
        result.mant=0x8000+(result.mant>>1);
    
        //Calcul du résultat final
        result.exp = (k_exp >> EXP_PRECISION_SHIFT);
        
        //Normalisation et arrondi
        normalize_and_round(&result); //Normalisation et arrondi
    }

    return compose_half(result);
}

/**
 * @brief Calcule la puissance d'un demi-flottant
 * 
 * Cette fonction calcule base^exp où base et exp sont des demi-flottants (half-float).
 * Elle utilise la formule base^exp = e^(exp * ln(base)) pour effectuer le calcul.
 *
 * @param base Le demi-flottant représentant la base
 * @param exp Le demi-flottant représentant l'exposant
 * @return Le résultat de base^exp sous forme de demi-flottant
 */
uint16_t hf_pow(uint16_t base, uint16_t exp) {
    half_float result;
    half_float base_hf = decompose_half(base);
    half_float exp_hf = decompose_half(exp);
    
    result.sign = 0;

    // Gestion des cas spéciaux
    if (is_nan(base_hf) || is_nan(exp_hf)) {
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if (is_zero(base_hf)) {
        if (is_zero(exp_hf)) {
            result.exp = HF_EXP_BIAS;
            result.mant = 0;
        } else if (exp_hf.sign == 0) {
            result = base_hf;
        } else {
            result.exp = HF_EXP_FULL;
            result.mant = 0;
        }
    } else if (is_infinity(base_hf)) {
        if (is_zero(exp_hf)) {
            result.exp = HF_EXP_BIAS;
            result.mant = 0;
        } else if (exp_hf.sign == 0) {
            result = base_hf;
        } else {
            result.exp = 0;
            result.mant = 0;
        }
    } else if (base_hf.sign && (exp_hf.exp != HF_EXP_BIAS || exp_hf.mant != 0)) {
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else {
        int idx;
        int32_t k_exp;
        int32_t r_frac;
        int32_t fact;

        // 1. Calcul de ln(base)
        if (base_hf.exp == -HF_EXP_BIAS) {
            // Normalisation de la mantisse pour les nombres déréglés
            base_hf.exp++;
            while(base_hf.mant < (1<<(HF_MANT_BITS+HF_PRECISION_SHIFT))) {
                base_hf.mant<<=1;
                base_hf.exp--;
            }
        }       
        idx = (base_hf.mant >> HF_PRECISION_SHIFT) & 0x3ff;
        result.mant = base_hf.exp * LNI_2 + ln_table[idx];
        result.exp = 0;
    
        // 2. Multiplication par l'exposant : exp * ln(base)
        if(result.mant < 0) {
            result.mant = -result.mant;
            result.sign = 0x8000;
        }
    
        result.mant = (result.mant * (exp_hf.mant>>5)) >> HF_MANT_BITS;
        result.exp = exp_hf.exp;
        result.sign ^= (exp_hf.sign ? 0x8000 : 0);
    
        // 3. Calcul de e^(résultat de l'étape 2)
        result.mant = result.sign ? -result.mant : result.mant;
        result.mant <<= 8;
        result.mant = result.exp >= 0 ? result.mant << result.exp : result.mant >> -result.exp;
    
        k_exp = result.mant / LNI_2;
        k_exp -= k_exp & ((1 << EXP_PRECISION_SHIFT) - 1);
        r_frac = (result.mant - k_exp * LNI_2) * 128 / LNI_2;
    
        idx = (r_frac << EXP_TABLE_SIZE_SHIFT) >> EXP_TABLE_PRECISION;
        fact = (r_frac << EXP_TABLE_SIZE_SHIFT) - (idx << EXP_TABLE_PRECISION);
    
        result.mant = exp_table[idx];
        result.mant += ((exp_table[idx + 1] - result.mant) * fact) >> EXP_TABLE_PRECISION;
        result.mant=0x8000+(result.mant>>1);
   
        result.exp = (k_exp >> EXP_PRECISION_SHIFT);
        result.sign = 0;
    
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
 * @param angle L'angle en radians, représenté en format demi-précision.
 * @return Le sinus de l'angle en format demi-précision.
 * 
 * @see sinus_shiftable
 */
uint16_t hf_sin(uint16_t angle) {
    return sinus_shiftable(angle, 0);
}

/**
 * @brief Calcule le cosinus d'un angle exprimé en demi-précision.
 * 
 * Cette fonction est un wrapper autour de sinus_shiftable, spécialisé pour
 * le calcul du cosinus. Elle applique un décalage de phase de ?/2 (16384 en
 * représentation fixe) à l'angle d'entrée pour transformer le sinus en cosinus.
 * La fonction utilise une table de recherche et une interpolation linéaire
 * pour le calcul. Les cas spéciaux tels que NaN et l'infini sont gérés par
 * la fonction sous-jacente.
 *
 * @param angle L'angle en radians, représenté en format demi-précision.
 * @return Le cosinus de l'angle en format demi-précision.
 * 
 * @see sinus_shiftable
 */
uint16_t hf_cos(uint16_t angle) {
    return sinus_shiftable(angle, 16384);
}

/**
 * @brief Calcule la tangente d'un angle en demi-précision.
 *
 * Cette fonction calcule la tangente d'un angle donné. Elle utilise une
 * table de recherche et une interpolation linéaire pour améliorer la précision.
 *
 * @param angle L'angle en radians, représenté en format demi-précision.
 * @return La valeur de la tangente de l'angle en format demi-précision.
 */
uint16_t hf_tan(uint16_t angle) {
    half_float result;
    half_float angle_hf = decompose_half(angle);

    result.sign = 0;
    result.mant = 0;

    if (is_nan(angle_hf) || is_infinity(angle_hf)) {
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else {
        int32_t norm;
        int32_t delta;
        int idx0, idx1;

        result.exp = 0;

        // Normalisation de la mantisse pour obtenir une représentation fixe de l'angle
        angle_hf.mant = angle_hf.exp >= 0 ? angle_hf.mant << angle_hf.exp : angle_hf.mant >> -angle_hf.exp;

        // Rédution de l'angle à l'intervalle [0, 65535]
        norm = (int32_t)reduce_radian_uword(angle_hf.mant, 1);
        if (angle_hf.sign) norm = 65536 - norm;

        // Calcul de la différence absolue par rapport à ?/2
        delta = 32768 - norm;
        if (delta < 0)  delta = -delta;

        // Décision d'utiliser la méthode affine ou la table
        if (delta > LIMIT_TAN_AFFINE) {   
            //Calcul des indices pour l'interpolation dans la table de tangente
            idx0 = (norm & 0x7fff) >> 6;
            idx1 = idx0 + 1;

            //Ajustement des indices pour les quadrants descendants (1 et 3)
            if (norm & 0x8000) {
                idx0 = TAN_TABLE_SIZE - idx0;
                idx1 = idx0 - 1;
            }
    
            //Interpolation linéaire entre les valeurs de la table
            result.mant = (int32_t)tan_table[idx0];
            result.mant += (((int32_t)tan_table[idx1] - result.mant) * (norm & 0x3f) + 0x3f) >> 6;
    
            //Inversion du signe pour le bras descendant
            if (norm & 0x8000) result.mant = -result.mant;
    
            //Conversion du résultat en format demi-précision
            if (result.mant < 0) {
                result.sign = 0x8000;
                result.mant = -result.mant;
            }
        } else {
            //Ajustement du signe en utilisant le bit de signe de norm
            result.sign = norm & 0x8000;

            //Approximation affine quand "angle" s'approche de PI/2
            //Calcul en virgule fixe : tan(angle) ? 1 / (PI/2 - angle)
            if (delta != 0) {
                result.mant = ((1 << 15) * 32768) / delta; // virgule fixe
            } else {
                //Infini
                result.mant = 0;
                result.exp=HF_EXP_BIAS+1;
            }
        }

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
 * dans l'intervalle [0, 2?) ou [0, ?) selon la valeur de 'fact', tout en
 * maintenant une précision constante de 1/65536.
 *
 * @param angle_rad_fixed Angle en radians en format virgule fixe (1.31)
 * @param fact Facteur de réduction : 0 pour réduction à 2?, 1 pour réduction à ?
 * @return Angle réduit sur 16 bits, représentant :
 *         - [0, 2?) avec une précision de 1/65536 si fact = 0
 *         - [0, ?) avec une précision de 1/65536 si fact = 1
 *         Dans les deux cas, la valeur de retour couvre l'intégralité des 65536 valeurs possibles.
 */
uint16_t reduce_radian_uword(uint32_t angle_rad_fixed, int fact) {
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
 * de ?/2) ou d'autres fonctions trigonométriques décalées. La fonction utilise une
 * table de recherche et une interpolation linéaire pour améliorer la précision.
 *
 * @param angle L'angle en radians, représenté en format demi-précision.
 * @param shift Le décalage de phase à appliquer (0 pour sinus, 16384 pour cosinus).
 * @return La valeur du sinus (ou cosinus) de l'angle en format demi-précision.
 */
uint16_t sinus_shiftable(uint16_t angle, uint16_t shift) {
    half_float result;
    half_float angle_hf = decompose_half(angle);

    result.sign = 0;

    //Gestion des cas spéciaux : NaN et infini
    if (is_nan(angle_hf) || is_infinity(angle_hf)) {
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else {
        int32_t norm;
        int idx0, idx1;

        //Normalisation de la mantisse pour obtenir une représentation fixe de l'angle
        norm = angle_hf.mant;
        norm = angle_hf.exp >= 0 ? norm << angle_hf.exp : norm >> -angle_hf.exp;

        //Rédution de l'angle à l'intervalle [0, 65535] et application du décalage
        norm = (int32_t)reduce_radian_uword(norm, 0);
        if (angle_hf.sign) norm = 65536 - norm;
        norm = (norm + shift) & 0xffff;
        
        //Calcul des indices pour l'interpolation dans la table de sinus
        idx0 = (norm & 0x3fff) >> 5;
        idx1 = idx0 + 1;

        //Ajustement des indices pour les quadrants descendants (1 et 3)
        if (norm & 0x4000) {
            idx0 = SIN_TABLE_SIZE - idx0;
            idx1 = idx0 - 1;
        }

        //Interpolation linéaire entre les valeurs de la table
        result.mant = (int32_t)sin_table[idx0];
        result.mant += (((int32_t)sin_table[idx1] - result.mant) * (norm & 0x1f) + 0x1f) >> 5;

        //Inversion du signe pour les quadrants négatifs (2 et 3)
        if (norm & 0x8000) result.mant = -result.mant;

        //Conversion du résultat en format demi-précision
        if (result.mant < 0) {
            result.sign = 0x8000;
            result.mant = -result.mant;
        }

        result.exp = 0;

        //Normalisation et arrondi du résultat final
        normalize_and_round(&result);
    }

    return compose_half(result);
}

// Toutes les fonctions et commentaires sont maintenant en UTF-8, corrigeant les caractères corrompus.
