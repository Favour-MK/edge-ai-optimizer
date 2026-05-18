global quantize_asm

section .text

; rdi  = input array pointer
; rsi  = output array pointer
; edx  = length
; xmm0 = scale
; ecx  = zero_point

quantize_asm:
    ; --- SETUP PHASE ---
    ; 1. Broadcast 'scale' to all 4 slots of xmm0
    ; SHUFPS shuffles the floats in a register. 0x00 tells it to copy the 0th float into all slots.
    shufps xmm0, xmm0, 0x00     ; xmm0 now equals [scale, scale, scale, scale]

    ; 2. Broadcast 'zero_point' to all 4 slots of a new register (xmm2)
    ; First, move the 32-bit integer from ecx into the lowest slot of xmm2
    movd xmm2, ecx              
    ; PSHUFD shuffles doublewords (integers). 0x00 copies the 0th int to all slots.
    pshufd xmm2, xmm2, 0x00     ; xmm2 now equals [zp, zp, zp, zp]

    xor eax, eax                ; i = 0

.loop_start:
    cmp eax, edx                
    jge .loop_end

    ; --- SIMD MATH PHASE ---
    ; 1. LOAD 4 FLOATS AT ONCE
    ; MOVUPS = Move Unaligned Packed Single-precision. Loads 16 bytes (4 floats).
    movups xmm1, [rdi + rax*4]

    ; 2. DIVIDE 4 FLOATS AT ONCE
    ; DIVPS = Divide Packed Single-precision.
    divps xmm1, xmm0

    ; 3. ROUND 4 FLOATS TO 4 INTEGERS
    ; CVTTPS2DQ = Convert with Truncation Packed Single to Packed Doubleword (32-bit) Ints
    cvtps2dq xmm1, xmm1

    ; 4. ADD ZERO-POINT TO ALL 4 INTEGERS
    ; PADDD = Packed Add Doubleword
    paddd xmm1, xmm2

    ; --- EXTRACTION PHASE ---
    ; Right now, xmm1 holds FOUR 32-bit integers. We need to store them as FOUR 8-bit integers.
    ; PEXTRB extracts a single byte from the XMM register and writes it to memory.
    ; Byte 0 is the bottom of the 1st int, Byte 4 is the bottom of the 2nd int, etc.
    pextrb byte [rsi + rax], xmm1, 0        ; Store 1st int8
    pextrb byte [rsi + rax + 1], xmm1, 4    ; Store 2nd int8
    pextrb byte [rsi + rax + 2], xmm1, 8    ; Store 3rd int8
    pextrb byte [rsi + rax + 3], xmm1, 12   ; Store 4th int8

    ; INCREMENT BY 4 (because we just processed 4 elements!)
    add eax, 4                 
    jmp .loop_start

.loop_end:
    ret