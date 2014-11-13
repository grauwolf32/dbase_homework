all:
	g++ -c allocator.cpp -g -ggdb
	g++ -c btree.cpp -g -ggdb
	g++ -c mydb.cpp -g -ggdb
	g++ -c cache.cpp -g -ggdb
	g++ main.cpp allocator.o btree.o mydb.o -o mydb -g -ggdb -O0

