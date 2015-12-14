#include "main.h"

static char panel_limit_dp[cPANEL_SIZE_PATTERNS][cPANELS_MAX][eDIR_MAX];

void set_field_data(FIELD* f, char w, char h, char end_x, char end_y) {
  f->width = w;
  f->height = h;

  f->end_x = end_x;
  f->end_y = end_y;

  f->target_idx = -1;
  f->field_hash = NULL;
}

void copy_field(FIELD* source, FIELD* dest) {
  dest->width = source->width;
  dest->height = source->height;
  dest->end_x = source->end_x;
  dest->end_y = source->end_y;

  dest->panel_count = source->panel_count;
  dest->target_idx = source->target_idx;

  memcpy((void*)&dest->panels[0], &source->panels[0], sizeof(dest->panels));

  dest->field_hash = NULL;
}

void create_panel_hash(PANEL* p) {
  p->hash[0] = (p->width << 4) | (p->height);
  p->hash[1] = (p->x << 4) | (p->y);

  //printf("%02x:%02x:%02x:%02x - %02x:%02x\n", p->width, p->height, p->x, p->y, p->hash[0], p->hash[1]);
  // 必ずtarget(娘)はサイズがユニークであること！
  // https://ja.wikipedia.org/wiki/%E7%AE%B1%E5%85%A5%E3%82%8A%E5%A8%98_(%E3%83%91%E3%82%BA%E3%83%AB)

//  p->hash[2] = p->type;
/*
  p->hash[0] = p->width;
  p->hash[1] = p->height;
  p->hash[2] = p->x;
  p->hash[3] = p->y;

  p->hash[4] = p->type;
*/
}

void delete_field_hash(FIELD* f) {
  if (f->field_hash != NULL) {
    free(f->field_hash);
  }
  f->field_hash = NULL;
}

void create_field_hash(FIELD* f) {
  int i = 0;
  int j = 0;
  int hash = 0;
  int tmp_hash = 0;

  int hash_length = f->panel_count*cPANEL_HASH_LENGTH;
  if (f->field_hash != NULL) {
    free(f->field_hash);
    f->field_hash = NULL;
  }

  int hash_ary[cPANELS_MAX];
  memset(hash_ary, 0, sizeof(hash_ary));
  for (i = 0; i < f->panel_count; i++) {
    hash = (f->panels[i].hash[0] << 8) | (f->panels[i].hash[1]);
    //printf("%02x-%02x : %04x\n", f->panels[i].hash[0], f->panels[i].hash[1], hash);
    for (j = 0; j < i; j++) {
      if (hash < hash_ary[j]) {
        break;
      }
    }
    memcpy(&hash_ary[j + 1], &hash_ary[j], sizeof(int)*(cPANELS_MAX - j - 1));
    hash_ary[j] = hash;
  }
  //printf("mokyun\n");

  f->field_hash = (char*)malloc(hash_length);
  char* hp = f->field_hash;
  for (i = 0; i < f->panel_count; i++) {
    memcpy((void*)hp, (void*)&hash_ary[i], cPANEL_HASH_LENGTH);
    hp += cPANEL_HASH_LENGTH;
//    printf("%04x\n", hash_ary[i]);
  }

/*
  for (i = 0; i < hash_length; i++) {
    printf("%02x:", f->field_hash[i]);
  }
  printf("\n");
*/

/*
  f->field_hash = (char*)malloc(hash_length);
  for (i = 0; i < f->panel_count; i++) {
    PANEL* p = &f->panels[i];
    char* hp = f->field_hash + i*cPANEL_HASH_LENGTH;
    memcpy((void*)hp, (void*)p->hash, cPANEL_HASH_LENGTH);
  }
*/
}

// 前回のハッシュを参考にするパターン
// 1パネルしか変更がない場合に有効
/*
void create_field_hash_from_before(FIELD* f, char* before, int idx) {
  int hash_length = f->panel_count*cPANEL_HASH_LENGTH;
  if (f->field_hash != NULL) {
    free(f->field_hash);
    f->field_hash = NULL;
  }
  f->field_hash = (char*)malloc(hash_length);

  memcpy((void*)f->field_hash, (void*)before, hash_length);
  char* hp = f->field_hash + idx*cPANEL_HASH_LENGTH;
  PANEL* p = &f->panels[idx];
  memcpy((void*)hp, (void*)p->hash, cPANEL_HASH_LENGTH);
}
*/

void add_panel_to_field(FIELD* field, char x, char y, char w, char h, char type) {
  PANEL* p = &field->panels[field->panel_count];
  memset(p, 0, sizeof(PANEL));

  p->width = w;
  p->height = h;
  p->x = x;
  p->y = y;
  p->type = type;

  if (p->type == cPANELTYPE_TARGET) {
    field->target_idx = field->panel_count;
    //printf("%d\n", field->target_idx);
  }
  field->panel_count++;

  create_panel_hash(p);
}

int panel_collision(PANEL* p0, PANEL* p1) {
  int x0 = p0->x;
  int x1 = p0->x + p0->width;
  int x2 = p1->x;
  int x3 = p1->x + p1->width;

  int y0 = p0->y;
  int y1 = p0->y + p0->height;
  int y2 = p1->y;
  int y3 = p1->y + p1->height;

  if (x0 >= x2 && x0 < x3) {
    if (y0 >= y2 && y0 < y3) {
      return TRUE;
    }
    if (y1 > y2 && y1 <= y3) {
      return TRUE;
    }
  }
  if (x1 > x2 && x1 <= x3) {
    if (y1 > y2 && y1 <= y3) {
      return TRUE;
    }
    if (y0 >= y2 && y0 < y3) {
      return TRUE;
    }
  }
  return FALSE;
}

// 移動判定のメモ化配列の初期化
void init_panel_limit_dp() {
  memset(panel_limit_dp, -1, sizeof(panel_limit_dp));
}

int chk_panel_limit(FIELD* f, PANEL* p) {
  if (p->y < 0) {
    return FALSE;
  }
  if (p->y + p->height > f->height) {
    return FALSE;
  }
  if (p->x < 0) {
    return FALSE;
  }
  if (p->x + p->width > f->width) {
    return FALSE;
  }
  return TRUE;
}

int chk_panel_move(FIELD* field, int panel_idx, int dir) {
  if (panel_idx >= field->panel_count) {
    return FALSE;
  }

  PANEL* p = &field->panels[panel_idx];
  PANEL tmp_panel = *p;

  int dir_move_arr[eDIR_MAX][2] = {
    {0, -1},
    {0, +1},
    {-1, 0},
    {+1, 0}
  };
  tmp_panel.x += dir_move_arr[dir][0];
  tmp_panel.y += dir_move_arr[dir][1];

  int limit_flg = FALSE;
  int size_idx = (tmp_panel.width << 2) | (tmp_panel.height); // 0~15
  int p_idx = (tmp_panel.x << 3) | (tmp_panel.y); // 0~63
  int dir_idx = dir; // 0~3

  if (panel_limit_dp[size_idx][p_idx][dir_idx] != -1) {
    if (panel_limit_dp[size_idx][p_idx][dir_idx] == 1) {
      return FALSE;
    }
  }
  else {
    if (chk_panel_limit(field, &tmp_panel) == FALSE) {
      panel_limit_dp[size_idx][p_idx][dir_idx] = 1;
      return FALSE;
    }
    panel_limit_dp[size_idx][p_idx][dir_idx] = 0;
  }

  int i = 0;
  for (i = 0; i < field->panel_count; i++) {
      if (i == panel_idx) {
        continue;
      }
      PANEL *target = &field->panels[i];
      if (panel_collision(&tmp_panel, target)) {
        //printf("collision:%d:%d\n", panel_idx, i);
        return FALSE;
      }
  }
  return TRUE;
}

int move_panel(FIELD* field, int panel_idx, int dir) {
  if (panel_idx >= field->panel_count) {
    return FALSE;
  }

  PANEL* p = &field->panels[panel_idx];

  int dir_move_arr[eDIR_MAX][2] = {
    {0, -1},
    {0, +1},
    {-1, 0},
    {+1, 0}
  };
  p->x += dir_move_arr[dir][0];
  p->y += dir_move_arr[dir][1];

  create_panel_hash(p);

  return TRUE;
}

int data_validate(FIELD* field) {
  if (field->target_idx < 0) {
    return FALSE;
  }

  if (field->width <= 0 || field->width >= cFIELD_SIZE_LIMIT) {
    return FALSE;
  }
  if (field->height <= 0 || field->height >= cFIELD_SIZE_LIMIT) {
    return FALSE;
  }

  int i = 0;
  for (i = 0; i < field->panel_count; i++) {
    PANEL* p = &field->panels[i];
    if (p->width <= 0 || p->width > cPANEL_SIZE_LIMIT) {
      return FALSE;
    }
    if (p->height <= 0 || p->height > cPANEL_SIZE_LIMIT) {
      return FALSE;
    }

    if (p->x < 0 || p->x > field->width) {
      return FALSE;
    }
    if (p->y < 0 || p->y > field->height) {
      return FALSE;
    }
  }

  return TRUE;
}

int dataread_from_file(char* fname, FIELD* field) {
  char str[256];
  FILE *fp = fopen(fname, "r");
  if (fp == NULL) {
    printf("%s\n", "file not found");
    return FALSE;
  }
  fgets(str, sizeof(str), fp);

  char* str_work = str;
  int w = atoi(str_work);
  str_work = strchr(str_work, ',');
  if (str_work == NULL) {
    return FALSE;
  }
  str_work++;
  int h = atoi(str_work);

  fgets(str, sizeof(str), fp);
  str_work = str;
  int end_x = atoi(str_work);
  str_work = strchr(str_work, ',');
  if (str_work == NULL) {
    return FALSE;
  }
  str_work++;
  int end_y = atoi(str_work);

  set_field_data(field, w, h, end_x, end_y);
  field->panel_count = 0;

  while (fgets(str, sizeof(str), fp) != NULL) {
    str_work = str;
    int panel_x = atoi(str_work);
    str_work = strchr(str_work, ',');
    if (str_work == NULL) {
      return FALSE;
    }
    str_work++;

    int panel_y = atoi(str_work);
    str_work = strchr(str_work, ',');
    if (str_work == NULL) {
      return FALSE;
    }
    str_work++;

    int panel_w = atoi(str_work);
    str_work = strchr(str_work, ',');
    if (str_work == NULL) {
      return FALSE;
    }
    str_work++;

    int panel_h = atoi(str_work);
    str_work = strchr(str_work, ',');
    if (str_work == NULL) {
      return FALSE;
    }
    str_work++;

    int type = atoi(str_work);

    //printf("%d,%d,%d,%d,%d\n", panel_x, panel_y, panel_w, panel_h, type);
    add_panel_to_field(field, panel_x, panel_y, panel_w, panel_h, type);
  }
  fclose(fp);

  create_field_hash(field);

  return TRUE;
}

/*
void chk_panel_move_test(FIELD* field, int panel_idx) {
  int i = 0;

  char dir_comments[eDIR_MAX][8] = {
    "UP", "DOWN", "LEFT", "RIGHT"
  };

  for (i = 0; i < eDIR_MAX; i++) {
    if (chk_panel_move(field, panel_idx, i)) {
      printf("chk_panel_move:%d:%s - TRUE\n", panel_idx, dir_comments[i]);
    }
    else {
      printf("chk_panel_move:%d:%s - FALSE\n", panel_idx, dir_comments[i]);
    }
  }
}
*/

int chk_clear_field(FIELD* f) {
  int idx = f->target_idx;

  PANEL* p = &f->panels[idx];
  if ((p->x+p->width == f->end_x) && (p->y+p->height == f->end_y)) {
    return TRUE;
  }

  //printf("%d - %d==%d && %d==%d\n", idx, p->x+p->width, f->end_x, p->y+p->height, f->end_y);
  return FALSE;
}

void init_solve_leaf(SOLVE_TREE* leaf, int depth) {
  leaf->depth = depth;
  leaf->leaves_count = 0;
  int i = 0;
  for (i = 0; i < cSOLVE_LEAVES_MAX; i++) {
    leaf->leaves[i] = NULL;
  }
}

void destroy_solve_tree(SOLVE_TREE* leaf) {
  int i = 0;
  if (leaf == NULL) {
    return;
  }

  for (i = 0; i < leaf->leaves_count; i++) {
    destroy_solve_tree(leaf->leaves[i]);
  }

  delete_field_hash(&leaf->field);
  free(leaf);
}

int chk_hash_from_root(SOLVE_TREE* leaf, FIELD* f) {
  if (leaf->field.field_hash == NULL || f->field_hash == NULL) {
    printf("chk_hash_from_root:error!\n");
    return TRUE;
  }

  int hash_length = f->panel_count*cPANEL_HASH_LENGTH;

  if (memcmp(leaf->field.field_hash, f->field_hash, hash_length) == 0) {
    return TRUE;
  }

  int i = 0;
  for (i = 0; i < leaf->leaves_count; i++) {
    int ret = 0;
    SOLVE_TREE* p = leaf->leaves[i];
    ret = chk_hash_from_root(p, f);
    if (ret == TRUE) {
      return TRUE;
    }
  }

  return FALSE;
}

int count_leaves_from_depth(SOLVE_TREE* leaf, int depth) {
  int ret = leaf->leaves_count;
  int i = 0;

  if (leaf->depth >= depth) {
    return ret;
  }

  for (i = 0; i < leaf->leaves_count; i++) {
    ret += count_leaves_from_depth(leaf->leaves[i], depth);
  }
  return ret;
}

int grow_solve_tree(SOLVE_TREE* root, SOLVE_TREE* leaf, int depth) {
  int i = 0;
  int j = 0;

  if (leaf->depth < depth) {
    int ret = eSOLVESTATE_FAILED;
    for (i = 0; i < leaf->leaves_count; i++) {
      int tmp = grow_solve_tree(root, leaf->leaves[i], depth);
      switch (tmp) {
        case eSOLVESTATE_CONTINUE:
          ret = eSOLVESTATE_CONTINUE;
        break;

        case eSOLVESTATE_FAILED:
        break;

        case eSOLVESTATE_SUCCEED:
          return eSOLVESTATE_SUCCEED;
      }
    }
    return ret;
  }

  if (chk_clear_field(&leaf->field) == TRUE) {
    //printf("ok!!!!!!!!!\n");
    return eSOLVESTATE_SUCCEED;
  }

  for (i = 0; i < leaf->field.panel_count; i++) {
    PANEL* p = &leaf->field.panels[i];
    for (j = 0; j < eDIR_MAX; j++) {
      if (!chk_panel_move(&leaf->field, i, j)) {
        continue;
      }
      //printf("%d:%d\n", i, j);

      if ( (leaf->leaves_count+1) >= cSOLVE_LEAVES_MAX) {
        printf("error over leaves\n");
        return eSOLVESTATE_FAILED;
      }

      leaf->leaves[leaf->leaves_count] = (SOLVE_TREE*)malloc(sizeof(SOLVE_TREE));
      SOLVE_TREE* new_leaf = leaf->leaves[leaf->leaves_count];
      copy_field(&leaf->field, &new_leaf->field);
      move_panel(&new_leaf->field, i, j);
      //create_field_hash_from_before(&new_leaf->field, leaf->field.field_hash, i);
      create_field_hash(&new_leaf->field);

      new_leaf->depth = depth + 1;
      new_leaf->leaves_count = 0;

      // compare hash
      if (chk_hash_from_root(root, &new_leaf->field) == TRUE) {
        destroy_solve_tree(new_leaf);
        leaf->leaves[leaf->leaves_count] = NULL;
        continue;
      }

      leaf->leaves_count++;
    }
  }

  if (leaf->leaves_count == 0) {
    return eSOLVESTATE_FAILED;
  }

  return eSOLVESTATE_CONTINUE;
}

int solve_field(FIELD* f) {
  int i = 0;
  int depth = 0;
  int max_depth = 82;
  SOLVE_TREE* root = (SOLVE_TREE*)malloc(sizeof(SOLVE_TREE));

  root->depth = 0;
  root->leaves_count = 0;
  copy_field(f, &root->field);
  create_field_hash(&root->field);

  int ret = 0;
  int leaves_count = 0;
  for (i = 0; i < max_depth; i++) {
    ret = grow_solve_tree(root, root, i);
    switch (ret) {
      case eSOLVESTATE_CONTINUE:
        printf("solve_field:CONTINUE - depth:%d\n", i);
      break;

      case eSOLVESTATE_FAILED:
        printf("solve_field:FAILED - depth:%d\n", i);
      break;

      case eSOLVESTATE_SUCCEED:
        printf("solve_field:SUCCEED - depth:%d\n", i);
      break;
    }

    leaves_count = count_leaves_from_depth(root, i);
    printf("depth:%d - leaves:%d\n", i, leaves_count);

    if (ret == eSOLVESTATE_FAILED || ret == eSOLVESTATE_SUCCEED) {
      break;
    }
    //printf("depth:%d\n", i);
  }

  destroy_solve_tree(root);

  return FALSE;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("file not found\n");
    return 1;
  }

  FIELD field;
  dataread_from_file(argv[1], &field);

  if (data_validate(&field) == FALSE) {
    printf("data error\n");
    return 1;
  }

/*
  char test = 0;
  test = (3 << 4) | (0);
  printf("%02x\n", test);
*/
/*
  int i = 0;
  for (i = 0; i < 11; i++) {
    chk_panel_move_test(&field, i);
  }
*/
  init_panel_limit_dp();
  solve_field(&field);

  return 0;
}
