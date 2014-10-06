all:
	g++ -c allocator.cpp
	g++ -c btree.cpp
	g++ -c mydb.c
	g++ main.cpp allocator.o btree.o mydb.o -o mydb

