#include "kuria_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

// verify that the string corresponds to the right index
// as given by config_str_t
const char* hlo_x4_config_str[CONFIG_STR_MAX] = {
    "MIN",
    "MAX",
    "ITR",
    "PPS",
    "DWN",
    "FPS",
    "TXF"
    // add new configuration here
        
};

void pabort(const char *s)
{
	perror(s);
	abort();
}

// TODO - not sure how accurate this delay(usleep) will be. due to latencies, 
// this delay might be more than specified
void hlo_delay_us (uint32_t delay_us) {
    int status;

    status = usleep (delay_us);
    if (status) {
        perror ("usleep error: \n");
    }
    return;
}

int32_t hlo_x4_read_config_from_file (char* filename, hlo_x4_config_t* config) {

    FILE* fp;
    size_t len = 0;
    size_t nread;
    uint32_t num;
    char buf[256];
    char* line = NULL;
    char* ret_str = NULL;

    fp = fopen (filename, "r+");
    if (fp == NULL) {
        printf ("no config file found\n");
        return -1;
    }

    bool match_found = false;

    while ( (nread = getline (&line, &len, fp) ) != -1 ) {

        match_found = false;

        for (uint32_t i=0; i<CONFIG_STR_MAX;i++ ) {

            if ( (ret_str = strstr (line, hlo_x4_config_str[i]) ) != NULL) {

                match_found = true;
                uint32_t val = 0;
                sscanf (line, "%[^0-9] %u\n", buf, &val);
                switch (i) {
                    case CONFIG_STR_DAC_MIN:
                        config->dac_min = val;
                        printf ("found dac_min: %d\n", config->dac_min);
                        break;
                    case CONFIG_STR_DAC_MAX:
                        config->dac_max = val;
                        printf ("found dac_max: %d\n", config->dac_max);
                        break;
                    case CONFIF_STR_ITR:
                        config->iterations = val;
                        printf ("found iter: %d\n", config->iterations);
                        break;
                    case CONFIG_STR_PPS:
                        config->pps = val;
                        printf ("found pps: %d\n", config->pps);
                        break;
                    case CONFIG_STR_DWN:
                        config->downconversion_en = val;
                        printf ("found dwn: %d\n", config->downconversion_en);
                        break;
                    case CONFIG_STR_FPS:
                        config->fps = val;
                        printf ("found fps: %d\n", config->fps);
                        break;
                    case CONFIG_STR_TXF:
                        config->tx_center_freq = val;
                        printf ("found tx freq: %d\n", config->tx_center_freq);
                        break;
                    default:
                        // ignore params
                        match_found = false;
                        break;
                } // switch
                if (match_found) break;
            } // strstr
        } // for loop
    } // while getline

    free (line);
    fclose (fp);

    return 0;
}

