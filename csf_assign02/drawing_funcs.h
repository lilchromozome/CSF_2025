#ifndef DRAWING_FUNCS_H
#define DRAWING_FUNCS_H

#include <stdint.h>
#include "image.h"

struct Rect {
  int32_t x, y, width, height;
};

// Helper function 
int32_t in_bounds(struct Image *img, int32_t x, int32_t y);
uint32_t compute_index(struct Image *img, int32_t x, int32_t y);
uint8_t get_r(uint32_t color);
uint8_t get_g(uint32_t color);
uint8_t get_b(uint32_t color);
uint8_t get_a(uint32_t color);


void draw_pixel(struct Image *img, int32_t x, int32_t y, uint32_t color);

void draw_rect(struct Image *img,
               const struct Rect *rect,
               uint32_t color);

void draw_circle(struct Image *img,
                 int32_t x, int32_t y, int32_t r,
                 uint32_t color);

void draw_tile(struct Image *img,
               int32_t x, int32_t y,
               struct Image *tilemap,
               const struct Rect *tile);

void draw_sprite(struct Image *img,
                 int32_t x, int32_t y,
                 struct Image *spritemap,
                 const struct Rect *sprite);

#endif // DRAWING_FUNCS_H
