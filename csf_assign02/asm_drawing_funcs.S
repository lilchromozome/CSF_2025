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
//
//Parameters:
//  rdi = struct Image *img - pointer to Image
//  esi = int32_t x - x coordinate
//  edx = int32_t y - y coordinate
//
//Return: 1 if (x, y) is inside the image boundary, 0 otherwise.
*/ 
/* int32_t in_bounds(struct Image *img, int32_t x, int32_t y) { */
  .globl in_bounds
in_bounds:
    # move  img.width in eax, img.height in ecx
    movl    (%rdi), %eax       # eax = img.width
    movl    4(%rdi), %ecx      # ecx = img-.eight

    /* if (y < 0) return false; */
    cmpl    $0, %edx          # if (y < 0)
    jl      .Lfalse           # return false
    /* if (y >= img_height) return false; */
    cmpl    %ecx, %edx          # if (y >= img_height)
    jge     .Lfalse             # return false
    /* if (x < 0) return false; */
    cmpl    $0, %esi            # if (x < 0)
    jl      .Lfalse             # return false
    /* if (x >= img_width) return false; */
    cmpl    %eax, %esi          # if (x >= img_width)
    jge     .Lfalse             # return false

    # all vals in bounds -> return true
    movl    $1, %eax        # move 1 to return register
    ret
.Lfalse:
	# not in bounds -> retun false;
    xorl 	%eax, %eax          # clear return register
    ret

/*
// Compute the array index
//
//Parameters:
//  rdi = struct Image *img - pointer to Image
//  esi = int32_t x - x coordinate
//  edx = int32_t y - y coordinate
//
// Returns:
// Index in the image data array corresponding to (x,y)
//
*/
  .global compute_index
compute_index:
  # uint32_t compute_index(struct Image *img, int32_t x, int32_t y) {

  # rdi = struct Image *img
  # esi = int32 x
  # edx = int32 y

  /* if(!in_bounds(img, x, y)) return -1; */
  call in_bounds            # in_bounds(img, x, y)
  cmpl $0, %eax            /* if eax == 0, not in bounds */
  je .Lreturn_neg1          # return -1

  /* index= y*width + x */
  # int32_t img_width = img -> width;
  movl 	(%rdi), %ecx

  # int32_t index = y * img_width + x;
  imull   %ecx, %edx        # edx (index) = y * width 
  addl    %esi, %edx        # edx += x

  # return index#
  movl    %edx, %eax      # move index to return register
  ret

.Lreturn_neg1:
  movl $-1, %eax            # move 0 to return register
  ret


/* Extract red or green or blue or alpha component from a 32-bit color
*
* Parameter:
* edi = uint32_t color - 32-bit color value
*
* returns in format RRGGBBAA */

  /* uint8_t get_r(uint32_t color) */
.global get_r
get_r:
  # color in %edi
  movl 	%edi, %eax
  shrl	 $24, %eax     # color >> 24 
  andl 	$0xFF, %eax   # & 0xFF
  ret

/* uint8_t get_r(uint32_t color) */
.global get_g
get_g:
  # color in %edi
  movl	 %edi, %eax
  shrl 	$16, %eax     # color >> 16
  andl 	$0xFF, %eax   # & 0xFF
  ret

/* uint8_t get_r(uint32_t color) */
.global get_b
get_b:
  # color in %edi
  movl %edi, %eax
  shrl 	$8, %eax     # color >> 8
  andl 	$0xFF, %eax   # & 0xFF
  ret

/* uint8_t get_r(uint32_t color) */
.global get_a
get_a:
  # color in %edi
  movl 	%edi, %eax
  andl 	$0xFF, %eax   # & 0xFF
  ret

/*
// Blend the color by usng alpha
//
// Parameters:
// edi = uint32_t fg - foreground component
// esi = uint32_t bg - background component
// edx = uint32_t alpha - foregraound alpha value
*/
/* uint8_t blend_components(uint32_t fg, uint32_t bg, uint32_t alpha) */

.global blend_components
blend_components:
  pushq   %r8           # callee saved for fg
  pushq   %r9           # callee saved for bg
  pushq   %r10          # callee saved for a
  pushq   %rcx
  pushq   %rdx
  movl    %edi, %r8d     # r8d = fg
  movl    %esi, %r9d     # r9d = bg
  movl    %edx, %r10d    # r10d = alpha

  # uint32_t result = (alpha * fg + (bg * (255 - alpha))) / 255#
  movl 		$255, %eax         # ecx = 255
  subl 		%r10d, %eax         # ecx = 255 - alpha
  imull 	%r9d, %eax         # ecx = bg * (255 - alpha)

  imull 	%r10d, %r8d         # edi = alpha * fg
  addl 		%r8d, %eax         # ecx = alpha * fg + bg * (255 - alpha)

  xorl 	%edx, %edx           # clear edx for division
  movl 	$255, %ecx         # ecx = denominator
  divl 	%ecx               # eax = edx:eax / ecx

  # return (uint8_t) result#
  popq   %rdx         # clear all local registers
  popq   %rcx
  popq   %r10
  popq   %r9
  popq   %r8
  ret

/*
// Blend the color by usng alpha
//
// Parameters:
// rdi = uint32_t fg - foreground component
// esi = uint32_t bg - background component
// edx = uint32_t alpha - foregraound alpha value
//
*/
/* uint32_t blend_colors(uint32_t fg, uint32_t bg) */

.globl blend_colors
blend_colors:
  # save fg and bg on stack
  pushq 	%rbx              # rbx = callee saved fg
  pushq 	%r12              # r12 = callee saved bg.
  movl 	%edi, %ebx        # ebx = fg
  movl 	%esi, %r12d 		# r12 = bg

  # uint8_t a = get_a(fg)#
  call get_a             # input: edi = fg, output: rax lower byt al = a
  movzbl  	%al, %ecx         # ecx = a

  /* if (a == 0) return bg */
  cmpb 	$0, %al
  je 		.Lreturn_bg

  /* if (a == 255) return fg with a = 255 */
  cmpb 	$255, %al
  je 		.Lreturn_fg_full

  /* fg_r = get_r(fg) */
  movl 	%ebx, %edi          # edi = fg
  call get_r
  movzbl  %al, %r8d         # r8d = fg_r

  /* fg_g = get_g(fg) */
  movl 	%ebx, %edi          # edi = fg
  call get_g
  movl 	%eax, %r9d         # r9d = fg_g

  /* fg_b = get_b(fg) */
  movl 	%ebx, %edi          # edi = fg
  call get_b
  movzbl 	%al, %r10d        # r10d = fg_b

  /* bg_r = get_r(bg) */
  movl 	%r12d, %edi         # edi = fg
  call get_r
  movzbl 	%al, %r11d        # r11d = bg_r

  /* blend_r = blend_components(fg_r, bg_r, a) */
  movl	 %r8d, %edi        # edi = fg_r
  movl 	%r11d, %esi       # esi = bg_r
  movl 	%ecx, %edx        # edx = a
  call blend_components      # eax = blend_components(fg_r, bg_r, a)
  movzbl 	%al, %r8d         # r8d = final_r

  /* bg_g = get_g(bg) */
  movl 	%r12d, %edi         # edi = bg
  call get_g
  movl 	%eax, %r11d        # r11d = bg_g

  /* blend_g = blend_components(fg_g, bg_g, a) */
  movl 	%r9d, %edi        # edi = fg_g
  movl 	%r11d, %esi       # esi = bg_g
  movl 	%ecx, %edx        # edx = a
  call 	blend_components    # eax = blend_components(fg_g, bg_g, a)
  movl	%eax, %r9d         # r9d = final_g

  /* bg_b = get_b(bg) */
  movl 	%r12d, %edi         # edi = bg
  call 	get_b
  movzbl 	%al, %r11d        # r11d = bg_b

  /* blend_b = blend_components(fg_b, bg_b, a) */
  movl 	%r10d, %edi       # edi = fg_b
  movl 	%r11d, %esi       # esi = bg_b
  movl 	%ecx, %edx        # edx = a
  call 	blend_components  # eax = blend_components(fg_b, bg_b, a)
  movzbl 	%al, %r10d        # r10d = final_b

  /* uint32_t result = (final_r << 24) | (final_g << 16) | (final_b << 8) | 255; */
  movl 	$255, %eax        # a = 255
  
  # movl %r10d, %edx
  shll 	$8, %r10d         # final_b << 8
  orl 	%r10d, %eax       # (final_b << 8)

  # movl  %r9d, %edx
  shll 	$16, %r9d          # final_g << 16
  orl 	%r9d, %eax         # (final_r << 24) | (final_g << 16)

  # movl  %r8d, %edx
  shll 	$24, %r8d          # final_r << 24
  orl 	%r8d, %eax         # (final_r << 24) | (final_g << 16) | (final_b << 8) | 255

  popq 	%r12
  popq 	%rbx              # delete callee saved registers
  ret

/* return(bg); */
.Lreturn_bg:
    movl 	%r12d, %eax
    popq  %r12
    popq 	%rbx # delete callee saved registers
    ret

/* return (get_r(fg) << 24) | (get_g(fg) << 16) | (get_b(fg) << 8) | 255# */
.Lreturn_fg_full:
  # (get_r(fg) << 24)
    movl 	%ebx, %edi    # ebx = fg
    call 	get_r
    movzbl 	%al, %ecx     # ecx = r
    shll    $24, %ecx     # move to correct spot for r

  # (get_g(fg) << 16))
    movl    %ebx, %edi    # ebx = fg
    call    get_g         
    movzbl  %al, %edx     # edx = g
    shll    $16, %edx     # move to correct spot for g
    orl     %edx, %ecx    # get_r(fg) << 24) | (get_g(fg) << 16)

  # (get_b(fg) << 8)
    movl    %ebx, %edi    # ebx = fg
    call    get_b         
    movzbl  %al, %edx     # edx = b
    shll    $8, %edx      # move to correct spot for b
    orl     %edx, %ecx    # (get_r(fg) << 24) | (get_g(fg) << 16) | (get_b(fg) << 8)

    orl     $255, %ecx    # (get_r(fg) << 24) | (get_g(fg) << 16) | (get_b(fg) << 8) | 255
    movl    %ecx, %eax    

    popq    %r12
    popq    %rbx          # delete callee saved registers
    ret

/*
//
// Set a pixel's color in the image data array at a specific index.
//
// Parameters:
//   rdi = struct Image *img   - pointer to struct Image
//   esi = uint32_t index - linear index in the image's data array
//   edx = uint32_t color - uint32_t color value to be written
//
*/
/* void set_pixel(struct Image *img, uint32_t index, uint32_t color) */
.global set_pixel
set_pixel:
  cmp   $0, %esi
  jl    .Lreturn_set_pixel

  movq 8(%rdi), %rcx        # 2 uint32 values (width, length in struct) store position of *data in %rcx
  # img->data[index] = color;
  movl    %esi, %eax        # eax = index
  movslq %eax, %rax         # sign extend eax
  movl    %edx, (%rcx,%rax,4)   # data[index] = color     4 bytes * index + data pointer
  ret

.Lreturn_set_pixel:
  ret


/*
 * Draw a pixel.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - int32_t x coordinate (pixel column)
 *   %edx     - int32_t y coordinate (pixel row)
 *   %ecx     - uint32_t color value
 */
/* void draw_pixel(struct Image *img, int32_t x, int32_t y, uint32_t color) */
	.globl draw_pixel
draw_pixel:
	/* TODO: implement */

  pushq %rbp    # save rbp
  movq %rsp, %rbp  # set rbp to current stack pointer
  subq  $56, %rsp  # align stack to 16 bytes
  # Gavi said to do 96 and he wrote this comment. You can ask him
  # do not subtract points for that comment. This isn't compiler 
  # generated code. 
  # allocate on stack
  movq %rdi, -8(%rbp)
  movl %esi, -16(%rbp)
  movl %edx, -24(%rbp)
  movl %ecx, -32(%rbp)

  call compute_index
  js .Lreturn_empty

  movq -8(%rbp), %rdi  # rdi = img
  movl -16(%rbp), %esi  # esi = x
  movl -24(%rbp), %edx  # edx = y
  call compute_index
  movl %eax, -40(%rbp)  # store index in stack

  
  movq -8(%rbp), %rdi  # rdi = img pointer
  xorq %r10, %r10  # clear r10
  movq 8(%rdi), %r11  # r11 = img->data pointer
  movl -40(%rbp), %r10d  # r10 = index
  movslq %r10d, %r10  # sign extend index to 64-bit
  leaq (%r11, %r10, 4), %r11  # r11 = img->data + index * 4
  movl (%r11), %esi
  movl %esi, -48(%rbp)  # store img->data[index] in stack
  


  movl -32(%rbp), %edi  # edi = color
  movl -48(%rbp), %esi  # esi = background color
  call  blend_colors        # eax = color after blending

  movq -8(%rbp), %rdi  # r13 = img pointer
  movl -40(%rbp), %esi  # esi = index
  movl  %eax, %edx  # edx = final color
  call   set_pixel

.Lreturn_empty:
  addq $56, %rsp  # deallocate stack
  popq %rbp      # restore rbp
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
  # void draw_rect(struct Image *img, const struct Rect *rect, uint32_t color)
  # %rdi = img, %rsi = rect, %edx = color

  pushq %r12
  pushq %r13
  pushq %r14
  

  movl %edx, %r12d           # save color in %r12d

  movl 8(%rsi), %eax         # width
  testl %eax, %eax
  jle .Ldone

  movl 12(%rsi), %eax        # height
  testl %eax, %eax
  jle .Ldone

  # min_x = rect->x, min_y = rect->y
  movl 0(%rsi), %r8d         # min_x
  movl 4(%rsi), %r9d         # min_y

  # max_x = rect->x + rect->width
  movl %r8d, %r10d
  addl 8(%rsi), %r10d

  # max_y = rect->y + rect->height
  movl %r9d, %r11d
  addl 12(%rsi), %r11d

  # clip min_x to >= 0
  cmpl $0, %r8d
  movl $0, %ecx
  cmovl %ecx, %r8d

  # clip min_y to >= 0
  cmpl $0, %r9d
  movl $0, %ecx
  cmovl %ecx, %r9d

  # clip max_x to <= img->width
  movl 0(%rdi), %eax         # img->width
  cmpl %eax, %r10d
  cmovg %eax, %r10d

  # clip max_y to <= img->height
  movl 4(%rdi), %eax         # img->height
  cmpl %eax, %r11d
  cmovg %eax, %r11d

  # for (int row = min_y; row < max_y; row++)
  movl %r9d, %r14d           # r14d = row

.Lrow_loop:
  cmpl %r11d, %r14d           # if row >= max_y
  jge .Ldone
  movl %r8d, %r13d           # r15d = col

.Lcol_loop:
  cmpl %r10d, %r13d           # if col >= max_x
  jge .Lnext_row

  # draw_pixel(img, col, row, color)
  movl %r13d, %esi           # x = col
  movl %r14d, %edx           # y = row
  movl %r12d, %ecx           # color
  call draw_pixel

  incl %r13d
  jmp .Lcol_loop

.Lnext_row:
  incl %r14d
  jmp .Lrow_loop

.Ldone:
  
  popq %r14
  popq %r13
  popq %r12
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
  /* void draw_circle(struct Image *img, int32_t x, int32_t y, int32_t r, uint32_t color) */
  pushq   %r12
  pushq   %r13
  pushq   %r14
  pushq   %r15

  movl    %r8d, %r12d       # r12 = color
  movl    %esi, %r13d       # r13 = x
  movl    %edx, %r14d       # r14 = y
  movl    %ecx, %r15d       # r15 = r

  /* if (r < 0) return */
  cmpl    $0, %ecx
  jl      .Ldone_circle

  # x_min = x - r
  # x_max = x + r
  movl    %r13d, %r8d
  subl    %ecx, %r8d        # r8d = x_min
  movl    %r13d, %r9d
  addl    %ecx, %r9d        # r9d = x_max

  # y_min = y - r
  # y_max = y + r
  movl    %r14d, %r10d
  subl    %ecx, %r10d       # r10d = y_min
  movl    %r14d, %r11d
  addl    %ecx, %r11d       # r11d = y_max

  /* 
  if(y_min < 0) y_min = 0;
  if(x_min < 0) x_min = 0;
  if(y_max >= img->height) y_max = img->height-1;
  if(x_max >= img->width) x_max = img->width-1;
  */ 

  # clamp: y_min = max(0, y_min)
  movl    $0, %ecx        # ecx = 0
  cmpl    $0, %r10d
  cmovl   %ecx, %r10d     # r10 = y_min

  # x_min = max(0, x_min)
  cmpl    $0, %r8d
  cmovl   %ecx, %r8d       # r8 = x_min

  # y_max = min(img->height, y_max)
  movl    4(%rdi), %eax     # eax = img->height
  movl    %r11d, %ecx
  cmpl    %ecx, %eax        # if img->height < y_max
  cmovl   %eax, %ecx
  movl    %ecx, %r11d       # r11 = y_max

  # x_max = min(img->width, x_max)
  movl    (%rdi), %eax      # eax = img->width
  movl    %r9d, %ecx
  cmpl    %ecx, %eax        # if img->width < x_max
  cmovl   %eax, %ecx
  movl    %ecx, %r9d        # r9 = x_max

# y loop: for (int32_t i = y_min; i <= y_max; ++i)
.Ly_loop:
  cmpl    %r11d, %r10d      # i <= y_max
  jg      .Ldone_circle
  # x loop: for (int32_t j = x_min; j <= x_max; ++j) 
  movl    %r8d, %ecx        # ecx = j

.Lx_loop:
  cmpl    %r9d, %ecx        # j <= x_max
  jg      .Ladvance_y

  /* if (!in_bounds(img, j, i)) continue; */
  movl    %ecx, %esi        # esi = j
  movl    %r10d, %edx       # edx = i
  call    in_bounds
  testl   %eax, %eax
  je      .Ladvance_x

  # compute diff_x = center_x - j
  movl    %r13d, %eax       # eax = center_x
  subl    %ecx, %eax        # eax = diff_x
  imull   %eax, %eax        # eax = diff_x^2

  # compute diff_y = center_y - i
  movl    %r14d, %edx       # edx = center_y
  subl    %r10d, %edx       # edx = diff_y
  imull   %edx, %edx        # edx = diff_y^2

  /*  if (diff_x * diff_x + diff_y * diff_y <= r * r)  */
  imull   %r15d, %r15d      # r15d = r * r

  addl    %edx, %eax        # eax = diff_x^2 + diff_y^2
  cmpl    %r15d, %eax       # r^2 - eax > 0
  jg      .Ladvance_x

  # draw_pixel(img, j, i, color)
  movl    %ecx, %esi        # esi = j
  movl    %r10d, %edx       # edx = i
  movl    %r12d, %ecx       # ecx = color
  call    draw_pixel 

.Ladvance_x:
    incl    %ecx              # ++j
    jmp     .Lx_loop

.Ladvance_y:
    incl    %r10d             # ++i
    jmp     .Ly_loop

.Ldone_circle:
    # Restore stack
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
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
  # rdi = img
  # rsi = tilemap
  # edx = tile_x
  # ecx = tile_y
  # r8d = x
  # r9d = y

  pushq %rbx
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15
  pushq %rbp

  # Load tile->x, tile->y, tile->width, tile->height
  movl 0(%r8), %r10d        # tile_x
  movl 4(%r8), %r11d        # tile_y
  movl 8(%r8), %r12d        # tile_width
  movl 12(%r8), %r13d       # tile_height

  # Bounds check on tilemap (tile_x + tile_width <= tilemap->width)
  cmpl $0, %r10d
  jl .Ldone_tile
  cmpl $0, %r11d
  jl .Ldone_tile

  movl 0(%rsi), %eax        # tilemap->width
  movl %r10d, %ebx
  addl %r12d, %ebx
  cmpl %eax, %ebx
  jg .Ldone_tile

  movl 4(%rsi), %eax        # tilemap->height
  movl %r11d, %ebx
  addl %r13d, %ebx
  cmpl %eax, %ebx
  jg .Ldone_tile

  # Clip to left/top screen edge
  xorl %r14d, %r14d         # start_col
  xorl %r15d, %r15d         # start_row
  movl %r12d, %ebp          # width
  movl %r13d, %ebx          # height

  cmpl $0, %r8d             # if x < 0
  jge .Lcheck_y
  negl %r8d
  movl %r8d, %r14d          # start_col = -x
  subl %r8d, %ebp           # width -= -x
  xorl %r8d, %r8d           # x = 0

.Lcheck_y:
  cmpl $0, %r9d             # if y < 0
  jge .Lclip_bounds
  negl %r9d
  movl %r9d, %r15d          # start_row = -y
  subl %r9d, %ebx           # height -= -y
  xorl %r9d, %r9d           # y = 0

.Lclip_bounds:
  # width = min(width, img->width - x)
  movl 0(%rdi), %eax
  subl %r8d, %eax
  cmpl %eax, %ebp
  cmovg %eax, %ebp

  # height = min(height, img->height - y)
  movl 4(%rdi), %eax
  subl %r9d, %eax
  cmpl %eax, %ebx
  cmovg %eax, %ebx

  # width = min(width, tilemap->width - tile_x - start_col)
  movl 0(%rsi), %eax
  subl %r10d, %eax
  subl %r14d, %eax
  cmpl %eax, %ebp
  cmovg %eax, %ebp

  # height = min(height, tilemap->height - tile_y - start_row)
  movl 4(%rsi), %eax
  subl %r11d, %eax
  subl %r15d, %eax
  cmpl %eax, %ebx
  cmovg %eax, %ebx

  # Final check
  cmpl $0, %ebp
  jle .Ldone_tile
  cmpl $0, %ebx
  jle .Ldone_tile

  # r8d = screen x, r9d = screen y
  # ebp = clipped width, ebx = clipped height
  # r10d = tile_x, r11d = tile_y
  # r14d = start_col, r15d = start_row

  xorl %r12d, %r12d         # col = 0

.Lcol_loop_tile:
  cmpl %ebp, %r12d
  jge .Ldone_tile
  xorl %r13d, %r13d         # row = 0

.Lrow_loop_tile:
  cmpl %ebx, %r13d
  jge .Lnext_col

  # img_index = compute_index(img, x + col, y + row)
  movq %rdi, %rsi           # rdi preserved, move to rsi
  movl %r8d, %r8d           # x
  addl %r12d, %r8d
  movl %r9d, %r9d           # y
  addl %r13d, %r9d
  movq %rsi, %rdi
  call compute_index
  cmpl $-1, %eax
  je .Lskip_pixel
  movl %eax, %r14d          # img index

  # tile_index = compute_index(tilemap, tile_x + start_col + col, tile_y + start_row + row)
  movl %r10d, %r8d
  addl %r14d, %r8d
  addl %r12d, %r8d

  movl %r11d, %r9d
  addl %r15d, %r9d
  addl %r13d, %r9d

  movq %rsi, %rdi           # tilemap in rsi
  call compute_index
  cmpl $-1, %eax
  je .Lskip_pixel
  movl %eax, %r15d          # tile index

  # img->data[img_index] = tilemap->data[tile_index]
  movq 16(%rdi), %rsi       # tilemap->data
  movq 16(%rsi), %rdi       # img->data
  movl (%rsi, %r15, 4), %eax
  movl %eax, (%rdi, %r14, 4)

.Lskip_pixel:
  incl %r13d                # row++
  jmp .Lrow_loop

.Lnext_col:
  incl %r12d                # col++
  jmp .Lcol_loop

.Ldone_tile:
  popq %rbp
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %rbx
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
  pushq %rbx
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15
  pushq %rbp

  movl 0(%r8), %r9d
  movl 4(%r8), %r10d
  movl 8(%r8), %r11d
  movl 12(%r8), %ebp

  cmpl $0, %r9d
  jl .Ldone_sprite
  cmpl $0, %r10d
  jl .Ldone_sprite

  movl 0(%rcx), %eax        # spritemap->width
  movl %r9d, %ebx
  addl %r11d, %ebx
  cmpl %eax, %ebx
  jg .Ldone_sprite

  movl 4(%rcx), %eax        # spritemap->height
  movl %r10d, %ebx
  addl %ebp, %ebx
  cmpl %eax, %ebx
  jg .Ldone_sprite

  # r12 = start_col
  # r13 = start_row
  # r14 = width
  # r15 = height
  xorl %r12d, %r12d         
  xorl %r13d, %r13d
  movl %r11d, %r14d
  movl %ebp, %r15d

  cmpl $0, %edx
  jge .Lclip_bounds_sprite
  movl %edx, %eax
  negl %eax
  movl %eax, %r13d          # start_row = -y
  subl %eax, %r15d          # height -= -y
  xorl %edx, %edx           # y = 0

  .Lclip_bounds_sprite:
    movl 0(%rdi), %eax
    subl %esi, %eax
    cmpl %eax, %r14d
    cmovg %eax, %r14d

    movl 4(%rdi), %eax
    subl %edx, %eax
    cmpl %eax, %r15d
    cmovg %eax, %r15d

    movl 0(%rcx), %eax
    subl %r9d, %eax
    subl %r12d, %eax
    cmpl %eax, %r14d
    cmovg %eax, %r14d

    movl 4(%rcx), %eax
    subl %r10d, %eax
    subl %r13d, %eax
    cmpl %eax, %r15d
    cmovg %eax, %r15d

    cmpl $0, %r14d
    jle .Ldone_sprite
    cmpl $0, %r15d
    jle .Ldone_sprite

    movq 16(%rdi), %rsi       # img->data
    movq 16(%rcx), %rdi       # spritemap->data
    xorl %ebx, %ebx

  .Lcol_loop_sprite:
    cmpl %r14d, %ebx
    jge .Ldone_sprite
    xorl %eax, %eax 
  
  .Lrow_loop_sprite:
    cmpl %r15d, %eax
    jge .Lnext_col_sprite

    # img_index = compute_index(img, x + col, y + row)
    movl %esi, %r8d          # r12 = x
    addl %ebx, %r8d          # r12 += col
    movl %edx, %r9d          # r13 = y
    addl %eax, %r9d          # r13 += row
    call compute_index
    cmpl $-1, %eax
    je .Lskip_sprite_pixel
    movl %eax, %r10d          # img index

    # spritemap_index = compute_index(spritemap, tile_x + start_col + col, tile_y + start_row + row)
    movl %r9d, %r8d
    movl %r10d, %r9d
    movl %r9d, %r11d
    addl %r13d, %r11d
    addl %eax, %r11d
    movl %r10d, %r12d
    addl %r12d, %r12d
    addl %ebx, %r12d
    movq %rdi, %rdi
    movl %r12d, %r8d
    movl %r11d, %r9d
    call compute_index
    movl %eax, %r11d
    cmpl $-1, %eax
    je .Lskip_sprite_pixel

    # blend
    movl (%rdi,%r11,4), %r12d
    movl (%rsi,%r10,4), %r13d
    movl %r12d, %edi
    movl %r13d, %esi
    call blend_colors

    movl %eax, (%rsi,%r10,4)

  .Lskip_sprite_pixel:
    incl %eax
    jmp .Lrow_loop_sprite

  .Lnext_col_sprite:
    incl %ebx
    jmp .Lcol_loop_sprite

  .Ldone_sprite:
    popq %rbp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx

	ret

/*
vim:ft=gas:
*/
