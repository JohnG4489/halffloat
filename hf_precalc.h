#ifndef HF_PRECALC_H
#define HF_PRECALC_H


#define SIN_TABLE_SIZE 512
#define LN_TABLE_SIZE 1024
#define EXP_TABLE_SIZE_SHIFT 7
#define EXP_TABLE_SIZE (1<<EXP_TABLE_SIZE_SHIFT)
#define EXP_TABLE_PRECISION 15
#define EXP_PRECISION_SHIFT 8
#define TAN_TABLE_SIZE 512

#define LNI_2 45426 // ln(2) * 65536
#define LIMIT_TAN_AFFINE 10

void fill_sin_table();
void fill_ln_table();
void fill_exp_table();
void fill_tan_table();

extern uint16_t sin_table[SIN_TABLE_SIZE+1];
extern uint16_t ln_table[LN_TABLE_SIZE];
extern uint16_t exp_table[EXP_TABLE_SIZE+1];
extern uint32_t tan_table[TAN_TABLE_SIZE+1];

#endif // HF_PRECALC_H
// Toutes les fonctions et commentaires sont maintenant en UTF-8, corrigeant les caractères corrompus.
