#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

#include "hdf5.h"
#include "hdf5_hl.h"

#include "ModelicaSDFFunctions.h"


#define COMMENT_ATTR_NAME           "COMMENT"
#define DISPLAY_NAME_ATTR_NAME      "NAME"
#define UNIT_ATTR_NAME              "UNIT"
#define DISPLAY_UNIT_ATTR_NAME      "DISPLAY_UNIT"
#define RELATIVE_QUANTITY_ATTR_NAME "RELATIVE_QUANTITY"

#define MAX_MESSAGE_LENGTH 4096


char error_message[MAX_MESSAGE_LENGTH];

static void configureMessageHandling() {
//#ifndef _DEBUG
	// turn off automatic error message printing
	H5Eset_auto1(NULL, NULL);
//#endif
}

void set_error_message(const char *msg, ...) {
	va_list vargs;
	va_start(vargs, msg);
	vsnprintf(error_message, MAX_MESSAGE_LENGTH, msg, vargs);
	va_end(vargs);
}

static herr_t delete_dataset(hid_t loc_id, const char *dataset_name) {
	
	int rank;

	// delete the dataset if it already exists
	if (H5LTget_dataset_ndims(loc_id, dataset_name, &rank) == 0) {
		if (H5Ldelete(loc_id, dataset_name, H5P_DEFAULT) < 0) {
			set_error_message("Failed to delete dataset '%s'", dataset_name);
			return -1;
		}
	}

	return 0;
}

static hid_t open_or_create_file(const char *filename) {

	hid_t file_id = -1;

	// open the file
	if ((file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
		
		// create a new one if it does not exist
		if ((file_id = H5Fcreate(filename, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT)) < 0) {

			set_error_message("Failed to create file '%s'", filename); 

			return -1;
		}
	}

	return file_id;
}

// TODO: change to assert_unit()
static int assert_string_attribute(hid_t loc_id, const char *obj_name, const char *attr_name, const char *attr_value) {
	
	H5T_class_t type_class = H5T_NO_CLASS;
	size_t type_size = 0;
	int rank = -1;
	char *buffer = NULL;
	size_t len1, len2;
	const char * qualified_name = obj_name;
	char name_buffer[100];
	int status = 0;
	
	// get a proper name for the error message
	if (strcmp(obj_name, ".") == 0) {
		H5Iget_name(loc_id, name_buffer, 100);
		qualified_name = name_buffer;
	}

	if (H5LTget_attribute_info(loc_id, obj_name, attr_name, NULL, &type_class, &type_size) < 0) {
		set_error_message("Missing required attribute '%s' in '%s' (expected '%s')", attr_name, qualified_name, attr_value);
		status = 1;
		goto out;
	}

	if (H5LTget_attribute_ndims(loc_id, obj_name, attr_name, &rank) < 0) {
		set_error_message("Failed to retrieve dimensions for attribute '%s' in '%s' (expected '%s')", attr_name, qualified_name, attr_value);
		status = 1;
		goto out;
	}

	// check the rank
	if (rank > 1) {
		set_error_message("Expected rank less or equal 1 for attribute '%s' in '%s' but was %d", attr_name, qualified_name, rank);
		status = 1;
		goto out;
	}

	// check the type
	if (type_class != H5T_STRING) {
		set_error_message("Attribute '%s' in '%s' has the wrong type. Expected H5T_STRING (=5) but was %d.", attr_name, qualified_name, type_class);
		status = 1;
		goto out;
	}

	buffer = (char *)calloc((type_size + 1), sizeof(char));

	if (H5LTget_attribute_string(loc_id, obj_name, attr_name, buffer) < 0) {
		set_error_message("Failed to read attribute '%s' in '%s'", attr_name, qualified_name);
		status = 1;
		goto out;
	}

	// compare the lengths first (HDF5 strings sometimes come in Fortran format)
	len1 = strlen(attr_value);
	len2 = strlen(buffer);

    if(len2 > type_size) {
		len2 = type_size;
	}
	
	if (len1 != len2 || strncmp(attr_value, buffer, type_size) != 0) {
		set_error_message("Attribute '%s' in '%s' has the wrong value. Expected '%s' but was '%.*s'.", attr_name, qualified_name, attr_value, type_size, buffer);
		status = 1;
	}

out:
	free(buffer);

	return status;
}

static herr_t visit_time_series_scale(hid_t dset, unsigned dim, hid_t scale, void *visitor_data) {
	
	size_t len = H5Iget_name(scale, NULL, 0);
	
	*(char **)visitor_data = (char *)calloc(len + 1, sizeof(char));

	H5Iget_name(scale, *(char **)visitor_data, len + 1);

	return 1;
}

static char *get_scale_name(hid_t file_id, const char *dataset_name, unsigned int dim) {

	hid_t dset_id = H5I_INVALID_HID;
	char *scale_name = NULL; 
	int idx = 0;

	// get the dataset id
	if ((dset_id = H5Oopen(file_id, dataset_name, H5P_DEFAULT)) < 0) {
		goto out;
	}

	if (H5DSiterate_scales(dset_id, dim, &idx, visit_time_series_scale, &scale_name) < 0) {
		goto out;
	}

out:
	H5Oclose(dset_id);

	return scale_name;
}

static int check_dataset_1d(hid_t file_id, const char *dataset_name, const char *unit, hsize_t numel) {

	H5T_class_t type_class = H5T_NO_CLASS;
	size_t type_size = 0;
	hsize_t dims[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int ndims = -1;

	if (H5LTget_dataset_ndims(file_id, dataset_name, &ndims) < 0) {
		set_error_message("Failed to get dimensions for dataset '%s'", dataset_name);
		return 1;
	}

	if (ndims != 1) {
		set_error_message("Dataset '%s' is not one-dimensional", dataset_name);
		return 1;
	}

	if (H5LTget_dataset_info(file_id, dataset_name, dims, &type_class, &type_size) < 0) {
		set_error_message("Failed to get info for dataset '%s'", dataset_name);
		return 1;
	}

	if (dims[0] != numel) {
		set_error_message("Dataset '%s' has the wrong number of elements", dataset_name);
		return 1;
	}
		
	// check the unit
	if (unit != NULL && strlen(unit) > 0 && assert_string_attribute(file_id, dataset_name, UNIT_ATTR_NAME, unit)) {
		return 1;
	}

	return 0;
}

static herr_t set_dataset_attributes(hid_t handle, const char *filename, const char *dataset_name, const char *comment, const char *display_name, const char *unit, const char *display_unit, int relative_quantity) {

	// set the comment
	if (comment != NULL && strlen(comment) > 0) {
		if (H5LTset_attribute_string(handle, dataset_name, COMMENT_ATTR_NAME, comment) < 0) {
			set_error_message("Failed to set attribute COMMENT for dataset %s in %s", dataset_name, filename);
			return -1;
		}
	}

	// set the display_name
	if (display_name != NULL && strlen(display_name) > 0) {
		if (H5LTset_attribute_string(handle, dataset_name, DISPLAY_NAME_ATTR_NAME, display_name) < 0) {
			set_error_message("Failed to set attribute NAME for dataset %s in %s", dataset_name, filename);
			return -1;
		}
	}

	// set the unit
	if (unit != NULL && strlen(unit) > 0) {
		if (H5LTset_attribute_string(handle, dataset_name, UNIT_ATTR_NAME, unit) < 0) {
			set_error_message("Failed to set attribute UNIT for dataset %s in %s", dataset_name, filename);
			return -1;
		}
	}

	// set the display_unit
	if (display_unit != NULL && strlen(display_unit) > 0) {
		if (H5LTset_attribute_string(handle, dataset_name, DISPLAY_UNIT_ATTR_NAME, display_unit) < 0) {
			set_error_message("Failed to set attribute DISPLAY_UNIT for dataset %s in %s", dataset_name, filename);
			return -1;
		}
	}
	
	// set the relative quantity
	if (relative_quantity != 0) {
		if (H5LTset_attribute_string(handle, dataset_name, RELATIVE_QUANTITY_ATTR_NAME, "TRUE") < 0) {
			set_error_message("Failed to set attribute RELATIVE_QUANTITY for dataset %s in %s", dataset_name, filename);
			return -1;
		}
	}

	return 0;
}

const char * ModelicaSDF_get_table_data_size(const char *filename, const char *dataset_name, int *size) {
	
	hid_t file_id = H5I_INVALID_HID;
	H5T_class_t type_class = H5T_NO_CLASS;
	size_t type_size = 0;
	hsize_t dims[32] = {0};
	int i = -1, ndims = -1, ndata = -1;

	configureMessageHandling();
	
	set_error_message("");

	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open file '%s'", filename);
		goto out;
	}

	if (H5LTget_dataset_ndims(file_id, dataset_name, &ndims) < 0) {
		set_error_message("Failed to open dataset '%s' in '%s'", dataset_name, filename);
		goto out;
	}

	if (H5LTget_dataset_info(file_id, dataset_name, dims, &type_class, &type_size) < 0) {
		set_error_message("Failed to open dataset '%s' in '%s'", dataset_name, filename);
		goto out;
	}

	*size = 1 + ndims;

	ndata = 1;

	for (i = 0; i < ndims; i++) {
		*size += (int)dims[i];
		ndata *= (int)dims[i];
	}

	*size += ndata;

out:
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_read_table_data(const char *filename, const char *dataset_name, const int ndims, const char *unit, const char **scale_units, double *data) {
	
	hid_t file_id = H5I_INVALID_HID;
	H5T_class_t type_class = H5T_NO_CLASS;
	size_t type_size = 0;
	hsize_t dims[32] = {0};
	int rank = -1, i = -1, j = -1;
	const char *scale_name = NULL;

	configureMessageHandling();
	
	set_error_message("");
	
	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open file '%s'", filename);
		goto out;
	}

	if (H5LTget_dataset_ndims(file_id, dataset_name, &rank) < 0) {
		set_error_message("Failed to open dataset '%s' in '%s'", dataset_name, filename);
		goto out;
	}

	if (rank != ndims) {
		set_error_message("Dataset '%s' in '%s' has the wrong number of dimension. Expected %d but was %d.", dataset_name, filename, ndims, rank);
		goto out;
	}

	if (H5LTget_dataset_info(file_id, dataset_name, dims, &type_class, &type_size) < 0) {
		set_error_message("Failed to open dataset '%s' in '%s'", dataset_name, filename);
		goto out;
	}

	*data++ = ndims;

	for (i = 0; i < ndims; i++) {
		*data++ = (int)dims[i];
	}

	// read scales
	for (i = 0; i < ndims; i++) {

		scale_name = get_scale_name(file_id, dataset_name, i);

		if (!scale_name) {
			set_error_message("Dataset '%s' in '%s' has no scale for dimension %d", dataset_name, filename, i + 1);
			goto out;
		}

		if (check_dataset_1d(file_id, scale_name, scale_units[i], dims[i])) {
			goto out;
		}

		if (H5LTread_dataset_double(file_id, scale_name, data) < 0) {
			set_error_message("Failed to read dataset '%s' in '%s'", scale_name, filename);
			goto out;
		}

		// check monotonicity
		for (j = 0; j < dims[i] - 1; j++) {
			if (data[j] >= data[j+1]) {
				set_error_message("Scale '%s' in '%s' is not strictly monotonic increasing", scale_name, filename);
				goto out;
			}
		}

		data += dims[i];

		free((void *)scale_name);
		scale_name = NULL;
	}

	// read data
	if(H5LTread_dataset_double(file_id, dataset_name, data) < 0) {
		set_error_message("Failed to read dataset '%s' in '%s'", dataset_name, filename);
		goto out;
	}

out:
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_get_time_series_size(const char *filename, const char **dataset_names, int *size) {
	
	hid_t file_id = H5I_INVALID_HID;
	H5T_class_t type_class = H5T_NO_CLASS;
	size_t type_size = 0;
	hsize_t dims[32] = {0};
	int ndims = -1;

	set_error_message("");

	if (strlen(filename) > 4 && !strcmp(filename + strlen(filename) - 4, ".mat")) {
		get_time_series_size_dsres(filename, dataset_names, size);
		goto out;
	}

	configureMessageHandling();	

	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open file '%s'", filename);
		goto out;
	}

	if (H5LTget_dataset_ndims(file_id, dataset_names[0], &ndims) < 0) {
		set_error_message("Failed to open dataset '%s' in '%s'", dataset_names[0], filename);
		goto out;
	}

	if (ndims != 1) {
		set_error_message("Dataset '%s' in '%s' is not one-dimensional", dataset_names[0], filename);
		goto out;
	}

	if (H5LTget_dataset_info(file_id, dataset_names[0], dims, &type_class, &type_size) < 0) {
		set_error_message("Failed to open dataset '%s' in '%s'", dataset_names[0], filename);
		goto out;
	}

	*size = (int)dims[0];

out:
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_read_time_series(const char *filename, const int ndatasets, const char **dataset_names, const char **dataset_units, const char *scale_unit, int nsamples, double *data) {
	
	hid_t file_id = H5I_INVALID_HID;
	int i, j;
	char *first_scale_name = NULL;
	char *scale_name = NULL;
	double *buffer = NULL;

	configureMessageHandling();
	
	set_error_message("");

	if (strlen(filename) > 4 && !strcmp(filename + strlen(filename) - 4, ".mat")) {
		read_dsres(filename, ndatasets, dataset_names, dataset_units, scale_unit, nsamples, data);
		return error_message;
	}

	// check number of datasets
	if (ndatasets < 1) {
		set_error_message("Number of datasets must be > 0");
		goto out;
	}

	// check number of samples
	if (nsamples < 1) {
		set_error_message("Number of samples must be > 0");
		goto out;
	}

	// open the file
	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open '%s'", filename);
		goto out;
	}

	buffer = (double *)malloc(sizeof(double) * nsamples);

	// iterate over the datasets
	for (i = 0; i < ndatasets; i++) {
		
		if (i == 0) {

			first_scale_name = get_scale_name(file_id, dataset_names[i], 0);

			if (!first_scale_name) {
				set_error_message("Dataset '%s' in '%s' has no scale", dataset_names[i], filename);
				goto out;
			}

			// check size and unit
			if (check_dataset_1d(file_id, first_scale_name, scale_unit, nsamples)) {
				goto out;
			}
			
			// read time from scale
			if (H5LTread_dataset_double(file_id, first_scale_name, buffer) < 0) {
				set_error_message("Failed to read dataset '%s' in '%s'", scale_name, filename);
				goto out;
			}
		
			// store the time
			for (j = 0; j < nsamples; j++) {
				data[j * (ndatasets + 1)] = buffer[j];
			}

		} else {

			scale_name = get_scale_name(file_id, dataset_names[i], 0);

			if (!scale_name) {
				set_error_message("Dataset '%s' in '%s' has no scale", dataset_names[i], filename);
				goto out;
			}

			// make sure the dataset has the same scale
			if (strcmp(scale_name, first_scale_name)) {
				set_error_message("Dataset '%s' in '%s' must have the same scale as the previous dataset", dataset_names[i], filename);
				goto out;
			}

			free(scale_name);
		}

		// check size and unit
		if (check_dataset_1d(file_id, dataset_names[i], dataset_units[i], nsamples)) {
			goto out;
		}

		// read the data
		if (H5LTread_dataset_double(file_id, dataset_names[i], buffer) < 0) {
			set_error_message("Failed to read dataset '%s' in '%s'", dataset_names[i], filename);
			goto out;
		}
		
		// add the data
		for (j = 0; j < nsamples; j++) {
			data[j * (ndatasets + 1) + 1 + i] = buffer[j];
		}

	}

out:

	free(buffer);
	free(first_scale_name);

	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_create_group(const char *filename, const char *group_name, const char *comment) {
	
	hid_t file_id = H5I_INVALID_HID;
	hid_t group_id = H5I_INVALID_HID;

	configureMessageHandling();
	
	set_error_message("");

	// open the file
	if ((file_id = open_or_create_file(filename)) < 0) {
		goto out;
	}

	// create the group
	if((group_id = H5Gopen2(file_id, group_name, H5P_DEFAULT)) < 0) {
		// create the group if it does not exist
		if((group_id = H5Gcreate2(file_id, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) { 
			set_error_message("Failed to create group '%s' in '%s'", group_name, filename);
			goto out; 
		}
	}

	// set the comment
	if(comment && strlen(comment) > 0) {
		if(H5LTset_attribute_string(group_id, ".", COMMENT_ATTR_NAME, comment) < 0) { 
			set_error_message("Failed to set comment for group '%s' in '%s'", group_name, filename);
		}
	}

out:
	if (group_id >= 0) H5Gclose(group_id);
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_get_dataset_dims(const char *filename, const char *dataset_name, int dims[]) {

	hid_t file_id = H5I_INVALID_HID;
	H5T_class_t type_class = H5T_NO_CLASS;
	size_t size = 0;
	hsize_t dimsbuf[32] = {0};
	int i = -1;

	configureMessageHandling();
	
	set_error_message("");

	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open file '%s'", filename);
		goto out;
	}

	if (H5LTget_dataset_info(file_id, dataset_name, dimsbuf, &type_class, &size) < 0) {
		set_error_message("Failed to open dataset '%s' in '%s'", dataset_name, filename);
		goto out;
	}

	for (i = 0; i < 32; i++) {
		dims[i] = (int)dimsbuf[i];
	}

out:
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_read_dataset_double(const char *filename, const char *dataset_name, const char *unit, double *buffer) {

	hid_t file_id = H5I_INVALID_HID;

	configureMessageHandling();
	
	set_error_message("");

	// open the file
	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open '%s'", filename);
		goto out;
	}

	// read the dataset
	if (H5LTread_dataset_double(file_id, dataset_name, buffer) < 0) {
		set_error_message("Failed to read double dataset '%s' from '%s'", dataset_name, filename);
		goto out;
	}

	// check the unit
	if (unit != NULL && strlen(unit) > 0 && assert_string_attribute(file_id, dataset_name, UNIT_ATTR_NAME, unit) != 0) {
		goto out;
	}

out:
	// close the file
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_read_dataset_int(const char *filename, const char *dataset_name, const char *unit, int *buffer) {
	
	hid_t file_id = H5I_INVALID_HID;

	configureMessageHandling();
	
	set_error_message("");
	
	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open '%s'", filename);
		goto out;
	}

	if (H5LTread_dataset_int(file_id, dataset_name, buffer) < 0) {
		set_error_message("Failed to read integer dataset '%s' from '%s'", dataset_name, filename);
		goto out;
	}

	// check the unit
	if (unit != NULL && strlen(unit) > 0 && assert_string_attribute(file_id, dataset_name, UNIT_ATTR_NAME, unit)) {
		goto out;
	}

out:
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_make_dataset_double(
	
	const char *filename,
	const char *dataset_name,
	int ndims,
	const int dims[],
	const double *data,
	const char *comment,
	const char *display_name,
	const char *unit,
	const char *display_unit,
	int relative_quantity) {

	hid_t file_id = H5I_INVALID_HID;
	int i = -1;
	hsize_t dimsbuf[32] = {0};

	configureMessageHandling();

	set_error_message("");
	
	for (i = 0; i < ndims; i++) {
		dimsbuf[i] = (hsize_t)dims[i];
	}

	// open the file
	if ((file_id = open_or_create_file(filename)) < 0) {
		goto out;
	}

	if (delete_dataset(file_id, dataset_name) < 0) {
		// delete_dataset() will set the error message
		goto out;
	}

	if (H5LTmake_dataset_double(file_id, dataset_name, ndims, dimsbuf, data) < 0) {
		set_error_message("Failed to create dataset %s in %s", dataset_name, filename);
		goto out;
	}

	if (set_dataset_attributes(file_id, filename, dataset_name, comment, display_name, unit, display_unit, relative_quantity) < 0) {
		// set_dataset_attributes() will set the error message
		goto out;
	}

out:
	// close the file
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char *   ModelicaSDF_make_dataset_int(
	const char *filename,
	const char *dataset_name,
	int ndims,
	const int dims[],
	const int *data,
	const char *comment,
	const char *display_name,
	const char *unit,
	const char *display_unit,
	int relative_quantity) {

	hid_t file_id = H5I_INVALID_HID;
	int i = -1;
	hsize_t dimsbuf[32] = {0};

	configureMessageHandling();
	
	set_error_message("");

	for (i = 0; i < ndims; i++) {
		dimsbuf[i] = (hsize_t)dims[i];
	}

	// open the file
	if ((file_id = open_or_create_file(filename)) < 0) {
		goto out;
	}

	if (delete_dataset(file_id, dataset_name) < 0) {
		// delete_dataset() will set the error message
		goto out;
	}

	if (H5LTmake_dataset_int(file_id, dataset_name, ndims, dimsbuf, data) < 0) {
		set_error_message("Failed to create dataset %s in %s", dataset_name, filename);
		goto out;
	}
	
	if (set_dataset_attributes(file_id, filename, dataset_name, comment, display_name, unit, display_unit, relative_quantity) < 0) {
		// set_dataset_attributes() will set the error message
		goto out;
	}

out:
	// close the file
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_attach_scale(const char *filename, const char *dataset_name, const char *scale_name, const char *dim_name, int dim) {
	
	hid_t file_id  = H5I_INVALID_HID;
	hid_t dset_id  = H5I_INVALID_HID;
	hid_t scale_id = H5I_INVALID_HID;

	configureMessageHandling();

	set_error_message("");

	if ((file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open '%s'", dataset_name, filename);
		goto out;
	}

	if ((dset_id = H5Dopen2(file_id, dataset_name, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open dataset '%s'", dataset_name);
		goto out;
	}

	if ((scale_id = H5Dopen2(file_id, scale_name, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open dataset '%s'", scale_name);
		goto out;
	}

	if (dim_name && strlen(dim_name) == 0) {
		dim_name = NULL;
	}

	if (H5DSset_scale(scale_id, dim_name) < 0) {
		set_error_message("Failed to set scale on '%s'", scale_name);
		goto out;
	}

	if (H5DSattach_scale(dset_id, scale_id, dim) < 0) {
		set_error_message("Failed to attach scale");
		goto out;
	}

out:
	if (scale_id >= 0) H5Dclose(scale_id);
	if (dset_id >= 0) H5Dclose(dset_id);
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_get_attribute_string_length(const char *filename, const char *dataset_name, const char *attr_name, int *size) {
	
	hid_t file_id = H5I_INVALID_HID;
	hsize_t dims = 0;
	H5T_class_t type_class = H5T_NO_CLASS;
	size_t type_size = 0;

	configureMessageHandling();

	set_error_message("");

	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open %s", filename);
		goto out;
	}

	if (H5LTget_attribute_info(file_id, dataset_name, attr_name, &dims, &type_class, &type_size) < 0) {
		set_error_message("Failed to retrieve information for attribute '%s' of dataset '%s' in %s", attr_name, dataset_name, filename);
		goto out;
	}

	if (type_class != H5T_STRING) {
		set_error_message("Attribute '%s' of dataset '%s' in %s is not a string", attr_name, dataset_name, filename);
		goto out;
	}
	
	*size = type_size;

out:
	if (file_id >= 0)  H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_get_attribute_string(const char *filename, const char *dataset_name, const char *attr_name, char **buffer) {

	hid_t file_id = H5I_INVALID_HID;

	configureMessageHandling();

	set_error_message("");

	if ((file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open %s", filename);
		goto out;
	}

	if (H5LTget_attribute_string(file_id, dataset_name, attr_name, *buffer) < 0) {
		set_error_message("Failed to get string attribute '%s' of dataset '%s' in %s", attr_name, dataset_name, filename);
		goto out;
	}

out:
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}

const char * ModelicaSDF_set_attribute_string(const char *filename, const char *dataset_name, const char *attr_name, const char *data) {
	
	hid_t file_id = H5I_INVALID_HID;
	
	configureMessageHandling();

	set_error_message("");

	if ((file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
		set_error_message("Failed to open %s", filename);
		goto out;
	}

	if (H5LTset_attribute_string(file_id, dataset_name, attr_name, data) < 0) {
		set_error_message("Failed to set string attribute '%s' of dataset '%s' in %s", attr_name, dataset_name, filename);
		goto out;
	}

out:
	if (file_id >= 0) H5Fclose(file_id);

	return error_message;
}
