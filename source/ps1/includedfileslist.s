# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# to include the font data and various other parts
# without converting them to header files
	
	.section .text.wrapper, "x", @progbits
	
	.set push
	.set noreorder
    
	.global screen_tim
	.global chip8_rom
	
	.align 4			


screen_tim:
		.incbin "../../assets/Test.tim"
		.align 4

chip8_rom:
		.incbin "../../thirdparty/chip8-roms/games/Tetris [Fran Dachille, 1991].ch8"
		.align 4
	
	.set pop


