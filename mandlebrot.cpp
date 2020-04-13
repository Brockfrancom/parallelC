#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <algorithm>
#include <math.h>
#include <string>
#include <vector>
#include <chrono>
using namespace std::chrono;

//These control the size, definition and max iteration for the image.
const int MAX_ITERATION = 1000;
const int RESOLUTION = 512;
const bool INVERT_COLORS = false;
const std::string OUTPUT_FILE = "parallel_mandelbrot.ppm";

struct Color{
public:
	int r;
	int g;
	int b;

	Color(){r,g,b=0;}

	Color(int r, int g, int b){
		this->r = r;
		this->g = g;
		this->b = b;
	}
};

//This gets the correct color for each pixel
Color getColor(int iteration){
	int r = iteration%145;
	int g = iteration%200;
	int b = iteration%255;
	if(INVERT_COLORS){
		r = 255 - r;
		g = 255 - g;
		b = 255 - b;
	}
	return Color(r,g,b);
}

//Function to write to a file
void writeToFile(std::string message, std::ofstream &mandelbrot_file){
	mandelbrot_file << message;
}

//plots the image to the file.
void plotImage(std::vector<int> plot){
  std::ofstream mandelbrot_file;
  mandelbrot_file.open(OUTPUT_FILE);
	mandelbrot_file << "P3" << std::endl;
	mandelbrot_file << RESOLUTION << " " << RESOLUTION << std::endl;
	mandelbrot_file << "255" << std::endl;
	for(int i = 0; i < RESOLUTION; i++){
		for(int j = 0; j < RESOLUTION; j++){
			Color c = getColor(plot[i*RESOLUTION+j]);
      std::string color = std::to_string(c.r) + " " + std::to_string(c.g) + " " + std::to_string(c.b) + " ";
			writeToFile(color, mandelbrot_file);
		}
		writeToFile("\n", mandelbrot_file);
	}
	mandelbrot_file.close();
}

//This funciton is where the work is done for each pixel.
int calculatePixel(int px, int py){
  double x0 = -2 + px * (2.5/RESOLUTION);
	double y0 = -1.25 + py * (2.5/RESOLUTION);
	double x = 0.0;
	double y = 0.0;
	int iteration = 0;
	while(x*x + y*y < 4 && iteration < MAX_ITERATION){
		double xtemp = x*x - y*y + x0;
		y = 2*x*y + y0;
		x = xtemp;
		iteration += 1;
	}
	return iteration;
}

std::vector<int> initPlot(int size){
	std::vector<int> plot(size*RESOLUTION,0);
	return plot;
}

void mandelbrot(int world_size, int world_rank){
	int offset = RESOLUTION/world_size;
	std::vector<int> plot = initPlot(RESOLUTION);
	int start = world_rank*offset;
	for(int i = start; i < start+offset; i++){
		for(int j = 0; j < RESOLUTION; j++){
			plot[i*RESOLUTION+j] = calculatePixel(j, i);
		}
	}
	if(world_rank==0){
		std::vector<int> recv_data = initPlot(RESOLUTION);
		MPI_Gather(&plot[start*RESOLUTION],RESOLUTION*offset,MPI_INT,&recv_data.front(),RESOLUTION*offset,MPI_INT,0,MPI_COMM_WORLD);
		plotImage(recv_data);
	} else{
		MPI_Gather(&plot[start*RESOLUTION],RESOLUTION*offset,MPI_INT,NULL,RESOLUTION*offset,MPI_INT,0,MPI_COMM_WORLD);
	}
}

int main(int argc, char** argv) {
  auto start = high_resolution_clock::now();
	MPI_Init(&argc, &argv);
	srand(time(NULL));
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	mandelbrot(world_size, world_rank);
  if(!world_rank){
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout<< "Duration (microseconds): " << duration.count() << std::endl;
  }
	MPI_Finalize();
	return 0;
}
