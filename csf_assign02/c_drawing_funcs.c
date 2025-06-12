// C implementations of drawing functions (and helper functions)

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "drawing_funcs.h"

////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////

// TODO: implement helper functions
int32_t in_bounds(struct Image *img, int32_t x, int32_t y) {
  int32_t img_width = img -> width;
  int32_t img_height = img -> height;

  if (y < 0 || y >= img_height || x < 0 || x >= img_width){
    return 0;
  }

  return 1;
}

uint32_t compute_index(struct Image *img, int32_t x, int32_t y) {
  //index= y*width + x
  int32_t img_width = img -> width;

  int32_t index = y * img_width + x;

  return index;
}

uint32_t compute_x(struct Image *img, uint32_t index) {
  int32_t img_width = img -> width;
  int32_t x = index % img_width;
  return x;
}

uint32_t compute_y(struct Image *img, uint32_t index) {
  int32_t img_width = img -> width;
  int32_t y = index / img_width;
  return y;
}

uint8_t get_r(uint32_t color){
  return (color >> 24) & 0xFF;
}

uint8_t get_g(uint32_t color){
  return (color >> 16) & 0xFF;
}

uint8_t get_b(uint32_t color){
  return (color >> 8) & 0xFF;
}

uint8_t get_a(uint32_t color){
  return (color) & 0xFF;
}

uint8_t blend_components(uint32_t fg, uint32_t bg, uint32_t alpha){
  uint32_t result = (alpha * fg + (bg * (255 - alpha))) / 255;
  return (uint8_t) result;
}

uint32_t blend_colors(uint32_t fg, uint32_t bg){
  uint8_t a = get_a(fg);
  if(a==0){
    return(bg);
  }

  if(a == 255) {
    return (get_r(fg) << 24) | (get_g(fg) << 16) | (get_b(fg) << 8) | 255;
  }

  uint8_t fg_r = get_r(fg);
  uint8_t fg_g = get_g(fg);
  uint8_t fg_b = get_b(fg);

  uint8_t bg_r = get_r(bg);
  uint8_t bg_g = get_g(bg);
  uint8_t bg_b = get_b(bg);

  uint8_t final_r = blend_components(fg_r, bg_r, a);
  uint8_t final_g = blend_components(fg_g, bg_g, a);
  uint8_t final_b = blend_components(fg_b, bg_b, a);

  uint32_t result = (final_r << 24) | (final_g << 16) | (final_b << 8) | 255;
  return result; 
}

void set_pixel(struct Image *img, uint32_t index, uint32_t color){
  img->data[index] = color;
}
////////////////////////////////////////////////////////////////////////
// API functions
////////////////////////////////////////////////////////////////////////

//
// Draw a pixel.
//
// Parameters:
//   img   - pointer to struct Image
//   x     - x coordinate (pixel column)
//   y     - y coordinate (pixel row)
//   color - uint32_t color value
//
void draw_pixel(struct Image *img, int32_t x, int32_t y, uint32_t color) {
  // TODO: implement
  //debug
  //printf("drawing pixel at (%d, %d) with color 0x%08X\n", x, y, color);

  if (!in_bounds(img, x, y)){
    return;
  }

  uint32_t index = compute_index(img, x, y);
  uint32_t final_color = blend_colors(color, img->data[index]);
  set_pixel(img, index, final_color);
}

//
// Draw a rectangle.
// The rectangle has rect->x,rect->y as its upper left corner,
// is rect->width pixels wide, and rect->height pixels high.
//
// Parameters:
//   img     - pointer to struct Image
//   rect    - pointer to struct Rect
//   color   - uint32_t color value
//
void draw_rect(struct Image *img,
               const struct Rect *rect,
               uint32_t color) {
  // TODO: implement

  if (rect->width <= 0 || rect->height <= 0) return;

  int32_t min_x = rect->x;
  int32_t min_y = rect->y;
  int32_t max_x = rect->x + rect->width;
  int32_t max_y = rect->y + rect->height;

  // Clamp coordinates to image boundaries
  if (min_x < 0) min_x = 0;
  if (min_y < 0) min_y = 0;
  if (max_x > img->width) max_x = img->width;
  if (max_y > img->height) max_y = img->height;

  for (int32_t col = min_x; col < max_x; ++col) {
    for (int32_t row = min_y; row < max_y; ++row) {
      draw_pixel(img, col, row, color);
    }
  }
}

//
// Draw a circle.
// The circle has x,y as its center and has r as its radius.
//
// Parameters:
//   img     - pointer to struct Image
//   x       - x coordinate of circle's center
//   y       - y coordinate of circle's center
//   r       - radius of circle
//   color   - uint32_t color value
//
void draw_circle(struct Image *img,
                 int32_t x, int32_t y, int32_t r,
                 uint32_t color) {
  // TODO: implement
  if(r <= 0) return;

  int32_t x_min = x - r;
  int32_t x_max = x + r;
  int32_t y_min = y - r;
  int32_t y_max = y + r;

  if(y_min < 0) y_min = 0;
  if(x_min < 0) x_min = 0;
  if(y_max >= img->height) y_max = img->height-1;
  if(x_max >= img->width) x_max = img->width-1;

  for (int32_t i = y_min; i <= y_max; ++i) {
    for (int32_t j = x_min; j <= x_max; ++j) {
      if (i < 0 || j < 0 || j >= img->width || i >= img->height)
        continue;

      int32_t diff_x = x - j;
      int32_t diff_y = y - i;

      if (diff_x * diff_x + diff_y * diff_y <= r * r) {
        // printf("draw %d, %d, %d\n", color);
        draw_pixel(img, j, i, color);
      }
    }
  }
}

//
// Draw a tile by copying all pixels in the region
// enclosed by the tile parameter in the tilemap image
// to the specified x/y coordinates of the destination image.
// No blending of the tile pixel colors with the background
// colors should be done.
//
// Parameters:
//   img     - pointer to Image (dest image)
//   x       - x coordinate of location where tile should be copied
//   y       - y coordinate of location where tile should be copied
//   tilemap - pointer to Image (the tilemap)
//   tile    - pointer to Rect (the tile)
//
void draw_tile(struct Image *img,
               int32_t x, int32_t y,
               struct Image *tilemap,
               const struct Rect *tile) {
 // TODO: implement
  int32_t width = tile->width;
  int32_t height = tile->height;

  if (tile->x < 0 || tile->y < 0 || tile->x + width > tilemap->width || tile->y + height > tilemap->height) return;

  int32_t start_col = 0;
  int32_t start_row = 0;

  if(x<0) {
    start_col = -x;
    width -= start_col;
    x=0;
  }
  if(y < 0){
    start_row = -y;
    height -= start_row;
    y = 0;
  }
  if (x + width > img->width ) width = img -> width -x;
  if (y + height > img->height) height = img->height - y;
  if(width<=0 || height <= 0) return;
  
  
  int32_t tilemap_max_x = tilemap->width - tile->x;
  int32_t img_max_x = img->width - x;
  if(tilemap_max_x < width ) width = tilemap_max_x;
  if(img_max_x < width) width = img_max_x;

  int32_t tilemap_max_y = tilemap->height - tile->y;
  int32_t img_max_y = img -> height - y;
  if(tilemap_max_y < height ) height = tilemap_max_y;
  if(img_max_y < height) height = img_max_y;

  for(size_t col = 0; col < width; ++col){
    for(size_t row = 0; row < height; ++row){


      uint32_t index_img = compute_index(img, x + col, y + row);
      uint32_t index_tile = compute_index(tilemap, tile->x + start_col + col,tile ->y + start_row + row);
      img->data[index_img] = tilemap->data[index_tile];
    }
  }
}

//
// Draw a sprite by copying all pixels in the region
// enclosed by the sprite parameter in the spritemap image
// to the specified x/y coordinates of the destination image.
// The alpha values of the sprite pixels should be used to
// blend the sprite pixel colors with the background
// pixel colors.
//
// Parameters:
//   img       - pointer to Image (dest image)
//   x         - x coordinate of location where sprite should be copied
//   y         - y coordinate of location where sprite should be copied
//   spritemap - pointer to Image (the spritemap)
//   sprite    - pointer to Rect (the sprite)
//
void draw_sprite(struct Image *img,
                 int32_t x, int32_t y,
                 struct Image *spritemap,
                 const struct Rect *sprite) {
  // TODO: implement
  if(sprite->x < 0|| sprite->y <0 || sprite->x + sprite-> width > spritemap -> width || sprite->y + sprite->height > spritemap->height) return;

  for (int32_t col = 0; col < sprite->width; ++col) {
    for (int32_t row = 0; row < sprite->height; ++row) {
      int32_t out_x = x + col;
      int32_t out_y = y + row;

      if (out_x < 0 || out_x >= img->width || out_y < 0 || out_y >= img->height) 
        continue;

      if (out_x < 0 || out_x >= img->width || out_y < 0 || out_y >= img->height)
          continue;

      uint32_t index = compute_index(spritemap, sprite->x + col, sprite->y + row);
      uint32_t color = spritemap->data[index];
      draw_pixel(img, out_x, out_y, color);
    }
  }
}
