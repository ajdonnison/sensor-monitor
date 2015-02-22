#ifndef _SENSOR_CONFIG_H
#define _SENSOR_CONFIG_H

#include <string>

/*
 * Defines a sensor config file format that can be
 * used to configure a remote sensor
 */

class SensorConfig {
  public:
    int id;
    int mode;
    int low_point;
    int high_point;
    int low_time;
    int high_time;
    int reference;

    SensorConfig() : id(-1), mode(0), low_point(0), high_point(0), low_time(0), high_time(0), reference(0) {};
    SensorConfig(int Id) : id(Id), mode(0), low_point(0), high_point(0), low_time(0), high_time(0), reference(0) {};
    SensorConfig(int Id, int Mode): id(Id), mode(Mode), low_point(0), high_point(0), low_time(0), high_time(0), reference(0) {};

    bool save(void);
    bool save(int Id) { id = Id; return save(); };
    bool load(void);
    bool load(int Id) { id = Id; return load(); };
    void set(std::string item, int value);
};

#endif // _SENSOR_CONFIG_H
