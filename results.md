# Изоляция на отдельное ядро

```
sudo nice -n -20 taskset -c 0 ./hash
```

![htop_1](scrins/htop_1.png)

![htop_2](scrins/htop_2.png)


```
cnt input words: 1165861
cnt search words: 5131469 * 10 = 51314690
```


# base_O0

1: $21644574432$ \
2: $21726231378$ \
3: $21748913046$

Avg: $21706572952$


# base_O3

1: $17630238186$ \
2: $17702316397$ \
3: $17794250043$

Avg: $17708934875$

Ускорение: на $22.57$ процентов

![base_t](scrins/base3_t.png)

![base](scrins/base3.png)

![base_0](scrins/base3_0.png)

![base_1](scrins/base3_1.png)

![base_6](scrins/base3_6.png)

![base_2](scrins/base3_2.png)

![base_3](scrins/base3_3.png)

![base_4](scrins/base3_4.png)

![base_5](scrins/base3_5.png)



Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 35.26        | 155                   | 
HashCrc32            | 34.06        | 63                    | 
Strlen               | -            | -                     | 
Strcmp               | 13.35        | 20                    | 


# crc32

1: $15468634388$ \
2: $15546548296$ \
3: $15598880794$

Avg: $15538021159$

Ускорение: на $13.97$ процента

![crc32_t](scrins/crc32_t.png)

![crc32](scrins/crc32.png)

![crc32_0](scrins/crc32_0.png)

![crc32_1](scrins/crc32_1.png)

![crc32_8](scrins/crc32_8.png)

![crc32_2](scrins/crc32_2.png)

![crc32_3](scrins/crc32_3.png)

![crc32_4](scrins/crc32_4.png)

![crc32_5](scrins/crc32_5.png)

![crc32_6](scrins/crc32_6.png)

![crc32_7](scrins/crc32_7.png)



Было:
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


Использовала интрисики:
```
size_t i = 0;
for (; i + 8 <= len; i += 8) {
    unsigned long long cur_ptr = *(const unsigned long long*)(str + i);
    hash = _mm_crc32_u64(hash, cur_ptr);
}
```

В годболте это преобразовалось в:
```
crc32   rcx, QWORD PTR [rbx-8+rax]
mov     rdx, rax
lea     rax, [rax+8]
cmp     rsi, rax
jnb     .L4
mov     eax, ecx
```


Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 38.41        | 147                   | 
HashCrc32            | 22.18        | 53                    | 
Strlen               | 7.84         | 14                    | 
Strcmp               | 13.44        | 20                    | 


# strlen

1: $15352624454$ \
2: $15369602398$ \
3: $15487699347$

Avg: $15403308733$

Ускорение: на $0.87$ процента

![strlen_t](scrins/strlen_t.png)

![strlen](scrins/strlen.png)

![strlen_0](scrins/strlen_0.png)

![strlen_1](scrins/strlen_1.png)

![strlen_8](scrins/strlen_8.png)

![strlen_2](scrins/strlen_2.png)

![strlen_3](scrins/strlen_3.png)

![strlen_4](scrins/strlen_4.png)

![strlen_5](scrins/strlen_5.png)

![strlen_6](scrins/strlen_6.png)

![strlen_7](scrins/strlen_7.png)


Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 38.96        | 137                   | 
HashCrc32            | 22.92        | 46                    | 
Strlen               | 4.75         | 8                     | 
Strcmp               | 14.24        | 20                    | 


# strcmp

1: $15300917415$ \
2: $15347362175$ \
3: $15456730300$

Avg: $15368336630$

Ускорение: на $0.23$ процента

![strcmp_t](scrins/strcmp_t.png)

![strcmp](scrins/strcmp.png)

![strcmp_0](scrins/strcmp_0.png)

![strcmp_1](scrins/strcmp_1.png)

![strcmp_8](scrins/strcmp_8.png)

![strcmp_2](scrins/strcmp_2.png)

![strcmp_3](scrins/strcmp_3.png)

![strcmp_4](scrins/strcmp_4.png)

![strcmp_5](scrins/strcmp_5.png)

![strcmp_6](scrins/strcmp_6.png)

![strcmp_7](scrins/strcmp_7.png)


Func Name            | Self (%)     | Instructions per call |  
---------------------|--------------|-----------------------|
ChainSearch          | 38.46        | 136                   | 
HashCrc32            | 23.00        | 46                    | 
Strlen               | 4.77         | 8                     | 
Strcmp               | 14.60        | 20                    | 


# pgo

1: $15724697484$ \
2: $15721374364$ \
3: $15706433244$

Avg: $15717501697$

Ускорение: на $12.67$ процента

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
base_O0              | 2.17e+10        | 4.48e+07        | 0.21%
base_O3              | 1.57e+10        | 2.87e+06        | 0.02%
crc32                | 1.55e+10        | 5.35e+07        | 0.34%
strlen               | 1.54e+10        | 6.01e+07        | 0.39%
strcmp               | 1.57e+10        | 7.94e+06        | 0.05%

![graphic](graphic.png)