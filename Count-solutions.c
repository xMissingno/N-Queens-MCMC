#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include <float.h>

// A text file with partial estimates for each N = 100..1000 by 100
const char *FILENAME_PARTIAL_ESTIMATES[] = 
{"partialestimates0.csv",
	"partialestimates1.csv",
	"partialestimates2.csv",
	"partialestimates3.csv",
	"partialestimates4.csv",
	"partialestimates5.csv",
	"partialestimates6.csv",
	"partialestimates7.csv",
	"partialestimates8.csv",
	"partialestimates9.csv"};
// A text file with beta stars generated by FitBetas.ipynb
const char FILENAME_FIT_BETASTARS[] = "fitbetastars.csv";


// Number of known counts
const int NUM_KNOWN = 27 - 4 + 1;
const int NUM_UNKNOWN = 10;

/**
 * Returns log(n!).
 */
double logfact(int n) {
	double res = 0;
	for (int i = 1; i <= n; i++) {
		res += log((double) i);
	}
	return res;
}

/**
 * Fills in z with integers from min to max by step inplace.
 */
void arange(int* z, int min, int max, int step) 
{	
	for (int i = 0; i < max-min; i += step) {
		z[i] = min + i;
	}
}

/**
 * Finds the largest element in an unsorted array.
 */
 double arr_max(double* z, int len) {
	 double largest_so_far = -DBL_MAX;
	 for (int i = 0; i < len; i++) {
		 if (largest_so_far < z[i]) {
			largest_so_far = z[i];
		 }
	 }
	 return largest_so_far;
}

/**
 * Returns the sum of all elements in the array.
 */
double sum(double* z, int len) {
	double s = 0;
	for (int i=0; i < len; i++) {
		s += z[i];
	}
	
	return s;
}

/**
 * Exponentiate each element of an array. Inplace.
 */
double* exp_arr(double* z, int len) {
	 for (int i=0; i < len; i++) {
		 z[i] = exp(z[i]);
	 }
	 return z;
}

/**
 * Multiply each element of z by a scalar. Inplace.
 */
double* scal_prod(double r, double* z, int len) {
	 for (int i=0; i < len; i++) {
		 z[i] *= r;
	 }
	 return z;
}

/**
 * Prints an array of integers
 */
void print_int_arr(int* z, int length)
{
	printf("[ ");
	for (int i = 0; i < length; i++) {
		printf("%d ", z[i]);
	}
	printf("]\n");
}

/**
 * Prints an array of long long unsigned
 */
void print_llu_arr(long long unsigned* z, int length)
{
	printf("[ ");
	for (int i = 0; i < length; i++) {
		printf("%I64u ", z[i]);
	}
	printf("]\n");
}

/**
 * Prints an array of doubles
 */
void print_lf_arr(double* z, int length)
{
	printf("[ ");
	for (int i = 0; i < length; i++) {
		printf("%.3lf ", z[i]);
	}
	printf("]\n");
}


/**
 * Swaps two array indices inplace.
 */
void swap(int* z, int i, int j) 
{
	int t = z[i];
	z[i] = z[j];
	z[j] = t;
}

/**
 * Calculates number of unique threats on board.
 */
int loss(int* z, int* idx_pairs, int C) 
{
	int sum = 0;
	for (int k = 0; k < 2 * C; k += 2) {
		int i = idx_pairs[k]; int j = idx_pairs[k+1];
		sum += (j - i == abs(z[j] - z[i]));
	}
	return sum;
}

/**
 * Calculates all pairwise combinations of indices less than N
 * and stores them contiguously in dst.
 */
void pairs(int* dst, int N) 
{
	int k = 0;
	for (int i = 0; i < N-1; i++) {
		for (int j = i+1; j < N; j++) {
			dst[k] = i;
			dst[k+1] = j;
			k += 2;
		}
	}
}

/**
 * Returns the max of two doubles.
 */
double max_lf(double a, double b) {
	return a*(a>b) + b*(a<=b);
}

/**
 * Returns the max of two long long unsigned integers.
 */
long long unsigned max_llu(long long unsigned a, long long unsigned b) {
	return a*(a>b) + b*(a<=b);
}

/**
 * Percentage difference between two long long unsigned integers.
 */
double percent_diff(long long unsigned x, long long unsigned y) {
	return (double) abs(x-y) / max_llu(x,y);
}


/**
 * Returns number of queens threatening queen i.
 */
int threats(int* z, int i, int N) {
	int sum = 0;
	for (int k = 0; k < N; k++) {
		sum += abs(k-i) == abs(z[k]-z[i]);
	}
	sum -= 1;
	return sum;
}

/**
 * One-step loss difference.
 */
int loss_diff(int* z, int i, int j, int N) {
	int old = threats(z, i, N) + threats(z, j, N);
	swap(z, i, j);
	int new = threats(z, i, N) + threats(z, j, N);
	swap(z, i, j); // Swap back.
	return new - old;
}

/**
 * Perform K random swaps on a list of integers.
 */
void shuffle(int* z, int K, int* idx_pairs, int nb_pairs) 
{
	for (int k = 0; k < K; k++) {
		int pair_idx = (rand() % nb_pairs) * 2;
		int i = idx_pairs[pair_idx];
		int j = idx_pairs[pair_idx+1];
		swap(z, i, j);
	}
}

/**
 * Assumes memory has been allocated. Copies len elements of 
 * src into dst.
 */
void arrcopy(int* src, int* dst, int len) 
{
	for (int i = 0; i < len; i++) {
		dst[i] = src[i];
	}
}

/**
 * Reads long long unsigned integer array from files with format
 * a1,a2,a3,a4,a5,...
 */
void load_llu_arr(const char* filename, long long unsigned *dst, int len) {
	
	FILE *fp = fopen(filename, "r");
	if(fp == NULL) {
        printf("file can't be opened\n");
        exit(1);
    }
    
    for (int i = 0; i < len; i++) {
		fscanf(fp, "%I64u,", &dst[i]);
	}
}

/**
 * Reads double array from files with format
 * a1,a2,a3,a4,a5,...
 */
void load_lf_arr(const char* filename, double *dst, int len) {
	
	FILE *fp = fopen(filename, "r");
	if(fp == NULL) {
        printf("file can't be opened\n");
        exit(1);
    }
    
    for (int i = 0; i < len; i++) {
		fscanf(fp, "%lf,", &dst[i]);
	}
}

/**
 * Saves double array from files with format
 * a1,a2,a3,a4,a5
 */
void save_llu_arr(const char* filename, long long unsigned *src, int len) {
	
	FILE *fp = fopen(filename, "wb");
	if(fp == NULL) {
        printf("file can't be opened\n");
        exit(1);
    }
    
    for (int i = 0; i < len; i++) {
		if (i != len-1) {
			fprintf(fp, "%I64u,", src[i]);
		} else {
			fprintf(fp, "%I64u", src[i]);
		}
	}
}

/**
 * Saves double array from files with format
 * a1,a2,a3,a4,a5
 */
void save_lf_arr(const char* filename, double *src, int len, bool append) {
	
	FILE *fp = fopen(filename, append ? "ab" : "wb");
	if(fp == NULL) {
        printf("file can't be opened\n");
        exit(1);
    }
    
    fflush(fp);
    for (int i = 0; i < len; i++) {
		if (i != len-1) {
			fprintf(fp, "%lf,", src[i]);
		} else {
			fprintf(fp, "%lf\n", src[i]);
		}
	}
	printf("Saving...\n");
	
}

void log_count_solutions(int M, double beta_del, bool doclear) {
	
	// Clear partial estimates
	if (doclear) {
		for (int i = 0; i < 10; i++) {
			fclose(fopen(FILENAME_PARTIAL_ESTIMATES[i], "w"));
		}
	}
	/* Load fitted betastars for unknown boards N= 50 to 1000 by 50
	 */
	double* fit_beta_stars = malloc(NUM_UNKNOWN * sizeof(double));
	load_lf_arr(FILENAME_FIT_BETASTARS, fit_beta_stars, NUM_UNKNOWN);
	print_lf_arr(fit_beta_stars,NUM_UNKNOWN);
	
	// Estimate counts.
	double* log_count_estimates = malloc(NUM_UNKNOWN * sizeof(double));
	
	int N = 100;
	
	// For every N for which the number of solutions is known
	for (int i = 0; i < NUM_UNKNOWN; i++, N+=100) {
		
		printf("N=%d\n", N);
		
		// Pairs of queen indices
		int C = N * (N-1) / 2;
		int *idx_pairs = malloc(2 * C * sizeof(int));
		pairs(idx_pairs, N);

		double log_Z_partial = 0; // Current estimate.
		int T = fit_beta_stars[i] / beta_del;
		
		printf("T=%d\n", T);
		// Record the partial estimates.
		double *partial_estimates = malloc(T * sizeof(double)); 
		
		// Approach the real number of solutions
		for (int t = 1; t <= T; t++) {
			
			// Starting position
			int *z = malloc(N * sizeof(int));
			arange(z, 1, N+1, 1);
			
			// Shuffle the starting position
			shuffle(z, 2 * N, idx_pairs, C);
			
			// Calculate current loss
			int l = loss(z, idx_pairs, C);
			
			double *losses = malloc(M * sizeof(double));
			
			// Draw M samples according to the Metropolis Algorithm.
			for (int k = 0; k < M; k++) {
				
				int r = (rand() % C);
				int m = idx_pairs[2*r]; 
				int n = idx_pairs[2*r+1];
				double diff = (double)loss_diff(z, m, n, N);
				double acc = (diff < 0) ? 1 : exp(-t*beta_del*diff);
				double u = ((double)rand() / (double)(RAND_MAX));
				if (u < acc) {
					swap(z, m, n);
					l += diff;
				}
				losses[k] = (double) l;
			}
			
			// Estimate ratio
			double log_Z_beta_t_ratio = 
				log(sum(exp_arr(scal_prod(
							-beta_del,losses,M),M),M)/M);
			
			// Otherwise, incorporate the next term.
			log_Z_partial += log_Z_beta_t_ratio;
			partial_estimates[t-1] = log_Z_partial + logfact(N);
			
			// Free
			free(z);
			free(losses);
			
		}
		
		// Record approximation.
		log_count_estimates[i] = log_Z_partial+logfact(N);
		save_lf_arr(FILENAME_PARTIAL_ESTIMATES[i], partial_estimates, T, 1);
		
		// Free
		free(idx_pairs);
		free(partial_estimates);
	}
	
	// Print count estimates.
	printf("Log solution count estimates\n");
	print_lf_arr(log_count_estimates, NUM_UNKNOWN);
	
	free(fit_beta_stars);
}

int main()
{	
	// Timing start
	time_t then;
	time(&then);
	
	// Reproducibility
	srand(time(NULL));
	
	// Parameters
	int M = 1000;
	double beta_del = 0.001;
	
	printf("Running...\n");
	
	// UNCOMMENT to estimate unknown solutions
	log_count_solutions(M, beta_del, 0);
	
	
	// Timing end
	time_t now;
	time(&now);
	
	time_t diff = now - then;
	
	printf("Began: %s", ctime(&then));
	printf("Ended: %s", ctime(&now));
	printf("Diff: %d minutes, %d seconds", diff / 60, diff % 60);
	
	// Free
	// free(log_count_estimates);
	
	// Finish
	return 0;
}
