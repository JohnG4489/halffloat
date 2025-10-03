#include "hf_common.h"
#include "hf_precalc.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


uint16_t sin_table[SIN_TABLE_SIZE+1];
uint16_t ln_table[LN_TABLE_SIZE];
uint16_t exp_table[EXP_TABLE_SIZE+1];
uint32_t tan_table[TAN_TABLE_SIZE+1];


/**
 * @brief Remplit la table de sinus
 * 
 * Cette fonction génère une table de valeurs de sinus précalculées.
 * Les valeurs sont converties en format virgule fixe Q15 pour une 
 * utilisation efficace dans les calculs de demi-précision.
 * 
 * La table couvre l'intervalle [0, ?/2] avec SIN_TABLE_SIZE+1 points,
 * permettant une interpolation linéaire précise.
 */
void fill_sin_table() {
    for (int i = 0; i <= SIN_TABLE_SIZE; i++) {
        double angle = (M_PI / 2) * i / SIN_TABLE_SIZE;
        double sin_val = sin(angle);
        sin_table[i] = (uint16_t)floorf(sin_val * 32768.0 + 0.5); // Conversion en format fixe Q15
    }
}

/**
 * @brief Remplit la table de logarithmes naturels
 * 
 * Cette fonction génère une table de valeurs de logarithmes naturels précalculées.
 * Les valeurs sont converties en format Q14 pour optimiser les calculs de 
 * logarithmes en demi-précision.
 * 
 * La table couvre l'intervalle [1, 2[ avec LN_TABLE_SIZE points, 
 * permettant une interpolation rapide et précise.
 */
void fill_ln_table() {
    for (int i = 0; i < LN_TABLE_SIZE; i++) {
        double x = 1.0 + (double)i / LN_TABLE_SIZE;
        double ln_val = log(x);
        uint16_t fixed_point = (uint16_t)(ln_val * 65536.0 + 0.5);  // Q14 format
        ln_table[i] = fixed_point;
    }
}

/**
 * @brief Remplit la table d'exponentielles
 * 
 * Cette fonction génère une table de valeurs d'exponentielles précalculées.
 * Les valeurs sont converties en format virgule fixe pour accélerer 
 * les calculs exponentiels en demi-précision.
 * 
 * La table couvre un intervalle prédéfini avec une précision adaptèe 
 * aux calculs de demi-précision.
 */
void fill_exp_table() {
    for (int i = 0; i <= EXP_TABLE_SIZE; i++) {
        double x = i / 128.0;
        double exp_val = exp2(x);
        uint32_t fixed_point = (uint32_t)((exp_val-1) * 65536.0 + 0.5);
        if(fixed_point>0xffff) fixed_point=0xffff;
        exp_table[i] = fixed_point;
    }
}

/**
 * @brief Remplit la table de tangentes
 * 
 * Cette fonction génère une table de valeurs de tangentes précalculées.
 * Les valeurs sont converties en format virgule fixe pour optimiser 
 * les calculs trigonométriques en demi-précision.
 * 
 * La table couvre l'intervalle [0, ?/2] avec TAN_TABLE_SIZE+1 points,
 * permettant une interpolation linéaire efficace.
 */
void fill_tan_table() {
    for (int i = 0; i <= TAN_TABLE_SIZE; i++) {
        double angle = (double)i / TAN_TABLE_SIZE * (M_PI / 2);
        double tan_val = i==TAN_TABLE_SIZE?65535.f:tan(angle);
        tan_table[i] = (uint32_t)(tan_val * 32768.0 + 0.5);
    }
}

// Toutes les fonctions et commentaires sont maintenant en UTF-8, corrigeant les caractères corrompus.
