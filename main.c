#include <stdio.h>
#include "hf_common.h"
#include "hf_lib.h"
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
    
    // Initialisation des tables nécessaires
    fill_sin_table();
    fill_exp_table();
    fill_ln_table();
    fill_tan_table();

    // Message d'introduction
    printf("===== Half-Float Library Test Suite =====\n\n");

    // Série de tests pour différentes fonctions mathématiques
    debug_cos();
    debug_sin();
    debug_tan();
    debug_exp();
    debug_pow();
    debug_ln();
    debug_sqrt();
    debug_add();
    debug_mul();
    debug_div();
    debug_int();

    // Message de conclusion
    printf("\n===== All Tests Completed =====\n");

    return 0;
}
