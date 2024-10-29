.model FLAT

;
;
;
; Shared Constants
;
;

PUBLIC flt_half         ; 0.5f
PUBLIC flt_one_and_half ; 1.5f
PUBLIC dbl_one          ; 1.0

EXTRN	__fltused:NEAR

CONST SEGMENT
flt_half         DD 03f000000r         ; 0.5f
flt_one_and_half DD 03fc00000r         ; 1.5f
dbl_one          DQ 03ff0000000000000r ; 1.0
CONST ENDS


;
;
; VS6 clib
;
;

PUBLIC	_Q_rsqrt_VS6_clib
_TEXT	SEGMENT
_number$ = 8
_Q_rsqrt_VS6_clib PROC NEAR
	fld	DWORD PTR _number$[esp-4]
	fsqrt
	fdivr	QWORD PTR dbl_one
	ret	0
_Q_rsqrt_VS6_clib ENDP
_TEXT	ENDS

;
;
; VS6 newton
;
;

PUBLIC	_Q_rsqrt_VS6_newton
_TEXT	SEGMENT
_number$ = 8
_i$ = -4
_y$ = -4
_Q_rsqrt_VS6_newton PROC NEAR
	push	ecx
	mov	eax, DWORD PTR _number$[esp]
	mov	edx, 1597463007				; 5f3759dfH
	fld	DWORD PTR _number$[esp]
	fmul	DWORD PTR flt_half
	mov	ecx, eax
	mov	DWORD PTR _y$[esp+4], eax
	sar	ecx, 1
	sub	edx, ecx
	mov	DWORD PTR _i$[esp+4], edx
	fmul	DWORD PTR _i$[esp+4]
	fmul	DWORD PTR _i$[esp+4]
	fsubr	DWORD PTR flt_one_and_half
	fmul	DWORD PTR _i$[esp+4]
	pop	ecx
	ret	0
_Q_rsqrt_VS6_newton ENDP
_TEXT	ENDS

;
;
; VS2003 clib
;
;

PUBLIC	_Q_rsqrt_VS2003_clib
_TEXT	SEGMENT
_number$ = 8						; size = 4
_Q_rsqrt_VS2003_clib PROC NEAR
	fld	DWORD PTR _number$[esp-4]
	fsqrt
	fdivr	QWORD PTR dbl_one
	ret	0
_Q_rsqrt_VS2003_clib ENDP
_TEXT	ENDS

;
;
; VS2003 newton
;
;

PUBLIC	_Q_rsqrt_VS2003_newton
_TEXT	SEGMENT
_i$ = -4						; size = 4
_y$ = -4						; size = 4
_number$ = 8						; size = 4
_Q_rsqrt_VS2003_newton PROC NEAR
	push	ecx
	mov	eax, DWORD PTR _number$[esp]
	fld	DWORD PTR _number$[esp]
	fmul	DWORD PTR flt_half
	mov	ecx, eax
	sar	ecx, 1
	mov	edx, 1597463007				; 5f3759dfH
	sub	edx, ecx
	mov	DWORD PTR _i$[esp+4], eax
	mov	DWORD PTR _y$[esp+4], edx
	fmul	DWORD PTR _y$[esp+4]
	fmul	DWORD PTR _y$[esp+4]
	fsubr	DWORD PTR flt_one_and_half
	fmul	DWORD PTR _y$[esp+4]
	pop	ecx
	ret	0
_Q_rsqrt_VS2003_newton ENDP
_TEXT	ENDS

END
