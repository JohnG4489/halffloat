/**
 * @file hf_tests.h
 * @brief Interface des fonctions de test pour Half-Float IEEE 754
 * 
 * Ce fichier définit l'interface pour la suite de tests complète de validation
 * de la bibliothèque Half-Float. Les tests couvrent tous les cas IEEE 754,
 * les cas limites, les débordements et la précision des calculs.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#ifndef HF_TESTS_H
#define HF_TESTS_H

//Fonctions de débogage et tests pour les opérations half-float
void debug_abs(void);
void debug_neg(void);
void debug_add(void);
void debug_mul(void);
void debug_div(void);
void debug_sqrt(void);
void debug_pow(void);
void debug_exp(void);
void debug_int(void);
void debug_ln(void);
void debug_sin(void);
void debug_cos(void);
void debug_tan(void);
void debug_denormal_values(void);
void debug_mathematical_identities(void);
void debug_ieee754_edge_cases(void);
void debug_precision_stress_test(void);
void debug_comparative_accuracy(void);
void debug_boundary_conditions(void);
void debug_special_constants(void);
void debug_inverse_functions(void);

#endif //HF_TESTS_H
