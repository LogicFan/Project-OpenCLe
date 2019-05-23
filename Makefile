test: ./build/test/logger_test_1 ./build/test/logger_test_2 ./build/test/basic_test
	./build/test/logger_test_1
	./build/test/logger_test_2
	./build/test/basic_test

./build/test/logger_test_1: ./src/util/logger/logger_test.cpp ./build/test
	g++ -std=c++17 -g ./src/util/logger/logger_test.cpp -o ./build/test/logger_test_1
./build/test/logger_test_2: ./src/util/logger/logger_test.cpp ./build/test
	g++ -std=c++17 -g ./src/util/logger/logger_test.cpp -o ./build/test/logger_test_2 -D NDEBUG
./build/test/basic_test: ./src/sample.cpp
	g++ -std=c++17 -g ./src/sample.cpp -o ./build/test/basic_test -lOpenCL

./build/test: ./build
	mkdir ./build/test
./build:
	mkdir ./build

clean:
	rm -rf ./build

