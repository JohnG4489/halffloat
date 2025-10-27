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

//Prototypes des fonctions
void debug_abs(void);
void debug_neg(void);
void debug_add(void);
void debug_mul(void);
void debug_div(void);
void debug_inv(void);
void debug_sqrt(void);
void debug_rsqrt(void);
void debug_pow(void);
void debug_exp(void);
void debug_int(void);
void debug_ln(void);
void debug_sin(void);
void debug_cos(void);
void debug_tan(void);
void debug_asin(void);
void debug_acos(void);
void debug_denormal_values(void);
void debug_mathematical_identities(void);
void debug_ieee754_edge_cases(void);
void debug_precision_stress_test(void);
void debug_comparative_accuracy(void);
void debug_boundary_conditions(void);
void debug_special_constants(void);
void debug_inverse_functions(void);
void debug_rsqrt_comparison(void);

//Fonction utilitaire locale
static void print_formatted_table(const char *title, const char **headers, int num_cols, float data[][8], int num_rows);

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
    float results[30][8]; /* Tableau statique étendu pour tous les tests */
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
    float results[30][8]; /* Tableau statique étendu pour tous les tests */
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
    float results[50][8]; /* Tableau statique suffisant pour tous les tests */
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
    float results[50][8]; /* Tableau statique suffisant pour tous les tests */
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
    float results[50][8]; /* Tableau statique suffisant pour tous les tests */
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
 * @brief Fonction de débogage pour tester la fonction hf_inv avec divers cas de test
 *
 * Cette fonction teste l'inversion de demi-flottants avec des cas incluant
 * les nombres normaux, dénormalisés, les cas spéciaux IEEE 754,
 * et compare les résultats avec l'inversion standard en float.
 */
void debug_inv(void) {
    float test_cases[] = {
        //Inversions simples
        1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 0.5f, 0.25f, 0.125f, 0.0625f,
        //Cas spéciaux IEEE 754
        0.0f, -0.0f, -1.0f, half_to_float(HF_INFINITY_POS),
        half_to_float(HF_INFINITY_NEG), half_to_float(HF_NAN),
        //Grandes valeurs (proche overflow après inversion)
        65504.0f, 32768.0f, 10000.0f, 1000.0f, 100.0f,
        -65504.0f, -32768.0f, -10000.0f, -1000.0f, -100.0f,
        //Petites valeurs (risque de débordement après inversion)
        0.0001f, 0.00001f, 0.000001f, 6e-8f, 1e-7f, 1e-6f,
        -0.0001f, -0.00001f, -0.000001f, -6e-8f, -1e-7f,
        //Valeurs dénormalisées et très petites
        5.96e-8f, -5.96e-8f, 1e-10f, 1e-15f,
        //Cas edge et précision
        0.999999f, 1.000001f, 3.0f, 7.0f, 10.0f,
        //Valeurs fractionnaires
        0.1f, 0.01f, 0.001f, 0.0001f,
        //Valeurs négatives
        -2.0f, -4.0f, -0.5f, -0.25f, -10.0f, -100.0f
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    //Préparer les données pour le tableau formaté
    float results[100][8]; //Tableau statique suffisant pour tous les tests
    const char *headers[] = {"Value", "Result (my_inv)", "Result (1.0/x)", "Difference"};

    for(i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);
        uint16_t result_half = hf_inv(value_half);
        float result_float = half_to_float(result_half);
        float std_result = 1.0f / half_to_float(value_half);
        float diff = fabsf(result_float - std_result);

        //Stocker les résultats dans le tableau
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    //Afficher le tableau formaté
    print_formatted_table("### HF_INV", headers, 4, results, num_tests);
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
    float results[50][8]; /* Tableau statique suffisant pour tous les tests étendus */
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
 * @brief Fonction de débogage pour tester la fonction hf_rsqrt avec divers cas de test
 *
 * Cette fonction teste la racine carrée inverse de demi-flottants incluant les nombres
 * positifs, négatifs (qui doivent donner NaN), zéro (qui doit donner l'infini), 
 * infini et NaN, et compare les résultats avec 1/sqrt() standard.
 */
void debug_rsqrt(void) {
    float test_cases[] = {
        //Valeurs classiques
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 16.0f,
        
        //Cas spéciaux IEEE 754 (déjà présents)
        0.0f, -0.0f, 0.25f, -1.0f, 
        half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG), 
        half_to_float(HF_NAN),
        
        //Valeurs dénormalisées et petites
        0.000061035f, 5.96e-8f, 1e-10f,
        
        //Valeurs limites du half-float
        65504.0f,      //Max half-float normal
        6.10e-5f,      //Min half-float normal (environ 2^-14)
        6.0e-8f,       //Valeur dénormalisée
        5.96e-8f,      //Plus petite valeur dénormalisée non-nulle
        
        //Puissances de 2 (doivent être exactes)
        0.0625f,       //2^-4 -> rsqrt = 4
        0.125f,        //2^-3 -> rsqrt = 2*sqrt(2)
        0.5f,          //2^-1 -> rsqrt = sqrt(2)
        1.0f,          //2^0  -> rsqrt = 1
        4.0f,          //2^2  -> rsqrt = 0.5
        16.0f,         //2^4  -> rsqrt = 0.25
        64.0f,         //2^6  -> rsqrt = 0.125
        256.0f,        //2^8  -> rsqrt = 0.0625
        1024.0f,       //2^10 -> rsqrt = 0.03125
        
        //Valeurs proches de 1 (test précision)
        0.99f, 0.999f, 0.9999f,
        1.001f, 1.01f, 1.1f,
        
        //AJOUTS: Nombres négatifs variés
        -0.5f, -2.0f, -100.0f,
        
        //Grandes valeurs
        100.0f, 10000.0f,
        
        //Valeurs fractionnaires
        0.1f, 0.01f, 0.001f, 0.0001f,
        
        //Valeurs intermédiaires
        2.0f, 3.0f
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;
   
    float results[50][8];
    const char *headers[] = {"Value", "Result (my_rsqrt)", "Result (1/std::sqrt)", "Difference"};
   
    for(i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);
        uint16_t result_half = hf_rsqrt(value_half);
        float result_float = half_to_float(result_half);
       
        //Calcul de référence: 1/sqrt(x)
        float value_converted = half_to_float(value_half);
        float std_result, diff;
        
        if(isnan(value_converted)) {
            std_result = NAN;
        } else if(isinf(value_converted)) {
            std_result = (value_converted > 0.0f) ? 0.0f : NAN;
        } else if(value_converted == 0.0f) {
            std_result = INFINITY;
        } else if(value_converted < 0.0f) {
            std_result = NAN;
        } else {
            std_result = 1.0f / sqrtf(value_converted);
        }
        
        diff = fabsf(result_float - std_result);
       
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
   
    print_formatted_table("### HF_RSQRT", headers, 4, results, num_tests);
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
        {-2.0f, 0.5f}, {-4.0f, 0.25f}, {-1.0f, 2.5f},
        //Cas spéciaux IEEE 754 - Puissances avec base -1
        {-1.0f, 0.0f},       //(-1)^0 = 1
        {-1.0f, -0.0f},      //(-1)^(-0) = 1
        {-1.0f, 1.0f},       //(-1)^1 = -1
        {-1.0f, 2.0f},       //(-1)^2 = +1
        {-1.0f, 3.0f},       //(-1)^3 = -1
        {-1.0f, 4.0f},       //(-1)^4 = +1
        {-1.0f, -1.0f},      //(-1)^(-1) = -1
        {-1.0f, -2.0f},      //(-1)^(-2) = +1
        {-1.0f, -3.0f},      //(-1)^(-3) = -1
        {-1.0f, 0.5f},       //(-1)^0.5 = NaN (indéfini en réel)
        {-1.0f, -0.5f},      //(-1)^(-0.5) = NaN (idem)
        {-1.0f, 2.5f},       //(-1)^2.5 = NaN
        {-1.0f, half_to_float(HF_INFINITY_POS)},  //(-1)^+Inf = NaN
        {-1.0f, half_to_float(HF_INFINITY_NEG)}  //(-1)^-Inf = NaN
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    /* Préparer les données pour le tableau formaté */
    float results[70][8]; /* Tableau statique étendu pour tous les tests */
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
    float results[50][8]; /* Tableau statique suffisant pour tous les tests étendus */
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
    float results[50][8]; /* Tableau statique suffisant pour tous les tests étendus */
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
    float results[50][8]; /* Tableau statique suffisant pour tous les tests étendus */
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
    float results[25][8]; /* Tableau statique suffisant pour tous les tests */
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
    float results[25][8]; /* Tableau statique suffisant pour tous les tests */
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
    float results[70][8]; /* Tableau statique suffisant pour tous les tests */
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
 * @brief Fonction de débogage pour tester la fonction hf_asin avec divers cas de test
 *
 * Cette fonction teste l'arc sinus de demi-flottants avec des valeurs variées
 * incluant les valeurs remarquables (0, +/-0.5, +/-1), les valeurs limites du domaine,
 * les cas spéciaux IEEE 754, et compare avec asinf() standard.
 */
void debug_asin(void) {
    float test_cases[] = {
        //Zéros
        0.0f, -0.0f,
        //Valeurs remarquables
        0.5f, -0.5f,              //asin(+/-0.5) = +/-pi/6
        0.7071067811865476f,      //asin(sqrt(2)/2) = pi/4
        -0.7071067811865476f,     //asin(-sqrt(2)/2) = -pi/4
        0.8660254037844387f,      //asin(sqrt(3)/2) = pi/3
        -0.8660254037844387f,     //asin(-sqrt(3)/2) = -pi/3
        //Limites du domaine
        1.0f, -1.0f,              //asin(+/-1) = +/-pi/2
        0.999999f, -0.999999f,    //Très proche de +/-1
        0.9999f, -0.9999f,
        //Petites valeurs
        0.1f, -0.1f,
        0.01f, -0.01f,
        0.001f, -0.001f,
        0.0001f, -0.0001f,
        0.000061035f, -0.000061035f,
        //Valeurs intermédiaires pour tester l'interpolation
        0.25f, -0.25f,
        0.75f, -0.75f,
        0.3f, -0.3f,
        0.6f, -0.6f,
        //Hors domaine (doivent donner NaN)
        1.0001f, -1.0001f,
        2.0f, -2.0f,
        10.0f, -10.0f,
        65504.0f, -65504.0f,
        //Cas spéciaux IEEE 754
        half_to_float(HF_INFINITY_POS),
        half_to_float(HF_INFINITY_NEG),
        half_to_float(HF_NAN)
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;
    //Préparer les données pour le tableau formaté
    float results[50][8]; //Tableau statique suffisant pour tous les tests
    const char *headers[] = {"Value", "Result (hf_asin)", "Result (asinf)", "Difference"};
    
    for(i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);
        uint16_t result_half = hf_asin(value_half);
        float result_float = half_to_float(result_half);
        float std_result = asinf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);
        
        //Stocker les résultats dans le tableau
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
    }
    
    //Afficher le tableau formaté
    print_formatted_table("### HF_ASIN", headers, 4, results, num_tests);
    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_acos avec divers cas de test
 *
 * Cette fonction teste l'arc cosinus de demi-flottants avec des valeurs variées
 * incluant les valeurs remarquables (0, +/-0.5, +/-1), les valeurs limites du domaine,
 * les cas spéciaux IEEE 754, et compare avec acosf() standard.
 */
void debug_acos(void) {
    float test_cases[] = {
        //Zéros
        0.0f, -0.0f,
        //Valeurs remarquables
        0.5f, -0.5f,              //acos(0.5) = pi/3, acos(-0.5) = 2pi/3
        0.7071067811865476f,      //acos(sqrt(2)/2) = pi/4
        -0.7071067811865476f,     //acos(-sqrt(2)/2) = 3pi/4
        0.8660254037844387f,      //acos(sqrt(3)/2) = pi/6
        -0.8660254037844387f,     //acos(-sqrt(3)/2) = 5pi/6
        //Limites du domaine
        1.0f, -1.0f,              //acos(1) = 0, acos(-1) = pi
        0.999999f, -0.999999f,    //Très proche de +/-1
        0.9999f, -0.9999f,
        //Petites valeurs
        0.1f, -0.1f,
        0.01f, -0.01f,
        0.001f, -0.001f,
        0.0001f, -0.0001f,
        0.000061035f, -0.000061035f,
        //Valeurs intermédiaires pour tester l'interpolation
        0.25f, -0.25f,
        0.75f, -0.75f,
        0.3f, -0.3f,
        0.6f, -0.6f,
        //Hors domaine (doivent donner NaN)
        1.0001f, -1.0001f,
        2.0f, -2.0f,
        10.0f, -10.0f,
        65504.0f, -65504.0f,
        //Cas spéciaux IEEE 754
        half_to_float(HF_INFINITY_POS),
        half_to_float(HF_INFINITY_NEG),
        half_to_float(HF_NAN)
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;
    //Préparer les données pour le tableau formaté
    float results[50][8]; //Tableau statique suffisant pour tous les tests
    const char *headers[] = {"Value", "Result (hf_acos)", "Result (acosf)", "Difference"};
   
    for(i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);
        uint16_t result_half = hf_acos(value_half);
        float result_float = half_to_float(result_half);
        float std_result = acosf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);
       
        //Stocker les résultats dans le tableau
        results[i][0] = value;
        results[i][1] = result_float;
        results[i][2] = std_result;
        results[i][3] = diff;
        results[i][4] = 0.0f; //Colonne inutilisée
    }
   
    //Afficher le tableau formaté
    print_formatted_table("### HF_ACOS", headers, 4, results, num_tests);
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
    //Valeurs dénormalisées critiques en FP16
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
    float results[10][8];
    const char *headers[] = {"Value", "Sqrt", "Exp", "Ln", "Sin"};
    
    for(i = 0; i < num_denormal; i++) {
        float val = denormal_values[i];
        uint16_t hf_val = float_to_half(val);
        float sqrt_result = half_to_float(hf_sqrt(hf_val));
        float exp_result = half_to_float(hf_exp(hf_val));
        float ln_result = half_to_float(hf_ln(hf_val));
        float sin_result = half_to_float(hf_sin(hf_val));
        
        results[i][0] = val;
        results[i][1] = sqrt_result;
        results[i][2] = exp_result;
        results[i][3] = ln_result;
        results[i][4] = sin_result;
    }
    
    print_formatted_table("### TESTS DES VALEURS DENORMALISEES FP16", headers, 5, results, num_denormal);
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
    float results_trig[5][8];
    const char *headers_trig[] = {"Value (x)", "sin^2+cos^2", "Error"};
    float results_exp_ln[5][8];
    const char *headers_exp_ln[] = {"Value (x)", "exp(ln(x))", "Relative Error"};
    
    printf("\n### TESTS DES IDENTITES MATHEMATIQUES\n");
    printf("======================================\n\n");
    
    for(i = 0; i < num_values; i++) {
        float x = test_values[i];
        uint16_t hf_x = float_to_half(x);
        float sin_val = half_to_float(hf_sin(hf_x));
        float cos_val = half_to_float(hf_cos(hf_x));
        float identity_result = sin_val * sin_val + cos_val * cos_val;
        float error = fabsf(identity_result - 1.0f);
        
        results_trig[i][0] = x;
        results_trig[i][1] = identity_result;
        results_trig[i][2] = error;
    }
    
    print_formatted_table("Identite sin^2(x) + cos^2(x) = 1", headers_trig, 3, results_trig, num_values);
    printf("\n");
    
    for(i = 0; i < num_values; i++) {
        float x = test_values[i];
        uint16_t hf_x = float_to_half(x);
        uint16_t ln_result = hf_ln(hf_x);
        float exp_ln_result = half_to_float(hf_exp(ln_result));
        float error = fabsf((exp_ln_result - x) / x);
        
        results_exp_ln[i][0] = x;
        results_exp_ln[i][1] = exp_ln_result;
        results_exp_ln[i][2] = error;
    }
    
    print_formatted_table("Identite exp(ln(x)) = x", headers_exp_ln, 3, results_exp_ln, num_values);
    printf("\n");
}

/**
 * @brief Tests de cas edge IEEE 754 spéciaux
 * 
 * Cette fonction teste des cas edge très spécifiques qui peuvent
 * causer des problèmes dans les implémentations FP16.
 */
void debug_ieee754_edge_cases(void) {
    //Tests de valeurs limites FP16
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
    //Tests de précision de représentation FP16
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
    float results_edge[12][8];
    const char *headers_edge[] = {"Value", "Add(+1)", "Mul(*2)", "Sqrt", "Exp"};
    float results_precision[5][8];
    const char *headers_precision[] = {"Original Value", "Converted FP16", "Relative Error"};
    
    printf("\n### TESTS DES CAS EDGE IEEE 754\n");
    printf("================================\n\n");
    
    for(i = 0; i < num_edge; i++) {
        float val = edge_values[i];
        uint16_t hf_val = float_to_half(val);
        uint16_t hf_one = float_to_half(1.0f);
        uint16_t hf_two = float_to_half(2.0f);
        float add_result = half_to_float(hf_add(hf_val, hf_one));
        float mul_result = half_to_float(hf_mul(hf_val, hf_two));
        float sqrt_result = half_to_float(hf_sqrt(hf_val));
        float exp_result = half_to_float(hf_exp(hf_val));
        
        results_edge[i][0] = val;
        results_edge[i][1] = add_result;
        results_edge[i][2] = mul_result;
        results_edge[i][3] = sqrt_result;
        results_edge[i][4] = exp_result;
    }
    
    print_formatted_table("Test des operations avec valeurs limites", headers_edge, 5, results_edge, num_edge);
    printf("\n");
    
    for(i = 0; i < num_precision; i++) {
        float original = precision_values[i];
        uint16_t hf_val = float_to_half(original);
        float converted = half_to_float(hf_val);
        float error = fabsf((converted - original) / original);
        
        results_precision[i][0] = original;
        results_precision[i][1] = converted;
        results_precision[i][2] = error;
    }
    
    print_formatted_table("Test de precision de representation FP16", headers_precision, 3, results_precision, num_precision);
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
        1.5707963f,      //pi/2 exact (théorique)
        1.5703125f,      //Légèrement en dessous
        1.5712890625f,   //Légèrement au dessus
        1.5695f,         //Plus loin
        1.5720f          //Plus loin
    };
    float small_values[] = {
        1e-4f, 1e-5f, 5.96e-8f, 1e-7f, 1e-6f
    };
    float extreme_values[] = {
        11.0f,    //Proche de la limite d'overflow
        -11.0f,   //Proche de la limite d'underflow
        10.5f,    //Valeur intermédiaire
        -10.5f,   //Valeur intermédiaire négative
        0.0f      //Zéro (cas spécial)
    };
    int i;
    float results_tan[5][8];
    const char *headers_tan[] = {"Value", "tan(x)", "tanf(x)", "Relative Error"};
    float results_sincos[5][8];
    const char *headers_sincos[] = {"Value", "sin(x)", "cos(x)", "sin^2+cos^2"};
    float results_exp_ln[5][8];
    const char *headers_exp_ln[] = {"Value", "exp(x)", "ln(exp(x))", "Error"};
    
    printf("### TESTS DE STRESS DE PRECISION\n");
    printf("================================\n\n");
    
    for(i = 0; i < 5; i++) {
        float val = pi_half_values[i];
        uint16_t hf_val = float_to_half(val);
        float converted_val = half_to_float(hf_val);
        uint16_t result_hf = hf_tan(hf_val);
        float result_float = half_to_float(result_hf);
        float std_result = tanf(converted_val);
        float relative_error = fabsf((result_float - std_result) / std_result);
        
        results_tan[i][0] = converted_val;
        results_tan[i][1] = result_float;
        results_tan[i][2] = std_result;
        results_tan[i][3] = relative_error;
    }
    
    print_formatted_table("Test tan() pres de pi/2", headers_tan, 4, results_tan, 5);
    printf("\n");
    
    for(i = 0; i < 5; i++) {
        float val = small_values[i];
        uint16_t hf_val = float_to_half(val);
        uint16_t sin_hf = hf_sin(hf_val);
        uint16_t cos_hf = hf_cos(hf_val);
        float sin_result = half_to_float(sin_hf);
        float cos_result = half_to_float(cos_hf);
        float identity_check = sin_result*sin_result + cos_result*cos_result;
        
        results_sincos[i][0] = half_to_float(hf_val);
        results_sincos[i][1] = sin_result;
        results_sincos[i][2] = cos_result;
        results_sincos[i][3] = identity_check;
    }
    
    print_formatted_table("Test sin/cos avec tres petites valeurs", headers_sincos, 4, results_sincos, 5);
    printf("\n");
    
    for(i = 0; i < 5; i++) {
        float val = extreme_values[i];
        uint16_t hf_val = float_to_half(val);
        uint16_t exp_hf = hf_exp(hf_val);
        uint16_t ln_exp_hf = hf_ln(exp_hf);
        float exp_result = half_to_float(exp_hf);
        float ln_exp_result = half_to_float(ln_exp_hf);
        float error = fabsf(ln_exp_result - half_to_float(hf_val));
        
        results_exp_ln[i][0] = half_to_float(hf_val);
        results_exp_ln[i][1] = exp_result;
        results_exp_ln[i][2] = ln_exp_result;
        results_exp_ln[i][3] = error;
    }
    
    print_formatted_table("Test exp/ln aux limites de representation", headers_exp_ln, 4, results_exp_ln, 5);
    printf("\n");
}

/**
 * @brief Test de précision comparative entre différentes implémentations
 * 
 * Compare la précision de nos implémentations avec les fonctions standard
 * sur des valeurs de référence mathématiques connues.
 */
void debug_comparative_accuracy(void) {
    float constants_values[] = {
        3.141592653589793f,   //Pi
        2.718281828459045f,   //e
        1.4142135623730951f,  //sqrt(2)
        1.7320508075688772f,  //sqrt(3)
        0.6931471805599453f,  //ln(2)
        2.302585092994046f,   //ln(10)
        0.3183098861837907f,  //1/pi
        1.5707963267948966f,  //pi/2
        0.7853981633974483f,  //pi/4
        6.283185307179586f    //2*pi
    };
    float trig_inputs[] = {
        3.141592653589793f/6.0f,   //pi/6 pour sin
        3.141592653589793f/3.0f,   //pi/3 pour cos
        3.141592653589793f/2.0f,   //pi/2 pour sin
        3.141592653589793f/2.0f,   //pi/2 pour cos
        3.141592653589793f,        //pi pour sin
        3.141592653589793f         //pi pour cos
    };
    float trig_expected[] = {
        0.5f,   //sin(pi/6)
        0.5f,   //cos(pi/3)
        1.0f,   //sin(pi/2)
        0.0f,   //cos(pi/2)
        0.0f,   //sin(pi)
        -1.0f   //cos(pi)
    };
    int i;
    float results_constants[10][8];
    const char *headers_constants[] = {"Theoretical", "HF16 Value", "Relative Error"};
    float results_trig[6][8];
    const char *headers_trig[] = {"Input", "Theoretical", "HF16 Result", "Error"};
    
    printf("### TESTS DE PRECISION COMPARATIVE\n");
    printf("==================================\n\n");
    
    for(i = 0; i < 10; i++) {
        float theoretical = constants_values[i];
        uint16_t hf_val = float_to_half(theoretical);
        float hf_converted = half_to_float(hf_val);
        float relative_error = fabsf((hf_converted - theoretical) / theoretical);
        
        results_constants[i][0] = theoretical;
        results_constants[i][1] = hf_converted;
        results_constants[i][2] = relative_error;
    }
    
    print_formatted_table("Test des constantes mathematiques", headers_constants, 3, results_constants, 10);
    printf("\n");
    
    for(i = 0; i < 6; i++) {
        uint16_t hf_input = float_to_half(trig_inputs[i]);
        uint16_t result_hf;
        float result;
        float error;
        
        //Alterner entre sin et cos
        if(i == 0 || i == 2 || i == 4) {
            result_hf = hf_sin(hf_input);
        } else {
            result_hf = hf_cos(hf_input);
        }
        
        result = half_to_float(result_hf);
        error = fabsf(result - trig_expected[i]);
        
        results_trig[i][0] = trig_inputs[i];
        results_trig[i][1] = trig_expected[i];
        results_trig[i][2] = result;
        results_trig[i][3] = error;
    }
    
    print_formatted_table("Test trigonometrique sur constantes", headers_trig, 4, results_trig, 6);
    printf("\n");
}

/**
 * @brief Test des conditions aux limites
 * 
 * Teste le comportement aux limites exactes de la représentation FP16.
 */
void debug_boundary_conditions(void) {
    uint16_t boundary_values[] = {
        0x0000,  //+0
        0x8000,  //-0
        0x0001,  //Plus petit dénormalisé positif
        0x8001,  //Plus petit dénormalisé négatif
        0x03FF,  //Plus grand dénormalisé positif
        0x83FF,  //Plus grand dénormalisé négatif
        0x0400,  //Plus petit normalisé positif
        0x8400,  //Plus petit normalisé négatif
        0x7BFF,  //Plus grand fini positif
        0xFBFF,  //Plus grand fini négatif
        0x7C00,  //+inf
        0xFC00,  //-inf
        0x7E00,  //NaN
        0xFE00   //-NaN
    };
    uint16_t large_val;
    uint16_t add_overflow;
    uint16_t tiny_val;
    uint16_t div_underflow;
    int i;
    float results_boundary[14][8];
    const char *headers_boundary[] = {"Value", "sqrt", "exp", "ln"};
    float results_transitions[2][8];
    const char *headers_transitions[] = {"Input", "Output", "Expected", "IEEE 754 OK"};
    
    printf("### TESTS DES CONDITIONS AUX LIMITES\n");
    printf("====================================\n\n");
    
    for(i = 0; i < 14; i++) {
        uint16_t val = boundary_values[i];
        uint16_t sqrt_result = hf_sqrt(val);
        uint16_t exp_result = hf_exp(val);
        uint16_t ln_result = hf_ln(val);
        float val_f = half_to_float(val);
        float sqrt_f = half_to_float(sqrt_result);
        float exp_f = half_to_float(exp_result);
        float ln_f = half_to_float(ln_result);
        
        results_boundary[i][0] = val_f;
        results_boundary[i][1] = sqrt_f;
        results_boundary[i][2] = exp_f;
        results_boundary[i][3] = ln_f;
    }
    
    print_formatted_table("Test des operations sur les valeurs limites", headers_boundary, 4, results_boundary, 14);
    printf("\n");
    
    //Test de débordement
    large_val = float_to_half(65000.0f);
    add_overflow = hf_add(large_val, large_val);
    results_transitions[0][0] = 65000.0f;
    results_transitions[0][1] = half_to_float(add_overflow);
    results_transitions[0][2] = INFINITY;
    results_transitions[0][3] = (add_overflow == 0x7C00) ? 1.0f : 0.0f;
    
    //Test de sous-débordement
    tiny_val = 0x0001;
    div_underflow = hf_div(tiny_val, float_to_half(2.0f));
    results_transitions[1][0] = half_to_float(tiny_val);
    results_transitions[1][1] = half_to_float(div_underflow);
    results_transitions[1][2] = 0.0f;
    results_transitions[1][3] = (div_underflow == 0x0000) ? 1.0f : 0.0f;
    
    print_formatted_table("Test des transitions critiques", headers_transitions, 4, results_transitions, 2);
    printf("\n");
}

/**
 * @brief Test des constantes spéciales et cas particuliers
 * 
 * Teste la gestion des constantes mathématiques et cas particuliers.
 */
void debug_special_constants(void) {
    float power_bases[] = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 4.0f, 16.0f};
    float power_exps[] = {1.0f, 2.0f, 3.0f, 10.0f, -1.0f, -2.0f, 2.0f, 0.5f};
    float power_expected[] = {2.0f, 4.0f, 8.0f, 1024.0f, 0.5f, 0.25f, 16.0f, 4.0f};
    float sqrt_inputs[] = {0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 64.0f};
    float sqrt_expected[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f};
    int i;
    float results_pow[8][8];
    const char *headers_pow[] = {"Base", "Exponent", "Result HF16", "Expected", "Exact"};
    float results_sqrt[8][8];
    const char *headers_sqrt[] = {"Input", "Result HF16", "Expected", "Exact"};
    
    printf("### TESTS DES CONSTANTES SPECIALES\n");
    printf("==================================\n\n");
    
    for(i = 0; i < 8; i++) {
        uint16_t base_hf = float_to_half(power_bases[i]);
        uint16_t exp_hf = float_to_half(power_exps[i]);
        uint16_t result_hf = hf_pow(base_hf, exp_hf);
        float result = half_to_float(result_hf);
        int is_exact = (fabsf(result - power_expected[i]) < 1e-6f);
        
        results_pow[i][0] = power_bases[i];
        results_pow[i][1] = power_exps[i];
        results_pow[i][2] = result;
        results_pow[i][3] = power_expected[i];
        results_pow[i][4] = is_exact ? 1.0f : 0.0f;
    }
    
    print_formatted_table("Test des puissances de 2 (doivent etre exactes)", headers_pow, 5, results_pow, 8);
    printf("\n");
    
    for(i = 0; i < 8; i++) {
        uint16_t input_hf = float_to_half(sqrt_inputs[i]);
        uint16_t result_hf = hf_sqrt(input_hf);
        float result = half_to_float(result_hf);
        int is_exact = (fabsf(result - sqrt_expected[i]) < 1e-6f);
        
        results_sqrt[i][0] = sqrt_inputs[i];
        results_sqrt[i][1] = result;
        results_sqrt[i][2] = sqrt_expected[i];
        results_sqrt[i][3] = is_exact ? 1.0f : 0.0f;
    }
    
    print_formatted_table("Test des racines exactes", headers_sqrt, 4, results_sqrt, 8);
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
    float results_exp_ln[7][8];
    const char *headers_exp_ln[] = {"x", "ln(x)", "exp(ln(x))", "Relative Error"};
    float results_sqrt_sq[8][8];
    const char *headers_sqrt_sq[] = {"x", "x^2", "sqrt(x^2)", "|x|", "Exact"};
    float results_trig_identity[9][8];
    const char *headers_trig_identity[] = {"x", "sin(x)", "cos(x)", "sin^2+cos^2", "Error"};
    
    printf("### TESTS DES FONCTIONS INVERSES\n");
    printf("================================\n\n");
    
    for(i = 0; i < 7; i++) {
        float x = test_values[i];
        uint16_t x_hf = float_to_half(x);
        uint16_t ln_hf = hf_ln(x_hf);
        uint16_t exp_ln_hf = hf_exp(ln_hf);
        float ln_result = half_to_float(ln_hf);
        float exp_ln_result = half_to_float(exp_ln_hf);
        float relative_error = fabsf((exp_ln_result - x) / x);
        
        results_exp_ln[i][0] = x;
        results_exp_ln[i][1] = ln_result;
        results_exp_ln[i][2] = exp_ln_result;
        results_exp_ln[i][3] = relative_error;
    }
    
    print_formatted_table("Test exp(ln(x)) = x", headers_exp_ln, 4, results_exp_ln, 7);
    printf("\n");
    
    for(i = 0; i < 8; i++) {
        float x = sqrt_test_values[i];
        uint16_t x_hf = float_to_half(x);
        uint16_t x2_hf = hf_mul(x_hf, x_hf);
        uint16_t sqrt_x2_hf = hf_sqrt(x2_hf);
        float x2_result = half_to_float(x2_hf);
        float sqrt_x2_result = half_to_float(sqrt_x2_hf);
        float abs_x = fabsf(x);
        int is_exact = (fabsf(sqrt_x2_result - abs_x) < 1e-4f);
        
        results_sqrt_sq[i][0] = x;
        results_sqrt_sq[i][1] = x2_result;
        results_sqrt_sq[i][2] = sqrt_x2_result;
        results_sqrt_sq[i][3] = abs_x;
        results_sqrt_sq[i][4] = is_exact ? 1.0f : 0.0f;
    }
    
    print_formatted_table("Test sqrt(x^2) = |x|", headers_sqrt_sq, 5, results_sqrt_sq, 8);
    printf("\n");
    
    for(i = 0; i < 9; i++) {
        float x = trig_values[i];
        uint16_t x_hf = float_to_half(x);
        uint16_t sin_hf = hf_sin(x_hf);
        uint16_t cos_hf = hf_cos(x_hf);
        float sin_result = half_to_float(sin_hf);
        float cos_result = half_to_float(cos_hf);
        float identity = sin_result*sin_result + cos_result*cos_result;
        float error = fabsf(identity - 1.0f);
        
        results_trig_identity[i][0] = x;
        results_trig_identity[i][1] = sin_result;
        results_trig_identity[i][2] = cos_result;
        results_trig_identity[i][3] = identity;
        results_trig_identity[i][4] = error;
    }
    
    print_formatted_table("Test sin^2(x) + cos^2(x) = 1 (valeurs etendues)", headers_trig_identity, 5, results_trig_identity, 9);
    printf("\n");
}

/**
 * @brief Test comparatif des différentes méthodes de calcul de rsqrt
 *
 * Compare hf_rsqrt() avec hf_div(1, hf_sqrt()) et hf_inv(hf_sqrt())
 * pour vérifier leur cohérence et leurs performances relatives.
 */
void debug_rsqrt_comparison(void) {
    float test_cases[] = {
        //Valeurs classiques
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 16.0f,
        
        //Cas spéciaux IEEE 754
        0.0f, -0.0f, 0.25f, -1.0f,
        
        //Valeurs dénormalisées et petites
        0.000061035f, 5.96e-8f, 1e-10f,
        
        //Valeurs limites du half-float
        65504.0f,      //Max half-float normal
        6.10e-5f,      //Min half-float normal (environ 2^-14)
        6.0e-8f,       //Valeur dénormalisée
        5.96e-8f,      //Plus petite valeur dénormalisée non-nulle
        
        //Puissances de 2 (doivent être exactes)
        0.0625f,       //2^-4 -> rsqrt = 4
        0.125f,        //2^-3 -> rsqrt = 2*sqrt(2)
        0.5f,          //2^-1 -> rsqrt = sqrt(2)
        4.0f,          //2^2  -> rsqrt = 0.5
        64.0f,         //2^6  -> rsqrt = 0.125
        256.0f,        //2^8  -> rsqrt = 0.0625
        1024.0f,       //2^10 -> rsqrt = 0.03125
        
        //Valeurs proches de 1 (test précision)
        0.99f, 0.999f, 0.9999f,
        1.001f, 1.01f, 1.1f,
        
        //Nombres négatifs variés
        -0.5f, -2.0f, -100.0f,
        
        //Grandes valeurs
        100.0f, 10000.0f,
        
        //Valeurs fractionnaires
        0.1f, 0.01f, 0.001f, 0.0001f,
        
        //Valeurs intermédiaires
        2.0f, 3.0f
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;
    float value, value_converted, std_result;
    uint16_t value_half, rsqrt_result, sqrt_result, one_half, div_result, inv_result;
    float rsqrt_float, div_float, inv_float;
    float err_rsqrt, err_div, err_inv;
    float results[100][8];
    const char *headers[] = {
        "Value", 
        "hf_rsqrt", 
        "1/sqrt (div)", 
        "inv(sqrt)", 
        "Ref (1/sqrtf)",
        "Err rsqrt",
        "Err div",
        "Err inv"
    };
    int count_rsqrt_best, count_div_best, count_inv_best, count_equal, valid_count;
    float max_err_rsqrt, max_err_div, max_err_inv;
    float sum_err_rsqrt, sum_err_div, sum_err_inv;
    
    printf("### COMPARAISON DES METHODES DE CALCUL DE RSQRT\n");
    printf("================================================\n\n");
    
    for(i = 0; i < num_tests; i++) {
        value = test_cases[i];
        value_half = float_to_half(value);
        
        //Méthode 1: hf_rsqrt directe
        rsqrt_result = hf_rsqrt(value_half);
        rsqrt_float = half_to_float(rsqrt_result);
        
        //Méthode 2: hf_div(1, hf_sqrt())
        sqrt_result = hf_sqrt(value_half);
        one_half = float_to_half(1.0f);
        div_result = hf_div(one_half, sqrt_result);
        div_float = half_to_float(div_result);
        
        //Méthode 3: hf_inv(hf_sqrt())
        inv_result = hf_inv(sqrt_result);
        inv_float = half_to_float(inv_result);
        
        //Calcul de référence: 1/sqrtf(x)
        value_converted = half_to_float(value_half);
        
        if(isnan(value_converted)) {
            std_result = NAN;
        } else if(isinf(value_converted)) {
            std_result = (value_converted > 0.0f) ? 0.0f : NAN;
        } else if(value_converted == 0.0f) {
            std_result = INFINITY;
        } else if(value_converted < 0.0f) {
            std_result = NAN;
        } else {
            std_result = 1.0f / sqrtf(value_converted);
        }
        
        //Calcul des erreurs absolues
        err_rsqrt = fabsf(rsqrt_float - std_result);
        err_div = fabsf(div_float - std_result);
        err_inv = fabsf(inv_float - std_result);
        
        //Gestion des cas NaN/Inf pour les erreurs
        if(isnan(std_result)) {
            err_rsqrt = isnan(rsqrt_float) ? 0.0f : INFINITY;
            err_div = isnan(div_float) ? 0.0f : INFINITY;
            err_inv = isnan(inv_float) ? 0.0f : INFINITY;
        } else if(isinf(std_result)) {
            err_rsqrt = isinf(rsqrt_float) ? 0.0f : INFINITY;
            err_div = isinf(div_float) ? 0.0f : INFINITY;
            err_inv = isinf(inv_float) ? 0.0f : INFINITY;
        }
        
        results[i][0] = value;
        results[i][1] = rsqrt_float;
        results[i][2] = div_float;
        results[i][3] = inv_float;
        results[i][4] = std_result;
        results[i][5] = err_rsqrt;
        results[i][6] = err_div;
        results[i][7] = err_inv;
    }
    
    print_formatted_table("Comparaison des méthodes", headers, 8, results, num_tests);
    
    //Statistiques comparatives
    printf("\n### STATISTIQUES COMPARATIVES\n");
    printf("==============================\n\n");
    
    count_rsqrt_best = 0;
    count_div_best = 0;
    count_inv_best = 0;
    count_equal = 0;
    max_err_rsqrt = 0.0f;
    max_err_div = 0.0f;
    max_err_inv = 0.0f;
    sum_err_rsqrt = 0.0f;
    sum_err_div = 0.0f;
    sum_err_inv = 0.0f;
    valid_count = 0;
    
    for(i = 0; i < num_tests; i++) {
        err_rsqrt = results[i][5];
        err_div = results[i][6];
        err_inv = results[i][7];
        
        //Ignorer les cas NaN/Inf pour les statistiques
        if(!isnan(err_rsqrt) && !isinf(err_rsqrt) &&
           !isnan(err_div) && !isinf(err_div) &&
           !isnan(err_inv) && !isinf(err_inv)) {
            
            valid_count++;
            sum_err_rsqrt += err_rsqrt;
            sum_err_div += err_div;
            sum_err_inv += err_inv;
            
            if(err_rsqrt > max_err_rsqrt) max_err_rsqrt = err_rsqrt;
            if(err_div > max_err_div) max_err_div = err_div;
            if(err_inv > max_err_inv) max_err_inv = err_inv;
            
            //Déterminer la meilleure méthode (avec tolérance)
            if(fabsf(err_rsqrt - err_div) < 1e-7f && 
               fabsf(err_rsqrt - err_inv) < 1e-7f) {
                count_equal++;
            } else if(err_rsqrt <= err_div && err_rsqrt <= err_inv) {
                count_rsqrt_best++;
            } else if(err_div <= err_rsqrt && err_div <= err_inv) {
                count_div_best++;
            } else {
                count_inv_best++;
            }
        }
    }
    
    printf("Nombre de tests valides : %d/%d\n\n", valid_count, num_tests);
    
    printf("Erreurs maximales :\n");
    printf("  hf_rsqrt()       : %.9f\n", max_err_rsqrt);
    printf("  1/sqrt (div)     : %.9f\n", max_err_div);
    printf("  inv(sqrt)        : %.9f\n\n", max_err_inv);
    
    if(valid_count > 0) {
        printf("Erreurs moyennes :\n");
        printf("  hf_rsqrt()       : %.9f\n", sum_err_rsqrt / valid_count);
        printf("  1/sqrt (div)     : %.9f\n", sum_err_div / valid_count);
        printf("  inv(sqrt)        : %.9f\n\n", sum_err_inv / valid_count);
    }
    
    printf("Meilleure méthode par cas :\n");
    printf("  hf_rsqrt()       : %d cas\n", count_rsqrt_best);
    printf("  1/sqrt (div)     : %d cas\n", count_div_best);
    printf("  inv(sqrt)        : %d cas\n", count_inv_best);
    printf("  Egalité          : %d cas\n\n", count_equal);
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
static void print_formatted_table(const char *title, const char **headers, int num_cols, float data[][8], int num_rows) {
    int col_widths[8] = {0}; /* Maximum 8 colonnes */
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
        for(j = 0; j < num_cols && j < 8; j++) {
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