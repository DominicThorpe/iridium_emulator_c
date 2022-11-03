start:
	LOAD $zero, $ua, $g9, @filename				; load addr of filename string into $ua/9
	ADDI $g8, $zero, 1							; put code for read mode (1) into $g8
	syscall 18									; open file in read mode
	
	LOAD $ua, $g7, $zero, @kernel_start			; load kernel start address into $ua/0
	LOAD $g0, $g7, $g8, @kernel_start
	LOAD $g1, $g8, $g9, @block_size				; load block size of 512 into $g1


init_block_loop:
	ADD  $g2, $zero, $zero						; init EOF flag to 0
	ADD  $g8, $zero, $zero						; init loop counter
	JUMP $g4, $g5, @block_loop					; start looping


block_loop:
	CMP $g2, $zero								; end if EOF flag != 0
	BNE $g4, $g5, @end

	JAL $g4, $g5, @init_eof_scan_loop			; check for EOF by looping
	
	ADD $g0, $g0, $g1							; go to next block and loop again
	ADDC $ua, $ua
	JUMP $g4, $g5, @block_loop


init_eof_scan_loop:
	ADD $g3, $zero, $zero						; set loop counter to 0
	MOVLI $g4, 0xC1								; put 0x00C1 into $g4
	MOVUI $g4, 0x00 

	JUMP $g5, $g6, @eof_scan_loop				; jump to start of loop


eof_scan_loop:
	STORE $ra, $sp, $zero						; push $ra to the stack and jump to set flag
	ADDI $sp, $sp, 1									

	LOAD $g5, $g0, $g3							; load the next byte in the file
	CMP $g4, $g5
	JAL $g6, $g7, @set_eof_flag

	CMP $g2, $zero								; unset EOF flag if all following bytes are not 0
	BNE $g6, $g7, @check_null_after_eof

eof_loop_end:	
	LOAD $ra, $sp, $zero						; restore the value at addr in $sp to $ra
	SUBC $sp, $sp
	JUMP $ra									; return to block loop


check_null_after_eof:
	CMP $g5, $zero
	BEQ $g6, $g7, @unset_eof_flag
	JUMP $g6, $g7, @eof_loop_end


set_eof_flag:
	ADDI $g2, $zero, 1							; set EOF flag = 1
	JUMP $ra


unset_eof_flag:
	ADD $g2, $zero, $zero						; set EOF flag = 0
	JUMP $g6, $g7, @eof_loop_end
	


end: HALT




data:
	filename: .text 15 "iridium_OS.ird"
	kernel_start: .long 0x00002000
	block_size: .int 512