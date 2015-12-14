#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FALSE (0)
#define TRUE (1)

#define cPANELTYPE_COMMON (0)
#define cPANELTYPE_TARGET (1)

#define cFIELD_SIZE_MAX (16)  // ハッシュ関数の都合で16以上にできない
#define cPANEL_SIZE_MAX (16) // ハッシュ関数の都合で16以上にできない

#define cPANELS_MAX (cFIELD_SIZE_MAX*cFIELD_SIZE_MAX)

#define cPANEL_HASH_LENGTH (2)

#define cSOLVE_LEAVES_MAX (16)

enum {
  eDIR_UP = 0,
  eDIR_DOWN,
  eDIR_LEFT,
  eDIR_RIGHT,
  eDIR_MAX
};

enum {
  eSOLVESTATE_CONTINUE = 0,
  eSOLVESTATE_FAILED,
  eSOLVESTATE_SUCCEED,
  eSOLVESTATE_MAX
};

typedef struct PANEL_t {
  char width;
  char height;
  char x;
  char y;

  char type;

  char hash[cPANEL_HASH_LENGTH];
}PANEL;

typedef struct FIELD_t {
  char width;
  char height;

  char end_x;
  char end_y;

  int panel_count;
  int target_idx;

  PANEL panels[cPANELS_MAX];

  char* field_hash;
}FIELD;

typedef struct SOLVE_TREE_t {
  int depth;
  int leaves_count;

  FIELD field;
  struct SOLVE_TREE_t* leaves[cSOLVE_LEAVES_MAX];
}SOLVE_TREE;
