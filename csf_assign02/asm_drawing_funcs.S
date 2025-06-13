/*
 * Assembly-language implementations of the drawing functions
 */

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET   0
#define IMAGE_HEIGHT_OFFSET  4
#define IMAGE_DATA_OFFSET    8

/* Offsets of struct Rect fields */
#define RECT_X_OFFSET        0
#define RECT_Y_OFFSET        4
#define RECT_WIDTH_OFFSET    8
#define RECT_HEIGHT_OFFSET   12

	.section .text

/***********************************************************************
   Helper functions
   You should declare these as .globl so that
   they can be called from the unit test code.
 ***********************************************************************/

/* TODO: implement your helper functions here */

/***********************************************************************
   Public API functions
 ***********************************************************************/

/*check if (x,y) is in the boundary of the image
*
*Parameters:
*  img - pointer to Image
*  x - x coordinate
*  y - y coordinate
*
*Return: 1 if (x, y) is inside the image boundary, 0 otherwise.
*/ 

  .globl in_bounds
in_bounds:
    # rdi = struct Image *img, esi = int32 x, edx = int32 y
    # move  img.width in eax, img.height in ecx
    movl    (%rdi), %eax       # eax = img.width
    movl    4(%rdi), %ecx      # ecx = img-.eight

    /* if (y < 0) return false; */
    cmpl    $0, %edx
    jl      .Lfalse
    /* if (y >= img_height) return false; */
    cmpl    %ecx, %edx
    jge     .Lfalse
    /* if (x < 0) return false; */
    cmpl    $0, %esi
    jl      .Lfalse
    /* if (x >= img_width) return false; */
    cmpl    %eax, %esi
    jge     .Lfalse

    # all vals in bounds -> return true
    movl    $1, %eax
    ret
.Lfalse:
	# not in bounds -> retun false;
    movl $0, %eax
    ret

/* Compute the array index
*
* Returns: Index in the image data array corresponding to (x,y)
*/
.global compute_index
compute_index:
  # rdi = struct Image *img, esi = int32 x, edx = int32 y
  # uint32_t compute_index(struct Image *img, int32_t x, int32_t y) {

  /* if(!in_bounds(img, x, y)) return -1; */
  call in_bounds

  cmpl $0, %eax /* eax has result of in_bounds, false = 0 */
  je .return_neg1

  /* index= y*width + x */
  ; int32_t img_width = img -> width;
  movl (%rdi), %ecx

  ; int32_t index = y * img_width + x;
  imull   %ecx, %edx        # edx (index) = y * width 
  addl    %esi, %edx        # edx += x

  ; return index;
    movl    %edx, %eax
    ret

.return_neg1
  movl $-1, %eax
  ret


/* Extract red or green or blue or alpha component from a 32-bit color
*
* Parameter:
* color - 32-bit color value
*
* RRGGBBAA */

.global get_r
get_r:
  /* uint8_t get_r(uint32_t color) */
  # color in %edi
  movl %edi, %eax
  shrl $24, %eax     ; color >> 24 
  andl $0xFF, %eax   ; & 0xFF
  ret
.global get_g
get_g:
  /* uint8_t get_r(uint32_t color) */
  # color in %edi
  movl %edi, %eax
  shrl $16, %eax     ; color >> 16
  andl $0xFF, %eax   ; & 0xFF
  ret
.global get_b
get_b:
  /* uint8_t get_r(uint32_t color) */
  # color in %edi
  movl %edi, %eax
  shrl $8, %eax     ; color >> 8
  andl $0xFF, %eax   ; & 0xFF
  ret
.global get_a
get_r:
  /* uint8_t get_r(uint32_t color) */
  # color in %edi
  movl %edi, %eax
  andl $0xFF, %eax   ; & 0xFF
  ret

/*
// Blend the color by usng alpha
//
// Parameters:
// fg - foreground component
// bg - background component
// alpha - foregraound alpha value
*/
/* uint8_t blend_components(uint32_t fg, uint32_t bg, uint32_t alpha) */
  ; edi = fg
  ; esi = bg
  ; edx = alpha

  ; uint32_t result = (alpha * fg + (bg * (255 - alpha))) / 255;
  movl $255, %ecx         ; ecx = 255
  subl %edx, %ecx         ; ecx = 255 - alpha
  imull %esi, %ecx         ; ecx = bg * (255 - alpha)

  imull %edx, %edi         ; edi = alpha * fg
  addl %edi, %ecx         ; ecx = alpha * fg + bg * (255 - alpha)

  movl $255, %eax
  movl $0, %edx           ; clear edx for division
  movl %ecx, %eax         ; eax = numerator (retuned later)
  movl $255, %ecx         ; ecx = denominator
  divl %ecx               ; eax = edx:eax / ecx

  ; return (uint8_t) result;
  ret

.globl blend_colors
blend_colors:
  /* uint32_t blend_colors(uint32_t fg, uint32_t bg) */
  ; edi = fg
  ; esi = bg

    # save fg and bg on stack
    pushq %rbx              # rbx = temp var
    movl %edi, %ebx        # ebx = fg

    # uint8_t a = get_a(fg);
    call get_a             # input: edi = fg, output: rax lower byt al = a
    movzbl  %al, %ecx         # ecx = rax

    /* if (a == 0) return bg */
    cmpb $0, %al
    je .return_bg

    /* if (a == 255) return fg with a = 255 */
    cmpb $255, %al
    je .return_fg_full

    /* fg_r = get_r(fg) */
    movl %ebx, %edi
    call get_r
    movzbl  %al, %r8d         ; r8d = fg_r

    /* fg_g = get_g(fg) */
    movl %ebx, %edi
    call get_g
    movzbl %al, %r9d         ; r9d = fg_g

    /* fg_b = get_b(fg) */
    movl %ebx, %edi
    call get_b
    movzbl %al, %r10d        ; r10d = fg_b

    /* bg_r = get_r(bg) */
    movl %esi, %edi
    call get_r
    movzbl %al, %r11d        ; r11d = bg_r

    /* blend_r = blend_components(fg_r, bg_r, a) */
    movl %r8d, %edi        ; fg_r
    movl %r11d, %esi       ; bg_r
    movl %ecx, %edx        ; a
    call blend_components
    movzbl %al, %r8d         ; r8d = final_r

    /* bg_g = get_g(bg) */
    movl %esi, %edi
    call get_g
    movzbl %al, %r11d        ; r11d = bg_g

    /* blend_g = blend_components(fg_g, bg_g, a) */
    movl    %r9d, %edi        ; fg_g
    movl    %r11d, %esi       ; bg_g
    movl    %ecx, %edx        ; a
    call    blend_components
    movzbl  %al, %r9d         ; r9d = final_g

    /* bg_b = get_b(bg) */
    movl    %esi, %edi
    call    get_b
    movzbl  %al, %r11d        ; r11d = bg_b

    /* blend_b = blend_components(fg_b, bg_b, a) */
    movl    %r10d, %edi       ; fg_b
    movl    %r11d, %esi       ; bg_b
    movl    %ecx, %edx        ; a
    call    blend_components
    movzbl  %al, %r10d        # r10d = final_b

    /* uint32_t result = (final_r << 24) | (final_g << 16) | (final_b << 8) | 255; */
    movl    $255, %eax        ; a = 255
    shll    $8, %r10d         ; final_b << 8
    orl     %r10d, %eax       ; (final_r << 24)

    shll    $16, %r9d          ; final_g << 16
    orl     %r9d, %eax         ; (final_r << 24) | (final_g << 16)

    shll    $24, %r8d          ; final_r << 24
    orl     %r8d, %eax         ; (final_r << 24) | (final_g << 16) | (final_b << 8) | 255

    popq    %rbx              ; delete temp var
    ret

.return_bg:
    movl    %esi, %eax
    popq    %rbx ; delete temp var
    ret

.return_fg_full:
  /* return (get_r(fg) << 24) | (get_g(fg) << 16) | (get_b(fg) << 8) | 255; */
  # (get_r(fg) << 24)
    movl    %ebx, %edi    ; ebx = fg
    call    get_r
    movzbl  %al, %ecx     ; ecx = r
    shll    $24, %ecx     ; move to correct spot for r

  # (get_g(fg) << 16))
    movl    %ebx, %edi    ; ebx = fg
    call    get_g         
    movzbl  %al, %edx     ; edx = g
    shll    $16, %edx     ; move to correct spot for g
    orl     %edx, %ecx    ; get_r(fg) << 24) | (get_g(fg) << 16)

  # (get_b(fg) << 8)
    movl    %ebx, %edi    ; ebx = fg
    call    get_b         
    movzbl  %al, %edx     ; edx = b
    shll    $8, %edx      ; move to correct spot for b
    orl     %edx, %ecx    ; (get_r(fg) << 24) | (get_g(fg) << 16) | (get_b(fg) << 8)

    orl     $255, %ecx    ; (get_r(fg) << 24) | (get_g(fg) << 16) | (get_b(fg) << 8) | 255
    movl    %ecx, %eax    
    popq    %rbx          ; delete temp var
    ret



/*
 * Draw a pixel.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - x coordinate (pixel column)
 *   %edx     - y coordinate (pixel row)
 *   %ecx     - uint32_t color value
 */
	.globl draw_pixel
draw_pixel:
	/* TODO: implement */
	ret


/*
 * Draw a rectangle.
 * The rectangle has rect->x,rect->y as its upper left corner,
 * is rect->width pixels wide, and rect->height pixels high.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %rsi     - pointer to struct Rect
 *   %edx     - uint32_t color value
 */
	.globl draw_rect
draw_rect:
	/* TODO: implement */
	ret

/*
 * Draw a circle.
 * The circle has x,y as its center and has r as its radius.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - x coordinate of circle's center
 *   %edx     - y coordinate of circle's center
 *   %ecx     - radius of circle
 *   %r8d     - uint32_t color value
 */
	.globl draw_circle
draw_circle:
	/* TODO: implement */
	ret

/*
 * Draw a tile by copying all pixels in the region
 * enclosed by the tile parameter in the tilemap image
 * to the specified x/y coordinates of the destination image.
 * No blending of the tile pixel colors with the background
 * colors should be done.
 *
 * Parameters:
 *   %rdi - pointer to Image (dest image)
 *   %esi - x coordinate of location where tile should be copied
 *   %edx - y coordinate of location where tile should be copied
 *   %rcx - pointer to Image (the tilemap)
 *   %r8  - pointer to Rect (the tile)
 */
	.globl draw_tile
draw_tile:
	/* TODO: implement */
	ret

/*
 * Draw a sprite by copying all pixels in the region
 * enclosed by the sprite parameter in the spritemap image
 * to the specified x/y coordinates of the destination image.
 * The alpha values of the sprite pixels should be used to
 * blend the sprite pixel colors with the background
 * pixel colors.
 *
 * Parameters:
 *   %rdi - pointer to Image (dest image)
 *   %esi - x coordinate of location where sprite should be copied
 *   %edx - y coordinate of location where sprite should be copied
 *   %rcx - pointer to Image (the spritemap)
 *   %r8  - pointer to Rect (the sprite)
 */
	.globl draw_sprite
draw_sprite:
	/* TODO: implement */
	ret

/*
vim:ft=gas:
*/
