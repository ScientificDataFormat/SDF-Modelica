#define CATCH_CONFIG_MAIN

#include "catch_amalgamated.hpp"

#define MODELICA_SDF_API typedef

#include "ModelicaSDFFunctions.h"

using namespace Catch::Matchers;

#ifndef _WIN32
#include <dlfcn.h>
#define HMODULE void*
#else
#include <Windows.h>
#endif

template<typename T> T *get(HMODULE libraryHandle, const char *functionName) {

# ifdef _WIN32
	auto *fp = GetProcAddress(libraryHandle, functionName);
# else
	auto *fp = dlsym(libraryHandle, functionName);
# endif

	return reinterpret_cast<T *>(fp);
}


void check_data(double data[502][3]) {
	REQUIRE(data[0][0] == 0);
	REQUIRE(data[501][0] == 3);

	REQUIRE(data[0][1] == 7700);
	REQUIRE(data[501][1] == 7700);

	REQUIRE(data[0][2] == 0);
	REQUIRE(data[501][2] == -0.40858271718025208);
}


TEST_CASE("read from and write to an SDF file", "[functions]") {

	// load the shared library
# ifdef _WIN32
	auto l = LoadLibraryA(SHARED_LIBRARY_PATH);
# else
	auto l = dlopen(SHARED_LIBRARY_PATH, RTLD_LAZY);
# endif

	REQUIRE(l != nullptr);

	// get the functions
	auto get_table_data_size         = get<ModelicaSDF_get_table_data_size>         (l, "ModelicaSDF_get_table_data_size");
	auto read_table_data             = get<ModelicaSDF_read_table_data>             (l, "ModelicaSDF_read_table_data");
	auto get_time_series_size        = get<ModelicaSDF_get_time_series_size>        (l, "ModelicaSDF_get_time_series_size");
	auto read_time_series            = get<ModelicaSDF_read_time_series>            (l, "ModelicaSDF_read_time_series");
	auto create_group                = get<ModelicaSDF_create_group>                (l, "ModelicaSDF_create_group");
	auto get_dataset_dims            = get<ModelicaSDF_get_dataset_dims>            (l, "ModelicaSDF_get_dataset_dims");
	auto read_dataset_double         = get<ModelicaSDF_read_dataset_double>         (l, "ModelicaSDF_read_dataset_double");
	auto read_dataset_int            = get<ModelicaSDF_read_dataset_int>            (l, "ModelicaSDF_read_dataset_int");
	auto make_dataset_double         = get<ModelicaSDF_make_dataset_double>         (l, "ModelicaSDF_make_dataset_double");
	auto make_dataset_int            = get<ModelicaSDF_make_dataset_int>            (l, "ModelicaSDF_make_dataset_int");
	auto attach_scale                = get<ModelicaSDF_attach_scale>                (l, "ModelicaSDF_attach_scale");
	auto get_attribute_string_length = get<ModelicaSDF_get_attribute_string_length> (l, "ModelicaSDF_get_attribute_string_length");
	auto get_attribute_string        = get<ModelicaSDF_get_attribute_string>        (l, "ModelicaSDF_get_attribute_string");
	auto set_attribute_string        = get<ModelicaSDF_set_attribute_string>        (l, "ModelicaSDF_set_attribute_string");

	const auto filename = TESTS_DIR "test.sdf";

	double ds1_data = 1.1;

	int ds2_dims[1] = { 2 };
	double ds2_data[2] = { 2.1, 2.2 };

	int ds3_dims[2] = { 2, 3 };
	double ds3_data[2][3] = { { 3.1, 3.2, 3.3 },{ 3.4, 3.5, 3.6 } };

	int ds4_data = 4;

	int ds5_dims[1] = { 3 };
	int ds5_data[3] = { 1, 2, 3 };

	int ds6_dims[2] = { 2, 3 };
	int ds6_data[2][3] = { { 1, 2, 3 },{ 4, 5, 6 } };

	const char *error = nullptr;


	SECTION("write to an SDF file") {

		// clean up
		remove(filename);

		// write a (scalar) double dataset
		error = make_dataset_double(filename, "/DS1", 0, nullptr, &ds1_data, "Comment 1", "Dataset 1", "U1", "DU1", 1);
		CHECK_THAT(error, Equals(""));

		// set a string attribute
		error = set_attribute_string(filename, "/DS1", "A1", "Attribute 1");
		CHECK_THAT(error, Equals(""));

		// write a Real vector
		error = make_dataset_double(filename, "/DS2", 1, ds2_dims, ds2_data, "Comment 2", "Dataset 2", "U2", "DU2", 0);
		CHECK_THAT(error, Equals(""));

		// write a Real matrix
		error = make_dataset_double(filename, "/DS3", 2, ds3_dims, reinterpret_cast<double *>(ds3_data), "Comment 3", "Dataset 3", "U3", "DU3", 0);
		CHECK_THAT(error, Equals(""));

		// write an Integer scalar
		error = make_dataset_int(filename, "/DS4", 0, nullptr, &ds4_data, "Comment 4", "Dataset 4", "U4", "DU4", 0);
		CHECK_THAT(error, Equals(""));


		// write an Integer vector
		error = make_dataset_int(filename, "/DS5", 1, ds5_dims, ds5_data, "Comment 5", "Dataset 5", "U5", "DU5", 0);
		CHECK_THAT(error, Equals(""));

		// write an Integer matrix
		error = make_dataset_int(filename, "/DS6", 2, ds6_dims, reinterpret_cast<int *>(ds6_data), "Comment 6", "Dataset 6", "U6", "DU6", 0);
		CHECK_THAT(error, Equals(""));

		// attach /DS2 as scale for the second dimension to /DS3
		error = attach_scale(filename, "/DS3", "/DS2", "My scale", 1);
		CHECK_THAT(error, Equals(""));
	}

	SECTION("read from an SDF file") {

		// read a Real scalar
		double ds1_buf = 0.0;

		// read Real with wrong unit
		error = read_dataset_double(filename, "/DS1", "X1", &ds1_buf);
		CHECK_THAT(error, Equals("Attribute 'UNIT' in '/DS1' has the wrong value. Expected 'X1' but was 'U1'."));

		// read with correct unit
		error = read_dataset_double(filename, "/DS1", "U1", &ds1_buf);
		CHECK_THAT(error, Equals(""));
		CHECK(ds1_buf == 1.1);

		// read without unit
		error = read_dataset_double(filename, "/DS1", "", &ds1_buf);
		CHECK_THAT(error, Equals(""));
		CHECK(ds1_buf == 1.1);

		// get a string attribute
		int a1_size = -1;
		get_attribute_string_length(filename, "/DS1", "A1", &a1_size);
		CHECK_THAT(error, Equals(""));
		CHECK(a1_size == 12);

		char *a1_buf = (char *)calloc(a1_size, sizeof(char));
		error = get_attribute_string(filename, "/DS1", "A1", &a1_buf);
		CHECK_THAT(error, Equals(""));
		CHECK_THAT(a1_buf, Equals("Attribute 1"));
		free(a1_buf);

		// read a Real vector
		double ds2_buf[2] = {0.0};
		error = read_dataset_double(filename, "/DS2", "U2", ds2_buf);
		CHECK_THAT(error, Equals(""));
		CHECK(ds2_buf[0] == ds2_data[0]);
		CHECK(ds2_buf[1] == ds2_data[1]);

		// read a Real matrix
		double ds3_buf[2][3] = {0.0};
		int dims[32] = {0};

		error = get_dataset_dims(filename, "/DS3", dims);
		CHECK_THAT(error, Equals(""));
		CHECK(dims[0] == 2);
		CHECK(dims[1] == 3);

		error = read_dataset_double(filename, "/DS3", "U3", reinterpret_cast<double *>(ds3_buf));
		CHECK_THAT(error, Equals(""));
		CHECK(ds3_buf[0][0] == ds3_data[0][0]);
		CHECK(ds3_buf[0][1] == ds3_data[0][1]);
		CHECK(ds3_buf[0][2] == ds3_data[0][2]);
		CHECK(ds3_buf[1][0] == ds3_data[1][0]);
		CHECK(ds3_buf[1][1] == ds3_data[1][1]);
		CHECK(ds3_buf[1][2] == ds3_data[1][2]);

		// read an Integer scalar
		int ds4_buf[1] = {0};
		error = read_dataset_int(filename, "/DS4", "U4", ds4_buf);
		CHECK_THAT(error, Equals(""));
		CHECK(ds4_buf[0] == 4);

		// read an Integer scalar
		int ds5_buf[3] = {0};
		error = read_dataset_int(filename, "/DS5", "U5", ds5_buf);
		CHECK_THAT(error, Equals(""));
		CHECK(ds5_buf[0] == 1);
		CHECK(ds5_buf[1] == 2);
		CHECK(ds5_buf[2] == 3);

		// read an Integer matrix
		int ds6_buf[2][3] = {0};

		error = get_dataset_dims(filename, "/DS6", dims);
		CHECK_THAT(error, Equals(""));
		CHECK(dims[0] == 2);
		CHECK(dims[1] == 3);

		error = read_dataset_int(filename, "/DS6", "U6", reinterpret_cast<int *>(ds6_buf));
		CHECK_THAT(error, Equals(""));
		CHECK(ds6_buf[0][0] == ds6_data[0][0]);
		CHECK(ds6_buf[0][1] == ds6_data[0][1]);
		CHECK(ds6_buf[0][2] == ds6_data[0][2]);
		CHECK(ds6_buf[1][0] == ds6_data[1][0]);
		CHECK(ds6_buf[1][1] == ds6_data[1][1]);
		CHECK(ds6_buf[1][2] == ds6_data[1][2]);

		// create a group
		error = create_group(filename, "/G1", "Group 1");
		CHECK_THAT(error, Equals(""));

		// write a Real scalar to G1
		double ds7_data = 7;
		error = make_dataset_double(filename, "/G1/DS7", 0, nullptr, &ds7_data, "", "", "", "", 0);
		CHECK_THAT(error, Equals(""));

		// read the Real scalar from G1
		double ds7_buf = 0;
		error = read_dataset_double(filename, "/G1/DS7", "", &ds7_buf);
		CHECK_THAT(error, Equals(""));
		CHECK(ds7_buf == ds7_data);
	}

	SECTION("read time series") {

		const char *dataset_names[2] = { "/boxBody1/density", "/boxBody1/frame_a/t[3]" };
		const char *dataset_units[2] = { "kg/m3", "N.m" };
		const char *scale_unit = "s";
		int nsamples = 1000;
		double data[502][3] = { 0 };
		int size = -1;

		SECTION("with illegal file name") {
			const char *fname = TESTS_DIR "DoesNotExist.mat";

			const char *message = get_time_series_size(fname, dataset_names, &size);
			REQUIRE_THAT(message, Equals("Failed to open '" TESTS_DIR "DoesNotExist.mat'"));

			message = read_time_series(fname, 2, dataset_names, dataset_units, scale_unit, size, reinterpret_cast<double *>(data));
			REQUIRE_THAT(message, Equals("Failed to open '" TESTS_DIR "DoesNotExist.mat'"));
		}

		SECTION("with scale unit") {
			const char *fname = TESTS_DIR "DoublePendulum_Dymola-2012.mat";

			scale_unit = "ms";

			const char *message = get_time_series_size(fname, dataset_names, &size);
			REQUIRE_THAT(message, Equals(""));
			REQUIRE(size == 502);

			message = read_time_series(fname, 2, dataset_names, dataset_units, scale_unit, size, reinterpret_cast<double *>(data));
			REQUIRE_THAT(message, Equals("The scale in '" TESTS_DIR "DoublePendulum_Dymola-2012.mat' has the wrong unit. Expected 'ms' but was 's'."));
		}

		SECTION("with wrong dataset unit") {
			const char *fname = TESTS_DIR "DoublePendulum_Dymola-2012.mat";

			dataset_units[0] = "";
			dataset_units[1] = "A.s";

			const char *message = get_time_series_size(fname, dataset_names, &size);
			REQUIRE_THAT(message, Equals(""));
			REQUIRE(size == 502);

			message = read_time_series(fname, 2, dataset_names, dataset_units, scale_unit, size, reinterpret_cast<double *>(data));
			REQUIRE_THAT(message, Equals("Variable '/boxBody1/frame_a/t[3]' in '" TESTS_DIR "DoublePendulum_Dymola-2012.mat' has the wrong unit. Expected 'A.s' but was 'N.m'."));
		}

		SECTION("Dymola 2012") {
			const char *fname = TESTS_DIR "DoublePendulum_Dymola-2012.mat";

			const char *message = get_time_series_size(fname, dataset_names, &size);
			REQUIRE_THAT(message, Equals(""));
			REQUIRE(size == 502);

			message = read_time_series(fname, 2, dataset_names, dataset_units, scale_unit, size, reinterpret_cast<double *>(data));
			REQUIRE_THAT(message, Equals(""));
			check_data(data);
		}

		SECTION("Dymola 2012 (Save as)") {
			const char *fname = TESTS_DIR "DoublePendulum_Dymola-2012-SaveAs.mat";

			const char *message = get_time_series_size(fname, dataset_names, &size);
			REQUIRE_THAT(message, Equals(""));
			REQUIRE(size == 502);

			message = read_time_series(fname, 2, dataset_names, dataset_units, scale_unit, size, reinterpret_cast<double *>(data));
			REQUIRE_THAT(message, Equals(""));
			check_data(data);
		}

		SECTION("Dymola 7.4") {
			const char *fname = TESTS_DIR "DoublePendulum_Dymola-7.4.mat";

			const char *message = get_time_series_size(fname, dataset_names, &size);
			REQUIRE_THAT(message, Equals(""));
			REQUIRE(size == 502);

			message = read_time_series(fname, 2, dataset_names, dataset_units, scale_unit, size, reinterpret_cast<double *>(data));
			REQUIRE_THAT(message, Equals(""));
			check_data(data);
		}

		SECTION("Dymola 2012 Save as plotted (not supported)") {
			const char *fname = TESTS_DIR "DoublePendulum_Dymola-2012-SaveAsPlotted.mat";

			const char * message = get_time_series_size(fname, dataset_names, &size);
			REQUIRE_THAT(message, Equals("'" TESTS_DIR "DoublePendulum_Dymola-2012-SaveAsPlotted.mat' has an unsupported file structure"));

			message = read_time_series(fname, 2, dataset_names, dataset_units, scale_unit, size, reinterpret_cast<double *>(data));
			REQUIRE_THAT(message, Equals("'" TESTS_DIR "DoublePendulum_Dymola-2012-SaveAsPlotted.mat' has an unsupported file structure"));
		}

	}

	// free the shared library
# ifdef _WIN32
	FreeLibrary(l);
# else
	// TODO
# endif

}
