// sensor_data.h

#ifndef SENSOR_DATA
#define SENSOR_DATA

struct sensor_data
{
        char host_name[32];
        int num_sensors;
        int sensor_idx;
        double reading_val;
        double min_reading;
        double max_reading;
        char timestamp[32];
        int action_req;
};

#endif