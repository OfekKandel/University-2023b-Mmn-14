; l2
L@bl: prn #-5
; l3
prn: prn #-5
; l4
LABEL: .define a = 5
; l5
.define
; l6
.define a 5
; l7
.define a = 5 hello
; l8
LABEL: 
; l9
fmd 323
; l10
mov ,r2
; l11
mov r1 r2
; l12
mov r1,
; l13
mov r1, r2, 32
; l14
.declare a = 5
; l23
mov #noconst, r2
; l24
LABEL: .data 5
mov #LABEL, r2
; l25
.data
; l26
.data ,
; l27
.data 5 6
; l28
.data 5,
; l29
LABEL: .data 6
; l31
.string
; l32
.string hello"
; l33
.string "hello
; l34
.string "hello" letsgo
; l35
.extern one, two
; l36
.mesay balh
; l37
.define const = 5
.define const = 6
; l40
mov r1
; l41
hlt r1,r2
; l42
prn #-5, #-6
; l43
lea Str[1], r2
; l44
lea LABEL, #-5
; l45
lea r1, r2
; l47
.entry symbol
.extern symbol
; l49
.entry nolabel
; l64
.data 5@
; l65
.data 5+3
; l66
prn arr[]
; l67
prn arr[1
; l68
prn arr[1]f

