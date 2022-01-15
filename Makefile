all:
	gcc -o ordering -O0 ordering.cpp -lpthread
	gcc -o fenced -DUSE_CPU_FENCE -O0 ordering.cpp -lpthread

clean:
	rm -rf ./ordering ./fenced