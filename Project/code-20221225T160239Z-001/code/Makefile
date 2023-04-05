build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out
