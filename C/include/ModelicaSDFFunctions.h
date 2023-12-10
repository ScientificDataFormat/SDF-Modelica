#ifndef MODELICAHDF5FUNCTIONS_H_
#define MODELICAHDF5FUNCTIONS_H_

#ifndef MODELICA_SDF_API

#ifdef _WIN32
#define MODELICA_SDF_API __declspec(dllexport)
#else
#define MODELICA_SDF_API
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

void read_dsres(const char *filename, const int ndatasets, const char **dataset_names, const char **dataset_units, const char *scale_unit, int nsamples, double *data);

void get_time_series_size_dsres(const char *filename, const char **dataset_names, int *size);

//extern char *error_message;

#define MAX_MESSAGE_LENGTH 4096

extern char error_message[MAX_MESSAGE_LENGTH];

void set_error_message(const char *msg, ...);


MODELICA_SDF_API const char * ModelicaSDF_get_table_data_size(const char *filename, const char *dataset_name, int *size);

MODELICA_SDF_API const char * ModelicaSDF_read_table_data(const char *filename, const char *dataset_name, const int ndims, const char *unit, const char **scale_units, double *data);


MODELICA_SDF_API const char * ModelicaSDF_get_time_series_size(const char *filename, const char **dataset_names, int *size);

MODELICA_SDF_API const char * ModelicaSDF_read_time_series(const char *filename, const int ndatasets, const char **dataset_names, const char **dataset_units, const char *scale_unit, int nsamples, double *data);


MODELICA_SDF_API const char * ModelicaSDF_create_group(const char *filename, const char *group_name, const char *comment);

/*! Retrieves the dimensions of a dataset
 * 
 * @param [in]	filename		the file name
 * @param [in]	dataset_name	the dataset name
 * @param [out]	dims			an int[32] for the dimensions
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char * ModelicaSDF_get_dataset_dims(const char *filename, const char *dataset_name, int dims[]);

/*! Reads the values of a double dataset
 * 
 * @param [in]	filename		the file name
 * @param [in]	dataset_name	the dataset name
 * @param [in]	unit			the expected unit
 * @param [out]	buffer			a buffer for the values
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char * ModelicaSDF_read_dataset_double(const char *filename, const char *dataset_name, const char *unit, double *buffer);

/*! Reads the values of a integer dataset
 * 
 * @param [in]	filename		the file name
 * @param [in]	dataset_name	the dataset name
 * @param [in]	unit			the expected unit
 * @param [out]	buffer			a buffer for the values
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char * ModelicaSDF_read_dataset_int(const char *filename, const char *dataset_name, const char *unit, int *buffer);

/*! Writes a double dataset with unit and comment
 * 
 * @param [in]	filename			the file name
 * @param [in]	dataset_name		the dataset name
 * @param [in]	ndims				the number of dimensions
 * @param [in]	dims				the dimensions
 * @param [in]	data				a buffer for the values
 * @param [in]	relative_quantity	absolute if 0, otherwise relative
 * @param [in]	unit				the unit (optional)
 * @param [in]	display_unit		the display unit (optional)
 * @param [in]	comment				the comment (optional)
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char * ModelicaSDF_make_dataset_double(
	const char *filename, 
	const char *dataset_name, 
	int ndims, 
	const int dims[], 
	const double *data, 
	const char *comment, 
	const char *display_name, 
	const char *unit, 
	const char *display_unit, 
	int relative_quantity);

/*! Writes an integer dataset with unit and comment
 * 
 * @param [in]	filename			the file name
 * @param [in]	dataset_name		the dataset name
 * @param [in]	ndims				the number of dimensions
 * @param [in]	dims				the dimensions
 * @param [in]	data				a buffer for the values
 * @param [in]	relative_quantity	absolute if 0, otherwise relative
 * @param [in]	unit				the unit (optional)
 * @param [in]	display_unit		the display unit (optional)
 * @param [in]	comment				the comment (optional)
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char *  ModelicaSDF_make_dataset_int(
	const char *filename,
	const char *dataset_name,
	int ndims,
	const int dims[],
	const int *data,
	const char *comment,
	const char *display_name,
	const char *unit,
	const char *display_unit,
	int relative_quantity);

/*! Sets a dataset as the scale for the dimension of another dataset
 * 
 * @param [in]	filename		the file name
 * @param [in]	dataset_name	the name of the dataset
 * @param [in]	scale_name		the name of the scale dataset
 * @param [in]	dim_name		the name of the dimension
 * @param [in]	dim				the index of the dimension
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char *  ModelicaSDF_attach_scale(const char *filename, const char *dataset_name, const char *scale_name, const char *dim_name, int dim);

/*! Gets the length of a string attribute's value
 * 
 * @param [in]	filename		the file name
 * @param [in]	dataset_name	the name of the dataset
 * @param [in]	attr_name		the name of the attribute
 * @param [out]	size		    the length of the string attribute's value
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char *  ModelicaSDF_get_attribute_string_length(const char *filename, const char *dataset_name, const char *attr_name, int *size);

/*! Gets the value of a string attribute of a dataset
 * 
 * @param [in]	filename		the file name
 * @param [in]	dataset_name	the name of the dataset
 * @param [in]	attr_name		the name of the attribute
 * @param [out]	data			the value of the attribute
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char *  ModelicaSDF_get_attribute_string(const char *filename, const char *dataset_name, const char *attr_name, char **data);

/*! Sets the value of a string attribute of a dataset
 * 
 * @param [in]	filename		the file name
 * @param [in]	dataset_name	the name of the dataset
 * @param [in]	attr_name		the name of the attribute
 * @param [in]	data			the value of the attribute
 *
 * @return		the error message ("" on success)
 */
MODELICA_SDF_API const char *  ModelicaSDF_set_attribute_string(const char *filename, const char *dataset_name, const char *attr_name, const char *data);


#ifdef __cplusplus
}
#endif

#endif /*MODELICAHDF5FUNCTIONS_H_*/