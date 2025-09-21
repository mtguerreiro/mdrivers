/*
 * temperatureDs18b20.c
 */

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "temperatureDs18b20.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
//=============================================================================

#define W1_PATH "/sys/bus/w1/devices"

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t temperatureDs18b20Initialize(void){

    return 0;
}
//-----------------------------------------------------------------------------
int32_t temperatureDs18b20Read(void *p, int32_t *temp){

    (void)p;

    char sensor_path[512];
    bool found_sensor = false;

    DIR *dir;
    struct dirent *entry;

    dir = opendir(W1_PATH);
    if (dir == NULL) {
        perror("Failed to open w1 devices directory");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "28-", 3) == 0) {
            snprintf(sensor_path, sizeof(sensor_path), "%s/%s/w1_slave", W1_PATH, entry->d_name);
            closedir(dir);
            found_sensor = true;
            break;
        }
    }
    if( found_sensor == false ){
        closedir(dir);

        fprintf(stderr, "No DS18B20 sensor found\n");
        return -1;
    }

    FILE *fp;
    char line[256];
    double temp_c = -1000;

    fp = fopen(sensor_path, "r");
    if (fp == NULL) {
        perror("Failed to open sensor file");
        return -1;
    }

    // Read first line (CRC check)
    if (fgets(line, sizeof(line), fp) == NULL) {
        perror("Failed to read line 1");
        fclose(fp);
        return -1;
    }

    // Read second line (contains temperature)
    if (fgets(line, sizeof(line), fp) == NULL) {
        perror("Failed to read line 2");
        fclose(fp);
        return -1;
    }

    // Find "t="
    char *t_ptr = strstr(line, "t=");
    if (t_ptr != NULL) {
        int temp_milli = atoi(t_ptr + 2);
        temp_c = temp_milli / 1000.0;
    }

    fclose(fp);

    *temp = (int32_t) temp_c;

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================
