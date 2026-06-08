#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define ROWS 24
#define COLS 60

static char canvas[ROWS][COLS];

void canvas_clear(void)
{
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = '_';
}

void canvas_show(void)
{
    /* top border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        printf("|");
        for (int c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }

    /* bottom border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");
}

/* safe plot – ignores out-of-range coordinates */
static void plot(int r, int c)
{
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = '*';
}

/* ------------------------------------------------------------------ */
/*  drawing primitives                                                  */
/* ------------------------------------------------------------------ */

/* Bresenham line between (r0,c0) and (r1,c1) */
void draw_line(int r0, int c0, int r1, int c1)
{
    int dr = abs(r1 - r0), dc = abs(c1 - c0);
    int sr = (r0 < r1) ? 1 : -1;
    int sc = (c0 < c1) ? 1 : -1;
    int err = dr - dc;

    for (;;) {
        plot(r0, c0);
        if (r0 == r1 && c0 == c1) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r0 += sr; }
        if (e2 <  dr) { err += dr; c0 += sc; }
    }
}

/* axis-aligned rectangle, top-left (tr,tc), width w, height h */
void draw_rect(int tr, int tc, int w, int h)
{
    /* four edges */
    draw_line(tr,       tc,       tr,       tc + w - 1);
    draw_line(tr + h-1, tc,       tr + h-1, tc + w - 1);
    draw_line(tr,       tc,       tr + h-1, tc);
    draw_line(tr,       tc + w-1, tr + h-1, tc + w - 1);
}

/* midpoint circle, centre (cr,cc), radius rad */
void draw_circle(int cr, int cc, int rad)
{
    if (rad <= 0) { plot(cr, cc); return; }

    int x = 0, y = rad, d = 1 - rad;
    while (x <= y) {
        plot(cr + y, cc + x);  plot(cr - y, cc + x);
        plot(cr + y, cc - x);  plot(cr - y, cc - x);
        plot(cr + x, cc + y);  plot(cr - x, cc + y);
        plot(cr + x, cc - y);  plot(cr - x, cc - y);
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

/* triangle given three vertices */
void draw_triangle(int r0, int c0, int r1, int c1, int r2, int c2)
{
    draw_line(r0, c0, r1, c1);
    draw_line(r1, c1, r2, c2);
    draw_line(r2, c2, r0, c0);
}

/* ------------------------------------------------------------------ */
/*  shape list                                                          */
/* ------------------------------------------------------------------ */

typedef enum { LINE, RECT, CIRCLE, TRIANGLE } ShapeKind;

typedef struct Shape {
    int id;
    ShapeKind kind;
    /* generic parameter store: p[0..7] */
    int p[8];
    struct Shape *next;
} Shape;

static Shape *head = NULL;
static int   next_id = 1;

/* append a new shape and return it */
static Shape *shape_new(ShapeKind k, int *params, int n)
{
    Shape *s = calloc(1, sizeof *s);
    s->id   = next_id++;
    s->kind = k;
    for (int i = 0; i < n && i < 8; i++) s->p[i] = params[i];
    /* append */
    if (!head) { head = s; return s; }
    Shape *t = head;
    while (t->next) t = t->next;
    t->next = s;
    return s;
}

int shape_add_line(int r0,int c0,int r1,int c1)
{
    int p[] = {r0,c0,r1,c1};
    return shape_new(LINE, p, 4)->id;
}
int shape_add_rect(int tr,int tc,int w,int h)
{
    int p[] = {tr,tc,w,h};
    return shape_new(RECT, p, 4)->id;
}
int shape_add_circle(int cr,int cc,int rad)
{
    int p[] = {cr,cc,rad};
    return shape_new(CIRCLE, p, 3)->id;
}
int shape_add_triangle(int r0,int c0,int r1,int c1,int r2,int c2)
{
    int p[] = {r0,c0,r1,c1,r2,c2};
    return shape_new(TRIANGLE, p, 6)->id;
}

/* remove shape by id; returns 1 on success */
int shape_delete(int id)
{
    Shape **pp = &head;
    while (*pp) {
        if ((*pp)->id == id) {
            Shape *victim = *pp;
            *pp = victim->next;
            free(victim);
            return 1;
        }
        pp = &(*pp)->next;
    }
    return 0;   /* not found */
}

/* modify any parameter of an existing shape */
int shape_modify(int id, int param_index, int new_val)
{
    if (param_index < 0 || param_index >= 8) return 0;
    for (Shape *s = head; s; s = s->next) {
        if (s->id == id) { s->p[param_index] = new_val; return 1; }
    }
    return 0;
}

/* (re-)render all shapes onto the canvas */
void render_all(void)
{
    canvas_clear();
    for (Shape *s = head; s; s = s->next) {
        switch (s->kind) {
            case LINE:
                draw_line(s->p[0],s->p[1],s->p[2],s->p[3]);
                break;
            case RECT:
                draw_rect(s->p[0],s->p[1],s->p[2],s->p[3]);
                break;
            case CIRCLE:
                draw_circle(s->p[0],s->p[1],s->p[2]);
                break;
            case TRIANGLE:
                draw_triangle(s->p[0],s->p[1],s->p[2],s->p[3],s->p[4],s->p[5]);
                break;
        }
    }
}

/* pretty-print the shape list */
void list_shapes(void)
{
    if (!head) { printf("  (no shapes)\n"); return; }
    static const char *names[] = {"LINE","RECT","CIRCLE","TRIANGLE"};
    for (Shape *s = head; s; s = s->next) {
        printf("  [%d] %s ", s->id, names[s->kind]);
        switch (s->kind) {
            case LINE:
                printf("(%d,%d)->(%d,%d)", s->p[0],s->p[1],s->p[2],s->p[3]);
                break;
            case RECT:
                printf("top-left=(%d,%d) w=%d h=%d",
                       s->p[0],s->p[1],s->p[2],s->p[3]);
                break;
            case CIRCLE:
                printf("centre=(%d,%d) r=%d",
                       s->p[0],s->p[1],s->p[2]);
                break;
            case TRIANGLE:
                printf("(%d,%d) (%d,%d) (%d,%d)",
                       s->p[0],s->p[1],s->p[2],s->p[3],s->p[4],s->p[5]);
                break;
        }
        printf("\n");
    }
}

/* free everything */
static void cleanup(void)
{
    while (head) shape_delete(head->id);
}

/* ------------------------------------------------------------------ */
/*  interactive menu                                                    */
/* ------------------------------------------------------------------ */

static void flush_stdin(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

static int read_int(const char *prompt)
{
    int v;
    printf("%s", prompt);
    while (scanf("%d", &v) != 1) { flush_stdin(); printf("%s", prompt); }
    flush_stdin();
    return v;
}

static void menu_add(void)
{
    printf("  Shape: 1=Line  2=Rect  3=Circle  4=Triangle : ");
    int ch = read_int("");
    int id = -1;
    switch (ch) {
        case 1: {
            int r0=read_int("  r0: "), c0=read_int("  c0: ");
            int r1=read_int("  r1: "), c1=read_int("  c1: ");
            id = shape_add_line(r0,c0,r1,c1);
            break;
        }
        case 2: {
            int tr=read_int("  top row: "), tc=read_int("  left col: ");
            int w =read_int("  width: "),  h =read_int("  height: ");
            id = shape_add_rect(tr,tc,w,h);
            break;
        }
        case 3: {
            int cr=read_int("  centre row: "), cc=read_int("  centre col: ");
            int rad=read_int("  radius: ");
            id = shape_add_circle(cr,cc,rad);
            break;
        }
        case 4: {
            int r0=read_int("  r0: "),c0=read_int("  c0: ");
            int r1=read_int("  r1: "),c1=read_int("  c1: ");
            int r2=read_int("  r2: "),c2=read_int("  c2: ");
            id = shape_add_triangle(r0,c0,r1,c1,r2,c2);
            break;
        }
        default:
            printf("  unknown shape.\n");
            return;
    }
    printf("  added shape id=%d\n", id);
}

static void menu_delete(void)
{
    int id = read_int("  id to delete: ");
    if (shape_delete(id))
        printf("  deleted id=%d\n", id);
    else
        printf("  id=%d not found.\n", id);
}

static void menu_modify(void)
{
    int id  = read_int("  shape id: ");
    int idx = read_int("  parameter index (0-based): ");
    int val = read_int("  new value: ");
    if (shape_modify(id, idx, val))
        printf("  updated.\n");
    else
        printf("  shape/param not found.\n");
}

void run_menu(void)
{
    canvas_clear();
    for (;;) {
        printf("\n--- 2-D editor ---\n");
        printf("  1  add shape\n");
        printf("  2  delete shape\n");
        printf("  3  modify shape\n");
        printf("  4  list shapes\n");
        printf("  5  render + display\n");
        printf("  0  quit\n");
        int choice = read_int("> ");

        switch (choice) {
            case 1: menu_add();    break;
            case 2: menu_delete(); break;
            case 3: menu_modify(); break;
            case 4: list_shapes(); break;
            case 5: render_all(); canvas_show(); break;
            case 0: cleanup(); return;
            default: printf("  try 0-5.\n");
        }
    }
}

/* ------------------------------------------------------------------ */
/*  demo – shown when you run without interactive input                 */
/* ------------------------------------------------------------------ */

static void demo(void)
{
    printf("=== demo render ===\n\n");

    int id_rect  = shape_add_rect    (1, 1, 20, 10);
    int id_circ  = shape_add_circle  (7, 40, 6);
    int id_tri   = shape_add_triangle(15, 5, 20, 15, 10, 25);
    int id_line  = shape_add_line    (0, 0, ROWS-1, COLS-1);

    render_all();
    canvas_show();

    printf("\nshape list:\n");
    list_shapes();

    /* move the circle to the left */
    printf("\n-- moving circle left by 10 cols --\n");
    shape_modify(id_circ, 1, 30);   /* param[1] = centre-col */
    render_all();
    canvas_show();

    /* delete the diagonal line */
    printf("\n-- deleting diagonal line (id %d) --\n", id_line);
    shape_delete(id_line);
    render_all();
    canvas_show();

    (void)id_rect; (void)id_tri;   /* suppress unused-variable warnings */
    cleanup();
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        demo();
    } else {
        run_menu();
    }
    return 0;
}
