// gameoflife.c
// Name: Hossein Souri
// JHED: hsouri1

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mpi.h"

/* 
 * Automata rules for cells that survive or die
 */
int newValue ( int one, int two, int three, int four, int five, int six, int seven, int eight, int value)
{
  int sum = one + two + three + four + five + six + seven + eight;
  if (sum == 3)
  {
    return 1;
  }
  else if (sum == 2) 
  {
    return value;
  }
  else 
  {
    return 0;
  } 
}

#define DEFAULT_ITERATIONS 64
#define GRID_WIDTH  256
#define DIM  16     // assume a square grid

int main ( int argc, char** argv ) {

  int global_grid[ 256 ] =  
   {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

  // MPI Standard variable
  int num_procs;
  int ID, j;
  int iters = 0;
  int num_iterations;

  // Setup number of iterations
  if (argc == 1) {
    num_iterations = DEFAULT_ITERATIONS;
  }
  else if (argc == 2) {
    num_iterations = atoi(argv[1]);
  }
  else {
    printf("Usage: ./gameoflife <num_iterations>\n");
    exit(1);
  }

  // Messaging variables
  MPI_Status stat;
  // TODO add other variables as necessary
  /* Iteration variables */
  int i,k,col,row;
  int num_els;    //in local array
  int offset;     //local-> global
  int prev;
  int next; 
  int lower[DIM],upper[DIM],first_row[DIM],last_row[DIM]; //value from lower neighbor,upper neighbor,first row and last row 
  int * grid, * tmp_grid;

  // MPI Setup
  if ( MPI_Init( &argc, &argv ) != MPI_SUCCESS )
  {
    printf ( "MPI_Init error\n" );
  }

  MPI_Comm_size ( MPI_COMM_WORLD, &num_procs ); // Set the num_procs
  MPI_Comm_rank ( MPI_COMM_WORLD, &ID );

  assert ( DIM % num_procs == 0 );
  // TODO Setup your environment as necessary
  next =  ( ID + 1 ) % num_procs;
  prev = ID == 0 ? num_procs -1 : ID-1;
  grid = malloc ( sizeof(int) * GRID_WIDTH / num_procs );  
  tmp_grid = malloc ( sizeof(int) * GRID_WIDTH / num_procs );
  num_els = GRID_WIDTH / num_procs;
  offset = ID * num_els;
  for ( i=0; i < num_els; i++ )
  {
    grid[i] = global_grid[ i + offset ];
  }

  for ( iters = 0; iters < num_iterations; iters++ ) {
    // TODO: Add Code here or a function call to you MPI code
    for (col = 0; col < DIM; col++)
    {
      first_row[col] = grid[col];
      last_row[col] = grid[num_els - DIM + col];
    }

    if ( ID % 2 == 0 )
    {
      MPI_Ssend ( last_row, DIM, MPI_INT, next, 0, MPI_COMM_WORLD ); 
      MPI_Ssend ( first_row, DIM, MPI_INT, prev, 0, MPI_COMM_WORLD ); 
      MPI_Recv ( lower, DIM, MPI_INT, prev, 0, MPI_COMM_WORLD, &stat );
      MPI_Recv ( upper, DIM, MPI_INT, next, 0, MPI_COMM_WORLD, &stat );
    }
    else 
    { 
      MPI_Recv ( lower, DIM, MPI_INT, prev, 0, MPI_COMM_WORLD, &stat );
      MPI_Recv ( upper, DIM, MPI_INT, next, 0, MPI_COMM_WORLD, &stat );
      MPI_Ssend ( last_row, DIM, MPI_INT, next, 0, MPI_COMM_WORLD ); 
      MPI_Ssend ( first_row, DIM, MPI_INT, prev, 0, MPI_COMM_WORLD ); 
      
    }

    /* update the values */
    tmp_grid[0] = newValue(lower[DIM-1], lower[0], lower[1], grid[DIM-1], grid[1], grid[2 * DIM - 1], grid[DIM], grid[DIM + 1], grid[0]);
    for ( j=1; j< DIM-1; j++ )
    {
      tmp_grid[j] = newValue(lower[j-1], lower[j], lower[j+1], grid[j-1], grid[j+1], grid[DIM + j-1], grid[DIM + j], grid[DIM + j+1], grid[j]);
    }
    tmp_grid[DIM-1] = newValue(lower[DIM-2], lower[DIM-1], lower[0], grid[DIM-2], grid[0], grid[2*DIM - 2], grid[2*DIM - 1], grid[DIM], grid[DIM-1]);

    for (row = 1; row < num_els/DIM - 1; row++)
    {      
      tmp_grid[row*DIM] = newValue(grid[row*DIM-1], grid[(row-1)*DIM], grid[(row-1)*DIM + 1], grid[(row + 1)*DIM-1], grid[row*DIM+1], grid[(row + 2)*DIM-1], grid[(row+1)*DIM], grid[(row+1)*DIM+1], grid[row * DIM]);
      for ( j=1; j< DIM-1; j++ )
      {
        tmp_grid[row*DIM+j] = newValue(grid[(row-1)*DIM + j-1], grid[(row-1)*DIM+j], grid[(row-1)*DIM+j+1], grid[row*DIM+j-1], grid[row*DIM+j+1], grid[(row+1)*DIM + j-1], grid[(row+1)*DIM + j], grid[(row+1)*DIM + j+1], grid[row*DIM+j]);
      }
      tmp_grid[(row+1)*DIM-1] = newValue(grid[row*DIM-2], grid[row*DIM-1], grid[(row-1)*DIM], grid[(row+1)*DIM-2], grid[row*DIM], grid[(row+2)*DIM-2], grid[(row+2)*DIM-1], grid[(row+1)*DIM], grid[(row+1)*DIM-1]); 
    }

    tmp_grid[num_els-DIM] = newValue(grid[num_els-DIM-1], grid[num_els-2*DIM], grid[num_els-2*DIM+1], grid[num_els-1], grid[num_els-DIM+1], upper[DIM-1], upper[0], upper[1], grid[num_els-DIM]);
    for ( j=1; j< DIM-1; j++ )
    {
      tmp_grid[num_els-DIM+j] = newValue(grid[num_els-2*DIM+j-1], grid[num_els-2*DIM+j], grid[num_els-2*DIM+j+1], grid[num_els-DIM+j-1], grid[num_els-DIM+j+1], upper[j-1], upper[j], upper[j+1], grid[num_els-DIM+j]);
    }
    tmp_grid[num_els-1] = newValue(grid[num_els-DIM-2], grid[num_els-DIM-1], grid[num_els-2*DIM], grid[num_els-2], grid[num_els-DIM], upper[DIM-2], upper[DIM - 1], upper[0], grid[num_els-1]);

    for ( j=0; j<num_els; j++ )
    {
      grid[j] = tmp_grid[j];
    }


    if ( ID != 0 )
      {
        MPI_Ssend(grid, num_els, MPI_INT, 0, 0, MPI_COMM_WORLD ); 
      }
    else 
      { 
        for (i=0; i<num_procs; i++)
        {
          if (i != 0)
          {
          MPI_Recv(tmp_grid, num_els, MPI_INT, i, 0, MPI_COMM_WORLD, &stat ); 
          }
          for (j=0; j<num_els; j++){
            global_grid[i*num_els + j] = tmp_grid[j];
          }
        }
      }

    // Output the updated grid state
    if ( ID == 0 ) {
      printf ( "\nIteration %d: final grid:\n", iters );
      for ( j = 0; j < GRID_WIDTH; j++ ) {
        if ( j % DIM == 0 ) {
          printf( "\n" );
        }
        printf ( "%d  ", global_grid[j] );
      }
      printf( "\n" );
    }
  }
  // TODO: Clean up memory
  free(tmp_grid);
  free(grid);
  MPI_Finalize(); // finalize so I can exit
}