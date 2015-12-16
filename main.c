#include "main.h"

static char         panel_limit_dp[cPANEL_SIZE_PATTERNS][cPANELS_MAX][eDIR_MAX];
static FIELD_INFO   g_field_info;
static char*        g_field_hashs[cFIELD_HASH_MAX];

// ---------------------------
void set_field_info(FIELD_INFO* info, char w, char h, char end_x, char end_y) {
  info->width = w;
  info->height = h;
  info->end_x = end_x;
  info->end_y = end_y;

  info->target_idx = -1;
  info->panel_count = 0;
  info->field_hash_count = 0;
}
void set_field_target(FIELD_INFO* info, int target_idx) {
  info->target_idx = target_idx;
}
void add_field_panel_count(FIELD_INFO* info) {
  info->panel_count++;
}

void init_field_hash(void) {
  memset(g_field_hashs, 0, sizeof(g_field_hashs));
}
char* create_field_hash(FIELD_INFO* info, FIELD* f) {
  int hash_length = info->panel_count*cPANEL_HASH_LENGTH;
  char* ret = (char*)malloc(hash_length);
  int i = 0;
  int j = 0;
  int hash = 0;
  int tmp_hash = 0;

  int hash_ary[cPANELS_MAX];
  memset(hash_ary, 0, sizeof(hash_ary));
  for (i = 0; i < info->panel_count; i++) {
    hash = (f->panels[i].hash[0] << 8) | (f->panels[i].hash[1]);
    for (j = 0; j < i; j++) {
      if (hash < hash_ary[j]) {
        break;
      }
    }
    memcpy(&hash_ary[j + 1], &hash_ary[j], sizeof(int)*(cPANELS_MAX - j - 1));
    hash_ary[j] = hash;
  }

  char* hp = ret;
  for (i = 0; i < info->panel_count; i++) {
    memcpy((void*)hp, (void*)&hash_ary[i], cPANEL_HASH_LENGTH);
    hp += cPANEL_HASH_LENGTH;
  }

  return ret;
}

void add_field_hash(FIELD_INFO* info, char* hash) {
  g_field_hashs[info->field_hash_count] = hash;
  info->field_hash_count++;
}
void delete_field_hashs(FIELD_INFO* info) {
  int i = 0;
  for (i = 0; i < info->field_hash_count; i++) {
    free(g_field_hashs[i]);
    g_field_hashs[i] = NULL;
  }
}

// ---------------------------
void copy_field(FIELD* source, FIELD* dest) {
  memcpy((void*)&dest->panels[0], &source->panels[0], sizeof(dest->panels));
}

void create_panel_hash(PANEL* p) {
  p->hash[0] = (p->width << 4) | (p->height);
  p->hash[1] = (p->x << 4) | (p->y);

  //printf("%02x:%02x:%02x:%02x - %02x:%02x\n", p->width, p->height, p->x, p->y, p->hash[0], p->hash[1]);
  // 必ずtarget(娘)はサイズがユニークであること！
  // https://ja.wikipedia.org/wiki/%E7%AE%B1%E5%85%A5%E3%82%8A%E5%A8%98_(%E3%83%91%E3%82%BA%E3%83%AB)
}

void add_panel_to_field(FIELD* field, char x, char y, char w, char h, char type) {
  FIELD_INFO* info = &g_field_info;
  PANEL* p = &field->panels[info->panel_count];
  memset(p, 0, sizeof(PANEL));

  p->width = w;
  p->height = h;
  p->x = x;
  p->y = y;
  p->type = type;

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
  FIELD_INFO* info = &g_field_info;

  if (p->y < 0) {
    return FALSE;
  }
  if (p->y + p->height > info->height) {
    return FALSE;
  }
  if (p->x < 0) {
    return FALSE;
  }
  if (p->x + p->width > info->width) {
    return FALSE;
  }
  return TRUE;
}

int chk_panel_move(FIELD* field, int panel_idx, int dir) {
  FIELD_INFO* info = &g_field_info;

  if (panel_idx >= info->panel_count) {
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
  for (i = 0; i < info->panel_count; i++) {
      if (i == panel_idx) {
        continue;
      }
      PANEL *target = &field->panels[i];
      if (panel_collision(&tmp_panel, target)) {
        return FALSE;
      }
  }
  return TRUE;
}

int move_panel(FIELD* field, int panel_idx, int dir) {
  FIELD_INFO* info = &g_field_info;

  if (panel_idx >= info->panel_count) {
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

int data_validate(FIELD* field, FIELD_INFO* info) {
  if (info->target_idx < 0) {
    return FALSE;
  }

  if (info->width <= 0 || info->width >= cFIELD_SIZE_LIMIT) {
    return FALSE;
  }
  if (info->height <= 0 || info->height >= cFIELD_SIZE_LIMIT) {
    return FALSE;
  }

  int i = 0;
  for (i = 0; i < info->panel_count; i++) {
    PANEL* p = &field->panels[i];
    if (p->width <= 0 || p->width > cPANEL_SIZE_LIMIT) {
      return FALSE;
    }
    if (p->height <= 0 || p->height > cPANEL_SIZE_LIMIT) {
      return FALSE;
    }

    if (p->x < 0 || p->x > info->width) {
      return FALSE;
    }
    if (p->y < 0 || p->y > info->height) {
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

  set_field_info(&g_field_info, w, h, end_x, end_y);

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
    if (type == cPANELTYPE_TARGET) {
      set_field_target(&g_field_info, g_field_info.panel_count);
    }
    add_field_panel_count(&g_field_info);
  }
  fclose(fp);

  return TRUE;
}

int chk_clear_field(FIELD* f) {
  FIELD_INFO* info = &g_field_info;
  int idx = info->target_idx;

  PANEL* p = &f->panels[idx];
  if ((p->x+p->width == info->end_x) && (p->y+p->height == info->end_y)) {
    return TRUE;
  }
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
  free(leaf);
}

int chk_hash_and_append(FIELD_INFO* info, FIELD* f) {
  char* hash = create_field_hash(info, f);
  char* diff_hash = NULL;
  int i = 0;
  int hash_length = info->panel_count*cPANEL_HASH_LENGTH;

  for (i = 0; i < info->field_hash_count; i++) {
    diff_hash = g_field_hashs[i];
    if (memcmp(diff_hash, hash, hash_length) == 0) {
      free(hash);
      return TRUE;
    }
  }

  add_field_hash(info, hash);
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
  FIELD_INFO* info = &g_field_info;
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
          // 葉が広がらないなら削除
          destroy_solve_tree(leaf->leaves[i]);
          for (j = i; j < cSOLVE_LEAVES_MAX - 1; j++) {
            leaf->leaves[j] = leaf->leaves[j + 1];
          }
          leaf->leaves_count--;
          i--;
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

  for (i = 0; i < info->panel_count; i++) {
    PANEL* p = &leaf->field.panels[i];
    for (j = 0; j < eDIR_MAX; j++) {
      if (!chk_panel_move(&leaf->field, i, j)) {
        continue;
      }

      if ( (leaf->leaves_count+1) >= cSOLVE_LEAVES_MAX) {
        printf("error over leaves\n");
        return eSOLVESTATE_FAILED;
      }

      leaf->leaves[leaf->leaves_count] = (SOLVE_TREE*)malloc(sizeof(SOLVE_TREE));
      SOLVE_TREE* new_leaf = leaf->leaves[leaf->leaves_count];
      copy_field(&leaf->field, &new_leaf->field);
      move_panel(&new_leaf->field, i, j);

      new_leaf->depth = depth + 1;
      new_leaf->leaves_count = 0;

      // compare hash
      if (chk_hash_and_append(&g_field_info, &new_leaf->field) == TRUE) {
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
  int max_depth = 200;
  SOLVE_TREE* root = (SOLVE_TREE*)malloc(sizeof(SOLVE_TREE));

  root->depth = 0;
  root->leaves_count = 0;
  copy_field(f, &root->field);
  chk_hash_and_append(&g_field_info, &root->field);

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

//    leaves_count = count_leaves_from_depth(root, i);
//    printf("depth:%d - leaves:%d\n", i, leaves_count);

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

  if (data_validate(&field, &g_field_info) == FALSE) {
    printf("data error\n");
    return 1;
  }

  init_panel_limit_dp();
  init_field_hash();
  solve_field(&field);
  delete_field_hashs(&g_field_info);

  return 0;
}
