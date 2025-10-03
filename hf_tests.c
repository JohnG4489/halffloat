#include <stdio.h>
#include <string.h>
#include "hf_tests.h"
#include "hf_lib.h"
#include <stdint.h>
#include <math.h>

// Prototypes des fonctions
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

void print_line(int len);
void print_table(const char *header[], float data[][2], int num_columns, int num_rows);


/**
 * @brief Fonction de débogage pour tester la fonction hf_add avec divers cas de test
 */
void debug_add() {
    float test_cases[][2] = {
        {60000.f, -80000.f}, {-80000.f, -80000.f}, {70000.f, 70000.f}, {-70000.f, 70000.f},
        {-50000.f, -50000.f}, {1.0f, 2.0f}, {-1.0f, 1.0f}, {1.0f, -1.0f},
        {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, -1.0f}, {1.0f, 1.0f},
        {1.0f, 65504.0f}, {-1.0f, -65504.0f}, {20000.f, -30000.f}, {20000.f, 40000.f},
        {20000.f, 50000.f}, {1.f, -65000.f}, {0.5f, 0.25f}
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    printf("### HF_ADD\n");
    printf("Value1     Value2     Result (my_add)     Result (std::add)    Difference\n");
    print_line(80);

    for (i = 0; i < num_tests; i++) {
        float value1 = test_cases[i][0];
        float value2 = test_cases[i][1];
        uint16_t value1_half = float_to_half(value1);
        uint16_t value2_half = float_to_half(value2);

        uint16_t result_half = hf_add(value1_half, value2_half);
        float result_float = half_to_float(result_half);
        float std_result = half_to_float(value1_half) + half_to_float(value2_half);
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-10.2f %-10.2f %-19.9f %-20.9f %-20.9f\n", value1, value2, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_mul avec divers cas de test
 */
void debug_mul() {
    float test_cases[][2] = {
        {half_to_float(HF_NAN), -8.3f}, {5.25f, -8.3f}, {1.0f, 2.0f}, {-1.0f, 1.0f},
        {1.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, -1.0f},
        {1.0f, 1.0f}, {1.0f, 65504.0f}, {-1.0f, -65504.0f}, {20000.f, -30000.f},
        {20000.f, 40000.f}, {20000.f, 50000.f}, {1.f, -65000.f}, {-70000.f, 70000.f},
        {-50000.f, -50000.f}, {0.5f, 0.25f}, {0.15f, 0.893f}
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    printf("### HF_MUL\n");
    printf("Value1          Value2          Result (my_mul)      Result (std::mul)    Difference\n");
    print_line(80);

    for (i = 0; i < num_tests; i++) {
        float value1 = test_cases[i][0];
        float value2 = test_cases[i][1];
        uint16_t value1_half = float_to_half(value1);
        uint16_t value2_half = float_to_half(value2);

        uint16_t result_half = hf_mul(value1_half, value2_half);
        float result_float = half_to_float(result_half);
        float std_result = half_to_float(value1_half) * half_to_float(value2_half);
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-15.8f %-15.8f %-20.9f %-20.9f %-20.9f\n", value1, value2, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_div avec divers cas de test
 */
void debug_div() {
    float test_cases[][2] = {
        {1.0f, 1.0f},     // Division par 1
        {-1.0f, 1.0f},    // Résultat négatif
        {1.0f, -1.0f},    // Diviseur négatif
        {0.0f, 1.0f},     // Zéro divisé par un nombre
        {1.0f, 0.0f},     // Division par zéro
        {0.0f, 0.0f},     // Zéro divisé par zéro
        {1.0f, 2.0f},     // Fraction simple
        {2.0f, 1.0f},     // Nombre entier
        {0.1f, 0.1f},     // Petits nombres
        {1000.0f, 1000.0f}, // Grands nombres
        {0.0001f, 1000.0f}, // Très petit / très grand
        {1000.0f, 0.0001f}, // Très grand / très petit
        {3.14159f, 1.0f}, // Pi
        {1.0f, 3.14159f}, // 1/Pi
        {65504.f, 2.0f},  // Près de la limite supérieure
        {0.0000123, 2.0f},  // Près de la limite inférieure
        {half_to_float(HF_INFINITY_POS), 2.0f},  // Infini positif
        {1.0f, half_to_float(HF_INFINITY_POS)},  // Division par l'infini
        {half_to_float(HF_NAN), 1.0f},  // NaN comme numérateur
        {1.0f, half_to_float(HF_NAN)},  // NaN comme dénominateur
        // Cas spécifiques pour tan(x) = sin(x) / cos(x)
        {0.8414709848f, 0.5403023059f},  // sin(1) / cos(1)
        {1.0f, 0.5403023059f},           // 1 / cos(1), proche de tan(1)
        {1.0f, 0.0000000874f},           // 1 / cos(?/2), devrait donner une grande valeur
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    printf("### HF_DIV\n");
    printf("Value1          Value2          Result (my_div)      Result (std::div)    Difference\n");
    print_line(80);

    for (i = 0; i < num_tests; i++) {
        float value1 = test_cases[i][0];
        float value2 = test_cases[i][1];
        uint16_t value1_half = float_to_half(value1);
        uint16_t value2_half = float_to_half(value2);

        uint16_t result_half = hf_div(value1_half, value2_half);
        float result_float = half_to_float(result_half);
        float std_result = value2 != 0.0f ? value1 / value2 : (value1 > 0 ? INFINITY : (value1 < 0 ? -INFINITY : NAN));
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-15.8f %-15.8f %-20.9f %-20.9f %-20.9f\n", value1, value2, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_sqrt avec divers cas de test
 */
void debug_sqrt() {
    float test_cases[] = {
        65504.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f,
        0.0f, -0.0f, 0.25f, -1.0f, half_to_float(HF_INFINITY_POS),
        half_to_float(HF_NAN), 0.000061035f, 5.96e-8f
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    printf("### HF_SQRT\n");
    printf("Value           Result (my_sqrt)     Result (std::sqrt)   Difference\n");
    print_line(70);

    for (i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);

        uint16_t result_half = hf_sqrt(value_half);
        float result_float = half_to_float(result_half);
        float std_result = sqrtf(value);
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-15.8f %-20.9f %-20.9f %-20.9f\n", value, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_pow avec divers cas de test
 */
void debug_pow() {
    float test_cases[][2] = {
        {2.0f, 3.0f}, {10.0f, 2.0f}, {3.0f, 4.0f}, {1.5f, 2.5f},
        {5.0f, -1.0f}, {0.5f, 3.0f}, {100.0f, 0.5f}, {2.0f, 10.0f},
        
        {1.0f, 5.0f}, {0.1f, 2.0f}, {2.0f, -3.0f}, {10.0f, -2.0f},
        {3.0f, -4.0f}, {1.5f, -2.5f}, {5.0f, -0.5f}, {0.5f, -3.0f},
        
        {100.0f, -0.5f}, {2.0f, -10.0f}, {1.0f, -5.0f}, {0.1f, -2.0f},
        {2.0f, 0.5f}, {2.0f, -0.5f}, {0.5f, 0.5f}, {0.5f, -0.5f}
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    printf("### HF_POW\n");
    printf("Base            Exp             Result (my_pow)   Result (std::pow) Difference\n");
    print_line(80);
    
    for (i = 0; i < num_tests; i++) {
        float base = test_cases[i][0];
        float exponent = test_cases[i][1];
        uint16_t base_half = float_to_half(base);
        uint16_t exponent_half = float_to_half(exponent);

        uint16_t result_half = hf_pow(base_half, exponent_half);
        float result_float = half_to_float(result_half);
        float std_result = powf(base, exponent);
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-15.8f %-15.8f %-17.9f %-17.9f %-17.9f\n", base, exponent, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_exp avec divers cas de test
 */
void debug_exp() {
    float test_cases[] = {
        0.035f, 0.1f, -0.012f, 12.0f, 11.0f, 0.0f, 1.0f, -1.0f,
        2.0f, -2.0f, half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG),
        half_to_float(HF_NAN), 10.0f, -10.0f
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    printf("### HF_EXP\n");
    printf("Value   Result (my_exp)     Result (std::exp)   Difference\n");
    print_line(59);

    for (i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);

        uint16_t result_half = hf_exp(value_half);
        float result_float = half_to_float(result_half);
        float std_result = expf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-7.2f %-19.9f %-19.9f %-19.9f\n", value, result_float, std_result, diff);
    }

    printf("\n");
}


/**
 * @brief Fonction de débogage pour tester la fonction hf_int avec divers cas de test
 */
void debug_int() {
    float test_cases[] = {
        65504.0f, 1.0f, 1.5f, 2.0f, 2.7f, 3.2f, -1.0f, -1.7f, -2.3f,
        0.0f, -0.0f, 0.7f, -0.7f, half_to_float(HF_INFINITY_POS),
        half_to_float(HF_INFINITY_NEG), half_to_float(HF_NAN),
        0.000061035f, -0.000061035f, 5.96e-8f, -5.96e-8f
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    printf("### HF_INT\n");
    printf("Value      Result (my_int)      Result (std::int)    Difference\n");
    print_line(70);

    for (i = 0; i < num_tests; i++) {
        float value = test_cases[i];
        uint16_t value_half = float_to_half(value);

        uint16_t result_half = hf_int(value_half);
        float result_float = half_to_float(result_half);
        float std_result = truncf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-10.2f %-20.9f %-20.9f %-20.9f\n", value, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_ln avec divers cas de test
 */
void debug_ln() {
    float test_cases[] = {
        0.000061035f, 5.96e-8f, 0.023f, 0.13f, 0.3f, 0.5f, 1.0f,
        2.0f, 0.5f, 3.14159f, 10.0f, 0.0f, -1.0f, 65504.0f,
        half_to_float(HF_INFINITY_POS), half_to_float(HF_INFINITY_NEG),
        half_to_float(HF_NAN)
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int i;

    printf("### HF_LN\n");
    printf("Value             Result (my_ln)     Result (std::ln)   Difference\n");
    print_line(65);

    for (i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);

        uint16_t result_half = hf_ln(value_half);
        float result_float = half_to_float(result_half);
        float std_result = logf(half_to_float(value_half));
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-16.9f %-18.9f %-18.9f %-18.9f\n", value, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_sin avec divers cas de test
 */
void debug_sin() {
    float test_cases[] = {
        0.0f, -0.0f,  // Zéros positif et négatif
        0.7853981633974483f, 1.5707963267948966f, 3.141592653589793f, 6.283185307179586f,  // ?/4, ?/2, ?, 2?
        -0.7853981633974483f, -1.5707963267948966f, -3.141592653589793f,  // -?/4, -?/2, -?
        1.0f, -1.0f,  // Valeurs unitaires
        0.5f, -0.5f,  // Valeurs fractionnaires
        3.0f, -3.0f,  // Valeurs hors de l'intervalle [-?, ?]
        65504.0f, -65504.0f,  // Valeurs maximales pour demi-flottant
        0.000061035f, -0.000061035f,  // Petites valeurs positives et négatives
        half_to_float(HF_INFINITY_POS), -half_to_float(HF_INFINITY_POS),  // Infinis
        half_to_float(HF_NAN)  // NaN
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("### HF_SIN\n");
    printf("Angle (rad)     Result (hf_sin)      Result (sinf)        Difference\n");
    print_line(70);

    for (int i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);

        uint16_t result_half = hf_sin(value_half);
        float result_float = half_to_float(result_half);
        float std_result = sinf(value);
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-15.8f %-20.9f %-20.9f %-20.9f\n", value, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_cos avec divers cas de test
 */
void debug_cos() {
    float test_cases[] = {
        0.0f, -0.0f,  // Zéros positif et négatif
        0.7853981633974483f, 1.5707963267948966f, 3.141592653589793f, 6.283185307179586f,  // ?/4, ?/2, ?, 2?
        -0.7853981633974483f, -1.5707963267948966f, -3.141592653589793f,  // -?/4, -?/2, -?
        1.0f, -1.0f,  // Valeurs unitaires
        0.5f, -0.5f,  // Valeurs fractionnaires
        3.0f, -3.0f,  // Valeurs hors de l'intervalle [-?, ?]
        65504.0f, -65504.0f,  // Valeurs maximales pour demi-flottant
        0.000061035f, -0.000061035f,  // Petites valeurs positives et négatives
        half_to_float(HF_INFINITY_POS), -half_to_float(HF_INFINITY_POS),  // Infinis
        half_to_float(HF_NAN)  // NaN
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("### HF_COS\n");
    printf("Angle (rad)     Result (hf_cos)      Result (cosf)        Difference\n");
    print_line(70);

    for (int i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);

        uint16_t result_half = hf_cos(value_half);
        float result_float = half_to_float(result_half);
        float std_result = cosf(value);
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-15.8f %-20.9f %-20.9f %-20.9f\n", value, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Fonction de débogage pour tester la fonction hf_tan avec divers cas de test
 */
void debug_tan() {
    float test_cases[] = {
        65504.0f, -65504.0f,  // Valeurs maximales pour demi-flottant
        0.f, 0.1f, 0.5f, 0.75f, 1.f, 1.5, 1.56, 1.57, 1.5701, 1.5702, 1.58, 2.f, 3.f, 3.14, 3.2, 3.5, 3.75, 3.95, 4.f, 5.f, 6.f, 6.1, 6.28, 6.3,
        1,2,3,4,5,6,7,8,9,10,11,12,
        0.0f, -0.0f,  // Zéros positif et négatif
        0.7853981633974483f, 1.5707963267948966f, 3.14159f, 6.283185307179586f,  // ?/4, ?/2, ?, 2?
        -0.7853981633974483f, -1.5707963267948966f, -3.141592653589793f,  // -?/4, -?/2, -?
        1.0f, -1.0f,  // Valeurs unitaires
        0.5f, -0.5f,  // Valeurs fractionnaires
        3.0f, -3.0f,  // Valeurs hors de l'intervalle [-?, ?]
        0.000061035f, -0.000061035f,  // Petites valeurs positives et négatives
        1.55f, -1.55f,  // Valeurs proches de ?/2
        4.71f, -4.71f,  // Valeurs proches de 3?/2
        half_to_float(HF_INFINITY_POS), -half_to_float(HF_INFINITY_POS),  // Infinis
        half_to_float(HF_NAN)  // NaN
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("### HF_TAN\n");
    printf("Angle (rad)     Result (hf_tan)      Result (tanf)        Difference\n");
    print_line(70);

    for (int i = 0; i < num_tests; i++) {
        uint16_t value_half = float_to_half(test_cases[i]);
        float value = half_to_float(value_half);

        uint16_t result_half = hf_tan(value_half);
        float result_float = half_to_float(result_half);
        float std_result = tanf(value);
        float diff = fabsf(result_float - std_result);

        // Affichage des résultats
        printf("%-15.8f %-20.9f %-20.9f %-20.9f\n", value, result_float, std_result, diff);
    }

    printf("\n");
}

/**
 * @brief Imprime une ligne de tirets de longueur spécifiée
 * @param len La longueur de la ligne de tirets
 */
void print_line(int len) {
    for (int i = 0; i < len; i++) {
        putchar('-');
    }
    putchar('\n');
}


/**
 * @brief Imprime un tableau de données sous forme de tableau ASCII
 * 
 * @param header Les en-têtes de colonnes
 * @param data Les données à imprimer
 * @param num_columns Le nombre de colonnes
 * @param num_rows Le nombre de lignes
 */
void print_table(const char *header[], float data[][2], int num_columns, int num_rows) {
    int column_widths[16];
    char tmp[32];
    int i, j, width;

    // Calculer la largeur maximale pour chaque colonne
    for (j = 0; j < num_columns; j++) {
        column_widths[j] = strlen(header[j]);
        for (i = 0; i < num_rows; i++) {
            sprintf(tmp, "%.9f", data[i][j]);
            width = strlen(tmp);
            if (width > column_widths[j]) column_widths[j] = width;
        }
    }

    // Imprimer l'en-tête
    for (j = 0; j < num_columns; j++) {
        printf("%-*s", column_widths[j] + 2, header[j]);
    }
    printf("\n");

    // Imprimer une ligne de séparation
    for (j = 0; j < num_columns; j++) {
        for (i = 0; i < column_widths[j] + 2; i++) printf("-");
    }
    printf("\n");

    // Imprimer les données
    for (i = 0; i < num_rows; i++) {
        for (j = 0; j < num_columns; j++) {
            printf("%-*.*f", column_widths[j] + 2, 9, data[i][j]);
        }
        printf("\n");
    }
}
