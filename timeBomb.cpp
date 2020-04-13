/*
Homework 2 timeBomb
Brock Francom, A02052161

Write an MPI program that simulates the game of Time Bomb.  One process
generates a time bomb with an initially random countdown timer.  The bomb
is passed to another randomly-selected process.  That process decrements
the timer, and passes it on.  Play continues until the timer reaches zero,
at which point the bomb "explodes". That process is declared The Loser.
Then the game ends and all processes terminate.  There is no winner in the
game; only a loser.
*/
#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MCW MPI_COMM_WORLD // MPI_COMM_WORLD = default set of processors.
using namespace std;

int main(int argc, char **argv) {
  int rank, size;
  int data;
  int timeBomb;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank); // Gets the rank that was already assigned to a processor.
  MPI_Comm_size(MCW, &size);

  if (rank == 0){
    srand (time(NULL)); //Initialize random seed.
    timeBomb = rand()%100; //Set the timer on the bomb
    //timeBomb = 10;
    int process = rand()%size; //Pick next process
    cout<<"I am process "<<rank<<" of "<<size<<"; Set bomb timer: "<<timeBomb<< "; Passing to process "<<process<<endl;
    MPI_Send(&timeBomb, 1, MPI_INT, process,0,MCW); //Send bomb
  }

  while(timeBomb >= 0){
    MPI_Recv(&timeBomb,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);
    if(timeBomb < 0) {
      break;
    }
    //Uncommenting this line makes all messages appear in order. 
    //usleep(1); //sleep one microsecond
    if(timeBomb != 0){
      int process = rand()%size; //Pick next process
      cout<<"I am process "<<rank<<" of "<<size<<"; Timer: "<<timeBomb<< "; Passing to process "<<process<<endl;
      timeBomb--; //Decrement bomb
      MPI_Send(&timeBomb, 1, MPI_INT, process,0,MCW); //Send bomb
    }
    else{
      cout<<"I am process "<<rank<<" of "<<size<<"; Timer: "<<timeBomb<< "; I am dead."<< endl;
      timeBomb = -1;
      for(int i=0; i<size; i++) {
        if (i != rank){
          MPI_Send(&timeBomb, 1, MPI_INT, i,0,MCW); //Send kill code to all processes.
        }
      }
    }
  }

  MPI_Finalize();
  return 0;
}
