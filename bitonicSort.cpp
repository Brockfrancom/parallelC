#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <algorithm>
#include <math.h>

int * getList(int size){ // Create a list of given size
	int * list = new int[size];
	for(int i = 0; i < size; i++){
    list[i] = (i*7)%10;
	}
	return list;
}

int * printList(int * list, int size){
	for(int i=0;i<size;++i){
		std::cout<<list[i]<<" ";
	}
	std::cout<<std::endl;
  return 0;
}

// Cube function we did in class
int cube(int i, int sendData){
	int rank;
	int size;
  int dest;
  int recvData;
  int mask=1;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	mask = mask << i;
	dest = rank ^ mask;
	MPI_Send(&sendData,1,MPI_INT,dest,0,MPI_COMM_WORLD);
	MPI_Recv(&recvData,1,MPI_INT,dest,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	if(rank < dest){
		if(sendData<recvData) return sendData;
		return recvData;
	}
	if(sendData<recvData) return recvData;
	return sendData;
}

void printSorted(int data, int rank, int size){
	int *dArray = new int [size];
	MPI_Gather(&data,1,MPI_INT,dArray,1,MPI_INT,0,MPI_COMM_WORLD);
	if(rank==0){
		std::cout << "Sorted List: " << std::endl;
		printList(dArray, size);
	}
	return;
}

void bitonicSort(int size, int rank, int list_size){
	int * list = new int[list_size];
  int recv_data;

  //Process 0 creates the list and sends it to processes
	if(rank==0) {
		list = getList(list_size);
		std::cout << "Unsorted List: " << std::endl;
		printList(list, list_size);
		for(int i = 0; i < size; i++){
			MPI_Send(&list[i],1,MPI_INT,i,0,MPI_COMM_WORLD);
		}
	}
  // Each process recieves data and cubes it the number of times.
	MPI_Recv(&recv_data,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  //for(int j = log2(size)-1; j >= 0; --j){
    for(int i = log2(size)-1; i >= 0; i--){
  		recv_data = cube(i, recv_data);
  	}
    // for(int i = 0; i < log2(size)-1; ++i){
    //   recv_data = cube(i, recv_data);
    // }
  //}
	MPI_Send(&recv_data,1,MPI_INT,0,0,MPI_COMM_WORLD);
	printSorted(recv_data, rank, list_size);
}

int main(int argc, char** argv) {
  int size;
  int rank;
	MPI_Init(&argc, &argv);
	srand(time(NULL));
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //We can assume power of 2 processors
	bitonicSort(size, rank, size);

	MPI_Finalize();
	return 0;
}
