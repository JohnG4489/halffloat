/**
 * @file main.c
 * @brief Programme de démonstration et test de la bibliothèque Half-Float
 * 
 * Ce programme principal lance la suite de tests complète de la bibliothèque
 * Half-Float IEEE 754 et démontre l'utilisation de toutes les fonctionnalités
 * arithmétiques et mathématiques disponibles.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "hf_common.h"
#include "hf_tests.h"
#include "hf_precalc.h"

/**
 * @brief Point d'entrée principal du programme de test des opérations half-float
 * 
 * @param argc Nombre d'arguments
 * @param argv Tableau des arguments
 */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
#ifdef _WIN32
    //Configuration de l'encodage UTF-8 pour Windows
    system("chcp 65001 >nul 2>&1");
#endif
    
    //Initialisation des tables nécessaires
    fill_sin_table();
    fill_asin_table();
    fill_atan_table();
    fill_exp_table();
    fill_ln_table();
    fill_tan_tables_dual();  //Nouvelle fonction dual-table optimisée à 75°

    //Message d'introduction
    printf("===== Half-Float Library Test Suite =====\n\n");

    //Série de tests pour différentes fonctions mathématiques
    debug_int();
    debug_ceil();
    debug_floor();
    debug_round();
    debug_trunc();

    debug_min();
    debug_max();
    debug_modf();
    debug_copysign();
    debug_frexp();

    debug_abs();
    debug_neg();
    debug_add();
    //debug_sub();
    debug_mul();
    debug_div();
    debug_inv();
    debug_sqrt();
    debug_rsqrt();
    debug_fma();

    debug_pow();
    debug_exp();
    debug_exp2();
    debug_exp10();
    debug_ln();
    debug_log2();
    debug_log10();

    debug_sin();
    debug_cos();
    debug_tan();
    debug_asin();
    debug_acos();
    debug_atan();
    debug_atan2();
    debug_sinh();
    debug_cosh();
    debug_tanh();
    debug_asinh();
    debug_acosh();
    debug_atanh();
    
    printf("\n=== ATAN2 SKIPPED FOR DEBUG ===\n");

    //Tests spécialisés ajoutés
    debug_denormal_values();
    debug_mathematical_identities();
    debug_ieee754_edge_cases();

    //Tests de validation exhaustive nouveaux
    debug_precision_stress_test();
    debug_comparative_accuracy();
    debug_boundary_conditions();
    debug_special_constants();
    debug_inverse_functions();
    debug_rsqrt_comparison();
    debug_tan_near_pi2();

    //Message de conclusion
    printf("\n===== All Tests Completed =====\n");

    return 0;
}
