/**
 * @file hf_precalc.c
 * @brief Génération et gestion des tables de précalcul pour Half-Float IEEE 754
 * 
 * Ce fichier implémente la génération des tables de valeurs précalculées
 * pour optimiser les calculs des fonctions transcendantes et trigonométriques.
 * Comprend les tables pour sin, cos, tan, ln et exp avec gestion complète
 * des cas spéciaux IEEE 754.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#include "hf_common.h"
#include "hf_precalc.h"
#include <math.h>

//Déclarations des fonctions publiques
void fill_sin_table(void);
void fill_asin_table(void);
void fill_ln_table(void);
void fill_exp_table(void);
void fill_tan_tables_dual(void);       //Tables duales optimales Q13/Q6

uint16_t sin_table[SIN_TABLE_SIZE+1];
uint16_t asin_table[ASIN_TABLE_SIZE + 1];
uint16_t ln_table[LN_TABLE_SIZE];
uint16_t exp_table[EXP_TABLE_SIZE+1];

//TABLES DUALES OPTIMISÉES Q13/Q6
uint16_t tan_table_low[TAN_DUAL_TABLE_SIZE+1];   //[0°, 75°] Q13 format (16-bit)
uint16_t tan_table_high[TAN_DUAL_TABLE_SIZE+1];  //[75°, 90°] Q6 format (16-bit)

/**
 * @brief Remplit la table de sinus
 * 
 * Cette fonction génère une table de valeurs de sinus précalculées.
 * Les valeurs sont converties en format virgule fixe Q15 pour une 
 * utilisation efficace dans les calculs de demi-précision.
 * 
 * La table couvre l'intervalle [0, pi/2] avec SIN_TABLE_SIZE+1 points,
 * permettant une interpolation linéaire précise.
 */
void fill_sin_table(void) {
    int i;

    for(i = 0; i <= SIN_TABLE_SIZE; i++) {
        double angle = (M_PI / 2) * i / SIN_TABLE_SIZE;
        double sin_val = sin(angle);
        sin_table[i] = (uint16_t)(uint32_t)(sin_val * 32768.0 + 0.5); //Conversion en format fixe Q15
    }
}

/**
 * @brief Remplit la table d'arc sinus
 *
 * Cette fonction génère une table de valeurs d'arc sinus précalculées.
 * Les valeurs sont converties en format virgule fixe Q15 pour une
 * utilisation efficace dans les calculs de demi-précision.
 *
 * La table couvre l'intervalle [0, 1] avec ASIN_TABLE_SIZE+1 points,
 * permettant une interpolation linéaire précise.
 * Les résultats sont dans l'intervalle [0, pi/2].
 */
void fill_asin_table(void) {
    int i;
    for(i = 0; i <= ASIN_TABLE_SIZE; i++) {
        double x = (double)i / ASIN_TABLE_SIZE;
        double asin_val = asin(x);
        asin_table[i] = (uint16_t)(uint32_t)(asin_val * 32768.0 + 0.5); //Conversion en format fixe Q15
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
void fill_ln_table(void) {
    int i;

    for(i = 0; i < LN_TABLE_SIZE; i++) {
        double x = 1.0 + (double)i / LN_TABLE_SIZE;
        double ln_val = log(x);
        uint16_t fixed_point = (uint16_t)(ln_val * 32768.0 + 0.5);  //Q15 format
        ln_table[i] = fixed_point;
    }
}

/**
 * @brief Remplit la table d'exponentielles
 * 
 * Cette fonction génère une table de valeurs d'exponentielles précalculées.
 * Les valeurs sont converties en format virgule fixe pour accélérer 
 * les calculs exponentiels en demi-précision.
 * 
 * La table couvre un intervalle prédéfini avec une précision adaptée 
 * aux calculs de demi-précision.
 */
void fill_exp_table(void) {
    int i;

    for(i = 0; i <= EXP_TABLE_SIZE; i++) {
        //La table doit couvrir [0, ln(2)], pas [0, 1] !
        double x = (double)i / EXP_TABLE_SIZE * log(2.0);
        double exp_val = exp(x);
        uint32_t fixed_point = (uint32_t)(exp_val * 32768.0 + 0.5);  //Q15 format
        if(fixed_point>0xffff) fixed_point=0xffff;
        exp_table[i] = (uint16_t)fixed_point;
    }
}

/**
 * @brief Remplit les tables de tangentes duales optimisées
 * 
 * Cette fonction génère deux tables de tangentes précalculées avec un point
 * de bascule optimal à 75° (5pi/12). Cette approche dual-table permet une 
 * précision maximale sur la zone critique près de 90°.
 * 
 * Table 1: [0°, 75°] - Zone quasi-linéaire, 256 points suffisent
 * Table 2: [75°, 90°] - Zone exponentielle, résolution maximale requise
 * 
 * Format Q15 32-bit pour précision optimale, amélioration ×8.4 vs table unique
 */
void fill_tan_tables_dual(void) {
    int i;
    
    //TABLE LOW Q13: [0°, 75°] = [0, 5pi/12]
    for(i = 0; i <= TAN_DUAL_TABLE_SIZE; i++) {
        double angle = TAN_SWITCH_RADIANS * (double)i / TAN_DUAL_TABLE_SIZE;
        double tan_val = tan(angle);
        
        //Protection Q13: max = 8.0 (marge 2.1x pour tan(75°)=3.73)
        if(tan_val > 8.0) {
            tan_val = 8.0;
        }
        
        tan_table_low[i] = (uint16_t)(tan_val * 8192.0 + 0.5);  //Q13 format
    }
    
    //TABLE HIGH Q6: [75°, 90°] = [5pi/12, pi/2]
    for(i = 0; i <= TAN_DUAL_TABLE_SIZE; i++) {
        double angle = TAN_SWITCH_RADIANS + (M_PI/2 - TAN_SWITCH_RADIANS) * (double)i / TAN_DUAL_TABLE_SIZE;
        double tan_val = tan(angle);
        
        //Protection Q6: max = 1024 (jusqu'à tan(89.94°), saturation 0.4%)
        if(tan_val > 1024.0) {
            tan_val = 1024.0;
        }
        
        tan_table_high[i] = (uint16_t)(tan_val * 64.0 + 0.5);  //Q6 format
    }
}
