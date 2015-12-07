#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FALSE (0)
#define TRUE (1)

#define cPANELS_MAX (256)

#define cPANELTYPE_COMMON (0)
#define cPANELTYPE_TARGET (1)

enum {
  eDIR_UP = 0,
  eDIR_DOWN,
  eDIR_LEFT,
  eDIR_RIGHT,
  eDIR_MAX
};

typedef struct {
  int width;
  int height;

  int x;
  int y;

  int type;
}PANEL;

typedef struct {
  int width;
  int height;

  int end_x;
  int end_y;

  int panel_count;
  PANEL panels[cPANELS_MAX];
}FIELD;

void set_field_data(FIELD* f, int w, int h, int end_x, int end_y) {
  f->width = w;
  f->height = h;

  f->end_x = end_x;
  f->end_y = end_y;
}

void add_panel_to_field(FIELD* field, int x, int y, int w, int h, int type) {
  PANEL* p = &field->panels[field->panel_count];
  p->width = w;
  p->height = h;
  p->x = x;
  p->y = y;
  p->type = type;

  field->panel_count++;
}

int panel_collision_x(PANEL* p0, PANEL* p1) {
  if ( (p0->x >= p1->x && p0->x <= p1->x+p1->width) ) {
    return TRUE;
  }
  if ( (p0->x+p0->width >= p1->x && p0->x+p0->width <= p1->x+p1->width) ) {
    return TRUE;
  }

  return FALSE;
}

int panel_collision_y(PANEL* p0, PANEL* p1) {
  if ( (p0->y >= p1->y && p0->y <= p1->y+p1->height) ) {
    return TRUE;
  }
  if ( (p0->y+p0->height >= p1->y && p0->y+p0->height <= p1->y+p1->height) ) {
    return TRUE;
  }

  return FALSE;
}

int panel_collision_to_panel(PANEL* p0, PANEL* p1) {
  if (panel_collision_x(p0, p1) && panel_collision_y(p0, p1)) {
    return TRUE;
  }
  return TRUE;
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
      if (tmp_panel.y + 1 >= field->height) {
        return FALSE;
      }
      tmp_panel.y += 1;
    break;

    case eDIR_LEFT:
      if (tmp_panel.x <= 0) {
        return FALSE;
      }
      tmp_panel.y -= 1;
    break;

    case eDIR_RIGHT:
      if (tmp_panel.x + 1 >= field->width) {
        return FALSE;
      }
      tmp_panel.y += 1;
    break;

    default:
      return FALSE;
  }

  for (int i = 0; i < field->panel_count; i++) {
      if (i == panel_idx) {
        continue;
      }
      PANEL *target = &field->panels[i];
      if (panel_collision_to_panel(&tmp_panel, target)) {
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
    return -1;
  }
  fgets(str, sizeof(str), fp);

  char* str_work = str;
  int w = atoi(str_work);
  str_work = strchr(str_work, ',');
  if (str_work == NULL) {
    return -1;
  }
  str_work++;
  int h = atoi(str_work);
  //printf("str:%s\n", str);
  //printf("w:%d,h:%d\n", w,h);

  fgets(str, sizeof(str), fp);
  str_work = str;
  int end_x = atoi(str_work);
  str_work = strchr(str_work, ',');
  if (str_work == NULL) {
    return -1;
  }
  str_work++;
  int end_y = atoi(str_work);
  printf("end_x:%d,end_y:%d\n", end_x,end_y);

  set_field_data(field, w, h, end_x, end_y);
  field->panel_count = 0;

  while (fgets(str, sizeof(str), fp) != NULL) {
    str_work = str;
    int panel_x = atoi(str_work);
    str_work = strchr(str_work, ',');
    if (str_work == NULL) {
      return -1;
    }
    str_work++;

    int panel_y = atoi(str_work);
    str_work = strchr(str_work, ',');
    if (str_work == NULL) {
      return -1;
    }
    str_work++;

    int panel_w = atoi(str_work);
    str_work = strchr(str_work, ',');
    if (str_work == NULL) {
      return -1;
    }
    str_work++;

    int panel_h = atoi(str_work);
    str_work = strchr(str_work, ',');
    if (str_work == NULL) {
      return -1;
    }
    str_work++;

    int type = atoi(str_work);

    //printf("x:%d,y:%d,w:%d,h:%d,type:%d\n", panel_x, panel_y, panel_w, panel_h, type);
    add_panel_to_field(field, panel_x, panel_y, panel_w, panel_h, type);
  }
  fclose(fp);

  return 0;
}

void chk_panel_move_test(FIELD* field, int panel_idx) {
  int i = 0;

  char dir_comments[eDIR_MAX][] = {
    "UP", "DOWN", "LEFT", "RIGHT"
  };

  for (i = 0; i < eDIR_MAX) {
    ;
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }

  FIELD field;
  dataread_from_file(argv[1], &field);

  // test "chk_panel_move"

  return 0;
}
