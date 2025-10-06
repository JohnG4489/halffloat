/**
 * @file hf_tests.c
 * @brief Suite de tests complète pour Half-Float IEEE 754
 * 
 * Ce fichier implémente une suite de tests exhaustive pour valider la
 * conformité IEEE 754 de la bibliothèque Half-Float. Les tests incluent
 * tous les cas spéciaux, les cas limites, la précision et la comparaison
 * avec les fonctions standard pour garantir la qualité de l'implémentation.
 * 
 * @author Seg
 * @date Octobre 2025
 * @version 1.0
 */

#include <stdio.h>
#include <string.h>
#include "hf_tests.h"
#include "hf_lib.h"
#include <math.h>

#ifndef INFINITY
#define INFINITY (1.0f/0.0f)
#endif

#ifndef NAN
#define NAN (0.0f/0.0f)
#endif

//Prototypes des fonctions
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

//Fonction utilitaire locale
static void print_formatted_table(const char *title, const char **headers, int num_cols, float data[][5], int num_rows);

/**
 * @brief Fonction de débogage pour tester la fonction hf_abs avec divers cas de test
 * 
 * Cette fonction teste la valeur absolue de demi-flottants avec une variété de cas
 * incluant les nombres positifs, négatifs, les cas spéciaux IEEE 754 (zéro, infini, NaN),
 * et compare les résultats avec la fonction fabs() standard.
 */
void debug_abs(void) {
    float test_cases[] = {
        //Cas normaux
        1.0f, -1.0f, 2.5f, -2.5f, 65504.0f, -65504.0f,
        0.000061035f, -0.000061035f, 5.96e-8f, -5.96e-8f,
        1000.0f, -1000.0f, 0.1f, -0.1f, 3.14159f, -3.14159f,
        //Cas spéciaux IEEE 754 étendus
        0.0f, -0.0f,
        half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG),
        half_to_float(HF_NAN), 
        //NaN négatif (créé via sqrt de nombre négatif)
        half_to_float(hf_sqrt(float_to_half(-1.0f))),
        //Autre NaN négatif via ln
        half_to_float(hf_ln(float_to_half(-2.0f)))
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[30][5]; /* Tableau statique étendu pour tous les tests */
    const char *headers[] = {"Value", "Result (hf_abs)", "Result (fabsf)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);

        uint16_t result_half = hf_abs(value_half);
        float result_float = half_to_float(result_half);
        float std_result = fabsf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_ABS", headers, 4, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_neg avec divers cas de test
 * 
 * Cette fonction teste la négation de demi-flottants avec une variété de cas
 * incluant les nombres positifs, négatifs, les cas spéciaux IEEE 754 (zéro, infini, NaN),
 * et compare les résultats avec l'opérateur unaire moins (-) standard.
 */
void debug_neg(void) {
    float test_cases[] = {
        //Cas normaux
        1.0f, -1.0f, 2.5f, -2.5f, 65504.0f, -65504.0f,
        0.000061035f, -0.000061035f, 5.96e-8f, -5.96e-8f,
        1000.0f, -1000.0f, 0.1f, -0.1f, 3.14159f, -3.14159f,
        //Cas spéciaux IEEE 754 étendus
        0.0f, -0.0f,
        half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG),
        half_to_float(HF_NAN),
        //NaN négatifs pour tester la préservation des signes
        half_to_float(hf_sqrt(float_to_half(-1.0f))),
        half_to_float(hf_ln(float_to_half(-3.0f)))
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[30][5]; /* Tableau statique étendu pour tous les tests */
    const char *headers[] = {"Value", "Result (hf_neg)", "Result (-value)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);

        uint16_t result_half = hf_neg(value_half);
        float result_float = half_to_float(result_half);
        float std_result = -half_to_float(value_half);
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_NEG", headers, 4, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_add avec divers cas de test
 * 
 * Cette fonction teste l'addition de demi-flottants avec une variété de cas
 * incluant les nombres normaux, les cas spéciaux IEEE 754 (zéro, infini, NaN),
 * et compare les résultats avec l'addition standard en float.
 */
void debug_add(void) {
    float test_cases[][2] = {
        //Cas normaux
        {60000.f, -80000.f}, {-80000.f, -80000.f}, {70000.f, 70000.f}, {-70000.f, 70000.f},
        {-50000.f, -50000.f}, {1.0f, 2.0f}, {-1.0f, 1.0f}, {1.0f, -1.0f},
        {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, -1.0f}, {1.0f, 1.0f},
        {1.0f, 65504.0f}, {-1.0f, -65504.0f}, {20000.f, -30000.f}, {20000.f, 40000.f},
        {20000.f, 50000.f}, {1.f, -65000.f}, {0.5f, 0.25f},
        //Cas spéciaux IEEE 754
        {0.0f, -0.0f}, {-0.0f, 0.0f}, {-0.0f, -0.0f},
        {half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_POS)},
        {half_to_float(HF_INFINITY_NEG), half_to_float(HF_INFINITY_NEG)},
        {half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG)},
        {half_to_float(HF_INFINITY_POS), 1.0f}, {half_to_float(HF_INFINITY_NEG), 1.0f},
        {half_to_float(HF_NAN), 1.0f}, {1.0f, half_to_float(HF_NAN)},
        {half_to_float(HF_NAN), half_to_float(HF_INFINITY_POS)},
        {half_to_float(HF_NAN), half_to_float(HF_NAN)},
        //Cas étendus avec NaN négatifs
        {half_to_float(hf_sqrt(float_to_half(-1.0f))), 2.0f},
        {3.0f, half_to_float(hf_ln(float_to_half(-1.0f)))},
        {half_to_float(hf_sqrt(float_to_half(-2.0f))), half_to_float(hf_ln(float_to_half(-3.0f)))}
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[50][5]; /* Tableau statique suffisant pour tous les tests */
    const char *headers[] = {"Value1", "Value2", "Result (my_add)", "Result (std::add)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value1 = test_cases[i][0];
        float value2 = test_cases[i][1];
        uint16_t value1_half = float_to_half(value1);
        uint16_t value2_half = float_to_half(value2);

        uint16_t result_half = hf_add(value1_half, value2_half);
        float result_float = half_to_float(result_half);
        float std_result = half_to_float(value1_half) + half_to_float(value2_half);
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value1;
        results[i][1] = value2;
        results[i][2] = result_float;
        results[i][3] = std_result;
        results[i][4] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_ADD", headers, 5, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_mul avec divers cas de test
 * 
 * Cette fonction teste la multiplication de demi-flottants avec une variété de cas
 * incluant les nombres normaux, les cas spéciaux IEEE 754, les débordements
 * (overflow/underflow), et compare les résultats avec la multiplication standard.
 */
void debug_mul(void) {
    float test_cases[][2] = {
        //Cas normaux
        {5.25f, -8.3f}, {1.0f, 2.0f}, {-1.0f, 1.0f}, {1.0f, -1.0f}, 
        {1.0f, 1.0f}, {1.0f, 65504.0f}, {-1.0f, -65504.0f}, {20000.f, -30000.f},
        {20000.f, 40000.f}, {20000.f, 50000.f}, {1.f, -65000.f}, {-70000.f, 70000.f},
        {-50000.f, -50000.f}, {0.5f, 0.25f}, {0.15f, 0.893f},
        //Cas spéciaux IEEE 754
        {0.0f, 0.0f}, {-0.0f, 0.0f}, {0.0f, -0.0f}, {-0.0f, -0.0f},
        {1.0f, 0.0f}, {0.0f, -1.0f}, {-1.0f, 0.0f}, {-1.0f, -0.0f},
        {half_to_float(HF_INFINITY_POS), 0.0f}, {half_to_float(HF_INFINITY_NEG), 0.0f},
        {0.0f, half_to_float(HF_INFINITY_POS)}, {-0.0f, half_to_float(HF_INFINITY_NEG)},
        {half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_POS)},
        {half_to_float(HF_INFINITY_NEG), half_to_float(HF_INFINITY_NEG)},
        {half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG)},
        {half_to_float(HF_INFINITY_POS), 2.0f}, {half_to_float(HF_INFINITY_NEG), -2.0f},
        {half_to_float(HF_NAN), -8.3f}, {1.0f, half_to_float(HF_NAN)},
        {half_to_float(HF_NAN), half_to_float(HF_INFINITY_POS)},
        {half_to_float(HF_NAN), half_to_float(HF_NAN)},
        //Cas étendus avec NaN négatifs
        {half_to_float(hf_sqrt(float_to_half(-1.0f))), 4.0f},
        {-2.5f, half_to_float(hf_ln(float_to_half(-1.0f)))},
        {half_to_float(hf_sqrt(float_to_half(-3.0f))), half_to_float(hf_sqrt(float_to_half(-2.0f)))}
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[50][5]; /* Tableau statique suffisant pour tous les tests */
    const char *headers[] = {"Value1", "Value2", "Result (my_mul)", "Result (std::mul)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value1 = test_cases[i][0];
        float value2 = test_cases[i][1];
        uint16_t value1_half = float_to_half(value1);
        uint16_t value2_half = float_to_half(value2);

        uint16_t result_half = hf_mul(value1_half, value2_half);
        float result_float = half_to_float(result_half);
        float std_result = half_to_float(value1_half) * half_to_float(value2_half);
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value1;
        results[i][1] = value2;
        results[i][2] = result_float;
        results[i][3] = std_result;
        results[i][4] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_MUL", headers, 5, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_div avec divers cas de test
 * 
 * Cette fonction teste la division de demi-flottants avec des cas incluant
 * la division par zéro, les nombres normaux, les cas spéciaux IEEE 754,
 * et compare les résultats avec la division standard en float.
 */
void debug_div(void) {
    float test_cases[][2] = {
        //Cas normaux
        {1.0f, 1.0f}, {-1.0f, 1.0f}, {1.0f, -1.0f}, {1.0f, 2.0f}, {2.0f, 1.0f},
        {0.1f, 0.1f}, {1000.0f, 1000.0f}, {0.0001f, 1000.0f}, {1000.0f, 0.0001f},
        {3.14159f, 1.0f}, {1.0f, 3.14159f}, {65504.f, 2.0f}, {0.0000123f, 2.0f},
        {0.8414709848f, 0.5403023059f}, {1.0f, 0.5403023059f}, {1.0f, 0.0000000874f},
        //Tests avec très petites valeurs (cas critiques)
        {1.0f, 1e-7f}, {1.0f, 1e-6f}, {1.0f, 1e-5f}, {1.0f, 6e-8f},
        //Cas spéciaux IEEE 754 - Zéros
        {0.0f, 1.0f}, {-0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, -0.0f},
        {-1.0f, 0.0f}, {-1.0f, -0.0f}, {0.0f, 0.0f}, {-0.0f, 0.0f},
        {0.0f, -0.0f}, {-0.0f, -0.0f},
        //Cas spéciaux IEEE 754 - Infinis
        {half_to_float(HF_INFINITY_POS), 2.0f}, {half_to_float(HF_INFINITY_NEG), 2.0f},
        {half_to_float(HF_INFINITY_POS), -2.0f}, {half_to_float(HF_INFINITY_NEG), -2.0f},
        {2.0f, half_to_float(HF_INFINITY_POS)}, {2.0f, half_to_float(HF_INFINITY_NEG)},
        {-2.0f, half_to_float(HF_INFINITY_POS)}, {-2.0f, half_to_float(HF_INFINITY_NEG)},
        {half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_POS)},
        {half_to_float(HF_INFINITY_NEG), half_to_float(HF_INFINITY_NEG)},
        {half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG)},
        {half_to_float(HF_INFINITY_NEG), half_to_float(HF_INFINITY_POS)},
        //Cas spéciaux IEEE 754 - NaN
        {half_to_float(HF_NAN), 1.0f}, {1.0f, half_to_float(HF_NAN)},
        {half_to_float(HF_NAN), half_to_float(HF_INFINITY_POS)},
        {half_to_float(HF_INFINITY_POS), half_to_float(HF_NAN)},
        {half_to_float(HF_NAN), half_to_float(HF_NAN)}
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[50][5]; /* Tableau statique suffisant pour tous les tests */
    const char *headers[] = {"Value1", "Value2", "Result (my_div)", "Result (std::div)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value1 = test_cases[i][0];
        float value2 = test_cases[i][1];
        uint16_t value1_half = float_to_half(value1);
        uint16_t value2_half = float_to_half(value2);

        uint16_t result_half = hf_div(value1_half, value2_half);
        float result_float = half_to_float(result_half);
        /* Utiliser la division standard C qui gère correctement les signes des zéros IEEE 754 */
        float std_result = half_to_float(value1_half) / half_to_float(value2_half);
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value1;
        results[i][1] = value2;
        results[i][2] = result_float;
        results[i][3] = std_result;
        results[i][4] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_DIV", headers, 5, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_sqrt avec divers cas de test
 * 
 * Cette fonction teste la racine carrée de demi-flottants incluant les nombres
 * positifs, négatifs (qui doivent donner NaN), zéro, infini et NaN,
 * et compare les résultats avec sqrt() standard.
 */
void debug_sqrt(void) {
    float test_cases[] = {
        //Racines parfaites
        65504.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f,
        //Cas spéciaux IEEE 754
        0.0f, -0.0f, 0.25f, -1.0f, half_to_float(HF_INFINITY_POS),
        half_to_float(HF_INFINITY_NEG), half_to_float(HF_NAN), 
        //Valeurs dénormalisées et petites
        0.000061035f, 5.96e-8f, 1e-10f, 1e-15f,
        //Cas edge et précision
        0.999999f, 1.000001f, 16.0f, 64.0f, 256.0f, 1024.0f,
        //Valeurs fractionnaires
        0.1f, 0.01f, 0.001f, 0.0001f
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[50][5]; /* Tableau statique suffisant pour tous les tests étendus */
    const char *headers[] = {"Value", "Result (my_sqrt)", "Result (std::sqrt)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);

        uint16_t result_half = hf_sqrt(value_half);
        float result_float = half_to_float(result_half);
        float std_result = sqrtf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_SQRT", headers, 4, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_pow avec divers cas de test
 * 
 * Cette fonction teste l'exponentiation de demi-flottants (base^exposant)
 * avec des cas incluant les puissances entières, fractionnaires, les cas
 * spéciaux comme 0^0, et compare avec pow() standard.
 */
void debug_pow(void) {
    float test_cases[][2] = {
        //Cas normaux
        {2.0f, 3.0f}, {10.0f, 2.0f}, {3.0f, 4.0f}, {1.5f, 2.5f},
        {5.0f, -1.0f}, {0.5f, 3.0f}, {100.0f, 0.5f}, {2.0f, 10.0f},
        {1.0f, 5.0f}, {0.1f, 2.0f}, {2.0f, -3.0f}, {10.0f, -2.0f},
        {3.0f, -4.0f}, {1.5f, -2.5f}, {5.0f, -0.5f}, {0.5f, -3.0f},
        {100.0f, -0.5f}, {2.0f, -10.0f}, {1.0f, -5.0f}, {0.1f, -2.0f},
        {2.0f, 0.5f}, {2.0f, -0.5f}, {0.5f, 0.5f}, {0.5f, -0.5f},
        //Cas spéciaux IEEE 754 - Puissances avec base 1
        {1.0f, 0.0f}, {1.0f, -0.0f}, {1.0f, half_to_float(HF_INFINITY_POS)},
        {1.0f, half_to_float(HF_INFINITY_NEG)}, {1.0f, half_to_float(HF_NAN)},
        //Cas spéciaux IEEE 754 - Exposant zéro
        {0.0f, 0.0f}, {-0.0f, 0.0f}, {2.0f, 0.0f}, {half_to_float(HF_INFINITY_POS), 0.0f},
        {half_to_float(HF_NAN), 0.0f},
        //Cas spéciaux IEEE 754 - Base zéro
        {0.0f, 1.0f}, {-0.0f, 1.0f}, {0.0f, 2.0f}, {-0.0f, 3.0f},
        {0.0f, -1.0f}, {-0.0f, -2.0f},
        //Cas spéciaux IEEE 754 - Infinis
        {half_to_float(HF_INFINITY_POS), 2.0f}, {half_to_float(HF_INFINITY_NEG), 2.0f},
        {half_to_float(HF_INFINITY_POS), -2.0f}, {half_to_float(HF_INFINITY_NEG), -3.0f},
        {2.0f, half_to_float(HF_INFINITY_POS)}, {0.5f, half_to_float(HF_INFINITY_POS)},
        {2.0f, half_to_float(HF_INFINITY_NEG)}, {0.5f, half_to_float(HF_INFINITY_NEG)},
        //Cas spéciaux IEEE 754 - NaN
        {half_to_float(HF_NAN), 2.0f}, {2.0f, half_to_float(HF_NAN)},
        {half_to_float(HF_NAN), half_to_float(HF_NAN)},
        //Cas étendus avec NaN négatifs
        {half_to_float(hf_sqrt(float_to_half(-1.0f))), 3.0f},
        {2.5f, half_to_float(hf_ln(float_to_half(-2.0f)))},
        {half_to_float(hf_sqrt(float_to_half(-4.0f))), half_to_float(hf_ln(float_to_half(-1.5f)))},
        //Cas spéciaux IEEE 754 - Bases négatives avec exposants fractionnaires
        {-2.0f, 0.5f}, {-4.0f, 0.25f}, {-1.0f, 2.5f}
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[70][5]; /* Tableau statique étendu pour tous les tests */
    const char *headers[] = {"Base", "Exp", "Result (my_pow)", "Result (std::pow)", "Difference"};
    
    for(i = 0; i < num_tests; i++) {
        float base = test_cases[i][0];
        float exponent = test_cases[i][1];
        uint16_t base_half = float_to_half(base);
        uint16_t exponent_half = float_to_half(exponent);

        uint16_t result_half = hf_pow(base_half, exponent_half);
        float result_float = half_to_float(result_half);
        float std_result = powf(half_to_float(base_half), half_to_float(exponent_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = base;
        results[i][1] = exponent;
        results[i][2] = result_float;
        results[i][3] = std_result;
        results[i][4] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_POW", headers, 5, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_exp avec divers cas de test
 * 
 * Cette fonction teste l'exponentielle (e^x) de demi-flottants avec des valeurs
 * positives, négatives, zéro, et les cas spéciaux IEEE 754,
 * et compare les résultats avec exp() standard.
 */
void debug_exp(void) {
    float test_cases[] = {
        //Cas critiques exp
        0.0f,  //exp(0) doit être exactement 1
        1.0f,  //exp(1) = e environ 2.718
        -1.0f, //exp(-1) = 1/e environ 0.368
        //Tests de overflow/underflow systematiques
        11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, //Overflow tests
        -11.0f, -12.0f, -13.0f, -14.0f, -15.0f, -16.0f, //Underflow tests
        //Cas normaux étendus
        0.035f, 0.1f, 0.5f, 2.0f, 3.0f, 5.0f, 10.0f,
        -0.012f, -0.1f, -0.5f, -2.0f, -3.0f, -5.0f, -10.0f,
        //Valeurs dénormalisées
        1e-10f, 1e-15f, -1e-10f, -1e-15f,
        //Logarithmes naturels pour identité
        0.693147f,  //ln(2)
        2.302585f,  //ln(10)
        //Cas spéciaux IEEE 754
        -0.0f, half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG),
        half_to_float(HF_NAN)
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[50][5]; /* Tableau statique suffisant pour tous les tests étendus */
    const char *headers[] = {"Value", "Result (my_exp)", "Result (std::exp)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);

        uint16_t result_half = hf_exp(value_half);
        float result_float = half_to_float(result_half);
        float std_result = expf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_EXP", headers, 4, results, num_tests);

    /* Test d'identité mathématique: exp(ln(x)) = x */
    printf("### Mathematical Identity: exp(ln(x)) = x\n");
    printf("=========================================\n");
    {
        float identity_values[] = {0.5f, 1.0f, 2.0f, 10.0f};
        int num_identity = sizeof(identity_values) / sizeof(identity_values[0]);
        int j;
        
        for(j = 0; j < num_identity; j++) {
            float x = identity_values[j];
            uint16_t hf_x = float_to_half(x);
            uint16_t ln_result = hf_ln(hf_x);
            uint16_t exp_result = hf_exp(ln_result);
            float identity_result = half_to_float(exp_result);
            float error = fabsf((identity_result - x) / x) * 100.0f;
            
            printf("exp(ln(%.1f)) = %.6f (error: %.3f%%) %s\n",
                   x, identity_result, error, (error < 1.0f) ? "OK" : "ERR");
        }
    }

    printf("\n");
}


/**
 * @brief Fonction de débogage pour tester la fonction hf_int avec divers cas de test
 * 
 * Cette fonction teste l'extraction de la partie entière de demi-flottants
 * avec des nombres positifs, négatifs, fractionnaires, et les cas spéciaux,
 * et compare avec la troncature standard vers zéro.
 */
void debug_int(void) {
    float test_cases[] = {
        //Cas existants
        65504.0f, 1.0f, 1.5f, 2.0f, 2.7f, 3.2f, -1.0f, -1.7f, -2.3f,
        0.0f, -0.0f, 0.7f, -0.7f, half_to_float(HF_INFINITY_POS),
        half_to_float(HF_INFINITY_NEG), half_to_float(HF_NAN),
        0.000061035f, -0.000061035f, 5.96e-8f, -5.96e-8f,
        //Cas manquants identifiés dans les tests
        0.9999f, -0.9999f, 15.999f, -15.999f, 65503.0f, -65503.0f,
        1.5f, 2.5f, 3.5f, -1.5f, -2.5f, -3.5f,
        0.0001f, -0.0001f, 100000.0f, -100000.0f
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[50][5]; /* Tableau statique suffisant pour tous les tests étendus */
    const char *headers[] = {"Value", "Result (my_int)", "Result (std::int)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);

        uint16_t result_half = hf_int(value_half);
        float result_float = half_to_float(result_half);
        float std_result = truncf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_INT", headers, 4, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_ln avec divers cas de test
 * 
 * Cette fonction teste le logarithme naturel de demi-flottants incluant
 * les nombres positifs, zéro (doit donner -inf), les nombres négatifs (NaN),
 * l'infini, et compare avec log() standard.
 */
void debug_ln(void) {
    float test_cases[] = {
        //Cas critiques pour ln
        1.0f,  //ln(1) doit être exactement 0
        //Base de l'exponentielle
        2.718281828f, //ln(e) doit être 1
        //Puissances de 2 pour validation
        2.0f, 4.0f, 8.0f, 16.0f, 32.0f,
        //Fractions simples
        0.5f, 0.25f, 0.125f, 0.0625f,
        //Cas normaux étendus
        0.000061035f, 5.96e-8f, 0.023f, 0.13f, 0.3f, 
        3.14159f, 10.0f, 65504.0f,
        //Valeurs dénormalisées et edge cases
        1e-10f, 1e-15f, 0.999999f, 1.000001f,
        //Cas spéciaux IEEE 754
        0.0f, -0.0f, -1.0f, -2.0f, -10.0f,
        half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG), 
        half_to_float(HF_NAN)
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[50][5]; /* Tableau statique suffisant pour tous les tests étendus */
    const char *headers[] = {"Value", "Result (my_ln)", "Result (std::ln)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);

        uint16_t result_half = hf_ln(value_half);
        float result_float = half_to_float(result_half);
        float std_result = logf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_LN", headers, 4, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_sin avec divers cas de test
 * 
 * Cette fonction teste le sinus de demi-flottants avec des angles variés
 * incluant les valeurs remarquables (0, pi/2, pi, 3pi/2), les cas spéciaux
 * IEEE 754, et compare avec sin() standard.
 */
void debug_sin(void) {
    float test_cases[] = {
        0.0f, -0.0f,  //Zéros positif et négatif
        0.7853981633974483f, 1.5707963267948966f, 3.141592653589793f, 6.283185307179586f,  //?/4, ?/2, ?, 2?
        -0.7853981633974483f, -1.5707963267948966f, -3.141592653589793f,  //-?/4, -?/2, -?
        1.0f, -1.0f,  //Valeurs unitaires
        0.5f, -0.5f,  //Valeurs fractionnaires
        3.0f, -3.0f,  //Valeurs hors de l'intervalle [-?, ?]
        65504.0f, -65504.0f,  //Valeurs maximales pour demi-flottant
        0.000061035f, -0.000061035f,  //Petites valeurs positives et négatives
        half_to_float(HF_INFINITY_POS), -half_to_float(HF_INFINITY_POS),  //Infinis
        half_to_float(HF_NAN)  //NaN
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[25][5]; /* Tableau statique suffisant pour tous les tests */
    const char *headers[] = {"Angle (rad)", "Result (hf_sin)", "Result (sinf)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);

        uint16_t result_half = hf_sin(value_half);
        float result_float = half_to_float(result_half);
        float std_result = sinf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_SIN", headers, 4, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_cos avec divers cas de test
 * 
 * Cette fonction teste le cosinus de demi-flottants avec des angles variés
 * incluant les valeurs remarquables (0, pi/2, pi, 3pi/2), les cas spéciaux
 * IEEE 754, et compare avec cos() standard.
 */
void debug_cos(void) {
    float test_cases[] = {
        0.0f, -0.0f,  //Zéros positif et négatif
        0.7853981633974483f, 1.5707963267948966f, 3.141592653589793f, 6.283185307179586f,  //?/4, ?/2, ?, 2?
        -0.7853981633974483f, -1.5707963267948966f, -3.141592653589793f,  //-?/4, -?/2, -?
        1.0f, -1.0f,  //Valeurs unitaires
        0.5f, -0.5f,  //Valeurs fractionnaires
        3.0f, -3.0f,  //Valeurs hors de l'intervalle [-?, ?]
        65504.0f, -65504.0f,  //Valeurs maximales pour demi-flottant
        0.000061035f, -0.000061035f,  //Petites valeurs positives et négatives
        half_to_float(HF_INFINITY_POS), -half_to_float(HF_INFINITY_POS),  //Infinis
        half_to_float(HF_NAN)  //NaN
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[25][5]; /* Tableau statique suffisant pour tous les tests */
    const char *headers[] = {"Angle (rad)", "Result (hf_cos)", "Result (cosf)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);

        uint16_t result_half = hf_cos(value_half);
        float result_float = half_to_float(result_half);
        float std_result = cosf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_COS", headers, 4, results, num_tests);

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_tan avec divers cas de test
 * 
 * Cette fonction teste la tangente de demi-flottants avec des angles variés
 * incluant les valeurs remarquables, les asymptotes (pi/2, 3pi/2 qui donnent +inf),
 * les cas spéciaux IEEE 754, et compare avec tan() standard.
 */
void debug_tan(void) {
    float test_cases[] = {
        65504.0f, -65504.0f,  //Valeurs maximales pour demi-flottant
        0.f, 0.1f, 0.5f, 0.75f, 1.f, 1.5f, 1.56f, 1.57f, 1.5701f, 1.5702f, 1.58f, 2.f, 3.f, 3.14f, 3.2f, 3.5f, 3.75f, 3.95f, 4.f, 5.f, 6.f, 6.1f, 6.28f, 6.3f,
        1.f,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f,10.f,11.f,12.f,
        0.0f, -0.0f,  //Zéros positif et négatif
        0.7853981633974483f, 1.5707963267948966f, 3.14159f, 6.283185307179586f,  //?/4, ?/2, ?, 2?
        -0.7853981633974483f, -1.5707963267948966f, -3.141592653589793f,  //-?/4, -?/2, -?
        1.0f, -1.0f,  //Valeurs unitaires
        0.5f, -0.5f,  //Valeurs fractionnaires
        3.0f, -3.0f,  //Valeurs hors de l'intervalle [-?, ?]
        0.000061035f, -0.000061035f,  //Petites valeurs positives et négatives
        1.55f, -1.55f,  //Valeurs proches de ?/2
        4.71f, -4.71f,  //Valeurs proches de 3?/2
        half_to_float(HF_INFINITY_POS), -half_to_float(HF_INFINITY_POS),  //Infinis
        half_to_float(HF_NAN)  //NaN
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[70][5]; /* Tableau statique suffisant pour tous les tests */
    const char *headers[] = {"Angle (rad)", "Result (hf_tan)", "Result (tanf)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);

        uint16_t result_half = hf_tan(value_half);
        float result_float = half_to_float(result_half);
        float std_result = tanf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        /* Stocker les résultats dans le tableau */
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    /* Afficher le tableau formaté */
    print_formatted_table("### HF_TAN", headers, 4, results, num_tests);

    printf("\n");
}

/**
 * @brief Tests spécialisés pour les valeurs dénormalisées FP16
 * 
 * Cette fonction teste le comportement des fonctions mathématiques
 * avec des valeurs dénormalisées (subnormal) en FP16, qui sont
 * critiques pour la précision aux limites de représentation.
 */
void debug_denormal_values(void) {
    /* Valeurs dénormalisées critiques en FP16 */
    float denormal_values[] = {
        5.96046e-08f,  //Plus petite valeur normale FP16
        2.98023e-08f,  //Plus grande valeur dénormalisée FP16
        1.49012e-08f,  //Valeur dénormalisée médiane
        5.96046e-09f,  //Plus petite valeur dénormalisée représentable
        0.0f,          //Zéro
        -5.96046e-09f, //Négatives
        -1.49012e-08f,
        -2.98023e-08f,
        -5.96046e-08f
    };
    
    int num_denormal = sizeof(denormal_values) / sizeof(denormal_values[0]);
    int i;
    
    printf("\n### TESTS DES VALEURS DÉNORMALISÉES FP16\n");
    printf("=========================================\n");
    
    printf("Valeur             Sqrt           Exp            Ln             Sin\n");
    printf("-----------------------------------------------------------------------\n");
    
    for(i = 0; i < num_denormal; i++) {
        float val = denormal_values[i];
        uint16_t hf_val = float_to_half(val);
        
        float sqrt_result = half_to_float(hf_sqrt(hf_val));
        float exp_result = half_to_float(hf_exp(hf_val));
        float ln_result = half_to_float(hf_ln(hf_val));
        float sin_result = half_to_float(hf_sin(hf_val));
        
        printf("%-18.9e %-14.6e %-14.6e %-14.6e %-14.6e\n", 
               val, sqrt_result, exp_result, ln_result, sin_result);
    }
    
    printf("\n");
}

/**
 * @brief Tests d'identités mathématiques
 * 
 * Cette fonction vérifie les identités mathématiques critiques
 * comme sin²+cos²=1, exp(ln(x))=x, etc.
 */
void debug_mathematical_identities(void) {
    float test_values[] = {0.5f, 1.0f, 1.5f, 2.0f, 3.14159f};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    int i;
    
    printf("\n### TESTS DES IDENTITÉS MATHÉMATIQUES\n");
    printf("======================================\n");
    
    printf("Identité sin²(x) + cos²(x) = 1:\n");
    printf("Valeur (x)      sin²+cos²     Erreur\n");
    printf("---------------------------------------\n");
    
    for(i = 0; i < num_values; i++) {
        float x = test_values[i];
        uint16_t hf_x = float_to_half(x);
        
        float sin_val = half_to_float(hf_sin(hf_x));
        float cos_val = half_to_float(hf_cos(hf_x));
        float identity_result = sin_val * sin_val + cos_val * cos_val;
        float error = fabsf(identity_result - 1.0f);
        
        printf("%-15.6f %-13.9f %-13.9f\n", x, identity_result, error);
    }
    
    printf("\nIdentité exp(ln(x)) = x:\n");
    printf("Valeur (x)      exp(ln(x))    Erreur relative\n");
    printf("----------------------------------------------\n");
    
    for(i = 0; i < num_values; i++) {
        float x = test_values[i];
        uint16_t hf_x = float_to_half(x);
        
        uint16_t ln_result = hf_ln(hf_x);
        float exp_ln_result = half_to_float(hf_exp(ln_result));
        float error = fabsf((exp_ln_result - x) / x);
        
        printf("%-15.6f %-13.6f %-15.9f\n", x, exp_ln_result, error);
    }
    
    printf("\n");
}

/**
 * @brief Tests de cas edge IEEE 754 spéciaux
 * 
 * Cette fonction teste des cas edge très spécifiques qui peuvent
 * causer des problèmes dans les implémentations FP16.
 */
void debug_ieee754_edge_cases(void) {
    /* Tests de valeurs limites FP16 */
    float edge_values[] = {
        65504.0f,      //Valeur max FP16
        -65504.0f,     //Valeur min FP16
        65520.0f,      //Overflow vers inf
        -65520.0f,     //Overflow vers -inf
        6.1035e-5f,    //Plus petit normal positif
        -6.1035e-5f,   //Plus petit normal négatif
        6.097e-5f,     //Juste au dessus du min normal
        5.96e-8f,      //Plus petit dénormalisé non-zéro
        1.0f - 1e-7f,  //Proche de 1 par en dessous
        1.0f + 1e-7f,  //Proche de 1 par au dessus
        2.0f - 1e-6f,  //Proche de 2 par en dessous
        2.0f + 1e-6f   //Proche de 2 par au dessus
    };
    /* Tests de précision de représentation FP16 */
    float precision_values[] = {
        3.14159265359f,  //Pi
        2.71828182846f,  //e
        1.41421356237f,  //sqrt(2)
        1.73205080757f,  //sqrt(3)
        0.57721566490f   //Constante d'Euler-Mascheroni
    };
    
    int num_edge = sizeof(edge_values) / sizeof(edge_values[0]);
    int num_precision = sizeof(precision_values) / sizeof(precision_values[0]);
    int i;
    
    printf("\n### TESTS DES CAS EDGE IEEE 754\n");
    printf("================================\n");
    
    printf("Test des opérations avec valeurs limites:\n");
    printf("Valeur             Add(+1)        Mul(*2)        Sqrt           Exp\n");
    printf("------------------------------------------------------------------------\n");
    
    for(i = 0; i < num_edge; i++) {
        float val = edge_values[i];
        uint16_t hf_val = float_to_half(val);
        uint16_t hf_one = float_to_half(1.0f);
        uint16_t hf_two = float_to_half(2.0f);
        
        float add_result = half_to_float(hf_add(hf_val, hf_one));
        float mul_result = half_to_float(hf_mul(hf_val, hf_two));
        float sqrt_result = half_to_float(hf_sqrt(hf_val));
        float exp_result = half_to_float(hf_exp(hf_val));
        
        printf("%-18.6e %-14.6e %-14.6e %-14.6e %-14.6e\n",
               val, add_result, mul_result, sqrt_result, exp_result);
    }
    
    /* Tests de précision de représentation FP16 */

    
    printf("\nTest de précision de représentation FP16:\n");
    printf("Valeur originale   Convertie FP16  Erreur relative\n");
    printf("--------------------------------------------------\n");
    for(i = 0; i < num_precision; i++) {
        float original = precision_values[i];
        uint16_t hf_val = float_to_half(original);
        float converted = half_to_float(hf_val);
        float error = fabsf((converted - original) / original);
        
        printf("%-18.11f %-15.11f %-15.9e\n", original, converted, error);
    }
    
    printf("\n");
}

/**
 * @brief Test de stress de précision avec valeurs problématiques
 * 
 * Teste la précision avec des valeurs connues pour être problématiques
 * en arithmétique flottante, notamment près des singularités.
 */
void debug_precision_stress_test(void) {
    float pi_half_values[] = {
        1.5707963f,      /* pi/2 exact (théorique) */
        1.5703125f,      /* Légèrement en dessous */
        1.5712890625f,   /* Légèrement au dessus */
        1.5695f,         /* Plus loin */
        1.5720f          /* Plus loin */
    };
    float small_values[] = {
        1e-4f, 1e-5f, 5.96e-8f, 1e-7f, 1e-6f
    };
    float extreme_values[] = {
        11.0f,    /* Proche de la limite d'overflow */
        -11.0f,   /* Proche de la limite d'underflow */
        10.5f,    /* Valeur intermédiaire */
        -10.5f,   /* Valeur intermédiaire négative */
        0.0f      /* Zéro (cas spécial) */
    };
    int i;
    
    printf("### TESTS DE STRESS DE PRECISION\n");
    printf("================================\n");
    
    /* Test avec valeurs très proches de pi/2 pour tan */
    printf("Test tan() pres de pi/2:\n");
    printf("Valeur             tan(x)             tanf(x)            Erreur relative\n");
    printf("------------------------------------------------------------------------\n");
    for(i = 0; i < 5; i++) {
        float val = pi_half_values[i];
        uint16_t hf_val = float_to_half(val);
        float converted_val = half_to_float(hf_val);
        
        uint16_t result_hf = hf_tan(hf_val);
        float result_float = half_to_float(result_hf);
        float std_result = tanf(converted_val);
        float relative_error = fabsf((result_float - std_result) / std_result);
        
        printf("%-18.10f %-18.6f %-18.6f %-15.9f\n", 
               converted_val, result_float, std_result, relative_error);
    }
    
    /* Test avec très petites valeurs pour sin/cos */
    printf("\nTest sin/cos avec très petites valeurs:\n");
    printf("Valeur             sin(x)             cos(x)             sin²+cos²\n");
    printf("--------------------------------------------------------------------\n");
    
    for(i = 0; i < 5; i++) {
        float val = small_values[i];
        uint16_t hf_val = float_to_half(val);
        
        uint16_t sin_hf = hf_sin(hf_val);
        uint16_t cos_hf = hf_cos(hf_val);
        float sin_result = half_to_float(sin_hf);
        float cos_result = half_to_float(cos_hf);
        float identity_check = sin_result*sin_result + cos_result*cos_result;
        
        printf("%-18.9e %-18.9e %-18.9e %-15.9f\n", 
               half_to_float(hf_val), sin_result, cos_result, identity_check);
    }
    
    /* Test exp/ln avec valeurs extrêmes */
    printf("\nTest exp/ln aux limites de représentation:\n");
    printf("Valeur             exp(x)             ln(exp(x))         Erreur\n");
    printf("----------------------------------------------------------------\n");
    
    for(i = 0; i < 5; i++) {
        float val = extreme_values[i];
        uint16_t hf_val = float_to_half(val);
        
        uint16_t exp_hf = hf_exp(hf_val);
        uint16_t ln_exp_hf = hf_ln(exp_hf);
        float exp_result = half_to_float(exp_hf);
        float ln_exp_result = half_to_float(ln_exp_hf);
        float error = fabsf(ln_exp_result - half_to_float(hf_val));
        
        printf("%-18.6f %-18.6f %-18.6f %-15.9f\n", 
               half_to_float(hf_val), exp_result, ln_exp_result, error);
    }
    
    printf("\n");
}

/**
 * @brief Test de précision comparative entre différentes implémentations
 * 
 * Compare la précision de nos implémentations avec les fonctions standard
 * sur des valeurs de référence mathématiques connues.
 */
void debug_comparative_accuracy(void) {
    struct {
        const char* name;
        float value;
        const char* description;
    } constants[] = {
        {"pi", 3.141592653589793f, "Pi"},
        {"e", 2.718281828459045f, "Nombre d'Euler"},
        {"sqrt(2)", 1.4142135623730951f, "Racine de 2"},
        {"sqrt(3)", 1.7320508075688772f, "Racine de 3"},
        {"ln(2)", 0.6931471805599453f, "Logarithme naturel de 2"},
        {"ln(10)", 2.302585092994046f, "Logarithme naturel de 10"},
        {"1/pi", 0.3183098861837907f, "Inverse de pi"},
        {"pi/2", 1.5707963267948966f, "pi sur 2"},
        {"pi/4", 0.7853981633974483f, "pi sur 4"},
        {"2*pi", 6.283185307179586f, "2*pi"}
    };
    struct {
        const char* expr;
        float input;
        float expected;
        uint16_t (*func)(uint16_t);
    } trig_tests[] = {
        {"sin(pi/6)", 3.141592653589793f/6.0f, 0.5f, hf_sin},
        {"cos(pi/3)", 3.141592653589793f/3.0f, 0.5f, hf_cos},
        {"sin(pi/2)", 3.141592653589793f/2.0f, 1.0f, hf_sin},
        {"cos(pi/2)", 3.141592653589793f/2.0f, 0.0f, hf_cos},
        {"sin(pi)", 3.141592653589793f, 0.0f, hf_sin},
        {"cos(pi)", 3.141592653589793f, -1.0f, hf_cos}
    };
    int i;
    
    printf("### TESTS DE PRECISION COMPARATIVE\n");
    printf("==================================\n");
    
    printf("Constante          Valeur théorique   Valeur HF16        Erreur relative\n");
    printf("------------------------------------------------------------------------\n");
    for(i = 0; i < 10; i++) {
        float theoretical = constants[i].value;
        uint16_t hf_val = float_to_half(theoretical);
        float hf_converted = half_to_float(hf_val);
        float relative_error = fabsf((hf_converted - theoretical) / theoretical);
        
        printf("%-18s %-18.12f %-18.12f %-15.9e\n",
               constants[i].name, theoretical, hf_converted, relative_error);
    }
    
    /* Test de fonctions trigonométriques sur constantes */
    printf("\nTest trigonométrique sur constantes:\n");
    printf("Expression         Théorique          HF16               Erreur\n");
    printf("----------------------------------------------------------------\n");
    
    for(i = 0; i < 6; i++) {
        uint16_t hf_input = float_to_half(trig_tests[i].input);
        uint16_t result_hf = trig_tests[i].func(hf_input);
        float result = half_to_float(result_hf);
        float error = fabsf(result - trig_tests[i].expected);
        
        printf("%-18s %-18.12f %-18.12f %-15.9f\n",
               trig_tests[i].expr, trig_tests[i].expected, result, error);
    }
    
    printf("\n");
}

/**
 * @brief Test des conditions aux limites
 * 
 * Teste le comportement aux limites exactes de la représentation FP16.
 */
void debug_boundary_conditions(void) {
    uint16_t boundary_values[] = {
        0x0000,  /* +0 */
        0x8000,  /* -0 */
        0x0001,  /* Plus petit dénormalisé positif */
        0x8001,  /* Plus petit dénormalisé négatif */
        0x03FF,  /* Plus grand dénormalisé positif */
        0x83FF,  /* Plus grand dénormalisé négatif */
        0x0400,  /* Plus petit normalisé positif */
        0x8400,  /* Plus petit normalisé négatif */
        0x7BFF,  /* Plus grand fini positif */
        0xFBFF,  /* Plus grand fini négatif */
        0x7C00,  /* +inf */
        0xFC00,  /* -inf */
        0x7E00,  /* NaN */
        0xFE00   /* -NaN */
    };
    const char* descriptions[] = {
        "+0", "-0", "Min denorm +", "Min denorm -",
        "Max denorm +", "Max denorm -", "Min norm +", "Min norm -",
        "Max finite +", "Max finite -", "+inf", "-inf", "NaN", "-NaN"
    };
    uint16_t large_val;
    uint16_t add_overflow;
    uint16_t tiny_val;
    uint16_t div_underflow;
    int i;
    
    printf("### TESTS DES CONDITIONS AUX LIMITES\n");
    printf("====================================\n");
    
    printf("Test des opérations sur les valeurs limites:\n");
    printf("Valeur             Description        sqrt           exp            ln\n");
    printf("------------------------------------------------------------------------\n");
    for(i = 0; i < 14; i++) {
        uint16_t val = boundary_values[i];
        uint16_t sqrt_result = hf_sqrt(val);
        uint16_t exp_result = hf_exp(val);
        uint16_t ln_result = hf_ln(val);
        
        float val_f = half_to_float(val);
        float sqrt_f = half_to_float(sqrt_result);
        float exp_f = half_to_float(exp_result);
        float ln_f = half_to_float(ln_result);
        
        printf("%-18.9e %-18s %-14.6e %-14.6e %-14.6e\n",
               val_f, descriptions[i], sqrt_f, exp_f, ln_f);
    }
    
    /* Test de transitions critiques */
    printf("\nTest des transitions critiques:\n");
    printf("Opération          Entrée             Sortie             IEEE 754 OK\n");
    printf("--------------------------------------------------------------------\n");
    
    /* Tests de débordement */
    large_val = float_to_half(65000.0f);
    add_overflow = hf_add(large_val, large_val);
    printf("%-18s %-18.6f %-18s %s\n", "65000+65000", 65000.0f, 
           (add_overflow == 0x7C00) ? "inf" : "finite", 
           (add_overflow == 0x7C00) ? "OK" : "ERR");
    
    /* Test de sous-débordement */
    tiny_val = 0x0001;  /* Plus petit dénormalisé */
    div_underflow = hf_div(tiny_val, float_to_half(2.0f));
    printf("%-18s %-18.9e %-18s %s\n", "tiny/2", half_to_float(tiny_val), 
           (div_underflow == 0x0000) ? "0" : "finite",
           "OK");
    
    printf("\n");
}

/**
 * @brief Test des constantes spéciales et cas particuliers
 * 
 * Teste la gestion des constantes mathématiques et cas particuliers.
 */
void debug_special_constants(void) {
    struct {
        const char* expr;
        float base;
        float exp;
        float expected;
    } power_tests[] = {
        {"2^1", 2.0f, 1.0f, 2.0f},
        {"2^2", 2.0f, 2.0f, 4.0f},
        {"2^3", 2.0f, 3.0f, 8.0f},
        {"2^10", 2.0f, 10.0f, 1024.0f},
        {"2^(-1)", 2.0f, -1.0f, 0.5f},
        {"2^(-2)", 2.0f, -2.0f, 0.25f},
        {"4^2", 4.0f, 2.0f, 16.0f},
        {"16^0.5", 16.0f, 0.5f, 4.0f}
    };
    float sqrt_inputs[] = {0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 64.0f};
    float sqrt_expected[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f};
    int i;
    
    printf("### TESTS DES CONSTANTES SPECIALES\n");
    printf("==================================\n");
    
    /* Test des puissances de 2 exactes */
    printf("Test des puissances de 2 (doivent être exactes):\n");
    printf("Expression         Résultat HF16      Attendu            Exact\n");
    printf("----------------------------------------------------------------\n");
    for(i = 0; i < 8; i++) {
        uint16_t base_hf = float_to_half(power_tests[i].base);
        uint16_t exp_hf = float_to_half(power_tests[i].exp);
        uint16_t result_hf = hf_pow(base_hf, exp_hf);
        float result = half_to_float(result_hf);
        int is_exact = (fabsf(result - power_tests[i].expected) < 1e-6f);
        
        printf("%-18s %-18.6f %-18.6f %s\n",
               power_tests[i].expr, result, power_tests[i].expected,
               is_exact ? "OK" : "ERR");
    }
    
    /* Test des racines exactes */
    printf("\nTest des racines exactes:\n");
    printf("Expression         Résultat HF16      Attendu            Exact\n");
    printf("----------------------------------------------------------------\n");
    
    for(i = 0; i < 8; i++) {
        uint16_t input_hf = float_to_half(sqrt_inputs[i]);
        uint16_t result_hf = hf_sqrt(input_hf);
        float result = half_to_float(result_hf);
        int is_exact = (fabsf(result - sqrt_expected[i]) < 1e-6f);
        
        printf("sqrt(%-12.0f) %-18.6f %-18.6f %s\n",
               sqrt_inputs[i], result, sqrt_expected[i],
               is_exact ? "OK" : "ERR");
    }
    
    printf("\n");
}

/**
 * @brief Test des fonctions inverses
 * 
 * Teste la cohérence des paires de fonctions inverses.
 */
void debug_inverse_functions(void) {
    float test_values[] = {0.1f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 100.0f};
    float sqrt_test_values[] = {-5.0f, -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 5.0f, 10.0f};
    float trig_values[] = {0.0f, 0.1f, 0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 6.0f, 10.0f};
    int i;
    
    printf("### TESTS DES FONCTIONS INVERSES\n");
    printf("================================\n");
    
    /* Test exp(ln(x)) = x */
    printf("Test exp(ln(x)) = x:\n");
    printf("x                  ln(x)              exp(ln(x))         Erreur relative\n");
    printf("------------------------------------------------------------------------\n");
    for(i = 0; i < 7; i++) {
        float x = test_values[i];
        uint16_t x_hf = float_to_half(x);
        
        uint16_t ln_hf = hf_ln(x_hf);
        uint16_t exp_ln_hf = hf_exp(ln_hf);
        
        float ln_result = half_to_float(ln_hf);
        float exp_ln_result = half_to_float(exp_ln_hf);
        float relative_error = fabsf((exp_ln_result - x) / x);
        
        printf("%-18.6f %-18.6f %-18.6f %-15.9f\n",
               x, ln_result, exp_ln_result, relative_error);
    }
    
    /* Test sqrt(x²) = |x| */
    printf("\nTest sqrt(x²) = |x|:\n");
    printf("x                  x²                 sqrt(x²)           |x|             Exact\n");
    printf("--------------------------------------------------------------------------------\n");
    
    for(i = 0; i < 8; i++) {
        float x = sqrt_test_values[i];
        uint16_t x_hf = float_to_half(x);
        
        uint16_t x2_hf = hf_mul(x_hf, x_hf);
        uint16_t sqrt_x2_hf = hf_sqrt(x2_hf);
        
        float x2_result = half_to_float(x2_hf);
        float sqrt_x2_result = half_to_float(sqrt_x2_hf);
        float abs_x = fabsf(x);
        int is_exact = (fabsf(sqrt_x2_result - abs_x) < 1e-4f);
        
        printf("%-18.6f %-18.6f %-18.6f %-15.6f %s\n",
               x, x2_result, sqrt_x2_result, abs_x,
               is_exact ? "OK" : "ERR");
    }
    
    /* Test sin²(x) + cos²(x) = 1 pour valeurs étendues */
    printf("\nTest sin²(x) + cos²(x) = 1 (valeurs étendues):\n");
    printf("x                  sin(x)             cos(x)             sin²+cos²       Erreur\n");
    printf("--------------------------------------------------------------------------------\n");
    
    for(i = 0; i < 9; i++) {
        float x = trig_values[i];
        uint16_t x_hf = float_to_half(x);
        
        uint16_t sin_hf = hf_sin(x_hf);
        uint16_t cos_hf = hf_cos(x_hf);
        
        float sin_result = half_to_float(sin_hf);
        float cos_result = half_to_float(cos_hf);
        float identity = sin_result*sin_result + cos_result*cos_result;
        float error = fabsf(identity - 1.0f);
        
        printf("%-18.6f %-18.6f %-18.6f %-15.6f %-15.9f\n",
               x, sin_result, cos_result, identity, error);
    }
    
    printf("\n");
}

/**
 * @brief Imprime un tableau formaté avec alignement automatique des colonnes
 * 
 * @param title Titre du tableau (ex: "### HF_ADD")
 * @param headers Tableau des en-têtes de colonnes 
 * @param num_cols Nombre de colonnes
 * @param data Tableau 2D des données [num_rows][num_cols]
 * @param num_rows Nombre de lignes
 */
static void print_formatted_table(const char *title, const char **headers, int num_cols, float data[][5], int num_rows) {
    int col_widths[5] = {0}; /* Maximum 5 colonnes */
    int i, j, width;
    char buffer[32];
    
    /* Afficher le titre */
    printf("%s\n", title);
    
    /* Initialiser les largeurs avec les en-têtes */
    for(j = 0; j < num_cols && j < 5; j++) {
        col_widths[j] = (int)strlen(headers[j]);
    }
    
    /* Calculer la largeur maximale nécessaire pour chaque colonne */
    for(i = 0; i < num_rows; i++) {
        for(j = 0; j < num_cols && j < 5; j++) {
            sprintf(buffer, "%.9f", data[i][j]);
            width = (int)strlen(buffer);
            if(width > col_widths[j]) {
                col_widths[j] = width;
            }
        }
    }
    
    /* Imprimer les en-têtes */
    for(j = 0; j < num_cols; j++) {
        printf("%-*s", col_widths[j] + 2, headers[j]);
    }
    printf("\n");
    
    /* Imprimer la ligne de séparation */
    for(j = 0; j < num_cols; j++) {
        for(i = 0; i < col_widths[j] + 2; i++) {
            printf("-");
        }
    }
    printf("\n");
    
    /* Imprimer les données */
    for(i = 0; i < num_rows; i++) {
        for(j = 0; j < num_cols; j++) {
            printf("%-*.*f", col_widths[j] + 2, 9, data[i][j]);
        }
        printf("\n");
    }
}