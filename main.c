#include "main.h"

void set_field_data(FIELD* f, char w, char h, char end_x, char end_y) {
  f->width = w;
  f->height = h;

  f->end_x = end_x;
  f->end_y = end_y;

  f->field_hash = NULL;
}

void copy_field(FIELD* source, FIELD* dest) {
  dest->width = source->width;
  dest->height = source->height;
  dest->end_x = source->end_x;
  dest->end_y = source->end_y;

  dest->panel_count = source->panel_count;

  memcpy((void*)&dest->panels[0], &source->panels[0], sizeof(dest->panels));

  dest->field_hash = NULL;
}

void create_panel_hash(PANEL* p) {
  p->hash[0] = p->width;
  p->hash[1] = p->height;
  p->hash[2] = p->x;
  p->hash[3] = p->y;

  p->hash[4] = p->type;
  p->hash[5] = p->padd[0];
  p->hash[6] = p->padd[1];
  p->hash[7] = p->padd[2];
}

void delete_field_hash(FIELD* f) {
  if (f->field_hash != NULL) {
    free(f->field_hash);
  }
  f->field_hash = NULL;
}

void create_field_hash(FIELD* f) {
  int i = 0;

  int hash_length = f->panel_count*cPANEL_HASH_LENGTH;
  if (f->field_hash != NULL) {
    free(f->field_hash);
    f->field_hash = NULL;
  }
  f->field_hash = (char*)malloc(hash_length);
  for (i = 0; i < f->panel_count; i++) {
    PANEL* p = &f->panels[i];
    char* hp = f->field_hash + i*cPANEL_HASH_LENGTH;
    memcpy((void*)hp, (void*)p->hash, cPANEL_HASH_LENGTH);
  }
/*
  for (i = 0; i < hash_length; i++) {
    printf("%d", f->field_hash[i]);
  }
  printf("\n");
*/
}

void add_panel_to_field(FIELD* field, char x, char y, char w, char h, char type) {
  PANEL* p = &field->panels[field->panel_count];
  memset(p, 0, sizeof(PANEL));

  p->width = w;
  p->height = h;
  p->x = x;
  p->y = y;
  p->type = type;

  field->panel_count++;

  create_panel_hash(p);
}

int panel_collision(PANEL* p0, PANEL* p1) {
  int x0 = p0->x;
  int x1 = p0->x + p0->x + p0->width;
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
  }
  if (x1 > x2 && x1 <= x3) {
    if (y1 > y2 && y1 <= y3) {
      return TRUE;
    }
  }
  if (x0 >= x2 && x0 < x3) {
    if (y1 > y2 && y1 <= y3) {
      return TRUE;
    }
  }
  if (x1 > x2 && x1 <= x3) {
    if (y0 >= y2 && y0 < y3) {
      return TRUE;
    }
  }
  return FALSE;
}

int chk_panel_move(FIELD* field, int panel_idx, int dir) {
  if (panel_idx >= field->panel_count) {
    return FALSE;
  }

  PANEL* p = &field->panels[panel_idx];
  PANEL tmp_panel = *p;

  switch (dir) {
    case eDIR_UP:
      if (tmp_panel.y <= 0) {
        return FALSE;
      }
      tmp_panel.y -= 1;
    break;

    case eDIR_DOWN:
      if (tmp_panel.y + tmp_panel.height + 1 > field->height) {
        return FALSE;
      }
      tmp_panel.y += 1;
    break;

    case eDIR_LEFT:
      if (tmp_panel.x <= 0) {
        return FALSE;
      }
      tmp_panel.x -= 1;
    break;

    case eDIR_RIGHT:
      if (tmp_panel.x + tmp_panel.width + 1 > field->width) {
        return FALSE;
      }
      tmp_panel.x += 1;
    break;

    default:
      return FALSE;
  }

  //printf("tmp_panel - w:%d,h:%d,x:%d,y:%d\n", tmp_panel.width, tmp_panel.height, tmp_panel.x, tmp_panel.y);
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
  switch (dir) {
    case eDIR_UP:
      p->y -= 1;
    break;
    case eDIR_DOWN:
      p->y += 1;
    break;
    case eDIR_LEFT:
      p->x -= 1;
    break;
    case eDIR_RIGHT:
      p->x += 1;
    break;

    default:
    break;
  }

  create_panel_hash(p);

  return TRUE;
}

int data_validate(FIELD* field) {
  if (field->width <= 0 || field->width > cFIELD_SIZE_MAX) {
    return FALSE;
  }
  if (field->height <= 0 || field->height > cFIELD_SIZE_MAX) {
    return FALSE;
  }

  int i = 0;
  for (i = 0; i < field->panel_count; i++) {
    PANEL* p = &field->panels[i];
    if (p->width <= 0 || p->width > cPANEL_SIZE_MAX) {
      return FALSE;
    }
    if (p->height <= 0 || p->height > cPANEL_SIZE_MAX) {
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
  int i = 0;
  for (i = 0; i < f->panel_count; i++) {
    PANEL* p = &f->panels[i];
    if (p->type == cPANELTYPE_TARGET) {
      if ((p->x+p->width == f->end_x) && (p->y+p->height == f->end_y)) {
        return TRUE;
      }
      else {
        return FALSE;
      }
    }
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

  delete_field_hash(&leaf->field);
  free(leaf);
}

int grow_solve_tree(SOLVE_TREE* leaf, int depth) {
  int i = 0;
  int j = 0;

  if (leaf->depth < depth) {
    int ret = eSOLVESTATE_FAILED;
    for (i = 0; i < leaf->leaves_count; i++) {
      int tmp = grow_solve_tree(leaf->leaves[i], depth);
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

  if (chk_clear_field(&leaf->field)) {
    return eSOLVESTATE_SUCCEED;
  }

  //printf("testmokyun:%d \n", leaf->field.panel_count);
  for (i = 0; i < leaf->field.panel_count; i++) {
    PANEL* p = &leaf->field.panels[i];
    for (j = 0; j < eDIR_MAX; j++) {
      if (chk_panel_move(&leaf->field, i, j)) {
        leaf->leaves[leaf->leaves_count] = (SOLVE_TREE*)malloc(sizeof(SOLVE_TREE));
        SOLVE_TREE* new_leaf = leaf->leaves[leaf->leaves_count];

        copy_field(&leaf->field, &new_leaf->field);

        new_leaf->depth = depth + 1;
        new_leaf->leaves_count = 0;

        move_panel(&new_leaf->field, i, j);
        create_field_hash(&new_leaf->field);

        leaf->leaves_count++;
      }
    }
    //printf("create leaves - %d\n", leaf->leaves_count);
  }

  if (leaf->leaves_count == 0) {
    return eSOLVESTATE_FAILED;
  }

  return eSOLVESTATE_CONTINUE;
}

int solve_field(FIELD* f) {
  int i = 0;
  int depth = 0;
  int max_depth = 5;
  SOLVE_TREE* root = (SOLVE_TREE*)malloc(sizeof(SOLVE_TREE));

  root->depth = 0;
  root->leaves_count = 0;
  copy_field(f, &root->field);

  int ret = 0;
  for (i = 0; i < max_depth; i++) {
    ret = grow_solve_tree(root, i);
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

      default:
      break;
    }
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

  solve_field(&field);

  return 0;
}
