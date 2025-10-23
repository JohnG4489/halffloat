/**
 * @file hf_precalc.h
 * @brief Tables précalculées pour l'optimisation des fonctions Half-Float
 * 
 * Ce fichier définit les tables de recherche précalculées utilisées pour
 * accélérer les calculs des fonctions transcendantes et trigonométriques.
 * Les tables incluent sin, ln, exp et tan avec interpolation linéaire.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#ifndef HF_PRECALC_H
#define HF_PRECALC_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SIN_TABLE_SIZE 1024
#define ASIN_TABLE_SIZE 1024
#define ASIN_TABLE_BITS 10
#define LN_TABLE_SIZE 1024
#define EXP_TABLE_SIZE_SHIFT 8
#define EXP_TABLE_SIZE (1<<EXP_TABLE_SIZE_SHIFT)
#define EXP_TABLE_PRECISION 15
#define EXP_PRECISION_SHIFT 8
#define ACOS_SHIFT ((uint32_t)(M_PI / 2.0 * 32768.0 + 0.5))

//DUAL-TABLE TAN OPTIMISÉ
#define TAN_DUAL_TABLE_SIZE 256              //256 entrées par table
#define TAN_SWITCH_RADIANS 1.30899693899575  //75° en radians = 5pi/12 (point de bascule)

#define LNI_2 22713 //ln(2) * 32768 (Q15) - constante utilisée par les fonctions optimisées

void fill_sin_table(void);
void fill_asin_table(void);
void fill_ln_table(void);
void fill_exp_table(void);
void fill_tan_tables_dual(void);       //Tables duales optimales Q13/Q6

extern uint16_t sin_table[SIN_TABLE_SIZE+1];
extern uint16_t asin_table[ASIN_TABLE_SIZE + 1];
extern uint16_t ln_table[LN_TABLE_SIZE];
extern uint16_t exp_table[EXP_TABLE_SIZE+1];

//TABLES DUALES OPTIMISÉES Q13/Q6
extern uint16_t tan_table_low[TAN_DUAL_TABLE_SIZE+1];   //[0°, 75°] Q13 format (16-bit)
extern uint16_t tan_table_high[TAN_DUAL_TABLE_SIZE+1];  //[75°, 90°] Q6 format (16-bit)

//Tables tan duales: Q13 haute précision (max=8.0) + Q6 grande plage (max=1024)

#endif //HF_PRECALC_H
