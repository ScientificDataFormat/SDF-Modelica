#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdio.h>

#include "NDTable.h"

#ifdef _WIN32
#define ISFINITE(x) _finite(x)
#else
#define ISFINITE(x) isfinite(x)
#endif

static char error_message[MAX_MESSAGE_LENGTH] = "";

void NDTable_set_error_message(const char *msg, ...) {
	va_list vargs;
	va_start(vargs, msg);
	vsprintf(error_message, msg, vargs);
	va_end(vargs);
}

NDTable_h NDTable_alloc_table() {
	return (NDTable_h )calloc(1, sizeof(NDTable_t));
}

void NDTable_free_table(NDTable_h table) {
	int i;

	if(!table) return;
	
	free(table->data);
	
	for(i = 0; i < MAX_NDIMS; i++) {
		free(table->scales[i]);
	}

	free(table);
}

const char * NDTable_get_error_message() {
	return error_message;
}

void NDTable_calculate_offsets(int ndims, const int dims[], int offs[]) {
	int i;

	if(ndims < 1) {
		return;
	}

	offs[ndims - 1] = 1;

	for(i = ndims-2; i >= 0; i--) {
		offs[i] = offs[i + 1] * dims[i + 1];  
	}
}

int NDTable_validate_table(NDTable_h table) {
	int i, j, numel, offs[MAX_NDIMS];
	double v;

	// check the rank
	if(table->ndims < 0 || table->ndims > 32) {
		NDTable_set_error_message("The rank of must be in the range [0;32] but was %d", table->ndims);
		return -1;
	}
	
	// check the extent of the dimensions
	for(i = 0; i < table->ndims; i++) {
		if(table->dims[i] < 1) {
			NDTable_set_error_message("Extent of dimension %d of must be >=0 but was %d", i, table->dims[i]);
			return -1;
		}
	}
	
	// check the number of values
	numel = 1;
	for(i = 0; i < table->ndims; i++) {
		numel = numel * table->dims[i];
	}
	
	if(table->numel != numel) {
		NDTable_set_error_message("The size of does not match its extent");
		return -1;
	}
	
	// check the offsets
	NDTable_calculate_offsets(table->ndims, table->dims, offs);
	for(i = 0; i < table->ndims; i++) {
		if(table->offs[i] != offs[i]) {
			NDTable_set_error_message("The offset[%d] of must be %d but was %d", i, offs[i], table->offs[i]);
			return -1;
		}
	}
	
	// check the scales
	for(i = 0; i < table->ndims; i++) {
		
		// make sure a scale is set
		if(table->scales[i] == NULL) {
			NDTable_set_error_message("Scale for dimension %d of is not set", i);
			return -1;
		}
		
		// check strict monotonicity
		v = table->scales[i][0];
		for(j = 1; j < table->dims[i]; j++) {
			if(v >= table->scales[i][j]) {
				NDTable_set_error_message("Scale for dimension %d of is not strictly monotonic increasing at index %d", i, j);
				return -1;
			}
			v = table->scales[i][j];
		}
		
		if(table->offs[i] != offs[i]) {
			NDTable_set_error_message("The offset[%d] of must be %d but was %d", i, offs[i], table->offs[i]);
			return -1;
		}
	}
	
	// check the data for non-finite values
	for(i = 0; i < table->numel; i++) {
		if(!ISFINITE(table->data[i])) {
			NDTable_set_error_message("The data value at index %d of is not finite", i);
			return -1;
		}
	}

	return 0;
}

int NDTable_calculate_numel(int ndims, const int dims[]) {
	int i, numel = 1;

	for(i = 0; i < ndims; i++) {
		numel = numel * dims[i];
	}

	return numel;
}

void NDTable_ind2sub(const int index, const NDTable_h table, int subs[]) {
	int i, n = index; // number of remaining elements

	for(i = 0; i < table->ndims; i++) {
		subs[i] = n / table->offs[i];
		n -= subs[i] * table->offs[i];
	}
}

void NDTable_sub2ind(const int *subs, const NDTable_h table, int *index) {
	int i, k = 1;

	(*index) = 0;

	for(i = table->ndims-1; i >= 0; i--) {
		(*index) += subs[i] * k; 
		k *= table->dims[i]; // TODO use pre-calculated offsets
	}
}

double NDTable_get_value_subs(const NDTable_h table, const int subs[]) {
	int index;
	NDTable_sub2ind(subs, table, &index);
	return table->data[index];
}

NDTable_h NDTable_create_table(int ndims, const int *dims, const double *data, const double **scales) {
	int i, j;
	NDTable_h table = NULL;

	// check scales for strict monotonicity
	for(i = 0; i < ndims; i++) {
		for(j = 0; j < dims[i] - 1; j++) {
			if (scales[i][j] >= scales[i][j + 1]) {
				NDTable_set_error_message("The scale for dimension %d is not strictly monotonic at index %d", i + 1, j + 1);
				goto out;
			}
		}
	}

	table = NDTable_alloc_table();

	table->ndims = ndims;
	
	table->numel = NDTable_calculate_numel(ndims, dims);

	NDTable_calculate_offsets(ndims, dims, table->offs);

	table->data = (double *)malloc(table->numel * sizeof(double));
	memcpy(table->data, data, table->numel * sizeof(double));

	for(i = 0; i < ndims; i++) {
		table->dims[i] = dims[i];
		table->scales[i] = (double *)malloc(dims[i] * sizeof(double));
		memcpy(table->scales[i], scales[i], dims[i] * sizeof(double));
	}

out:
	return table;
}
