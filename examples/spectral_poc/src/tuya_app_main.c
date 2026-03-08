#include "tal_system.h"    // tal_system_sleep()
#include "tal_thread.h"    // THREAD_CFG_T, THREAD_HANDLE
#include "tuya_cloud_types.h"
#include "tal_log.h"
#include <math.h>
#include <stdio.h>
#include <math.h>

#define SAMPLE_COUNT   128
#define SAMPLING_FREQ  2000
#define PI             3.1415926f

static float signal[SAMPLE_COUNT];
static float magnitude[SAMPLE_COUNT/2];

/* ---------- Appliance Fingerprint Database ---------- */
typedef struct {
    const char* name;
    float h3_ratio;
    float h5_ratio;
} appliance_t;

appliance_t appliance_db[] = {
    {"Heater", 0.05f, 0.02f},
    {"LED Bulb", 0.25f, 0.10f},
    {"Laptop Charger", 0.40f, 0.20f},
};

#define NUM_APPLIANCES (sizeof(appliance_db)/sizeof(appliance_db[0]))

/* ---------- Virtual Load Generator ---------- */
// load_type: 0=Heater, 1=LED Bulb, 2=Laptop Charger
void generate_virtual_load(float *buffer, int N, int load_type)
{
    float h3_amp = 0.05f; // Default Heater
    float h5_amp = 0.02f;

    if (load_type == 1) { h3_amp = 0.25f; h5_amp = 0.10f; }
    if (load_type == 2) { h3_amp = 0.40f; h5_amp = 0.20f; }

    for(int i = 0; i < N; i++)
    {
        float time = (float)i / SAMPLING_FREQ;
        buffer[i] = 1.0f * sinf(2 * PI * 50 * time) +
                    h3_amp * sinf(2 * PI * 150 * time) +
                    h5_amp * sinf(2 * PI * 250 * time);
    }
}

/* ---------- DFT Spectrum Calculation ---------- */
void compute_spectrum(float *input, float *mag, int N)
{
    for(int k = 0; k < N/2; k++)
    {
        float real = 0, imag = 0;
        for(int n = 0; n < N; n++)
        {
            float angle = 2 * PI * k * n / N;
            real += input[n] * cosf(angle);
            imag -= input[n] * sinf(angle);
        }
        mag[k] = sqrtf(real*real + imag*imag);
    }
}

/* ---------- Device Detection ---------- */
const char* detect_device(float h3, float h5)
{
    const char* detected = "Unknown";
    float min_dist = 1000.0f;

    for(int i = 0; i < NUM_APPLIANCES; i++)
    {
        float dh3 = h3 - appliance_db[i].h3_ratio;
        float dh5 = h5 - appliance_db[i].h5_ratio;
        float dist = dh3*dh3 + dh5*dh5;

        if(dist < min_dist)
        {
            min_dist = dist;
            detected = appliance_db[i].name;
        }
    }
    return detected;
}

/* ---------- Main Application Task ---------- */
void user_main(void *arg)
{
    int current_load = 0;
    int cycle_count   = 0;

    PR_NOTICE("=== Spectral NILM Appliance Detector ===");

    // Dynamic frequency bins based on SAMPLE_COUNT & SAMPLING_FREQ
    int bin_50  = (int)(50.0f  * SAMPLE_COUNT / SAMPLING_FREQ + 0.5f);
    int bin_150 = (int)(150.0f * SAMPLE_COUNT / SAMPLING_FREQ + 0.5f);
    int bin_250 = (int)(250.0f * SAMPLE_COUNT / SAMPLING_FREQ + 0.5f);

    while(1)
    {
        // Switch appliance every 3 cycles (15s)
        if (cycle_count >= 3) {
            current_load = (current_load + 1) % NUM_APPLIANCES;
            cycle_count = 0;
            PR_WARN("--- SWITCHING VIRTUAL LOAD TO: %s ---", appliance_db[current_load].name);
        }

        generate_virtual_load(signal, SAMPLE_COUNT, current_load);
        compute_spectrum(signal, magnitude, SAMPLE_COUNT);

        float fundamental = magnitude[bin_50];
        if (fundamental < 0.01f) fundamental = 1.0f;

        float h3_ratio = magnitude[bin_150] / fundamental;
        float h5_ratio = magnitude[bin_250] / fundamental;

        const char* device = detect_device(h3_ratio, h5_ratio);

        PR_INFO("Spectral Peaks: 50Hz[%.2f] 150Hz[%.2f] 250Hz[%.2f]",
                magnitude[bin_50], magnitude[bin_150], magnitude[bin_250]);

        PR_NOTICE(">> DETECTED: %s (Match Quality: h3=%.2f, h5=%.2f)",
                  device, h3_ratio, h5_ratio);

        cycle_count++;
        tal_system_sleep(5000);
    }
}

/* ---------- TuyaOS Entry Point ---------- */
void tuya_app_main(void) 
{
    THREAD_CFG_T thrd_param = { 
        .stackDepth = 4096 * 2, 
        .priority   = THREAD_PRIO_1, 
        .thrdname   = "spectral_nilm"
    };
    
    THREAD_HANDLE ty_app_thread = NULL;
    
    // Thread creation with correct SDK signature
    tal_thread_create_and_start(&ty_app_thread, NULL, NULL, user_main, NULL, &thrd_param);
}