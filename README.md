# 2D Character Graphics Editor

A terminal-based 2D graphics editor written in C. Uses `*` for drawn pixels and `_` for empty space, stored in a 2D character array.

## Features

- Draw **lines**, **rectangles**, **circles**, and **triangles**
- **Add**, **delete**, and **modify** shapes by ID
- All shapes are stored in a linked list and re-rendered on demand
- Interactive menu + `--demo` mode

## Build

```bash
gcc -Wall -o editor editor.c -lm
```

## Run

Interactive mode:
```bash
./editor
```

Demo (no input needed):
```bash
./editor --demo
```

## How it works

### Canvas
A `ROWS × COLS` char array. `canvas_clear()` fills it with `_`. `plot(r, c)` sets a cell to `*`.

### Drawing functions
| Function | Algorithm |
|---|---|
| `draw_line` | Bresenham's line |
| `draw_rect` | Four calls to `draw_line` |
| `draw_circle` | Midpoint circle algorithm |
| `draw_triangle` | Three calls to `draw_line` |

### Shape list
Each shape has an integer `id`, a `kind` enum, and up to 8 integer parameters. Shapes live in a singly-linked list. `render_all()` clears the canvas and redraws every shape from scratch.

### Modify
`shape_modify(id, param_index, new_value)` lets you change any single parameter of any shape. For example, to move a circle's centre column: `shape_modify(id, 1, new_col)`.

## Sample output

```
+------------------------------------------------------------+
|____________________________________________________________|
|_********************_______*****___________________________|
|_*__________________*______*_____*__________________________|
...
```
