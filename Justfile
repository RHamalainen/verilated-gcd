[private]
default:
	@just --list --unsorted

clean:
	rm -rf obj_dir

generate:
	python3 create_testcases.py

build:
	verilator -cc --exe top.v sim_main.cpp
	make -j -C obj_dir -f Vtop.mk

run: build
	./obj_dir/Vtop

trace:
	verilator -cc --exe --trace top.v sim_main.cpp
	make -j -C obj_dir -f Vtop.mk
	./obj_dir/Vtop +trace

yosys:
	yosys -s yosys.txt
