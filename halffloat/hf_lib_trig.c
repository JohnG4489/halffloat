/**
 * @file hf_lib_trig.c
 * @brief Implémentation des fonctions trigonométriques pour Half-Float
 * 
 * Module contenant l'implémentation des fonctions trigonométriques
 * et hyperboliques pour le format Half-Float IEEE 754.
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#include "hf_lib_trig.h"
#include "hf_lib_common.h"
#include "hf_lib_arith.h" //Pour hf_sub, hf_abs, hf_cmp
#include "hf_lib_exp.h" //Pour hf_exp utilisé dans hf_sinh
#include "hf_lib_misc.h"

//Déclarations des fonctions static (définies en fin de fichier)
static uint16_t sinus_shiftable(uint16_t hf, uint16_t shift);
static uint16_t asinus_shiftable(uint16_t hf, uint32_t shift);
static void cosh_sinh_helper(int32_t x_abs, half_float *result, int32_t *exp_pos, int32_t *exp_neg);

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
    const uint16_t COS_SHIFT = 16384;
    return sinus_shiftable(hfangle, COS_SHIFT);
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
    result.exp = 0;

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
        int32_t norm, input_norm, range_norm, frac, value;
        int interp_index;

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
            qshift = 9; //Q6->Q15
            table_ptr = tan_table_high;
        }
        
        //Interpolation via table_interpolate sur la table tan
        interp_index = (input_norm * TAN_DUAL_TABLE_SIZE) / range_norm;
        frac = ((input_norm * TAN_DUAL_TABLE_SIZE) % range_norm << 7) / range_norm;

        //la table contient TAN_DUAL_TABLE_SIZE+1 entrées pour permettre
        //un accès sûr à idx1 lors de l'interpolation
        //on passe donc la longueur complète pour que table_interpolate clamp correctement
        value = table_interpolate(table_ptr, TAN_DUAL_TABLE_SIZE + 1, (interp_index << 7) | frac, 7);
        value <<= qshift;
        result.mant = value;
        
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
 * @brief Fonction interne pour calculer asin et acos avec décalage
 * 
 * @param hf La valeur dont on cherche l'arc sinus/cosinus (doit être dans [-1, 1])
 * @param shift Décalage à appliquer (0 pour asin, pi/2 en Q15 pour acos)
 * @return L'angle en radians (asin: [-pi/2, pi/2], acos: [0, pi])
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
 * @brief Calcule l'arc tangente d'un demi-flottant
 * 
 * Utilise la table précalculée atan_table avec interpolation linéaire.
 * Pour |x| > 1, applique la réduction atan(x) = pi/2 - atan(1/x).
 * 
 * @param hf La valeur dont on cherche l'arc tangente
 * @return L'angle en radians dans [-pi/2, pi/2]
 */
uint16_t hf_atan(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    result.sign = input.sign;
    result.exp = 0;
    result.mant = 0;

    //Cas spéciaux: NaN
    if(is_nan(&input)) {
        //Propager le signe pour cohérence avec hf_asin
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    }
    //Cas spéciaux: Infini -> +/-pi/2
    else if(is_infinity(&input)) {
        result.mant = PI_1_2_Q15;
        normalize_and_round(&result);
    }
    else {
        int use_complement, value;
        int32_t norm = input.mant;
        int32_t ratio = 0;

        //Normaliser la mantisse selon l'exposant
        norm = input.exp >= 0 ? norm << input.exp : norm >> -input.exp;
        if(norm < 0) norm = -norm;

        //Déterminer si |x| > 1.0 pour réduction atan(x) = pi/2 - atan(1/x)
        use_complement = (norm > HF_MANT_NORM_MIN) ? 1 : 0;

        //Construire ratio en Q15 dans [0,1]
        if(norm != 0) {
            if(use_complement) {
                //ratio = 1/|x|
                ratio = (int32_t)(((int64_t)(1 << (HF_MANT_SHIFT + 15))) / norm);
                if(ratio > (1 << 15)) ratio = (1 << 15);
            } else {
                //ratio = |x|
                ratio = norm << (15 - HF_MANT_SHIFT);
                if(ratio > (1 << 15)) ratio = (1 << 15);
            }
        }

        //Interpolation via table_interpolate sur atan_table
        value = table_interpolate(atan_table, ATAN_TABLE_SIZE, ratio, ATAN_INDEX_SHIFT);

        //Complément si |x|>1 : angle = pi/2 - atan(1/|x|)
        if(use_complement) value = PI_1_2_Q15 - value;

        //Appliquer le signe (atan est impair)
        result.mant = value;

        normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Calcule l'arc tangente à deux arguments (atan2)
 * 
 * Calcule l'angle θ tel que tan(θ) = y/x, en utilisant les signes
 * pour déterminer le quadrant correct.
 * 
 * @param hfy Coordonnée Y
 * @param hfx Coordonnée X
 * @return Angle en radians dans [-pi, pi]
 */
uint16_t hf_atan2(uint16_t hfy, uint16_t hfx) {
    half_float result;
    half_float inputy = decompose_half(hfy);
    half_float inputx = decompose_half(hfx);
    
    result.sign = inputy.sign;
    result.exp = 0;
    result.mant = 0;
    
    //Cas spéciaux: NaN
    if(is_nan(&inputy) || is_nan(&inputx)) {
        result.sign = HF_ZERO_NEG;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    }
    //Cas: infinités
    else if(is_infinity(&inputy) || is_infinity(&inputx)) {
        if(is_infinity(&inputy) && is_infinity(&inputx)) {
            result.mant = inputx.sign ? PI_3_4_Q15 : PI_1_4_Q15;
        } else if(is_infinity(&inputy)) {
            result.mant = PI_1_2_Q15;
        } else if(inputx.sign) {
            result.mant = PI_Q15;
        }
        //inputx positif -> 0
        normalize_and_round(&result);
    }
    //Calcul normal (inclut atan2(0,0) qui retourne 0)
    else if(!(is_zero(&inputy) && is_zero(&inputx))) {
        half_float *numerator = &inputy;
        half_float *denominator = &inputx;
        int32_t exp_diff = inputy.exp - inputx.exp;
        int use_complement = (exp_diff > 0 || (exp_diff == 0 && inputy.mant > inputx.mant));
        int32_t ratio;
        
        if (use_complement) {
            exp_diff = -exp_diff;
            numerator = &inputx;
            denominator = &inputy;
        }
        
        //Calcul du ratio en Q15
        ratio = (int32_t)(((int64_t)numerator->mant << (Q15_SHIFT + exp_diff)) / denominator->mant);
        ratio = (ratio < 0) ? 0 : ((ratio > Q15_ONE) ? Q15_ONE : ratio);
        
        //Interpolation via table_interpolate sur atan_table
        result.mant = table_interpolate(atan_table, ATAN_TABLE_SIZE, ratio, ATAN_INDEX_SHIFT);
        if(use_complement) result.mant = PI_1_2_Q15 - result.mant;
        if(inputx.sign) result.mant = PI_Q15 - result.mant;
        
        normalize_and_round(&result);
    }
    
    return compose_half(&result);
}

/**
 * @brief Calcule la fonction hyperbolique sinh(x) en demi-précision.
 *
 * Utilise la table précalculée exp_table via exp_fixed() :
 * sinh(x) = (e^x - e^-x) / 2
 * 
 * L'overflow vers +/-inf est géré naturellement par normalize_and_round().
 * Les cas spéciaux IEEE 754 (NaN, +/-inf) sont traités en amont.
 *
 * @param hf Demi-flottant d'entrée (x)
 * @return Demi-flottant représentant sinh(x)
 */
uint16_t hf_sinh(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    //Initialisation optimisée pour les cas spéciaux
    result.sign = input.sign;
    result.exp = HF_EXP_FULL;
    result.mant = 0;

    //Gestion unifiée des cas spéciaux
    if(is_nan(&input)) {
        //NaN: propager le signe, mantisse non nulle
        result.mant = 1;
    }
    //Subnormaux:preserver les subnormaux (sinh(x) ~= x pour tres petits x)
    else if(is_subnormal(&input)) {
        //subnormal non nul -> renvoyer l'entree
        result = input;
    } else if(!is_infinity(&input)) {
        //Calcul via table exp : sinh(x) = sign(x) * (e^|x| - e^(-|x|)) / 2
        int32_t x_abs, exp_pos, exp_neg, diff;

        //Préparer |x| en format fixe
        x_abs = input.mant;
        x_abs = (input.exp >= 0) ? (x_abs << input.exp) : (x_abs >> -input.exp);
        if(x_abs < 0) x_abs = -x_abs;
        
        //Calculer e^|x| et e^(-|x|) via helper
        cosh_sinh_helper(x_abs, &result, &exp_pos, &exp_neg);
        
        //Soustraire: e^x - e^(-x)
        diff = exp_pos - exp_neg;
        result.mant = diff >> 1;  //Diviser par 2 directement
    
        if(result.mant == 0 && diff != 0) {
            result.exp--;
            result.mant = diff >> 1;
        }
        
        normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Calcule la fonction hyperbolique cosh(x) en demi-précision.
 *
 * Utilise la table précalculée exp_table via exp_fixed() :
 * cosh(x) = (e^|x| + e^(-|x|)) / 2
 * 
 * L'overflow vers +inf est géré naturellement par normalize_and_round().
 * Les cas spéciaux IEEE 754 (NaN, +/-inf) sont traités en amont.
 * Note: cosh est une fonction paire, donc cosh(-x) = cosh(x).
 *
 * @param hf Demi-flottant d'entrée (x)
 * @return Demi-flottant représentant cosh(x)
 */
uint16_t hf_cosh(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    //Initialisation optimisée pour les cas spéciaux
    result.sign = HF_ZERO_POS;  //cosh est toujours positif
    result.exp = HF_EXP_FULL;
    result.mant = 0;

    //Gestion unifiée des cas spéciaux
    if(is_nan(&input)) {
        //NaN: propager avec signe positif (cosh produit toujours des valeurs positives)
        result.mant = 1;
    } else if(!is_infinity(&input)) {
        //Calcul via table exp : cosh(x) = (e^|x| + e^(-|x|)) / 2
        int32_t x_abs, exp_pos, exp_neg, sum;

        //Préparer |x| en format fixe
        x_abs = input.mant;
        x_abs = (input.exp >= 0) ? (x_abs << input.exp) : (x_abs >> -input.exp);
        if(x_abs < 0) x_abs = -x_abs;
        
        //Calculer e^|x| et e^(-|x|) via helper
        cosh_sinh_helper(x_abs, &result, &exp_pos, &exp_neg);
        
        //Additionner: e^|x| + e^(-|x|)
        sum = exp_pos + exp_neg;
        result.mant = sum >> 1;  //Diviser par 2 directement
        
        if(result.mant == 0 && sum != 0) {
            result.exp--;
            result.mant = sum >> 1;
        }
        
        normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Calcule la tangente hyperbolique d'un demi-flottant
 *
 * Utilise la formule tanh(x) = sinh(x)/cosh(x) = (e^x - e^-x)/(e^x + e^-x)
 * Les cas spéciaux IEEE 754 (NaN, +/-inf) sont gérés explicitement.
 * Le calcul utilise les helpers et tables existantes (pas de nouvelle table).
 *
 * @param hf Demi-flottant d'entrée (x)
 * @return Demi-flottant représentant tanh(x)
 */
uint16_t hf_tanh(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    //Gestion des cas spéciaux en début de fonction
    result.sign = input.sign;
    result.exp = HF_EXP_FULL;
    result.mant = 0;

    if(is_nan(&input)) {
        //NaN : propager le signe, mantisse à 1
        result.mant = 1;
    } else if(is_infinity(&input)) {
        //tanh(+inf) = +1, tanh(-inf) = -1
        result.exp = 0;
        result.mant = HF_MANT_NORM_MIN;
    } else {
        //Fonctionnement général : tanh(x) = sinh(x)/cosh(x)
        int32_t x_abs, exp_pos, exp_neg;
        int32_t sinh_val, cosh_val;
        int sign = input.sign;

        //Préparer |x| en format fixe
        x_abs = input.mant;
        x_abs = (input.exp >= 0) ? (x_abs << input.exp) : (x_abs >> -input.exp);
        if(x_abs < 0) x_abs = -x_abs;

        //Calculer e^|x| et e^(-|x|) via helper
        cosh_sinh_helper(x_abs, &result, &exp_pos, &exp_neg);
        sinh_val = sign ? -(exp_pos - exp_neg) : (exp_pos - exp_neg);
        cosh_val = exp_pos + exp_neg;

        //Éviter la division par zéro (cosh(0) = 1, mais sécurité)
        if(cosh_val == 0) {
            result.mant = 1;
        } else {
            //Ratio tanh = sinh/cosh, arrondi et normalisation
            int32_t tanh_val = (sinh_val << 15) / cosh_val;
            if(tanh_val < 0) {
                result.sign = HF_ZERO_NEG;
                tanh_val = -tanh_val;
            } else {
                result.sign = HF_ZERO_POS;
            }
            result.exp = 0;
            result.mant = tanh_val > HF_MANT_NORM_MIN ? HF_MANT_NORM_MIN : tanh_val;

            normalize_and_round(&result);
        }
    }

    return compose_half(&result);
}

/**
 * @brief Calcule la fonction hyperbolique inverse asinh(x) en demi-précision.
 *
 * Utilise la formule mathématique asinh(x) = ln(x + sqrt(x^2 + 1)).
 * Pour la stabilité numérique, le calcul utilise la valeur absolue |x|
 * et applique le signe approprié au résultat final.
 * Les cas spéciaux IEEE 754 (NaN, +/-inf, zéro) sont gérés explicitement.
 *
 * @param hf Demi-flottant d'entrée (x)
 * @return Demi-flottant représentant asinh(x)
 */
uint16_t hf_asinh(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    result.sign = input.sign;
    result.exp = 0;
    result.mant = 0;

    if(is_nan(&input)) {
        //NaN: propager le signe
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(&input)) {
        //asinh(+/-inf) = +/-inf
        result.exp = HF_EXP_FULL;
    } else if(!is_zero(&input)) {
        //TODO: implémenter asinh(x)=ln(x+sqrt(x^2+1)) en fixe
        //Implémentation temporaire via fonctions existantes
        uint16_t absx = hf_abs(hf);                 //|x|
        uint16_t xsq  = hf_mul(absx, absx);         //x^2
        uint16_t sum  = hf_add(xsq, HF_ONE_POS);    //x^2 + 1
        uint16_t root = hf_sqrt(sum);               //sqrt(x^2+1)
        uint16_t inner = hf_add(absx, root);        //|x| + sqrt(...)
        uint16_t lnval = hf_ln(inner);              //ln(|x| + sqrt(...))

        result = decompose_half(lnval);
        result.sign = input.sign;
        //normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Calcule la fonction hyperbolique inverse acosh(x) en demi-précision.
 *
 * Utilise la formule mathématique acosh(x) = ln(x + sqrt(x^2 - 1)) pour x >= 1.
 * Pour la stabilité numérique, le calcul utilise la valeur absolue |x|
 * et applique le signe approprié au résultat final.
 * Les cas spéciaux IEEE 754 (NaN, +/-inf, zéro, x < 1) sont gérés explicitement.
 *
 * @param hf Demi-flottant d'entrée (x)
 * @return Demi-flottant représentant acosh(x)
 */
uint16_t hf_acosh(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    result.sign = HF_ZERO_POS;
    result.exp = 0;
    result.mant = 0;

    if(is_nan(&input)) {
        //NaN: propager le signe
        result.sign = input.sign;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(&input)) {
        if(input.sign) {
            //acosh(-inf) = NaN
            result.sign = HF_ZERO_NEG;
            result.exp = HF_EXP_FULL;
            result.mant = 1;
        } else {
            //acosh(+inf) = +inf
            result.exp = HF_EXP_FULL;
        }
    } else if(hf_cmp(hf, HF_ONE_POS) < 0) {
        //x < 1 -> retourner un NaN canonique (qNaN) avec signe positif
        //result.sign = HF_ZERO_POS;
        result.exp = HF_EXP_FULL;
        result.mant = 1; //qNaN mantisse non nulle
    }
    else {
        //TODO: Implémenter acosh(x) = ln(x + sqrt(x² - 1)) en arithmétique à virgule fixe
        //Implémentation temporaire en demi-précision via fonctions existantes
        uint16_t xsq  = hf_mul(hf, hf);             //x^2
        uint16_t diff = hf_sub(xsq, HF_ONE_POS);    //x^2 - 1
        uint16_t root = hf_sqrt(diff);              //sqrt(x^2 - 1)
        uint16_t inner = hf_add(hf, root);          //x + sqrt(...)
        uint16_t lnval = hf_ln(inner);              //ln(x + sqrt(...))
        result = decompose_half(lnval);
        //normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Calcule la fonction hyperbolique inverse atanh(x) en demi-précision.
 *
 * Utilise la formule mathématique atanh(x) = (1/2) * ln((1 + x) / (1 - x)) pour |x| < 1.
 * Pour la stabilité numérique, le calcul utilise la valeur absolue |x|
 * et applique le signe approprié au résultat final.
 * Les cas spéciaux IEEE 754 (NaN, +/-inf, +/-1, |x| > 1) sont gérés explicitement.
 *
 * @param hf Demi-flottant d'entrée (x)
 * @return Demi-flottant représentant atanh(x)
 */
uint16_t hf_atanh(uint16_t hf) {
    half_float result;
    half_float input = decompose_half(hf);

    result.sign = input.sign;
    result.exp = 0;
    result.mant = 0;

    if(is_nan(&input)) {
        //NaN: propager le signe
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(is_infinity(&input)) {
        //atanh(+/-inf) = NaN
        result.sign = HF_ZERO_NEG;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else if(hf_abs(hf) == HF_ONE_POS) {
        //+/-1 -> +/-inf
        result.sign = input.sign;
        result.exp = HF_EXP_FULL;
    } 
    else if(hf_cmp(hf_abs(hf), HF_ONE_POS) > 0) {
        //|x| > 1 -> retourner un NaN canonique positif (qNaN)
        result.sign = HF_ZERO_POS;
        result.exp = HF_EXP_FULL;
        result.mant = 1;
    } else {
        //TODO: Implémenter atanh(x) = 0.5 * ln((1+x)/(1-x)) en arithmétique à virgule fixe
        //Implémentation temporaire en demi-précision via fonctions existantes

        //Petits x (sous-normaux très proches de zéro) : atanh(x) ≈ x
        //Pour éviter que des valeurs dénormalisées soient réduites à 0
        //par des opérations intermédiaires, assigner directement le résultat
        //si l'entrée est un subnormal non nul.
        if(is_subnormal(&input)) {
            //subnormal non nul -> atanh(x) ≈ x (préserver la valeur)
            result = input;
        } else {
            uint16_t sum  = hf_add(HF_ONE_POS, hf);   //1+x
            uint16_t diff = hf_sub(HF_ONE_POS, hf);   //1-x
            uint16_t divv  = hf_div(sum, diff);       //(1+x)/(1-x)
            uint16_t lnval = hf_ln(divv);             //ln((1+x)/(1-x))
            uint16_t halfv = float_to_half(0.5f);     //0.5 en demi-précision
            uint16_t res   = hf_mul(halfv, lnval);    //0.5 * ln(...)
            result = decompose_half(res);
        }
        //normalize_and_round(&result);
    }

    return compose_half(&result);
}

/**
 * @brief Fonction interne pour calculer sin et cos avec décalage
 * 
 * @param hf L'angle en radians dont on cherche le sinus/cosinus
 * @param shift Décalage à appliquer (0 pour sin, 16384 pour cos)
 * @return Le résultat du sinus/cosinus
 */
static uint16_t sinus_shiftable(uint16_t hfangle, uint16_t shift) {
    half_float result;
    half_float angle_hf = decompose_half(hfangle);
    
    //Initialisation par défaut: calcul trigonométrique normal
    result.sign = HF_ZERO_POS;
    result.exp = 0;
    result.mant = 1;
    
    //Gestion unifiée des cas spéciaux
    if(is_nan(&angle_hf)) {
        //Propager le signe original pour les NaN
        result.sign = angle_hf.sign;
        result.exp = HF_EXP_FULL;
    } else if(is_infinity(&angle_hf)) {
        //Pour les infinis, retourner NaN négatif selon la convention
        result.sign = HF_ZERO_NEG;
        result.exp = HF_EXP_FULL;
    } else {
        //Calcul trigonométrique par table et interpolation
        int32_t norm = angle_hf.mant;
        uint32_t idx_q4;

        //Normalisation de la mantisse pour obtenir une représentation fixe de l'angle
        norm = angle_hf.exp >= 0 ? norm << angle_hf.exp : norm >> -angle_hf.exp;

        //Réduction de l'angle à l'intervalle [0, 65535] et application du décalage
        norm = (int32_t)reduce_radian_uword((uint32_t)norm, 0);
        if(angle_hf.sign) norm = 65536 - norm;
        norm = (norm + shift) & 0xffff;
        
        //Interpolation via table_interpolate sur sin_table
        //Construction d'un index Q4 monotone dans le premier quadrant
        //et réfléchi dans le second pour une interpolation croissante
        //Si le bit 14 (0x4000) est activé, on est dans le quadrant réfléchi
        idx_q4 = (unsigned int)(norm & 0x3fffu);
        if(norm & 0x4000) idx_q4 = 0x3fffu - idx_q4;

        //table_interpolate nécessite une entrée supplémentaire pour l'indexation sûre
        result.mant = table_interpolate(sin_table, SIN_TABLE_SIZE + 1, idx_q4, 4);
        
        //Application du signe pour les quadrants 2 et 3 (bit 15 indique la demi-onde)
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
 * @brief Fonction interne pour calculer asin et acos avec décalage
 * 
 * @param hf La valeur dont on cherche l'arc sinus/cosinus (doit être dans [-1, 1])
 * @param shift Décalage à appliquer (0 pour asin, pi/2 en Q15 pour acos)
 * @return L'angle en radians (asin: [-pi/2, pi/2], acos: [0, pi])
 */
static uint16_t asinus_shiftable(uint16_t hf, uint32_t shift) {
    half_float result;
    half_float input = decompose_half(hf);
    
    //Initialisation par défaut: NaN avec le signe de l'entrée
    result.sign = input.sign;
    result.exp = HF_EXP_FULL;
    result.mant = 1;
   
    //Calcul normal si pas NaN ni Infinity
    if(!is_nan(&input) && !is_infinity(&input)) {
        //Normaliser l'entrée
        int32_t norm = input.mant;
        norm = input.exp >= 0 ? norm << input.exp : norm >> -input.exp;
       
        //Calculer via la table asin si dans le domaine |x| <= 1.0
        if(norm <= HF_MANT_NORM_MIN) {
            const int bits = HF_MANT_SHIFT - ASIN_TABLE_BITS;

            //asin_table est allouée avec ASIN_TABLE_SIZE+1 entrées pour
            //permettre un accès sûr à idx1 lors de l'interpolation.
            //On passe donc la longueur complète à table_interpolate.
            result.exp = 0;
            result.mant = (int32_t)table_interpolate(asin_table, ASIN_TABLE_SIZE + 1, norm, bits);
           
            //Appliquer le shift pour acos, ou le signe pour asin
            if(shift) {
                //Pour acos (shift != 0) le résultat doit être dans [0, pi]
                //(valeur positive). Le signe de l'entrée a été utilisé pour
                //calculer le décalage, mais le résultat final doit rester
                //positif quel que soit le signe d'entrée.
                result.mant = input.sign ? shift + result.mant : shift - result.mant;
                result.sign = HF_ZERO_POS;
            } else {
                result.sign = input.sign;
            }
           
            normalize_and_round(&result);
        }
    }

    return compose_half(&result);
}

/**
 * @brief Helper pour sinh et cosh : calcule e^|x| et e^(-|x|) alignés
 * 
 * @param x_abs Valeur absolue de x en format fixe Q15
 * @param result Structure où stocker e^|x| (mantisse et exposant)
 * @param exp_pos Pointeur pour stocker la mantisse de e^|x|
 * @param exp_neg Pointeur pour stocker la mantisse de e^(-|x|) alignée
 */
static void cosh_sinh_helper(int32_t x_abs, half_float *result, int32_t *exp_pos, int32_t *exp_neg) {
    int32_t k_exp_neg;
    
    //Calculer e^|x| directement dans result
    exp_fixed(x_abs, result);
    *exp_pos = result->mant;
    *exp_neg = 0;

    //Calculer l'inverse Q31 : exp_neg = round((1<<31) / mant)
    if(result->mant > 0) {
        uint32_t one = 1U << 31;
        *exp_neg = (int32_t)(one / (uint32_t)result->mant);
    }

    //Calculer l'exposant de e^(-|x|)
    k_exp_neg = -result->exp - 1;

    //Aligner les exposants (e^(-x) deviendra ~0 si |x| grand)
    if(result->exp > k_exp_neg) {
        int shift = result->exp - k_exp_neg;
        if(shift < 31) *exp_neg >>= shift; else *exp_neg = 0;  //e^(-x) négligeable
    }
}
