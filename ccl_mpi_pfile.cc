#include <mpi.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>

#define BAND 16.0

int merge(int *,int, int);
void flatten(int *,int);

using namespace std;
int check_pixel_values(int a, int b)
{
  if(a == 0 || b == 0 || a == 555 || b == 555)
  {
    return 0;
  }
  int a_band = (int) a/BAND;
  int b_band = (int) b/BAND;
  if(a_band == b_band)
    return 1;
  else
    return 0;
}

int main(int argc, char *argv[])
{
  long long int row = 0, column = 0, number_of_pixels = 0, p_size, chunk, num_iter, start, size;
  int num,i = 0, j = 0, count = 1, k = 0;
  int tid, number_of_threads;
  int *image, *label, *p; // for all the processes except root
  int **label_root, **p_root; // for the root
  int **label_final, *p_final; // final values
  FILE *file_pointer;
  char ch;
  int dimensions[3];
  double start_time, end_time;

  //MPI Initialization
  int rank = 0, np = 0;

  MPI_Init (&argc, &argv);	/* starts MPI */
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* Get process id */
  MPI_Comm_size (MPI_COMM_WORLD, &np);	/* Get number of processes */
  start_time = MPI_Wtime();

  file_pointer = fopen(argv[1],"r");
  while(fscanf(file_pointer,"%d",&num) != EOF )
  {
    number_of_pixels++;
  }
  fseek(file_pointer,0,SEEK_SET);
  while((ch = fgetc(file_pointer)) != EOF )
  {
    if(ch == '\n')
    row++;
  }

  column = number_of_pixels/row;
  row = row + 2; //for padding
  column = column +2; // for padding
  number_of_pixels = row * column;

  image = (int *)calloc(row * column, sizeof(int));

  fseek(file_pointer,0,SEEK_SET);
  for(i = 0; i < row; i++)
  {
    for(j =0; j < column; j++)
    {
      if(i == 0 || j == 0 || i == row - 1 || j == column - 1)
      {
        image[(i* row) + j] = 555;
      }
      else
      {
        fscanf(file_pointer,"%d",&num);
        image[(i * row) + j] = num;
      }
    }
  }

  num_iter = (row-2)/2;
  if((row % 2) == 1)
  {
  	num_iter = num_iter + 1;
  }

  chunk = num_iter/np;
  size = 2 * chunk;
  start = 1 + rank*size;

  label = (int *)calloc(row * column, sizeof(int));
  p = (int *)calloc(number_of_pixels, sizeof(int));

  for(i=start; i<(start+size); i=(i+2))
  {
    if (i == start)
    {
    	count = i*column;
    }
    for(j=1; j<(column-1); j++)
    {
      if(i == start)
      {
        if(image[i*row + j] > 0)
        {
          if(check_pixel_values(image[i*row + j], image[i*row + (j-1)]) == 1)
          {
            label[i*row + j] = label[i*row + (j-1)];
          }
          else
          {
            if(check_pixel_values(image[i*row + j], image[(i+1)*row + (j-1)]) == 1)
            {
              label[i*row + j] = label[(i+1)*row + (j-1)];
            }
            else
            {
              label[i*row + j] = count;
              p[count] = count;
              count++;
            }
          }
        }
        if(check_pixel_values(image[i*row + j], image[(i+1)*row + j]) == 1)
        {
          label[(i+1)*row + j] = label[i*row + j];
        }
        else
        {
          if(image[(i+1)*row + j] > 0 && image[(i+1)*row + j] != 555)
          {
            if(check_pixel_values(image[(i+1)*row + j], image[i*row + (j-1)]) == 1)
            {
              label[(i+1)*row + j] = label[i*row + (j-1)];
            }
            else
            {
              if(check_pixel_values(image[(i+1)*row + j], image[(i+1)*row + (j-1)]) == 1)
              {
                label[(i+1)*row + j] = label[(i+1)*row + (j-1)];
              }
              else
              {
                label[(i+1)*row + j] = count;
                p[count] = count;
                count++;
              }
            }
          }
        }

      }
      else
      {
        if(image[i*row + j] > 0)
        {
          if(check_pixel_values(image[i*row + j], image[i*row + (j-1)]) == 0)
          {
            if(check_pixel_values(image[i*row + j], image[(i-1)*row + j]) == 1)
            {
              label[i*row + j] = label[(i-1)*row + j];
              if(check_pixel_values(image[i*row + j], image[(i+1)*row + (j-1)]) == 1)
              {
                merge(p,label[i*row + j],label[(i+1)*row + (j-1)]);
              }
            }
            else
            {
              if(check_pixel_values(image[i*row + j], image[(i+1)*row + (j-1)]) == 1)
              {
                label[i*row + j] = label[(i+1)*row + (j-1)];
                if(check_pixel_values(image[i*row + j], image[(i-1)*row + (j-1)]) == 1)
                {
                  merge(p,label[i*row + j],label[(i-1)*row + (j-1)]);
                }
                if(check_pixel_values(image[i*row + j], image[(i-1)*row + (j+1)]) == 1)
                {
                  merge(p,label[i*row + j],label[(i-1)*row + (j+1)]);
                }
              }
              else
              {
                if(check_pixel_values(image[i*row + j], image[(i-1)*row + (j-1)]) == 1)
                {
                  label[i*row + j] = label[(i-1)*row + (j-1)];
                  if(check_pixel_values(image[i*row + j], image[(i-1)*row + (j+1)]) == 1)
                  {
                    merge(p,label[i*row + j],label[(i-1)*row + (j+1)]);
                  }
                }
                else
                {
                  if(check_pixel_values(image[i*row + j], image[(i-1)*row + (j+1)]) == 1)
                  {
                    label[i*row + j] = label[(i-1)*row + (j+1)];
                  }
                  else
                  {
                    label[i*row + j] = count;
                    p[count] = count;
                    count++;
                  }
                }
              }
            }
          }
          else
          {
            label[i*row + j] = label[i*row + (j-1)];
            if(check_pixel_values(image[i*row + j], image[(i-1)*row + j]) == 0)
            {
              if(check_pixel_values(image[i*row + j], image[(i-1)*row + (j+1)]) == 1)
              {
                merge(p,label[i*row + j],label[(i-1)*row + (j+1)]);
              }
            }
          }
        }
        if(check_pixel_values(image[i*row + j], image[(i+1)*row + j]) == 1)
        {
          label[(i+1)*row + j] = label[i*row + j];
        }
        else
        {
          if(image[(i+1)*row + j] > 0 && image[(i+1)*row + j] != 555)
          {
            if(check_pixel_values(image[(i+1)*row + j], image[i*row + (j-1)]) == 1)
            {
              label[(i+1)*row + j] = label[i*row + (j-1)];
            }
            else
            {
              if(check_pixel_values(image[(i+1)*row + j], image[(i+1)*row + (j-1)]) == 1)
              {
                label[(i+1)*row + j] = label[(i+1)*row + (j-1)];
              }
              else
              {
                label[(i+1)*row + j] = count;
                p[count] = count;
                count++;
              }
            }
          }
        }
      }
    }
  }

  //Parallel Merge happens here
  for(i = 1; i < np; i *= 2)
  {
    if((rank/i) % 2 == 1)
    {
      int block_size = i*size;
      int *label_send = (int *)calloc(block_size*column, sizeof(int));
      int *p_send = (int *)calloc(block_size*column, sizeof(int));
      int local_index = 0;
      for(j=start; j < start + block_size; j++)
      {
        for(k=1; k < column-1; k++)
        {
          label_send[local_index*row + k] = label[j*row + k];
        }
        local_index++;
      }
      int count_send = start*column;
      local_index = 0;
      for(j=count_send; j < (start + block_size)*column; j++)
      {
        p_send[local_index++] = p[j];
      }
      MPI_Send(label_send, block_size*column, MPI_INT, rank - i, 0, MPI_COMM_WORLD);
      MPI_Send(p_send, block_size*column, MPI_INT, rank - i, 0, MPI_COMM_WORLD);
      break;
    }
    else
    {
      int block_size = i*size;
      int *label_recv = (int *)calloc(block_size*column, sizeof(int));
      int *p_recv = (int *)calloc(block_size*column, sizeof(int));
      MPI_Status status;
      MPI_Recv(label_recv, block_size*column, MPI_INT, rank + i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Recv(p_recv, block_size*column, MPI_INT, rank + i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      int recv_i = 1 + (rank + i)*size;
      int local_index = 0;
      for(j=recv_i; j < recv_i + block_size; j++)
      {
        for(k=1; k < column-1; k++)
        {
          label[j*row + k] = label_recv[local_index*row + k];
        }
        local_index++;
      }
      int count_send = recv_i*column;
      local_index = 0;
      for(j=count_send; j < (recv_i + block_size)*column; j++)
      {
        p[j] = p_recv[local_index++];
      }
      for(j=1; j<(column-1); j++)
      {
        if(image[recv_i*row + j] > 0)
        {
          if(check_pixel_values(image[(recv_i*row) + j], image[(recv_i-1)*row + j]) == 1)
          {
            merge(p,label[(recv_i*row) + j],label[(recv_i-1)*row + j]);
          }
          else
          {
            if(check_pixel_values(image[(recv_i*row) + j], image[(recv_i-1)*row + j-1]) == 1)
            {
              merge(p,label[(recv_i*row) + j],label[(recv_i-1)*row + j-1]);
            }
            if(check_pixel_values(image[(recv_i*row) + j], image[(recv_i-1)*row + j+1]) == 1)
            {
              merge(p,label[(recv_i*row) + j],label[(recv_i-1)*row + j+1]);
            }
          }
        }
      }
    }
  }

  if(rank == 0)
  {
    flatten(p,number_of_pixels);
    end_time = MPI_Wtime();
    printf("Elapsed Time = %f\n", end_time - start_time);
  }
  MPI_Finalize();
  return 0;
}

int merge (int *p, int x, int y)
{
  int rootx,rooty,z;
  rootx = x;
  rooty = y;
  while( p[rootx] != p[rooty] )
  {
    if ( p[rootx] > p[rooty] )
    {
      if( rootx == p[rootx] )
      {
        p[rootx] = p[rooty];
        return(p[rootx]);
      }
      z= p[rootx];
      p[rootx] = p[rooty];
      rootx = z;
    }
    else
    {
      if( rooty == p[rooty])
      {
        p[rooty] = p[rootx];
        return(p[rootx]);
      }
      z = p[rooty];
      p[rooty] = p[rootx];
      rooty = z;
    }
  }
  return(p[rootx]);
}

void flatten(int *p, int size)
{
  int k = 1,i;
  for(i=1;i < size; i++)
	{
	  if(p[i] < i)
	  p[i] = p[p[i]];
	  else
	  {
	    {
	      p[i] = k;
	      k++;
	    }
	  }
	}
  printf("%d\n",k-1);
}
