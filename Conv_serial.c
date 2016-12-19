#include<stdio.h>
int fmask(int label[10][10],int matrix[10][10],int m,int n,int i,int j){

	int a,b,c,min;
	a=i-1;
	b=j-1;
	c=j+1;
	
	if(a!=-1 && b!=-1 && c!=n){
		
		if(matrix[i][b]==0 && matrix[a][b]==0 && matrix[a][c]==0 && matrix[a][j]==0 ){
			return 0;
		}
		else{

			min=100;
			if(label[i][b]!=0){
				min=label[i][b];
			}
			if(label[a][b] < min && label[a][b]!=0){
				min=label[a][b];
			}
			if(label[a][c] < min && label[a][c]!=0){
				min=label[a][c];
			}
			if(label[a][j] < min && label[a][j]!=0){
				min=label[a][j];
			}
			return min;
		}
	}
	else{

		if(a==-1 && b==-1){
			return 0;
		}
		if(a==-1){
			return label[i][b];
		}
		if(b==-1){

			if(label[a][j] <= label[a][c]){

				if(label[a][j]==0){
					return label[a][c];
				}
				else{
					return label[a][j];
				}
			}
			else{	

				if(label[a][c]==0){
					return label[a][j];
				}
				else{
					return label[a][c];
				}
			}
		}
		if(c==n)
		{
			
			min=100;
			if(label[i][b]!=0){
				min=label[i][b];
			}
			if(label[a][b] < min && label[a][b]!=0){
				min=label[a][b];
			}
			if(label[a][j] < min && label[a][j]!=0){
				min=label[a][j];
			}
			if(min == 100){
				return 0;
			}
			else{
				return min;
			}
		}
		return 0;
	}

}
int bmask(int label[10][10],int matrix[10][10],int m,int n,int i,int j){

	int a,b,c,min;
	a=i+1;
	b=j-1;
	c=j+1;

	if(a!=m && b!=-1 && c!=n){

		if(matrix[i][c]==0 && matrix[a][b]==0 && matrix[a][c]==0 && matrix[a][j]==0 ){
			return 0;
		}
		else{

			min=100;
			if(label[i][c]!=0){
				min=label[i][c];
			}
			if(label[a][b] < min && label[a][b]!=0){
				min=label[a][b];
			}
			if(label[a][c] < min && label[a][c]!=0){
				min=label[a][c];
			}
			if(label[a][j] < min && label[a][j]!=0){
				min=label[a][j];
			}
			return min;
		}
	}
	else
	{
		if(a==m && c==n){
			return 0;
		}
		if(a==m){
			return label[i][c];
		}
		if(c==n){
			if(label[a][j] <= label[a][b]){

				if(label[a][j]==0){
					return label[a][b];
				}
				else{
					return label[a][j];
				}
			}
			else{

				if(label[a][b]==0){
					return label[a][j];
				}
				else{
					return label[a][b];
				}
			}
		}
		if(b==-1)
		{
			min=100;
			if(label[i][c]!=0){
				min=label[i][c];
			}
			if(label[a][c] < min && label[a][c]!=0){
				min=label[a][c];
			}
			if(label[a][j] < min && label[a][j]!=0){
				min=label[a][j];
			}
			if(min == 100){
				return 0;
			}
			else{
				return min;
			}
		}
		return 0;
	}
}

int suzuki(int matrix[10][10],int m, int n)
{
	int label[10][10],i,j,val,counter,flag;
	counter=0;
	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			if(matrix[i][j]==0){
				label[i][j]=0;
			}
			else{
				
				val=fmask(label,matrix,m,n,i,j);
				if(val == 0){
					counter++;
					label[i][j]=counter;
				}
				else{
					label[i][j]=val;
				}

			}
		}
	}
	flag=1;
	while(flag){
			
		for(i=0;i<m;i++){
			for(j=0;j<n;j++){
				if(matrix[i][j]!=0){
					val=bmask(label,matrix,m,n,i,j);
					if(val !=0 && val < label[i][j]){
						flag=0;
						label[i][j]=val;
					}
				}
			}
		}
		for(i=0;i<m;i++){
			for(j=0;j<n;j++){
				if(matrix[i][j]!=0){
					val=fmask(label,matrix,m,n,i,j);
					if(val!=0 && val < label[i][j]){
						flag=0;
						label[i][j]=val;
					}
				}
			}
		}
		flag=1-flag;
	}
	printf("the labelled components are : \n");
	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			printf("%d ", label[i][j]);
		}
		printf("\n");
	}

}
			

	
int main(){

	int matrix[10][10],i,j,n,m;
	scanf("%d %d",&m,&n);
	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			scanf("%d",&matrix[i][j]);
		}
	}
	suzuki(matrix,m,n);
	return 0;
}
