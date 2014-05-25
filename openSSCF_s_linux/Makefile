all: NLP_algorithm.o combinition.o libClassifier.o sqlite3.o TC_process.o theTree.o
	gcc -lstdc++ NLP_algorithm.o combinition.o libClassifier.o sqlite3.o TC_process.o theTree.o -lpthread -ldl -o openSSCF_server
theTree: theTree.cpp theTree.h
	gcc -lstdc++ -c theTree.cpp
TC_process: TC_process.cpp TC_process.h
	gcc -lstdc++ -c TC_process.cpp
libClassifier: libClassifier.cpp libClassifier.h
	gcc -lstdc++ -c libClassifier.cpp
sqlite3: sqlite3.c sqlite3.h
	gcc -c sqlite3.c
combinition: combinition.cpp combinition.h
	gcc -lstdc++ -c combinition.cpp
clean:
	rm -f openSSCF_server
