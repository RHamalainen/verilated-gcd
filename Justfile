[private]
default:
	@just --list --unsorted

clean:
	rm -rf obj_dir

build:
	verilator -cc --exe top.v sim_main.cpp
	make -j -C obj_dir -f Vtop.mk

run: build
	./obj_dir/Vtop
