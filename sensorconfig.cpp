#include "sensorconfig.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;
/*
 * Save a config (currently to a file)
 */
bool
SensorConfig::save(void)
{
  bool result = true;
  try {
    ostringstream ofs;
    ofs << "sensor." << setw(5) << setfill('0') << setbase(8) << id << ".cfg" << setbase(10) << ends;
    ofstream ofile(ofs.str().c_str(), ios::out | ios::trunc);
    ofile << "low_point " << low_point << endl;
    ofile << "high_point " << high_point << endl;
    ofile << "low_time " << low_time << endl;
    ofile << "high_time " << high_time << endl;
    ofile << "reference " << reference << endl;
    ofile << "mode " << mode << endl;
    ofile.close();
  }
  catch (...) {
    cerr << "Failed to save config" <<endl;
    result = false;
  }
  return result;
}

void
SensorConfig::set(string item, int value)
{
  if (item == "mode") {
    mode = value;
  }
  else if (item == "low_point") {
    low_point = value;
  }
  else if (item == "high_point") {
    high_point = value;
  }
  else if (item == "low_time") {
    low_time = value;
  }
  else if (item == "high_time") {
    high_time = value;
  }
  else if (item == "reference") {
    reference = value;
  }
}


/*
 * Load a config file for current node
 */
bool
SensorConfig::load(void)
{
    bool result = false;
    string line;
    string item;
    int value;

    ostringstream ofs;
    ofs << "sensor." << setw(5) << setfill('0') << setbase(8) << id << ".cfg" << setbase(10) << ends;
    try {
      ifstream ifile(ofs.str().c_str());
      if (ifile.is_open()) {
	while (getline(ifile, line)) {
	  stringstream _l(line);
	  _l >> item >> value;
	  set(item, value);
	  result = true;
	}
	ifile.close();
      }
    }
    catch (...) {
      cerr << "Failed to load config" << endl;
      result = false;
    }
    return result;
}
