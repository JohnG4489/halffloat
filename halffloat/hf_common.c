/**
 * @file hf_common.c
 * @brief Implémentation des fonctions utilitaires pour Half-Float IEEE 754
 * 
 * Ce fichier contient les fonctions de base pour la manipulation des nombres
 * flottants de demi-précision : conversion, décomposition, composition,
 * détection des cas spéciaux et fonctions utilitaires pour l'arithmétique.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#include "hf_common.h"

//Prototypes des fonctions

//Conversion entre float et demi-flottant
uint16_t float_to_half(float f);
float half_to_float(uint16_t half);

//Statut du demi-flottant
bool_t is_infinity(half_float hf);
bool_t is_nan(half_float hf);
bool_t is_zero(half_float hf);

//Décomposition et composition de demi-flottants
half_float decompose_half(uint16_t half);
uint16_t compose_half(half_float hf);

//Fonctions internes
void align_mantissas(half_float *a, half_float *b);
void normalize_and_round(half_float *result);


/**
 * @brief Convertit un float en demi-flottant (16 bits)
 * 
 * @param f Le float à convertir
 * @return La valeur uint16_t correspondante au demi-flottant
 */
uint16_t float_to_half(float f) {
    union { float f; uint32_t u; } conv = {f};
    uint32_t f_bits = conv.u; //Représentation binaire du float
    uint16_t sign = (uint16_t)((f_bits >> 31) & 0x1U); //Extraire le bit de signe
    int32_t exp = (int32_t)(((f_bits >> 23) & 0xFFU) - 127U); //Exposant du float
    uint32_t mant = f_bits & 0x7FFFFF; //Mantisse du float
    uint16_t result = sign << HF_SIGN_BITS; //Initialiser avec le bit de signe (bit 15)

    //Gestion des cas spéciaux
    if(exp == 128) { //Cas infini ou NaN
        result |= (mant == 0) ? HF_INFINITY_POS : HF_NAN;
    } else if(exp < -14) { //Cas sous-normaux
        if(exp >= -24) { //Cas sous-normal, mais encore représentable
            mant |= 0x800000; //Ajouter le bit implicite
            result |= (mant >> (-exp - 1)) & HF_MASK_MANT; //Décaler la mantisse
        }
    } else if(exp > 15) { //Cas normal, mais dépassement
        result |= HF_INFINITY_POS; //Retourner infini
    } else { //Cas normal
        exp += HF_EXP_BIAS; //Ajuster l'exposant
        mant |= 0x800000; //Ajouter le bit implicite
        mant = (mant >> 13); //Arrondir la mantisse
        result |= ((exp & HF_MASK_EXP) << HF_MANT_BITS) | (mant & HF_MASK_MANT); //Assembler le résultat
    }

    return result;
}

/**
 * @brief Convertit un demi-flottant (16 bits) en float
 * 
 * @param half La valeur uint16_t du demi-flottant à convertir
 * @return Le float correspondant
 */
float half_to_float(uint16_t half) {
    union { float f; uint32_t u; } conv = {0};
    uint32_t f_bits = 0;
    uint16_t sign = (half >> HF_SIGN_BITS) & 0x1;
    int16_t exp = ((half >> HF_MANT_BITS) & HF_MASK_EXP);
    uint16_t mant = half & HF_MASK_MANT;

    if(exp == HF_MASK_EXP) {
        if(mant == 0) {
            f_bits = (sign << 31) | (0xFF << 23); //Cas infini
        } else {
            f_bits = (sign << 31) | (0xFF << 23) | (mant << 13); //Cas NaN
        }
    } else if(exp == 0 && mant == 0) {
        f_bits = sign << 31; //Cas zéro
    } else {
        if(exp == 0) {
            while(!(mant & 0x400)) {
                mant <<= 1;
                exp--;
            }
            mant &= HF_MASK_MANT;
            exp++;
        }
        exp += 112;
        f_bits = (sign << 31) | (exp << 23) | (mant << 13);
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
    return(hf.exp == HF_EXP_FULL) && (hf.mant == 0);
}

/**
 * @brief Vérifie si un demi-flottant représente NaN (Not a Number)
 * 
 * @param hf Le demi-flottant à vérifier
 * @return true si le demi-flottant est NaN, false sinon
 */
bool_t is_nan(half_float hf) {
    return(hf.exp == HF_EXP_FULL) && (hf.mant != 0);
}

/**
 * @brief Vérifie si un demi-flottant représente zéro
 * 
 * @param hf Le demi-flottant à vérifier
 * @return true si le demi-flottant est zéro, false sinon
 */
bool_t is_zero(half_float hf) {
    return(hf.exp == -HF_EXP_BIAS) && (hf.mant == 0);
}

/**
 * @brief Décompose un uint16_t en demi-flottant
 * 
 * @param half La valeur uint16_t à décomposer
 * @return La structure half_float correspondante
 */
half_float decompose_half(uint16_t half) {
    half_float hf;
    hf.sign = half & HF_MASK_SIGN; //Extraire le bit de signe
    hf.exp = ((half >> HF_MANT_BITS) & HF_MASK_EXP) - HF_EXP_BIAS; //Extraire l'exposant et appliquer le biais
    hf.mant = (half & HF_MASK_MANT) << HF_PRECISION_SHIFT; //Extraire et décaler la mantisse

    //Vérification si le demi-flottant est ni infini, ni NaN
    if(hf.exp > -HF_EXP_BIAS && hf.exp<HF_EXP_FULL) {
        hf.mant |= (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT)); //Ajouter le bit implicite pour les mantisses normales
    }

    return hf;
}

/**
 * @brief Compose un demi-flottant en uint16_t
 * 
 * @param hf La structure half_float à composer
 * @return La valeur uint16_t correspondante
 */
uint16_t compose_half(half_float hf) {
    uint16_t exp_bits = 0;
    uint16_t mant_bits = 0;
    uint16_t result = 0; //Variable pour stocker le résultat final

    //Gestion des cas spéciaux
    if(hf.exp == HF_EXP_FULL) {
        result = hf.sign | (hf.mant != 0 ? HF_NAN : HF_INFINITY_POS);
    } else if(hf.exp < -14 || (hf.exp == -14 && hf.mant == 0)) {
        result = hf.sign; //Cas pour les sous-normaux et zéro
    } else {
        //Normalisation
        while(hf.mant >= (1<<(HF_MANT_BITS+HF_EXP_BITS+1))) {
            hf.mant >>= 1;   //Décalage à droite
            hf.exp++;
        }
        while(hf.mant < (1<<(HF_MANT_BITS+HF_EXP_BITS)) && hf.exp > -14) {
            hf.mant <<= 1;   //Décalage à gauche
            hf.exp--;
        }

        //Détermination du résultat après normalisation
        if(hf.exp > HF_EXP_BIAS) {
            result = hf.sign | HF_INFINITY_POS; //Retourner infini si l'exposant est trop grand
        } else if(hf.exp < -14) {
            result = hf.sign; //Retourner zéro pour les exposants trop petits
        } else {
            exp_bits = (hf.exp == -14) ? 0 : ((hf.exp + HF_EXP_BIAS) & HF_MASK_EXP); //Ajuster l'exposant
            mant_bits = (hf.mant >> HF_PRECISION_SHIFT) & HF_MASK_MANT; //Ajuster la mantisse
            result = hf.sign | (exp_bits << HF_MANT_BITS) | mant_bits; //Composer le résultat
        }
    }
    return result;
}

/**
 * @brief Aligne les mantisses de deux demi-flottants pour l'addition/soustraction
 * 
 * Cette fonction aligne deux demi-flottants en ajustant leurs mantisses
 * pour qu'ils aient le même exposant. Le nombre avec le plus petit exposant
 * voit sa mantisse décalée à droite pour compenser la différence.
 * 
 * @param hf1 Pointeur vers le premier demi-flottant (modifié)
 * @param hf2 Pointeur vers le second demi-flottant (modifié)
 */
void align_mantissas(half_float *hf1, half_float *hf2) {
    if(hf1->exp > hf2->exp) {
        int shift = hf1->exp - hf2->exp;
        shift = (shift > 31) ? 31 : shift;
        if(shift > 0 && shift < 31) {
            uint32_t lost = (uint32_t)hf2->mant & ((1u << shift) - 1u);
            hf2->mant >>= shift; //Décaler la mantisse du second nombre
            if(lost) hf2->mant |= 1; //Sticky bit pour préserver l'info perdue
        } else if(shift >= 31) {
            hf2->mant = (hf2->mant != 0) ? 1 : 0;
        }
        hf2->exp = hf1->exp; //Aligner les exposants
    } else if(hf2->exp > hf1->exp) {
        int shift = hf2->exp - hf1->exp;
        shift = (shift > 31) ? 31 : shift;
        if(shift > 0 && shift < 31) {
            uint32_t lost = (uint32_t)hf1->mant & ((1u << shift) - 1u);
            hf1->mant >>= shift; //Décaler la mantisse du premier nombre
            if(lost) hf1->mant |= 1; //Sticky bit pour préserver l'info perdue
        } else if(shift >= 31) {
            hf1->mant = (hf1->mant != 0) ? 1 : 0;
        }
        hf1->exp = hf2->exp; //Aligner les exposants
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
    //Normalisation de la mantisse
    if(result->mant != 0) {
        //Si la mantisse est trop petite, on la recadre à gauche
        while(result->mant < (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT))) {
            result->mant <<= 1;
            result->exp--;
        }

        //Si la mantisse est trop grande, on la recadre à droite
        while(result->mant >= (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT + 1))) {
            result->mant >>= 1;
            result->exp++;
        }
    }

    //On teste le bit de garde (bit 4)
    if(result->mant & 0x10) {
        //Extraire les bits d'arrondi et collants (bits 0-3)
        uint32_t round_sticky = result->mant & 0xF;

        //round-to-nearest, ties to even
        //Arrondir si > 0.5 ou si = 0.5 et bit LSB = 1 (ties to even)
        if(round_sticky > 0x8 || (round_sticky == 0x8 && (result->mant & 0x20))) {
            result->mant += 0x20; //Arrondir vers le haut
            //Vérifier si on dépasse la mantisse normalisée
            if(result->mant >= (1 << (HF_MANT_BITS + HF_PRECISION_SHIFT + 1))) {
                result->mant >>= 1; //Décalage à droite si dépassement
                result->exp++;
            }
        }
    }

    //Ajuster l'exposant pour infini si nécessaire
    if(result->exp >= HF_EXP_FULL) {
        result->exp = HF_EXP_FULL; //Ajuster pour infini
        result->mant = 0; //Fixer la mantisse pour infini
    }

    //Nettoyer les bits non significatifs
    result->mant &= 0xFFE0;
}
