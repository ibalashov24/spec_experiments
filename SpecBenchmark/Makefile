KERNEL="product_all"
LAUNCHER_SOURCE="launcher.cpp"

LDARGS=-lbenchmark -lpthread 

all:
	impala $(KERNEL).impala -O2 -emit-llvm
	llvm-as $(KERNEL).ll
	clang++ $(KERNEL).bc $(LAUNCHER_SOURCE) -o launch $(LDARGS)

clear:
	rm $(KERNEL).ll $(KERNEL).bc launch

