#include "matio.h"

#include "ModelicaSDFFunctions.h"
#include <string.h>

#include <vector>
#include <string>


using namespace std;


#include <algorithm>  // for std::find_if
#include <cctype>     // for std::isspace


std::string & rtrim(std::string & str)
{
	auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace(ch); });
	str.erase(it1.base(), str.end());
	return str;
}

vector<string> readStringMatrix(matvar_t *matvar, bool transpose) {

	vector<string> s;

	const size_t m = matvar->dims[0];
	const size_t n = matvar->dims[1];
	auto data = static_cast<const char *>(matvar->data);

	if (transpose) {

		for (int i = 0; i < n; i++) {
			string s_(&data[i * m], m);
			rtrim(s_);
			s.push_back(s_);
		}

	} else {

		for (int i = 0; i < m; i++) {

			vector<char> buf(n + 1, '\0');

			for (int j = 0; j < n; j++) {
				buf[j] = data[j * m + i];
			}

			string s_(buf.data());
			rtrim(s_);
			s.push_back(s_);
		}

	}

	return s;
}

mat_t *open_mat_file(const char *filename, bool *trans) {
	
	auto matfp = Mat_Open(filename, MAT_ACC_RDONLY);

	if (!matfp) {
		set_error_message("Failed to open '%s'", filename);
		return nullptr;
	}

	if (Mat_GetVersion(matfp) != MAT_FT_MAT4) {
		set_error_message("'%s' has an unsupported MAT file version", filename);
		return nullptr;
	}


	auto Aclass   = Mat_VarRead(matfp, "Aclass");
	auto dataInfo = Mat_VarRead(matfp, "dataInfo");
	auto name     = Mat_VarRead(matfp, "name");
	auto desc     = Mat_VarRead(matfp, "description");
	auto data_1   = Mat_VarRead(matfp, "data_1");
	auto data_2   = Mat_VarRead(matfp, "data_2");

	if (!Aclass || !dataInfo || !name || !desc || !data_1 || !data_2) {
		set_error_message("'%s' has an unsupported file structure", filename);
		return nullptr;
	}

	const auto formatInfo = readStringMatrix(Aclass, false);

	// check the dsres version
	if (formatInfo[1] != "1.1") {
		set_error_message("'%s' has an unsupported version", filename);
		return nullptr;
	}

	auto &formatType = formatInfo[3];

	if (formatType != "binTrans" && formatType != "binNormal") {
		set_error_message("'%s' has an unsupported format", filename);
		return nullptr;
	}

	*trans = formatType == "binTrans";
	
	return matfp;
}

string get_unit(const string &description) { //(const QString &description, QString &unit, QString &displayUnit, QString &comment, Dataset::DataType &dataType) {

	string unit = "";
	string comment = description;

	bool relative_quantity = false;

	// extract the meta information (unit, type) from the description
	if (comment.size() > 2 && comment.back() == ']') {

		// remove the trailing square bracket
		comment.pop_back();

		// get the unit
		while (comment.size() > 0 && comment.back() != '[' && comment.back() != '|') {
			auto last = comment.at(comment.size() - 1);
			unit.insert(0, &last, 1);
			comment.pop_back();
		}

		// remove the vertical bar (if any) and reset the unit
		if (comment.length() > 0 && comment.back() == '|') {
			unit = "";
			comment.pop_back();
		}

		// get the unit
		while (comment.length() > 0 && comment.back() != '[') {
			auto last = comment.at(comment.size() - 1);
			unit.insert(0, &last, 1);
			comment.pop_back();
		}
	}

	return unit;
}


void get_time_series_size_dsres(const char *filename, const char **dataset_names, int *size) {

	mat_t *matfp = nullptr;
	bool trans = false;

	matfp = open_mat_file(filename, &trans);

	if (!matfp) {
		return;
	}

	auto data_2 = Mat_VarRead(matfp, "data_2"); // trajectories

	*size = data_2->dims[trans ? 1 : 0];

	// close the file
	Mat_Close(matfp);
}



void read_dsres(const char *filename, const int ndatasets, const char **dataset_names, const char **dataset_units, const char *scale_unit, int nsamples, double *data) {

	mat_t *matfp = nullptr;
	bool trans = false;

	matfp = open_mat_file(filename, &trans);

	if (!matfp) {
		return;
	}

	auto info = Mat_VarRead(matfp, "dataInfo");
	auto name = Mat_VarRead(matfp, "name");
	auto desc = Mat_VarRead(matfp, "description");

	auto info_data = static_cast<const double *>(info->data);

	auto data_1 = Mat_VarRead(matfp, "data_1"); // constants
	auto data_1_info = Mat_VarReadInfo(matfp, "data_1");

	auto data_2 = Mat_VarRead(matfp, "data_2"); // trajectories
	auto data_2_info = Mat_VarReadInfo(matfp, "data_2");

	auto names = readStringMatrix(name, trans);
	auto descr = readStringMatrix(desc, trans);

	vector<string> var_names;

	// convert names to paths
	for (auto name : names) {
		auto var_name = name;
		std::replace(var_name.begin(), var_name.end(), '.', '/');
		var_names.push_back('/' + var_name);
	}


	// TODO: check nsamples == data_2->dims[1]
	//auto nsamples = data_2->dims[1];

	// store the time
	for (int j = 0; j < nsamples; j++) {

		double v;

		if (trans) {
			v = static_cast<double *>(data_2->data)[j * data_2->dims[0]];
		} else {
			v = static_cast<double *>(data_2->data)[j];
		}

		// check unit
		if (strlen(scale_unit) > 0) {
			auto unit = get_unit(descr[0]);
			if (unit != scale_unit) {
				set_error_message("The scale in '%s' has the wrong unit. Expected '%s' but was '%s'.", filename, scale_unit, unit.c_str());
				return;
			}
		}

		data[j * (ndatasets + 1)] = v;
	}

	for (int i = 0; i < ndatasets; i++) {

		int k = -1;

		string var_name = dataset_names[i];

		// find the index
		for (int j = 0; j < var_names.size(); j++) {
			if (var_names[j] == var_name) {
				k = j;
				break;
			}
		}

		if (k < 0) {
			set_error_message("Variable '%s' was not found in '%s'", var_name.c_str(), filename);
			return;
		}

		// check unit
		if (strlen(dataset_units[i]) > 0) {
			auto unit = get_unit(descr[k]);
			if (unit != dataset_units[i]) {
				set_error_message("Variable '%s' in '%s' has the wrong unit. Expected '%s' but was '%s'.", 
					var_name.c_str(), filename, dataset_units[i], unit.c_str());
				return;
			}
		}

		int d; // data block
		int x;

		if (trans) {
			d = info_data[k * info->dims[0]];
			x = info_data[k * info->dims[0] + 1];
		} else {
			d = info_data[k];
			x = info_data[info->dims[0] + k];
		}

		int c = abs(x) - 1;     // column
		int s = x < 0 ? -1 : 1; // sign

		// store the trajectory
		for (int j = 0; j < nsamples; j++) {

			double v;

			if (d == 1) {
				v = static_cast<double *>(data_1->data)[trans ? c : (c * data_1->dims[0])];
			} else if (d == 2) {
				v = static_cast<double *>(data_2->data)[trans ? (j * data_2->dims[0] + c) : (c * data_2->dims[0] + j)];
			} else {
				set_error_message("Unexpected data block");
				return;
			}

			data[j * (ndatasets + 1) + i + 1] = v * s;			 
		}
	}

	// close the file
	Mat_Close(matfp);
}
