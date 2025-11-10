/**
 * @file hf_lib_common.c
 * @brief Implementation des utilitaires communs pour Half-Float
 * 
 * Module contenant les fonctions utilitaires partagees entre
 * tous les modules de la bibliotheque Half-Float.
 *
 * @author Seg
 * @date Novembre 2025
 * @version 1.0
 */

#include <math.h>
#include "hf_lib_common.h"

/**
 * @brief Réduit un angle exprimé en radians (format fixe) dans [0, 2*pi)
 *
 * Prend un angle en représentation fixe (format 32-bit fixé par convention
 * du projet) et le ramène dans l'intervalle [0, 2*pi) sous forme d'un
 * mot non signé 16 bits (uword) adapté aux tables trigonométriques.
 *
 * Détails : la fonction travaille en arithmétique entière fixe. Le paramètre
 * `fact` contrôle des shifts pré-calculés pour adapter la précision/facteur
 * de réduction. Les constantes internes (`fixed_two_pi` et
 * `fixed_two_pi_inv`) sont adaptées à ce schéma fixe.
 *
 * Paramètres :
 *  - angle_rad_fixed : angle en représentation fixe (32-bit)
 *  - fact             : facteur de réduction (shift) utilisé pour ajuster
 *                       la précision interne
 *
 * Retour :
 *  - valeur 16-bit représentant l'angle réduit, prête à indexer des tables
 *    trigonométriques (échelle interne de la bibliothèque)
 *
 * Remarques :
 *  - La fonction s'assure d'éviter les dépassements d'index en ramenant la
 *    valeur dans la plage via une division entière et une multiplication.
 */
uint16_t reduce_radian_uword(uint32_t angle_rad_fixed, int fact) {
    const uint64_t fixed_two_pi = (uint64_t)((2.0 * M_PI) * (double)(1ULL << 32) + 0.5) >> fact;
    const uint64_t fixed_two_pi_inv = (uint64_t)(((1.0 / (2.0 * M_PI))) * (double)(1ULL << 32) + 0.5) << fact;
    
    uint64_t angle = ((uint64_t)angle_rad_fixed) << 17;
    uint64_t cnt = angle / fixed_two_pi;
    uint64_t red = angle - cnt * fixed_two_pi;
    uint64_t res = red * fixed_two_pi_inv;
    res >>= 32 + 16;

    return (uint16_t)res;
}

/**
 * @brief Compare deux demi-flottants décomposés
 *
 * Compare deux représentations décomposées de demi-flottants (struct
 * `half_float` : champs `sign`, `exp`, `mant`). La comparaison tient compte
 * des signes, exposants et mantisses et renvoie un code d'ordre adapté aux
 * usages internes.
 *
 * Retour :
 *  -2 : au moins une des valeurs est NaN (comparaison indéfinie)
 *  -1 : input1 < input2
 *   0 : input1 == input2
 *  +1 : input1 > input2
 *
 * Remarques :
 *  - L'ordre respecté tient compte du signe (valeurs négatives ordonnées
 *    inversement) pour obtenir un ordre numérique cohérent.
 */
int compare_half(const half_float *input1, const half_float *input2) {
    int cmp = 0;

    if(is_nan(input1) || is_nan(input2)) {
        cmp = -2;
    }
    else if(input1->sign != input2->sign) {
        cmp = input1->sign ? -1 : 1;
    }
    else if(input1->exp != input2->exp) {
        cmp = ((input1->exp < input2->exp) ^ input1->sign) ? -1 : 1;
    }
    else if(input1->mant != input2->mant) {
        cmp = ((input1->mant < input2->mant) ^ input1->sign) ? -1 : 1;
    }

    return cmp;
}

/**
 * @brief Vérifie si un demi-flottant représente un entier exact
 *
 * Analyse la décomposition d'un half-float pour déterminer si la valeur
 * correspond exactement à un entier représentable. Si oui, retourne la
 * valeur entière (ou 0 pour les cas où l'entier est nul). Sinon retourne
 * -1 pour indiquer "non entier".
 *
 * Comportement :
 *  - retourne 0 si la valeur est exactement 0 (y compris -0)
 *  - retourne la partie entière si elle est exactement représentable par la
 *    mantisse et l'exposant FP16
 *  - retourne -1 si la valeur est NaN, Inf, ou non entière
 *
 * Paramètre :
 *  - hf : pointeur vers la décomposition `half_float`
 */
int check_int_half(const half_float *hf) {
    int result = -1;

    if(!is_nan(hf) && !is_infinity(hf)) {
        if(hf->mant == 0 && hf->exp == -HF_EXP_BIAS) {
            result = 0;
        } else if(!(hf->exp == -HF_EXP_BIAS && hf->mant != 0)) {
            int shift = HF_MANT_SHIFT - hf->exp;

            if(shift <= 0) {
                result = 0;
            } else if(shift <= HF_MANT_SHIFT) {
                uint32_t mant = hf->mant;
                if(((mant >> shift) << shift) == mant) {
                    result = (int)(mant >> shift);
                }
            }
        }
    }

    return result;
}

/**
 * @brief Interpolation linéaire dans une table de valeurs entières
 *
 * Effectue une interpolation linéaire entre deux entrées consécutives
 * d'une table d'entiers 16-bit. L'indice est fourni sous forme fixe : la
 * partie entière est `index >> frac_bits` et la fraction (pour l'interp.)
 * est `index & ((1<<frac_bits)-1)`.
 *
 * Paramètres :
 *  - table     : pointeur vers la table (éléments uint16_t)
 *  - size      : nombre d'éléments dans la table
 *  - index     : indice en format fixe (valeur << frac_bits)
 *  - frac_bits : nombre de bits réservés à la fraction
 *
 * Retour : valeur interpolée (uint16_t)
 *
 * Remarques :
 *  - Si l'indice dépasse la taille, la fonction clamp les indices sur la
 *    dernière entrée disponible pour éviter les lectures hors-borne.
 */
uint16_t table_interpolate(const uint16_t *table, int size, uint32_t index, int frac_bits) {
    int idx0 = index >> frac_bits;
    int idx1 = idx0 + 1;
    int frac, val0, val1;
    
    if(idx0 >= size) idx0 = size - 1;
    if(idx1 >= size) idx1 = size - 1;
    
    frac = index & ((1 << frac_bits) - 1);
    val0 = table[idx0];
    val1 = table[idx1];
    
    return val0 + (((val1 - val0) * frac + (1 << (frac_bits-1))) >> frac_bits);
}

/**
 * @brief Calcule e^x approximé en virgule fixe (Q15) avec interpolation
 *
 * Cette routine calcule une approximation de e^x pour une entrée en
 * virgule fixe Q15. Elle réduit l'argument par rapport à ln(2), utilise
 * une table pré-calculée `exp_table` pour la mantisse et complète par une
 * interpolation linéaire entre entrées consécutives.
 *
 * Le résultat est renvoyé sous forme décomposée dans `result` : champs
 * `mant` (mantisse) et `exp` (exposant entier) représentant
 * result = mant * 2^{exp} (échelle interne). La fonction retourne en
 * outre l'inverse 1/e^x encodé en Q31 (utile pour calculer sinh).
 *
 * Paramètres :
 *  - x_fixed : argument x en format fixe Q15
 *  - result  : pointeur vers half_float pour recevoir mantisse/exposant
 *
 * Retour : valeur int32_t contenant approx(1/e^x) en Q31 (ou 0 si mant=0)
 *
 * Remarques :
 *  - La routine protège l'accès à la table (`EXP_TABLE_SIZE`) et ajuste
 *    k_exp pour gérer r_fixed négatif.
 *  - Ce calcul est une approximation contrôlée, destinée aux usages
 *    internes (ex. calcul de sinh) où la précision double n'est pas requise.
 */
void exp_fixed(int32_t x_fixed, half_float *result) {
    int32_t k_exp = x_fixed / LNI_2;
    int32_t r_fixed = x_fixed - k_exp * LNI_2;
    int index;

    //Réduction à [0, ln(2)]: ajuster si r négatif
    if(r_fixed < 0) {
        k_exp--;
        r_fixed += LNI_2;
    }

    //Index dans la table avec protection
    index = (r_fixed * EXP_TABLE_SIZE) / LNI_2;
    if(index >= EXP_TABLE_SIZE) index = EXP_TABLE_SIZE - 1;

    //Interpolation linéaire pour remplir result->mant
    result->mant = exp_table[index];
    if(index < EXP_TABLE_SIZE - 1) {
        int32_t frac = ((r_fixed * EXP_TABLE_SIZE) % LNI_2) << 8;
        result->mant += ((exp_table[index + 1] - result->mant) * frac / LNI_2) >> 8;
    }

    result->exp = k_exp;
}
