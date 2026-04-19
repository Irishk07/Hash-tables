CXX = g++

CPPFLAGS := -DNDEBUG -ggdb3 -std=c++17 -march=native -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat \
-Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy \
-Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op \
-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow \
-Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn \
-Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef \
-Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers \
-Wno-narrowing -Wno-old-style-cast -Wno-varargs -fcheck-new -fsized-deallocation -fstack-protector \
-fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=90000 -Wstack-usage=81920 -pie -fPIE -Werror=vla

SANITIZER_FLAGS := -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,$\
null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

#CPPFLAGS += $(SANITIZER_FLAGS)

AS = nasm
ASFLAGS = -f elf64 -g
ASSRC = my_strlen.s
ASOBJ = $(ASSRC:%.s=build/%.o)

HTSRC ?= ht.cpp
CPPSRC = main.cpp read.cpp $(HTSRC)
CPPOBJ := $(CPPSRC:%.cpp=build/%.o) 

HEADER_DEPENDS := $(CPPOBJ:%.o=%.d)

.PHONY: all
all: hash

.PHONY: base_O0
base_O0: 
	@$(MAKE) hash HTSRC=ht.cpp CPPFLAGS="$(CPPFLAGS) -O0 -DOUT_FILE='\"times/base_O5.txt\"'"

.PHONY: base_O3
base_O3: 
	@$(MAKE) hash HTSRC=ht.cpp CPPFLAGS="$(CPPFLAGS) -O3 -DOUT_FILE='\"times/base_O3.txt\"'"

.PHONY: crc32
crc32: 
	@$(MAKE) hash HTSRC=ht_crc32.cpp CPPFLAGS="$(CPPFLAGS) -O3 -DOUT_FILE='\"times/crc32.txt\"'"

.PHONY: strcmp
strcmp: 
	@$(MAKE) hash HTSRC=ht_strcmp.cpp CPPFLAGS="$(CPPFLAGS) -O3 -DOUT_FILE='\"times/strcmp.txt\"'"
	
.PHONY: strlen
strlen: 
	@$(MAKE) hash HTSRC=ht_strlen.cpp CPPFLAGS="$(CPPFLAGS) -O3 -DOUT_FILE='\"times/strlen.txt\"'"

.PHONY: valgrind
valgrind: 
	@$(MAKE) hash HTSRC=$(HTSRC) CPPFLAGS="$(CPPFLAGS) -g -O3 -DVALGRIND -mno-avx512f"
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./hash

.PHONY: pgo
pgo: 
	@mkdir -p pgo_data
	@$(CXX) $(CPPFLAGS) -DOUT_FILE='"times/pgo.txt"' -O3 -mavx2 -fprofile-generate=pgo_data ht.cpp main.cpp read.cpp -o hash
	./hash
	@$(CXX) $(CPPFLAGS) -DOUT_FILE='"times/pgo.txt"' -O3 -mavx2 -fprofile-use=pgo_data -fprofile-correction ht.cpp main.cpp read.cpp -o hash

build:
	mkdir -p build

$(CPPOBJ): build/%.o: %.cpp | build
	@$(CXX) $(CPPFLAGS) -MP -MMD -c $< -o $@

$(ASOBJ):  build/%.o: %.s | build
	@$(AS) $(ASFLAGS) $< -o $@

hash: $(CPPOBJ) $(ASOBJ)
	@$(CXX) $(CPPOBJ) $(ASOBJ) -o $@ $(CPPFLAGS)

-include $(HEADER_DEPENDS)

.PHONY: clean
clean:
	rm -f build/*.o
	rm -f build/*.d

.PHONY: pgo_clean
pgo_clean:
	rm -rf pgo_data
	rm -f *.gcda *.gcno