William Li
Sichen Deng

Thing didn't figure out:
1. draw_sprite
    it didn't passed all the test. the core logic is good for us, but the image output remains incorrect in some regions.
    Our suspicion is that this issue stems from either a register misuse or the subtle mistake in handling source and destination indices. Possible related to stack misalignement or alpha blending
    Due to time constraints, we were unable to further isolate the bug. We ensured the function follows the calling conventions and the parameter structure is correct, but the test still fails.
2. Valgrind memory leaks
    When running the program with Valgrind, it reports memory leaks. Fixing all of them would probably require rewriting big parts of the project, especially how memory is managed in the image code. 
    Because of time limits and the risk of breaking working code, we didn’t try to fully fix the leaks.

Functions and tests written in C

William:
in_bounds
compute_index
draw_rect
draw_circle
draw_sprite

Sichen:
get_r
get_g
get_b
get_a
blend_components
blend_colors
set_pixel
draw_tile


Functions and tests written in asm

William:
in_bounds
compute_index
draw_circle
draw_rect
get_r
get_g
get_b
get_a


Sichen:
draw_sprite
draw_tile
blend_components
blend_colors
set_pixel
draw_pixel