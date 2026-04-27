# Изоляция на отдельное ядро

```
sudo nice -n -20 taskset -c 3 ./hash
```

![htop_1](scrins/htop_1.png)

![htop_2](scrins/htop_2.png)


```
cnt input words: 1165861
cnt search words: 5131469 * 10 = 51314690
```


# base_O0

## Результаты

1: $22912489347$ \
2: $23097589969$ \
3: $23165119503$

Avg: $23058399606$


# base_O3

## Результаты

1: $17486060163$ \
2: $17505038429$ \
3: $17706989503$

Avg: $17566029365$

Ускорение: на $31.27$ процентов

![base_t](scrins/base3_t.png)

![base](scrins/base3.png)

![base_0](scrins/base3_0.png)

![base_1](scrins/base3_1.png)

![base_2](scrins/base3_2.png)

![base_3](scrins/base3_3.png)

![base_4](scrins/base3_4.png)

![base_5](scrins/base3_5.png)

Исходя из этого можно сделать вывод, что много времени тратится на цикл:
```
while (*key != '\0')
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ (uint8_t)*key++) & 0xFF];
```

![base_6](scrins/base3_6.png)



Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 35.03        | 154                   | 
HashCrc32            | 34.40        | 63                    | 
Strcmp               | 13.48        | 20                    | 
Strlen               | -            | -                     | 



# crc32


Заметим, что функции ChainSearch и HashCrc32 сравнимы по времени работы, поэтому будем оптимизировать второе

Выше мы заметили, что больше всего времени занимает цикл:
```
while (*key != '\0')
    crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ (uint8_t)*key++) & 0xFF];
```

В годболте:
```
mov     ecx, eax
shr     eax, 24
add     rdi, 1
xor     edx, eax
sal     ecx, 8
movzx   edx, dl
mov     eax, DWORD PTR crc32_table[0+rdx*4]
movzx   edx, BYTE PTR [rdi]
xor     eax, ecx
test    dl, dl
jne     .L5
```

У меня все слова по длине меньше 64 символов, а также выровнены по границе 32 байт \
Если длина слова меньше 64, то оставшиеся байты заполняются нулями
Для оптимизации я использовала интрисики:
```
#define HASH_CRC32_64_BITS(num)             \
    hash = _mm_crc32_u64(hash, data[num]);

...

unsigned long long hash = 0xFFFFFFFF;
int len = strlen(str);
const unsigned long long* data = (const unsigned long long*)str;

HASH_CRC32_64_BITS(0);
HASH_CRC32_64_BITS(1);
HASH_CRC32_64_BITS(2);
HASH_CRC32_64_BITS(3);

if (len > 31) {
    HASH_CRC32_64_BITS(4);
    HASH_CRC32_64_BITS(5);
    HASH_CRC32_64_BITS(6);
    HASH_CRC32_64_BITS(7);
}
```

## Результаты

1: $15588929844$ \
2: $15569286885$ \
3: $15649297992$

Avg: $15602504907$

Ускорение: на $12.58$ процента

![crc32_t](scrins/crc32_t.png)

![crc32](scrins/crc32.png)

![crc32_0](scrins/crc32_0.png)

![crc32_1](scrins/crc32_1.png)

![crc32_2](scrins/crc32_2.png)

![crc32_3](scrins/crc32_3.png)

![crc32_4](scrins/crc32_4.png)

![crc32_5](scrins/crc32_5.png)

My HashCrc32:

![crc32_6](scrins/crc32_6.png)

![crc32_7](scrins/crc32_7.png)


Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 40.71        | 128                   | 
HashCrc32            | 15.12        | 38                    | 
Strcmp               | 15.59        | 20                    | 
Strlen               | 8.22         | 14                    | 


# strcmp

По-хорошему теперь нужно оптимизировать ChainSearch, вот такой код выдал годболт:

1. 
```
unsigned int index = hash_table->hash_func(key->point, hash_table->capacity) % (unsigned int) hash_table->capacity;
```
превратилось в:
```
mov     esi, DWORD PTR [rdi+12]
mov     rdi, QWORD PTR [r12+8]
call    [QWORD PTR [rbx+24]]
```

2. 
```
chain_node_t* cur_node = hash_table->table[index];
```
превратилось в:
```
xor     edx, edx
div     DWORD PTR [rbx+12]
mov     rax, QWORD PTR [rbx]
mov     rbx, QWORD PTR [rax+rdx*8]
```

3. 
```
while (cur_node != NULL) {
    if (cur_node->key->size == key->size && strcmp(cur_node->key->point, key->point) == 0) 
        return true;

    cur_node = cur_node->next;
}
```
превратилось в:
```
        mov     ebp, DWORD PTR [r12]
        jmp     .L12
.L11:
        mov     rbx, QWORD PTR [rbx+8]
        test    rbx, rbx
        je      .L13
.L12:
        mov     rax, QWORD PTR [rbx]
        cmp     DWORD PTR [rax], ebp
        jne     .L11
        mov     rsi, QWORD PTR [r12+8]
        mov     rdi, QWORD PTR [rax+8]
        call    strcmp
        test    eax, eax
        jne     .L11
```
Сделать лучше я не смогу. Аналогично про HashCrc32 \
Поэтому будем оптимизировать strcmp


У меня все строки длиной < 64 символов, а так выровнены по границе 32 байт, поэтому можно вот так оптимизировать:
```
asm volatile (
    ".intel_syntax noprefix           \n\t"
    
    "mov al, [%[s1]]                  \n\t"     // check first bytes
    "cmp al, [%[s2]]                  \n\t"
    "jne 2f                           \n\t"

    "vmovdqa ymm0, [%[s1]]            \n\t"     // mov ymm0, 32 bytes of str1
    "vpcmpeqb ymm0, ymm0, [%[s2]]     \n\t"     // ymm0 = compare of ymm0 and 32 bytes of str2
    "vpmovmskb eax, ymm0              \n\t"     // make mask
    
    "xor %[res], %[res]               \n\t" 
    "cmp eax, -1                      \n\t"     // cmp with FFFFFFFF (equal)
    "jne 2f                           \n\t" 

    "vmovdqa ymm1, [%[s1] + 32]       \n\t"
    "vpcmpeqb ymm1, ymm1, [%[s2] + 32]\n\t"
    "vpmovmskb eax, ymm1              \n\t"
    
    "cmp eax, -1                      \n\t"
    "je 1f                            \n\t"

    "2:                               \n\t" 
    "mov %[res], 1                    \n\t" 

    "1:                               \n\t"
    "vzeroupper                       \n\t"
    ".att_syntax                      \n\t"

    : [res] "=&r" (result)                      // exit args (& uniq) (= only write)                            
    : [s1]  "r"   (str1),                       // enter args (r any reg)                        
      [s2]  "r"   (str2)
    : "ymm0", "rax", "cc", "memory"             // destroyed
);
```

## Результаты

1: $15414191522$ \
2: $15432695090$ \
3: $15511273544$

Avg: $15452720052$

Ускорение: на $0.97$ процента

![strcmp_t](scrins/strcmp_t.png)

![strcmp](scrins/strcmp.png)

![strcmp_0](scrins/strcmp_0.png)

![strcmp_1](scrins/strcmp_1.png)

![strcmp_2](scrins/strcmp_2.png)

Inline my strcmp:

![strcmp_3](scrins/strcmp_3.png)

![strcmp_4](scrins/strcmp_4.png)

![strcmp_5](scrins/strcmp_5.png)

![strcmp_6](scrins/strcmp_6.png)


Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 54.21        | 121                   | 
HashCrc32            | 15.83        | 38                    | 
Strcmp               | -            | -                     | 
Strlen               | 9.23         | 14                    | 



# strlen

Остается оптимизировать strlen, я это сделаю путем написания кода в отдельном файле my_strlen.s \
Опять же мы знаем, что длины всех слов не превышают 64:
```
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
```

## Результаты

1: $15352624454$ \
2: $15369602398$ \
3: $15487699347$

Avg: $15403308733$

Ускорение: на $0.32$ процента

![strlen_t](scrins/strlen_t.png)

![strlen](scrins/strlen.png)

![strlen_0](scrins/strlen_0.png)

![strlen_1](scrins/strlen_1.png)

![strlen_2](scrins/strlen_2.png)

![strlen_3](scrins/strlen_3.png)

![strlen_4](scrins/strlen_4.png)

My_strlen:

![strlen_5](scrins/strlen_5.png)

![strlen_6](scrins/strlen_6.png)


Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 56.62        | 114                   | 
HashCrc32            | 15.90        | 31                    | 
Strcmp               | -            | -                     | 
Strlen               | 5.53         | 8                     | 


# pgo

## Результаты

1: $15724697484$ \
2: $15721374364$ \
3: $15706433244$

Avg: $15717501697$

Ускорение: на $11.76$ процента

![pgo](scrins/pgo.png)

![pgo_0](scrins/pgo_0.png)

![pgo_1](scrins/pgo_1.png)

![pgo_2](scrins/pgo_2.png)


Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 65.93        | 141                   | 
HashCrc32            | -            | -                     | 
Strlen               | -            | -                     | 
Strcmp               | 14.58        | 20                    | 



# Таблица результатов


Test Name            | Average         | StdDev          | RelError  
---------------------|-----------------|-----------------|-----------
base_O0              | 2.31e+10        | 1.07e+08        | 0.46%
base_O3              | 1.76e+10        | 1.00e+08        | 0.57%
crc32                | 1.56e+10        | 3.40e+07        | 0.22%
strlen               | 1.54e+10        | 6.01e+07        | 0.39%
strcmp               | 1.55e+10        | 4.21e+07        | 0.27%
pgo                  | 1.57e+10        | 7.94e+06        | 0.05%

![graphic](graphic.png)