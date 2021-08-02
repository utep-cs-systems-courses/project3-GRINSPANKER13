	.arch msp4302553
	.p2align 1,0

	.data
	.align 2
BLUE:	.word 0xf800
GREEN:	.word 0x07e0
RED:	.word 0x001f
YELLOW:	.word 0x07ff
HEIGHT:	.byte 160
WIDTH:	.byte 128

	.text
	.global calculateWin
	;; u char col	1 byte	r12
	;; u char row	1 byte	r13
	;; u int color	2 byte	r14
	;; char retVal	1 byte
calculateWin:
	SUB.W   #6, R1			; Make room for vars
	MOV.B   R12, 3(R1)		; col
	MOV.B   R13, 2(R1)		; row
	MOV.W   R14, 0(R1)		; color
	MOV.B   #0, 5(R1)		; retVal
IF_1:	
	CMP.W   &BLUE, 0(R1)		; cmp BLUE with color
	JNE     IF_2
	MOV.B   #10, R12
	CMP.B   2(R1), R12		; cmp row with 10
	JLO	IF_2
	MOV.B   #1, 5(R1)		; retVal to 1 (winner)
IF_2:	
	CMP.W   &GREEN, 0(R1)		; cmp GREEN with color
	JNE     IF_3
	MOV.B   #10, R12
	CMP.B   3(R1), R12		; cmp col with 10
	JLO     IF_3
	MOV.B   #1, 5(R1)		; retVal to 1 (winner)
IF_3:	
	CMP.W   &RED, 0(R1)		; cmp RED with color
	JNE     IF_4
	MOV.B   2(R1), R12		; row into r12
	MOV.W   R12, R13		; r12 into r13 as word
	ADD.W   #20, R13		; add row + 20
	MOV.B   &HEIGHT, R12		; height into r12
	ADD.W   #-10, R12		; add height + -10
	CMP.W   R12, R13		; cmp (height - 10) and (row + 20)
	JL	IF_4
	MOV.B   #1, 5(R1)		; retVal to 1 (winner)
IF_4:
	CMP.W   &YELLOW, 0(R1)		; cmp YELLOW with color
	JNE     OUT
	MOV.B   3(R1), R12		; col into r12
	MOV.W   R12, R13		; r12 into r13 as word
	ADD.W   #20, R13		; add col + 20
	MOV.B   &WIDTH, R12		; width into r12
	ADD.W   #-10, R12		; add width + -10
	CMP.W   R12, R13		; cmp (width - 10) and (col + 20)
	JL 	OUT
	MOV.B   #1, 5(R1)		; retVal to 1 (winner)
OUT:
	MOV.B   5(R1), R12		; retVal for return into r12
	ADD.W   #6, R1			; restore stack pointer
	RET
