section .text
global My_strlen

My_strlen:
    vpxor ymm0, ymm0, ymm0       
    
    ; check first block
    vpcmpeqb ymm1, ymm0, [rdi]      ; compare first 32 bytes with 0
    vpmovmskb eax, ymm1             ; make mask
    test eax, eax                   ; if (eax != 0) => find \0
    jnz .found_first
    
    ; check second block
    vpcmpeqb ymm1, ymm0, [rdi + 32] ; compare second 32 bytes with 0
    vpmovmskb eax, ymm1
    test eax, eax
    jnz .found_second

    vzeroupper                      ; null up part of ymm
    ret

.found_first:
    bsf eax, eax                    ; Bit Scan Forward: find index of first bit=1
    vzeroupper
    ret

.found_second:
    bsf eax, eax
    add rax, 32                     ; add 32 bytes because in second part   
    vzeroupper
    ret

section .note.GNU-stack noalloc noexec nowrite progbits