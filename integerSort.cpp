/*
Homework 3 integer sort
Brock Francom, A02052161

Write an MPI program that performs an integer sort.  Make your program in the "master/slave" style. Have the master process send data segments to the slave processes, where the lists are sorted individually.  The lists should then be merged back together on the master process.
*/
#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <array>
#include <list>
#include <iterator>

#define MCW MPI_COMM_WORLD
using namespace std;
//name of array is pointer to the 0th element of array
void selectionSort(int a[], int n) {
   int i, j, min, temp;
   for (i = 0; i < n - 1; i++) {
      min = i;
      for (j = i + 1; j < n; j++)
         if (a[j] < a[min])
            min = j;
         temp = a[i];
         a[i] = a[min];
         a[min] = temp;
   }
}

int main(int argc, char **argv) {
  int rank, size;
  int arrayLength = 11;
  int data[arrayLength]; //unsorted array
  int sortedData[arrayLength*10]; //Final array
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);
  MPI_Status mystatus;

  if (rank == 0){ //I'm the master
    srand (time(NULL)); //Initialize random seed.
    //Create array
    int r = (arrayLength/(size-1))+1;
    for(int i=0;i<arrayLength;++i){
      data[i]=rand()%arrayLength;
    }
    //Print out array
    cout<<"Initial array: ";
    for(int i=0;i<arrayLength;++i){
      cout<<data[i]<<" ";
    }
    cout<<endl;
    //Send each slave a slice of the array.
    for(int j=1;j<size;++j){
      int temp[r];
      for(int i=0;i<=r;i++){
        int var = data[((j-1)*r)+i];
        if (((j-1)*r)+i < arrayLength){
          temp[i]=data[((j-1)*r)+i];
        }
        else{
          temp[i]=0;
        }
      }
      cout<<"I am master; Sending work to slave "<<j<<endl;
      MPI_Send(temp,r,MPI_INT,j,0,MCW);
    }
    int processes = size-1;
    while(processes > 0){
      //Recieve the sorted arrays and merge them together.
      MPI_Probe(MPI_ANY_SOURCE,0,MCW, &mystatus);
      int count;
      MPI_Get_count(&mystatus,MPI_INT,&count);
      int temp[count];
      MPI_Recv(temp,count,MPI_INT,MPI_ANY_SOURCE,0,MCW,&mystatus);
      int tempVar;
      for(int i=0;i<count;++i){
        for(int j=0;j<arrayLength;++j){
          if(sortedData[j] > temp[i]){
            continue;
          }
          else {
            tempVar = sortedData[j];
            sortedData[j]= temp[i];
            temp[i] = tempVar;
          }
        }
      }
      MPI_Send(temp,count,MPI_INT,mystatus.MPI_SOURCE,1,MCW);
      processes--;
    }
    //Print out the final sorted array.
    cout<<"Final array: ";
    for(int i=0;i<arrayLength;++i){
      cout<<sortedData[i]<<" ";
    }
    cout<<endl;
  }
  else{ //I'm a slave
    MPI_Probe(MPI_ANY_SOURCE,0,MCW, &mystatus);
    int count;
    MPI_Get_count(&mystatus,MPI_INT,&count);
    MPI_Recv(data,count,MPI_INT,MPI_ANY_SOURCE,0,MCW,&mystatus);
    cout<<"I am slave "<<rank<<"; Recieved work: ";
    for(int i=0;i<count;++i){
      cout<<data[i]<<" ";
    }
    cout<<endl;
    selectionSort(data, count);
    cout<<"I am slave "<<rank<<"; Finished work: ";
    for(int i=0;i<count;++i){
      cout<<data[i]<<" ";
    }
    cout<<endl;
    MPI_Send(data,count,MPI_INT,0,0,MCW);
  }
  MPI_Finalize();
  return 0;
}
