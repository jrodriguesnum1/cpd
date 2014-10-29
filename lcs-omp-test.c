#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h> 

int ** allocArray(int xdim, int ydim) {
	int **array = (int**)malloc(sizeof(int*)*xdim);
	int x;
	for(x = 0; x < xdim; x++) {
		array[x] = (int*)malloc(sizeof(int)*ydim);
		memset(array[x], (unsigned char)0, sizeof(int) * ydim);
	}
	return array;
}

void reverse(char * str) {
	int len = strlen(str);
	int i;
	char tmp;
	for(i = 0; i < len / 2; i++) {
		tmp = str[i];
		str[i] = str[len-1-i];
		str[len-1-i] = tmp;
	}
}

int max(int a, int b){
	return (a > b)? a : b;
}

void cell_limits(int tab_coord, int max_mat_coord, int n_processors, int  *coord_min, int *coord_max) {
	int remainder = max_mat_coord % n_processors;
	int  n_mat_coords_per_cell = max_mat_coord / n_processors;
	*coord_min = n_mat_coords_per_cell * tab_coord;
	*coord_max = (*coord_min) + n_mat_coords_per_cell;
	if(tab_coord == n_processors - 1) {
		*coord_max += remainder;
	}
}

typedef struct square {
	int tabX;
	int tabY;
	int x_min;
	int x_max;
	int y_min;
	int y_max;
} Square;

Square * new_square(int x, int y, int matx_max, int maty_max, int n_processors) {
	Square * new_square = (Square*)malloc(sizeof(Square));
	new_square->tabX = x;
	new_square->tabY = y;
	cell_limits(x,matx_max, n_processors, &new_square->x_min, &new_square->x_max);
	cell_limits(y,maty_max, n_processors, &new_square->y_min, &new_square->y_max);
	return new_square;
}

Square * squareWith2(int **table, int tabx_max, int taby_max, int matx_max, int maty_max  ,int n_processors ) {
	int x,y;
	for(x = 0; x < tabx_max; x++) {
		for(y = 0; y < taby_max; y++) {
			if(table[x][y] == 2) {
				return new_square(x, y, matx_max, maty_max, n_processors);
			}
		}
	}
	return NULL;
}

int notFinishedTab(int **table, int tabx_max, int taby_max) {
	int i,j;
	int number_of_3s = 0;
	for(i = 0; i < tabx_max; i++) {
		for(j = 0; j < taby_max; j++) {
			if(table[i][j] == 3)
				number_of_3s++;
		}
	}
	return (number_of_3s != tabx_max * taby_max)? 1 : 0;
}




void print(int **m, int x_max, int y_max) {
	int i, j;   
	for(i = 0; i < x_max; i++) {
		for(j = 0; j < y_max; j++) {
			printf("%d ", m[i][j]);
		}
		printf("\n");
	}
}


// read input
typedef struct file_info {
	int size_x;
	int size_y;
	char * X;
	char * Y;
} InputInfo;

InputInfo * readInput(char * fileName) {
	FILE * file = NULL;
	InputInfo * inputInfo = NULL;
	
	if((file = fopen(fileName,"r")) != NULL) {            
		inputInfo = (InputInfo*)malloc(sizeof(InputInfo));
		fscanf(file,"%d %d", &inputInfo->size_x, &inputInfo->size_y);
		inputInfo->size_x++;
		inputInfo->size_y++;
		inputInfo->X = (char*)malloc(inputInfo->size_x*sizeof(char));
		inputInfo->Y = (char*)malloc(inputInfo->size_x*sizeof(char)); 
	
		while((fscanf(file,"%s%s",inputInfo->X,inputInfo->Y)) != EOF){

		}                              
		fclose(file);
	} else {
		printf("cannot open file: %s", fileName);
		exit(0);
	}
	return inputInfo;
}

void calc(int x, int y, int **matrix, char * X, char * Y) {
	if (x == 0 || y == 0)
		matrix[x][y] = 0;    
	else if ((x>0 && y>0)&&(X[x-1] == Y[y-1]))
		matrix[x][y] = matrix[x-1][y-1] + 1;    
	else if ((x>0 && y>0)&&(matrix[x-1][y] != matrix[x][y-1]))
		matrix[x][y] = max(matrix[x-1][y], matrix[x][y-1]);	  
	else
		matrix[x][y] = matrix[x][y-1]; 
}

char * lcs(int **matrix, char * X, char * Y, int matx_max, int maty_max) {
	char * lcs = (char*)malloc(max(matx_max,maty_max) * sizeof(char));
	int k = matx_max - 1; 
	int p = maty_max - 1;
	int l = 0;
	while (k>0 && p>0){
		if (X[k-1] == Y[p-1]){
			lcs[l] = X[k-1];  //
			k--; p--; l++;		 		                
		} else if(matrix[k-1][p] >= matrix[k][p-1]) 
			k--;
		else 
			p--;
   }
   lcs[l] = '\0';
   reverse(lcs);
   return lcs;
}

int main(int argc, char **argv) {

	int n_processors = 4;
	char * lcs_result = NULL;
	InputInfo * inputInfo = NULL;
	
	// read input
	inputInfo = readInput(argv[1]);
	if(inputInfo == NULL) {
		printf("input info is NULL\n");
		return -1;
	}

	int matx_max = inputInfo->size_x;
	int maty_max = inputInfo->size_y;

	int i;
	// // check cell_limits func
	// int min, max;
	// for(i = 0; i < n_processors; i++) {
	// 	cell_limits(i, inputInfo->size_x, n_processors, &min, &max);
	// 	printf("%d-%d ", min, max);
	// }
	// printf("\n");
	// for(i = 0; i < n_processors; i++) {
	// 	cell_limits(i, inputInfo->size_y, n_processors, &min, &max);
	// 	printf("%d-%d ", min, max);
	// }

	int **matrix = allocArray(matx_max, maty_max);

	int tabx_max = n_processors;
	int taby_max = n_processors;

	int **table = allocArray(tabx_max, taby_max);

	int j;
	for(i = 0; i < tabx_max; i++) {
		for(j = 0; j < taby_max; j++) {
			if(i==0 && j==0)
				table[i][j] = 2;
			else if(i==0 || j==0)
				table[i][j] = 1;
			else
				table[i][j] = 0;
		}
	}

	// fill matrix
	Square * square = NULL;
	int x,y;
	while(notFinishedTab(table, tabx_max, taby_max)) {
		square = NULL;
		while(square == NULL) {
			//lock A {
				square = squareWith2(table, tabx_max, taby_max, matx_max, maty_max, n_processors);
			//}
		}
		//lock A {
			table[square->tabX][square->tabY]++;
		//}
		for(x = square->x_min; x < square->x_max; x++) {
			for(y = square->y_min; y < square->y_max; y++) {
				calc(x,y, matrix, inputInfo->X, inputInfo->Y);
			}
		}
		//lock A {
			if(square->tabX + 1 < tabx_max) {
				table[square->tabX + 1][square->tabY]++;
			}
			if(square->tabY +1 < taby_max) {
				table[square->tabX][square->tabY + 1]++;
			}
		//}
		// free(square);
		// print(table, tabx_max, taby_max);
		// print(matrix, matx_max, maty_max);
		// printf("\n");
	}

	// calc lcs
	lcs_result = lcs(matrix, inputInfo->X, inputInfo->Y, matx_max, maty_max);
	//printf("%d  %d\n", matx_max, maty_max);
	//printf("%d\n", matrix[matx_max-1][maty_max-1]);
	//printf("%s\n", lcs_result);
	print(matrix, matx_max, maty_max);
	return 0;
}