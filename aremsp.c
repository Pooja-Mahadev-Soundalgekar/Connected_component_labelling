						/*Experiments on Union-Find Algorithms for the Disjoint-Set Data Structure*/
						/*A New Two-Scan Algorithm for Labeling Connected Components in Binary Images*/
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int merge(int *,int, int);
void flatten(int *,int);
int main(int argc, char *argv[])
{
	int row=0,chart=0,column=0;
	int num,i=0,j=0,count=1;
	int **image,**label,*p;
	FILE *fp;
	char ch;
	double time_spent,t1,t2;
	struct timespec tp,tp1;
	fp = fopen(argv[1],"r");
	while(fscanf(fp,"%d",&num) != EOF )
	{
		chart++;
	}
	fseek(fp,0,SEEK_SET);
	while((ch = fgetc(fp)) != EOF )
	{
		if(ch == '\n')
			row++;
	}
	column = chart/row ;
	row = row + 2;
	column = column +2;
	image = (int **)calloc(row, sizeof(int *));
	label = (int **)calloc(row, sizeof(int *));
	p = (int *)calloc(chart, sizeof(int *));
	if(image == NULL)
	{
		fprintf(stderr, "out of memory\n");
		return(0);
	}
	if(label == NULL)
	{
		fprintf(stderr, "out of memory\n");
		return(0);
	}
	if(p == NULL)
	{
		fprintf(stderr, "out of memory\n");
		return(0);
	}
	for(i = 0; i < row; i++)
	{
		image[i] =(int *) calloc(column, sizeof(int));
		label[i] =(int *) calloc(column, sizeof(int));
		if(image[i] == NULL)
		{
			fprintf(stderr, "out of memory\n");
			return(0);
		}
		if(label[i] == NULL)
		{
			fprintf(stderr, "out of memory\n");
			return(0);
		}
	}
	fseek(fp,0,SEEK_SET);
	for(i=1;i<row-1;i++)
	{
		for(j=1;j<column-1;j++)
		{
			fscanf(fp,"%d",&num);
			image[i][j] = num;
		}
	}
	// Scaning Phase 
	clock_gettime(CLOCK_REALTIME,&tp);
	
	t1 = (((double)tp.tv_sec) * 1000000) + (((double)tp.tv_nsec) / 1000) ;
	
	for(i=1; i<(row-1); i=i+2)
	{
		for(j=1; j<(column-1); j++)
		{
			if(image[i][j] == 1)
			{
				if(image[i][j-1] == 0)
				{
					if(image[i-1][j] == 1)
					{
						label[i][j] = label[i-1][j];
						if(image[i+1][j-1] == 1)
							merge(p,label[i][j],label[i+1][j-1]);
					}
					else
					{
						if(image[i+1][j-1] == 1)
						{
							label[i][j] = label[i+1][j-1];
							if(image[i-1][j-1] == 1)
								merge(p,label[i][j],label[i-1][j-1]);
							if(image[i-1][j+1] == 1)
								merge(p,label[i][j],label[i-1][j+1]);
						}
						else
						{
							if(image[i-1][j-1] == 1)
							{
								label[i][j] = label[i-1][j-1];
								if(image[i-1][j+1] == 1)
									merge(p,label[i][j],label[i-1][j+1]);
							}
							else
							{
								if(image[i-1][j+1] == 1)
									label[i][j] = label[i-1][j+1];
								else
								{
									label[i][j] = count;
									p[count] = count;
									count++;	
								}
							}
						}
					}
				}
				else
				{
					label[i][j] = label[i][j-1];
					if(image[i-1][j] == 0)
					{
						if(image[i-1][j+1] == 1)
							merge(p,label[i][j],label[i-1][j+1]);
					}
				}
				if(image[i+1][j] == 1)
					label[i+1][j] = label[i][j];
			}
			else
			{
				if(image[i+1][j] == 1)
				{
					if(image[i][j-1] == 1)
						label[i+1][j] = label[i][j-1];
					else
					{
						if(image[i+1][j-1] == 1)
							label[i+1][j] = label[i+1][j-1];
						else
						{
							label[i+1][j] = count;
							p[count] = count;
							count++;
						}
					}
				}
			}
		}
	}
	
	clock_gettime(CLOCK_REALTIME,&tp1);
	t2 = (((double)tp1.tv_sec) * 1000000) + (((double)tp1.tv_nsec) / 1000) ;
	time_spent = t2 -t1;
	printf("%f\n",time_spent);
	
	count = count-1 ;
	
	//analysis phase
	
	flatten(p,count);
	
	//labeling phase
	
	for(i=1;i<row;i++)
	{
		for(j=1;j<column;j++)
			label[i][j] = p[label[i][j]];
	}

	fclose(fp);
	return(0);			
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
	for(i=1;i <= size; i++)
	{
		if(p[i] < i)
			p[i] = p[p[i]];
		else
		{
			p[i] = k;
			k++;
		}
	}
	printf("%d\n",k-1);
}
