#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "drawing_funcs.h"
#include "tctest.h"
// TODO: add prototypes for your helper functions

// an expected color identified by a (non-zero) character code
typedef struct {
  char c;
  uint32_t color;
} ExpectedColor;

// struct representing a "picture" of an expected image
typedef struct {
  ExpectedColor colors[20];
  const char *pic;
} Picture;

typedef struct {
  struct Image small;
  struct Image large;
  struct Image tilemap;
  struct Image spritemap;
} TestObjs;

// dimensions and pixel index computation for "small" test image (objs->small)
#define SMALL_W        8
#define SMALL_H        6
#define SMALL_IDX(x,y) ((y)*SMALL_W + (x))

// dimensions of the "large" test image
#define LARGE_W        24
#define LARGE_H        20

// create test fixture data
TestObjs *setup(void) {
  TestObjs *objs = (TestObjs *) malloc(sizeof(TestObjs));
  init_image(&objs->small, SMALL_W, SMALL_H);
  init_image(&objs->large, LARGE_W, LARGE_H);
  objs->tilemap.data = NULL;
  objs->spritemap.data = NULL;
  return objs;
}

// clean up test fixture data
void cleanup(TestObjs *objs) {
  free(objs->small.data);
  free(objs->large.data);
  free(objs->tilemap.data);
  free(objs->spritemap.data);

  free(objs);
}

uint32_t lookup_color(char c, const ExpectedColor *colors) {
  for (unsigned i = 0; ; i++) {
    assert(colors[i].c != 0);
    if (colors[i].c == c) {
      return colors[i].color;
    }
  }
}

void check_picture(struct Image *img, Picture *p) {
  unsigned num_pixels = img->width * img->height;
  assert(strlen(p->pic) == num_pixels);

  for (unsigned i = 0; i < num_pixels; i++) {
    char c = p->pic[i];
    uint32_t expected_color = lookup_color(c, p->colors);
    uint32_t actual_color = img->data[i];
    ASSERT(actual_color == expected_color);
  }
}

// prototypes of test functions
void test_draw_pixel(TestObjs *objs);
void test_draw_rect(TestObjs *objs);
void test_draw_circle(TestObjs *objs);
void test_draw_circle_clip(TestObjs *objs);
void test_draw_tile(TestObjs *objs);
void test_draw_sprite(TestObjs *objs);

// prototypes of helper functions
void test_in_bounds(TestObjs *objs);
void test_in_bounds_2(TestObjs *objs);
void test_in_bounds_3(TestObjs *objs);
void test_in_bounds_4(TestObjs *objs);
void test_compute_index(TestObjs *objs);
void test_compute_index_2(TestObjs *objs);
void test_compute_index_3(TestObjs *objs);
void test_compute_index_4(TestObjs *objs);
void test_get_color_components();
void test_get_color_components_2(); 
void test_set_pixel(TestObjs *objs);
void test_set_pixel_2(TestObjs *objs);
void test_blend_components();
void test_blend_components_2();
void test_blend_colors();
void test_blend_colors_2();


int main(int argc, char **argv) {
  if (argc > 1) {
    // user specified a specific test function to run
    tctest_testname_to_execute = argv[1];
  }

  TEST_INIT();

  // TODO: add TEST() directives for your helper functions
  // helper functions
  TEST(test_in_bounds);
  TEST(test_in_bounds_2);
  TEST(test_in_bounds_3);
  TEST(test_in_bounds_4);
  TEST(test_compute_index);
  TEST(test_compute_index_2);
  TEST(test_compute_index_3);
  TEST(test_compute_index_4);
  TEST(test_get_color_components);
  TEST(test_get_color_components_2);
  TEST(test_blend_components);
  TEST(test_blend_components_2);
  TEST(test_blend_colors);
  TEST(test_blend_colors_2);
  TEST(test_set_pixel);
  TEST(test_set_pixel_2);
  TEST(test_draw_pixel);
  TEST(test_draw_rect);
  TEST(test_draw_circle);
  TEST(test_draw_circle_clip);
  TEST(test_draw_tile);
  TEST(test_draw_sprite);

  TEST_FINI();
}

void test_in_bounds(TestObjs *objs) {
  struct Image img = {.width = 10, .height = 6, .data = NULL};

  ASSERT(in_bounds(&img, 0, 0));
  ASSERT(in_bounds(&img, 9, 5));
  ASSERT(!in_bounds(&img, -1, 0));
  ASSERT(!in_bounds(&img, 10, 0));
  ASSERT(!in_bounds(&img, 0, -1));
  ASSERT(!in_bounds(&img, 0, 6));
  ASSERT(!in_bounds(&img, -1, -1));
  ASSERT(!in_bounds(&img, 10, 6));
}

void test_in_bounds_2(TestObjs *objs) {
  struct Image img = {.width = 0, .height = 0, .data = NULL};

  ASSERT(!in_bounds(&img, 0, 0));
  ASSERT(!in_bounds(&img, 9, 5));
  ASSERT(!in_bounds(&img, -1, 0));
  ASSERT(!in_bounds(&img, 10, 0));
  ASSERT(!in_bounds(&img, 0, -1));
  ASSERT(!in_bounds(&img, 0, 6));
  ASSERT(!in_bounds(&img, -1, -1));
  ASSERT(!in_bounds(&img, 10, 6));
}

void test_in_bounds_3(TestObjs *objs) {
  struct Image img = { .width = 8, .height = 8, .data = NULL };

  ASSERT(in_bounds(&img, 4, 4));
  ASSERT(in_bounds(&img, 7, 0));
  ASSERT(!in_bounds(&img, 3, 8));
  ASSERT(!in_bounds(&img, -1, 5));
}

void test_in_bounds_4(TestObjs *objs) {
  struct Image img = { .width = 1, .height = 1, .data = NULL };

  ASSERT(in_bounds(&img, 0, 0));
  ASSERT(!in_bounds(&img, 1, 0));
  ASSERT(!in_bounds(&img, 0, 1));
  ASSERT(!in_bounds(&img, -1, -1));
}

void test_compute_index(TestObjs *objs) {
  struct Image img = { .width = 10, .height = 6, .data = NULL};

  ASSERT(compute_index(&img, 0, 0) == 0);          // top-left
  ASSERT(compute_index(&img, 7, 0) == 7);          // top-right
  ASSERT(compute_index(&img, 0, 1) == 10);          // beginning of second row
  ASSERT(compute_index(&img, 1, 1) == 11);          // second row, second column
  ASSERT(compute_index(&img, 7, 5) == 57);         // bottom-right
  ASSERT(compute_index(&img, 3, 2) == 23);         // middle-ish

  ASSERT(compute_index(&img, 3, 2) == 23);
}

void test_compute_index_2(TestObjs *objs) {
  struct Image img = { .width = 10, .height = 6, .data = NULL};

  ASSERT(compute_index(&img, 0, -1) == -1);       
  ASSERT(compute_index(&img, 400, 0) == -1);      
  ASSERT(compute_index(&img, 0, 90) == -1);      
  ASSERT(compute_index(&img, -1, 1) == -1);        
  ASSERT(compute_index(&img, 80, -5) == -1);       
  ASSERT(compute_index(&img, -3, -2) == -1);         
  ASSERT(compute_index(&img, 3, 2) == 23);
}

void test_compute_index_3(TestObjs *objs) {
  struct Image img = { .width = 5, .height = 5, .data = NULL };

  ASSERT(compute_index(&img, 0, 4) == 20);
  ASSERT(compute_index(&img, 4, 0) == 4);
  ASSERT(compute_index(&img, 2, 2) == 12);
}

void test_compute_index_4(TestObjs *objs) {
  struct Image img = { .width = 5, .height = 5, .data = NULL };

  ASSERT(compute_index(&img, -2, 3) == -1);
  ASSERT(compute_index(&img, 3, -2) == -1);
  ASSERT(compute_index(&img, 5, 2) == -1);
  ASSERT(compute_index(&img, 2, 5) == -1);
}


void test_get_color_components() {
  uint32_t color1 = 0x12345678;
  ASSERT(get_r(color1) == 0x12);
  ASSERT(get_g(color1) == 0x34);
  ASSERT(get_b(color1) == 0x56);
  ASSERT(get_a(color1) == 0x78);

  uint32_t color2 = 0xFF0000FF; 
  ASSERT(get_r(color2) == 0xFF);
  ASSERT(get_g(color2) == 0x00);
  ASSERT(get_b(color2) == 0x00);
  ASSERT(get_a(color2) == 0xFF);

  uint32_t color3 = 0x00FF00FF; 
  ASSERT(get_r(color3) == 0x00);
  ASSERT(get_g(color3) == 0xFF);
  ASSERT(get_b(color3) == 0x00);
  ASSERT(get_a(color3) == 0xFF);
}

void test_get_color_components_2() {
  uint32_t color = 0xA1B2C3D4;
  ASSERT(get_r(color) == 0xA1);
  ASSERT(get_g(color) == 0xB2);
  ASSERT(get_b(color) == 0xC3);
  ASSERT(get_a(color) == 0xD4);

  uint32_t color_inv = 0x00000000;
  ASSERT(get_r(color_inv) == 0x00);
  ASSERT(get_g(color_inv) == 0x00);
  ASSERT(get_b(color_inv) == 0x00);
  ASSERT(get_a(color_inv) == 0x00);
}

void test_blend_components() {
  ASSERT(blend_components(100, 200, 255) == 100);

  ASSERT(blend_components(100, 200, 0) == 200);

  ASSERT(blend_components(100, 200, 128) == 149);

  ASSERT(blend_components(50, 250, 64) == 199);

  ASSERT(blend_components(255, 255, 255) == 255);
}

void test_blend_components_2() {
  // fg darker than bg
  ASSERT(blend_components(50, 200, 128) == ((50*128 + 200*127) / 255));
  // fg and bg same
  ASSERT(blend_components(100, 100, 128) == 100);
}

void test_blend_colors() {
  // Case 1: Fully opaque red over black → result should be red with alpha = 255
  uint32_t result1 = blend_colors(0xFF0000FF, 0x000000FF);
  ASSERT(result1 == 0xFF0000FF);

  // Case 2: Fully transparent green over black → result should be black with alpha = 0
  uint32_t result2 = blend_colors(0x00FF0000, 0x000000FF); // A=0, R=255
  ASSERT(result2 == 0x000000FF);

  // Case 3: 50% transparent green (G=255) over black
  uint32_t result3 = blend_colors(0x00FF0080, 0x000000FF);
  // alpha = 128, G = (128*255 + 127*0)/255 = ~128, so RGB = 0x008000
  ASSERT(result3 == 0x008000FF); // final A=128, RGB=(0,128,0)

  // Case 4: Blend with background blue (fully opaque)
  uint32_t result4 = blend_colors(0x00FF8000, 0x0000FFFF);
  // fg = (R=0, G=255, B=128), alpha=0
  // bg = blue, full alpha
  ASSERT(result4 == 0x0000FFFF);
}

void test_blend_colors_2() {

  uint32_t fg = 0xFF000080; // R=255, A=128
  uint32_t bg = 0x0000FF80; // B=255, A=128
  uint32_t result = blend_colors(fg, bg);

  ASSERT(get_r(result) == 128);
  ASSERT(get_g(result) == 0);
  ASSERT(get_b(result) == 127);
  ASSERT(get_a(result) == 255);
}

void test_set_pixel(TestObjs *objs) {
  struct Image img = { .width = 10, .height = 6, .data = NULL};
  img.data = malloc(sizeof(uint32_t) * img.width * img.height);
  set_pixel(&img, 100, 0x12345678);
  set_pixel(&img, -1, 0x12345678);
  set_pixel(&img, 30, 0x12345678);
  set_pixel(&img, 0, 0x12345678);
  set_pixel(&img, 60, 0x12345678);
  ASSERT(img.data[30] == 0x12345678);
  ASSERT(img.data[0] == 0x12345678);

  free(img.data); 

}


void test_set_pixel_2(TestObjs *objs) {
  struct Image img = { .width = 4, .height = 4, .data = NULL };
  img.data = malloc(sizeof(uint32_t) * img.width * img.height);

  set_pixel(&img, 15, 0xDEADBEEF);
  ASSERT(img.data[15] == 0xDEADBEEF);
  set_pixel(&img, 14, 0xbababa);
  set_pixel(&img, 16, 0x1234);
  ASSERT(img.data[15] == 0xDEADBEEF);
  ASSERT(img.data[14] == 0xbababa);
  free(img.data);
}

void test_draw_pixel(TestObjs *objs) {
  // initially objs->small pixels are opaque black
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0x000000FFU);
  ASSERT(objs->small.data[SMALL_IDX(5, 4)] == 0x000000FFU);

  //debug
  //printf("actual:   0x%08X\n", objs->small.data[SMALL_IDX(5, 4)]);
  //printf("expected: 0x800080FF\n");

  // test drawing completely opaque pixels
  draw_pixel(&objs->small, 3, 2, 0xFF0000FF); // opaque red
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0xFF0000FF);
  draw_pixel(&objs->small, 5, 4, 0x800080FF); // opaque magenta (half-intensity)
  ASSERT(objs->small.data[SMALL_IDX(5, 4)] == 0x800080FF);

  // test color blending
  draw_pixel(&objs->small, 3, 2, 0x00FF0080); // half-opaque full-intensity green
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0x7F8000FF);
  draw_pixel(&objs->small, 4, 2, 0x0000FF40); // 1/4-opaque full-intensity blue
  ASSERT(objs->small.data[SMALL_IDX(4, 2)] == 0x000040FF);
}

void test_draw_rect(TestObjs *objs) {
  struct Rect red_rect = { .x = 2, .y = 2, .width=3, .height=3 };
  struct Rect blue_rect = { .x = 3, .y = 3, .width=3, .height=3 };
  draw_rect(&objs->small, &red_rect, 0xFF0000FF); // red is full-intensity, full opacity
  draw_rect(&objs->small, &blue_rect, 0x0000FF80); // blue is full-intensity, half opacity

  const uint32_t red   = 0xFF0000FF; // expected full red color
  const uint32_t blue  = 0x000080FF; // expected full blue color
  const uint32_t blend = 0x7F0080FF; // expected red/blue blend color
  const uint32_t black = 0x000000FF; // expected black (background) color

  Picture expected = {
    { {'r', red}, {'b', blue}, {'n', blend}, {' ', black} },
    "        "
    "        "
    "  rrr   "
    "  rnnb  "
    "  rnnb  "
    "   bbb  "
  };

  check_picture(&objs->small, &expected);
}

void test_draw_circle(TestObjs *objs) {
  Picture expected = {
    { {' ', 0x000000FF}, {'x', 0x00FF00FF} },
    "   x    "
    "  xxx   "
    " xxxxx  "
    "  xxx   "
    "   x    "
    "        "
  };

  draw_circle(&objs->small, 3, 2, 2, 0x00FF00FF);

  check_picture(&objs->small, &expected);
}

void test_draw_circle_clip(TestObjs *objs) {
  Picture expected = {
    { {' ', 0x000000FF}, {'x', 0x00FF00FF} },
    " xxxxxxx"
    " xxxxxxx"
    "xxxxxxxx"
    " xxxxxxx"
    " xxxxxxx"
    "  xxxxx "
  };

  draw_circle(&objs->small, 4, 2, 4, 0x00FF00FF);

  check_picture(&objs->small, &expected);
}

void test_draw_tile(TestObjs *objs) {
  ASSERT(read_image("img/PrtMimi.png", &objs->tilemap) == IMG_SUCCESS);

  struct Rect r = { .x = 4, .y = 2, .width = 16, .height = 18 };
  draw_rect(&objs->large, &r, 0x1020D0FF);

  struct Rect grass = { .x = 0, .y = 16, .width = 16, .height = 16 };
  draw_tile(&objs->large, 3, 2, &objs->tilemap, &grass);

  Picture pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x52ad52ff },
      { 'b', 0x1020d0ff },
      { 'c', 0x257b4aff },
      { 'd', 0x0c523aff },
    },
    "                        "
    "                        "
    "             a     b    "
    "            a      b    "
    "            a     ab    "
    "           ac      b    "
    "           ac a    b    "
    "      a a  ad  a   b    "
    "     a  a aad  aa ab    "
    "     a  a acd aaacab    "
    "    aa  cdacdaddaadb    "
    "     aa cdaddaaddadb    "
    "     da ccaddcaddadb    "
    "    adcaacdaddddcadb    "
    "   aaccacadcaddccaab    "
    "   aacdacddcaadcaaab    "
    "   aaaddddaddaccaacb    "
    "   aaacddcaadacaaadb    "
    "    bbbbbbbbbbbbbbbb    "
    "    bbbbbbbbbbbbbbbb    "
  };

  check_picture(&objs->large, &pic);
}

void test_draw_sprite(TestObjs *objs) {
  ASSERT(read_image("img/NpcGuest.png", &objs->spritemap) == IMG_SUCCESS);

  struct Rect r = { .x = 1, .y = 1, .width = 14, .height = 14 };
  draw_rect(&objs->large, &r, 0x800080FF);

  struct Rect sue = { .x = 128, .y = 136, .width = 16, .height = 15 };
  draw_sprite(&objs->large, 4, 2, &objs->spritemap, &sue);

  Picture pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x800080ff },
      { 'b', 0x9cadc1ff },
      { 'c', 0xefeae2ff },
      { 'd', 0x100000ff },
      { 'e', 0x264c80ff },
      { 'f', 0x314e90ff },
    },
    "                        "
    " aaaaaaaaaaaaaa         "
    " aaaaaaaaaaaaaa         "
    " aaaaaaaaaaaaaa         "
    " aaaaaaabccccccbc       "
    " aaaaacccccccccccc      "
    " aaaacbddcccddcbccc     "
    " aaacbbbeccccedbccc     "
    " aaacbbceccccebbbccc    "
    " aaabbbccccccccbbccc    "
    " aaaabbbcccccccb ccb    "
    " aaaabaaaaabbaa  cb     "
    " aaaaaaaaafffea         "
    " aaaaaaaaaffeea         "
    " aaaaaaacffffcc         "
    "        cffffccb        "
    "         bbbbbbb        "
    "                        "
    "                        "
    "                        "
  };

  check_picture(&objs->large, &pic);
}
