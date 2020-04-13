/*
The Game of Pit
    Jordan Knudsen
    Brock Francom

    
This file will contain a program that will "play" a game of pit. The master
process will start the game, by giving each process a list of cards. Then
then each process will enter a while loop, trading cards with other process
untill they have all cards needed to win.

The rules and gameplay have been slightly modified in this implemetation of the
game for simplicity.

Rules:
-The size of the deck will vary depending on the number of players in the game.
-The deck will consist of one complete suit (9 cards) per player.
-Once you have determined your deck size, shuffle the deck and deal the entire
deck out to the players.
-When trading begins, players may trade any number of cards to any player
 willing to accept the trade.
-Players do not take turns.
-Everyone trades simultaneously by yelling things like, “I’ve got two,” or “Four! Four!”
-All of the cards in the trade you are offering must be of the same commodity.
-Trades must be for an equal number of cards.
-Play continues in this manner until someone corners the market by obtaining
all nine cards in a commodity set.
-When a player corners the market, they announce it by identifying which
commodity they have cornered and revealing their cards.

Dan's request in his approval email:
Will you have each player count it’s request at the same time it b-casts it?
It would be hilarious if there were several levels of desperation.

Does anybody have any barley?
I’m looking for barley.
Barley!  Who has barley?
WHERE IS THE **&*&^&* BARLEY?!?!?  GIVE IT TO ME!!!!
*/
#include <iostream>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;

vector<int> cards;
vector<int> cardsToTrade;
vector<int> cardsRecievedInTrade;
bool verbose = false;

void printArray(vector<int> array, int rank){
  cout<<"Process: "<<rank<<", Cards: ";
  for(int i = 0;i<array.size();i++){
    cout << cards[i] << ", ";
  }
  cout << endl;
}

bool checkWin(int rank, int size, MPI_Request req){
  if(verbose){
    printArray(cards, rank);
  }
  int first = cards[0];
  bool win = true;
  for (int i = 1; i < cards.size(); i++){
    if(cards[i] != first){
      win = false;
    }
  }
  if(win){
    int winner = rank;
    cout<<"Process "<<rank<<" is the winner."<<endl;
    for(int i = 0;i<size;i++){
      MPI_Isend(&winner, 1, MPI_INT, i, 1, MPI_COMM_WORLD,&req);
      MPI_Request_free(&req);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  return win;
}

struct suitcount{
  int suit;
  int count;
};
bool countcompare(suitcount a, suitcount b) { return a.count > b.count; }

void addonetosuit(vector<suitcount> &suitcounts, int suit){
  bool suitfound = false;
  for(int i = 0;i<suitcounts.size();i++){
    if(suitcounts[i].suit == suit){
      suitcounts[i].count++;
      suitfound = true;
    }
  }
  if(!suitfound){
    suitcounts.push_back((suitcount){suit,1});
  }
}


void getCardsToTrade(int size,int rank){
  
  //cout up each suit, to find the one we have the most of, and to choose what to trade.
  vector<suitcount> suitcounts;

  for(int i = 0;i<9;i++){
    addonetosuit(suitcounts,cards[i]);
  }
  sort(suitcounts.begin(), suitcounts.end(), countcompare);

  int mostCommon = suitcounts[0].suit;

  //choose what suit to trade based off of whats in suitcounts
  int indexToTrade = rand()%(suitcounts.size()-1)+1;
  int suitToTrade = suitcounts[indexToTrade].suit;
  //choose how many cards of that suit to trade
  int numToTrade = rand()%suitcounts[indexToTrade].count+1;
  
  //add cards to trade to the vector, remove them from our hand.
  cardsToTrade.erase(cardsToTrade.begin(),cardsToTrade.end());
  for (int i=cards.size()-1;i>-1;i--){
    if(cards[i] == suitToTrade){
      cardsToTrade.push_back(cards[i]);
      cards.erase(cards.begin()+i);
      numToTrade--;
      if(numToTrade == 0){
        break;
      }
    }
  }
  if(verbose){
    cout << "rank " << rank << ": suits: ";
    for(int i = 0;i<suitcounts.size();i++){
      cout << suitcounts[i].suit << ": " << suitcounts[i].count << ", ";
    }
    cout << endl;
    cout << "rank " << rank << ": trade: " << suitToTrade << " cards: ";
    for(int i = 0;i<cardsToTrade.size();i++){
      cout << cardsToTrade[i] << ", ";
    }
    cout << endl;
  }
}

void trade(int size, int rank){
  MPI_Request req;
  int sizeofcardstotrade = cardsToTrade.size();
  if(verbose){
    cout<<"Process "<<rank<<" has "<<sizeofcardstotrade<<" cards to trade: ";
    for(int i = 0;i<cardsToTrade.size();i++){
      cout << cardsToTrade[i] << ", ";
    }
    cout << endl;
  }

  //Get amount of cards to trade from all processes
  int trades[size];
  for(int i = 0;i<size;i++){
    MPI_Gather(&sizeofcardstotrade,1,MPI_INT,&trades,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&trades,size,MPI_INT,0,MPI_COMM_WORLD);
  }
  if(verbose){
    cout<<"Process "<<rank<<" number of cards others want to trade: ";
    for(int i = 0;i<size;i++){
      cout << trades[i] << ", ";
    }
    cout << endl;
  }

  //Select all possible processes you can trade with
  vector<int> validtrades;
  for(int i = 0;i<size;i++){
    if(trades[i] == sizeofcardstotrade){
      validtrades.push_back(i);
    }
  }

  //Decide who trades with who
  // 2,3,7   5 cards
  // 2:2,3:7,7:3
  if(validtrades[0] == rank){
    random_shuffle(validtrades.begin(), validtrades.end());
    int i = 0;
    for(i = 0;i<validtrades.size()-1;i+=2){
      MPI_Send(&(validtrades[i+1]),1,MPI_INT,validtrades[i],2,MPI_COMM_WORLD);
      MPI_Send(&(validtrades[i]),1,MPI_INT,validtrades[i+1],2,MPI_COMM_WORLD);
    }
    if(i<validtrades.size()){
      MPI_Send(&(validtrades[i]),1,MPI_INT,validtrades[i],2,MPI_COMM_WORLD);
    }
  }

  //Execute the trade
  int tradewith;
  MPI_Recv(&tradewith,1,MPI_INT,MPI_ANY_SOURCE,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  MPI_Isend(&cardsToTrade[0], sizeofcardstotrade, MPI_INT, tradewith, 0, MPI_COMM_WORLD,&req);
  MPI_Request_free(&req);
  cardsRecievedInTrade.resize(sizeofcardstotrade);
  MPI_Recv(&cardsRecievedInTrade[0],sizeofcardstotrade,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

  //add recieved cards to the rest
  for(int i=0;i<sizeofcardstotrade;i++){
    cards.push_back(cardsRecievedInTrade[i]);
  }
  cards.resize(9);
  if(tradewith != rank){
    cout << "I am " << rank << " and I traded ";
    for(int i = 0;i<cardsToTrade.size();i++){
      cout << cardsToTrade[i] << " ";
    }
    cout <<" with " << tradewith << " for ";
    for(int i = 0;i<cardsRecievedInTrade.size();i++){
      cout << cardsRecievedInTrade[i] << ", ";
    }
    cout << endl;
  }
}

void playPit(int rank, int size){
  MPI_Request req;
  //Recieve initial cards
  int cardsSize;
  MPI_Recv(&cardsSize,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  cards.resize(cardsSize);
  MPI_Recv(&cards[0],cardsSize,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

  while(true){
    //Check if process wins with the current cards
    if(checkWin(rank, size, req)) break;

    //Tell all the other processes you won.
    MPI_Barrier(MPI_COMM_WORLD);
    int flag = 0;
    MPI_Iprobe(MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&flag,MPI_STATUS_IGNORE);
    if(flag){
      //There is a winner
      int winner;
      MPI_Recv(&winner,1,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
      break;
    }

    getCardsToTrade(size,rank);
    trade(size,rank);

    if(verbose){
      printArray(cards, rank);
    }
  }
  sleep(1);
  //print your final cards
  printArray(cards, rank);
}

int main(int argc, char** argv) {
  int world_size;
  int world_rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  srand(time(NULL)+world_rank); //Initialize random seed (different seed for each process)

  //create deck and send hands to each process.
  //9 cards per process, card is the rank, ie. nine "0" cards, nine "1" cards, etc.
  if (world_rank == 0){
    vector<int> deck;
    for(int process = 0; process < world_size; process++){
      for(int cardNum = 0; cardNum < 9; cardNum++){
        deck.push_back(process);
      }
  	}
    random_shuffle(deck.begin(), deck.end());
    //give 9 cards of deck to each process
    for(int process = 0; process < world_size; process++){
      std::vector<int> hand;
      for(int cardNum = 0; cardNum < 9; cardNum++){
        hand.push_back(deck.back());
        deck.pop_back();
      }
      int tosend_size = hand.size();
      MPI_Send(&tosend_size, 1, MPI_INT, process, 0, MPI_COMM_WORLD);
      MPI_Send(&hand[0],tosend_size,MPI_INT,process,0,MPI_COMM_WORLD);
  	}
  }

	playPit(world_rank, world_size);

  MPI_Finalize();
	return 0;
}
