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

  if(rank == 0)
  {
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

    label_root = (int **)calloc(np, sizeof(int *));
    p_root = (int **)calloc(np, sizeof(int *));

    label_final = (int **)calloc(row, sizeof(int *));
    p_final = (int *)calloc(number_of_pixels, sizeof(int));

    for(i = 1; i < np; i++)
    {
      label_root[i] = (int *)calloc(row*column, sizeof(int));
      p_root[i] = (int *)calloc(number_of_pixels, sizeof(int));
    }

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

    chunk = num_iter/(np-1);
    size = 2 * chunk;

    dimensions[0] = row;
    dimensions[1] = column;
    dimensions[2] = number_of_pixels;

    MPI_Bcast(dimensions, 3, MPI_INT, 0, MPI_COMM_WORLD); // broadcasting row, column, number of pixels
    MPI_Bcast(image, row*column, MPI_INT, 0, MPI_COMM_WORLD); // broadcasting the original image

    MPI_Status status;
    MPI_Recv(&i, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
    end_time = MPI_Wtime();
    printf("Elapsed Time = %f\n", end_time - start_time);
    MPI_Finalize();
    return 0;
  }
  else
  {
    MPI_Bcast(dimensions, 3, MPI_INT, 0, MPI_COMM_WORLD);
    row = dimensions[0];
    column = dimensions[1];
    number_of_pixels = dimensions[2];

    num_iter = (row-2)/2;
    if((row % 2) == 1)
    {
      num_iter = num_iter + 1;
    }

    chunk = num_iter/(np-1);
    size = 2 * chunk;
    start = 1 + (rank-1)*size;

    image = (int *)calloc(row * column, sizeof(int));
    label = (int *)calloc(row * column, sizeof(int));
    p = (int *)calloc(number_of_pixels, sizeof(int));

    MPI_Bcast(image, row*column, MPI_INT, 0, MPI_COMM_WORLD);

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
    int local_rank = rank - 1;
    for(i = 1; i < np - 1; i *= 2)
    {
      if((local_rank/i) % 2 == 1)
      {
        MPI_Send(label, row*column, MPI_INT, rank - i, 0, MPI_COMM_WORLD);
        MPI_Send(p, row*column, MPI_INT, rank - i, 0, MPI_COMM_WORLD);
        break;
      }
      else
      {
        int *label_recv = (int *)calloc(row*column, sizeof(int));
        int *p_recv = (int *)calloc(row*column, sizeof(int));
        MPI_Status status;
        MPI_Recv(label_recv, row*column, MPI_INT, rank + i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(p_recv, row*column, MPI_INT, rank + i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int recv_i = 1 + (local_rank + i)*size;
        for(j=recv_i; j < row - 1 ; j++)
        {
          for(k=1; k < column -1 ; k++)
          {
            label[j*row + k] = label_recv[j*row + k];
          }
        }
        int count_recv = recv_i*column;
        for(j=count_recv; j<number_of_pixels; j++)
        {
        	p[j] = p_recv[j];
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

    if(local_rank == 0)
    {
      flatten(p,number_of_pixels);
      i = 1;
      MPI_Send(&i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
  }
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
