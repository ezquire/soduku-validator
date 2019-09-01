// Tyler Gearing
// CS 450 Fall 2018

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define LENGTH 9
#define TYPE_LENGTH 7
#define ROW "Row"
#define COL "Column"
#define BOX "Box"

// Function Prototypes
struct data* constructor(int);
void* solve_row(struct data *);
void* solve_column(struct data *);
void* solve_grid(struct data *);
void print_result(struct data *);

static int **board;

const char *grid_map[LENGTH] = {
  "top left", "left middle", "bottom left",
  "top middle", "middle middle", "bottom middle",
  "top right", "right middle", "bottom right" };

struct data {
	int id; // location of row, column, or grid
	int invalid; // flag to indicate an invalid location
	int *valid; // validator array
	char *type; // string to hold the name for printing the result
};

int main(int argc, char **argv) {
	
	int in, id, is_invalid = 0;
	FILE *fptr;

	// Check for valid arguments
	if (argc != 2) {
		puts("Error: too few arguments | usage: ./sudoku.x <filename>\n");
		exit(2);
	}

	// Attempt to open the file
	fptr = fopen((char*)(argv[1]), "r");
	if (fptr == NULL) {
		puts("Error opening file.\n");
		exit(2);
	}

	// Allocate memory for the board
	board = malloc(LENGTH * sizeof(int*));
	for (int i = 0; i < LENGTH; ++i) {
		board[i] = malloc(LENGTH * sizeof(int));
	}
	
	// Read lines. input values to board, skip whitespace and newline
	while ((in = fgetc(fptr)) != EOF) {
		if (in != '\n' && in != ' ') {
			board[id / LENGTH][id % LENGTH] = ((int)in - '0');
			++id;
		}
	}
	
	int thread_count = 0;
	pthread_t threads[LENGTH * 3];
	
	for (int i = 0; i < LENGTH; i++) {

		struct data *data_row = constructor(i);
		pthread_create(&threads[thread_count++], NULL, (void*)solve_row,
					   (void*) data_row);

		struct data *data_col = constructor(i);
		pthread_create(&threads[thread_count++], NULL, (void*)solve_column,
					   (void*) data_col);

		struct data *data_grid = constructor(i);
		pthread_create(&threads[thread_count++], NULL, (void*)solve_grid,
					   (void*) data_grid);
	}

	void *retval;
	struct data *retval_d;

	for (int i = 0; i < LENGTH * 3; i++) {
		pthread_join(threads[i], &retval);
		retval_d = (struct data *) retval;
		if(retval_d->invalid) {
			print_result(retval_d);
			is_invalid = retval_d->invalid;
		}
	}

	// Free allocated data
	free(retval_d);

	for (int i = 0; i < LENGTH; i++)
		free(board[i]);
  
	free(board);

	if (is_invalid)
		puts("The input is not a valid Sudoku.");
	else
		puts("The input is a valid Sudoku.");

	return 0;
}

struct data* constructor(int id) {
	struct data *d = malloc(sizeof(struct data));
	d->id = id;
	d->invalid = 0;
	d->type = malloc(sizeof(char)); 
	d->valid = malloc(sizeof(int)*LENGTH);
	for(int i = 0; i < 10; ++i)
		d->valid[i] = 0;
	return d;
}

void* solve_row(struct data *d) {
	for(int i = 0; i < LENGTH; ++i) {
		int val = board[d->id][i];
		if(d->valid[val] != 0) {
			d->invalid = 1;
			d->type = ROW;
		}
		else
			d->valid[val] = 1;
	}
	return d;
}

void* solve_column(struct data *d) {
	for(int i = 0; i < LENGTH; ++i) {
		int val = board[i][d->id];
		if(d->valid[val] != 0) {
			d->invalid = 1;
			d->type = COL;
		}
		else
			d->valid[val] = 1;
	}
	return d;
}

void* solve_grid(struct data *d) {
	for (int i = (d->id % 3) * 3; i < (d->id % 3) * 3 + 3; ++i)
		for (int j = (d->id / 3) * 3; j < (d->id / 3) * 3 + 3; ++j) {
			int val = board[i][j] - 1;
			if(d->valid[val] != 0) {
				d->invalid = 1;
				d->type = BOX;
			}
			else
				d->valid[val] = 1;
		}
	return d;
}

void print_result(struct data *d) {
	if(d->type == ROW || d->type == COL)
		printf("%s %d doesn't have the required values.\n",
			   d->type, d->id + 1);
	else
		printf("The %s subgrid doesn't have the required values.\n",
			   grid_map[d->id]);		
	return;
}
