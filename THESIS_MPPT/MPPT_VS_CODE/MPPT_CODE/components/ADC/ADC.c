#include <stdio.h>
#include "ADC.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include  "esp_err.h"
#include "soc/soc_caps.h"
#include "esp_log.h"


//PROPER WAY TO DO STUFF IS TO DEFINE ALL CONSTANTS AT THE TOP ESP ONCE FOR EXAMPLE PUT IN SAY STRUCTS OR WHEREVER SO THAT THEY ARE CHANGEABLE EASILY
//FROM A LATEENCE POINT OF VIEW IF I SEE THAT THE IDEA OF DMA BUFFERING IS GONNA CAUSE A LOT OF DELAY MAYBE ITS BETTER TO USE ONE SHOT MODE OR USE AN EXTERNAL ADC VIA SPI/I2C
//WE CAN ALSO TRY TO REDUCE THE SIZE OF THE DMA BUFFER SO THAT THE READING IS FASTER AND LOWER LATENCY


//Idea is in the SOC , the way how the  output data us arraged differs from one chip to another
// so we have to make sure we use the right format type for the chip we are using
//It simply lies in how the bit fields are arranged in the output data struct
//for one type say channel number is at certain bit position and for other type its at another bit position
//hence we have to use the right format type to extract the right data from the output struct

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif

#define ADC_UNIT                            ADC_UNIT_1  
#define ADC_ATTEN                           ADC_ATTEN_DB_12
#define ADC_BITWIDTH                        ADC_BITWIDTH_12
#define ADC_CH_V_PV                         ADC_CHANNEL_4                //GPIO 32 USED TO MEASURE INPUT VOLTAEG FROM THE PV PANEL
#define ADC_CH_V_BOOST                      ADC_CHANNEL_5               //GPIO 33 USED TO MEASURE OUTPUT VOLTAGE FROM THE BOOST CONVERTER
#define  ADC_CH_I_INDUCTOR                  ADC_CHANNEL_6             //GPIO 34 USED TO MEASURE THE INDUCTOR CURRENT USING THE CURRENT SENSOR
#define NUMBER_OF_PATTERNS                  3
#define SAMPLE_FREQUENCY_KZ                 20* 1000 *3 //The last 2 is for taking care of the shared sampling frequency between the two channels as we are using continuous mode and hence the sampling frequency is shared between the channels
#define EXAMPLE_READ_LEN                    256
#define MAX_BUF_SIZE                        256 //1024
#define TIMEOUT                             10
#define TAG                                 "ADC"


#define MV_TO_V                 0.001f

#define V_PV_R_TOP       4700.0f  //R1 in the voltage divider for measuring PV voltage
#define V_PV_R_BOTTOM    750.0f
#define V_BOOST_R_TOP    47000.0f
#define V_BOOST_R_BOTTOM 2000.0f

#define V_PV_DIVIDER_GAIN    ((V_PV_R_TOP + V_PV_R_BOTTOM) / V_PV_R_BOTTOM)
#define V_BOOST_DIVIDER_GAIN ((V_BOOST_R_TOP + V_BOOST_R_BOTTOM) / V_BOOST_R_BOTTOM)



// Current sensor conversion
#define I_SENSOR_OFFSET_V       0.0f
#define INA240_GAIN_V_PER_V     200.0f
#define SHUNT_RESISTOR_OHMS     0.003f
#define I_SENSOR_V_PER_A        (INA240_GAIN_V_PER_V * SHUNT_RESISTOR_OHMS)   // = 0.6 V/A



uint32_t ret_num_bytes_read = 0;
static uint8_t adc_read_buf[EXAMPLE_READ_LEN];
float scaled_data =0.0;
int adc_measurement =0;



//THIS WILL LATER BE PUT IN PART OF THE CODE WHERE WE DO THE SAMPLING PROCESS 
//volatile double IL = 0.0;        // Inductor current in A
volatile double V_PV = 0.0;      // PV voltage in V
volatile double V_BOOST = 0.0;   // Boost output voltage in V
volatile double I_PV = 0.0;      // PV input current in A, if applicable


//Continuous mode  
//It’s a hardware data mover., It moves data from a peripheral (ADC) → RAM, without CPU involvement

//Initialisation  of Driver and handle creation
adc_continuous_handle_t handle = NULL;
adc_continuous_handle_cfg_t adc_config = {
    .max_store_buf_size = MAX_BUF_SIZE  ,
    .conv_frame_size = EXAMPLE_READ_LEN  ,      //CHECK THE flush_pool flag etc so we dont loose new results 
};



//ADC configurations for continuous mode


//Two Structs that are actually put under the adc_patterns array and we can do this for multiple channels
static adc_digi_pattern_config_t adc_patterns[3] = {
   {
            .atten = ADC_ATTEN,
            .channel = ADC_CH_V_PV,   // GPIO 32 
            .unit = ADC_UNIT,
            .bit_width = ADC_BITWIDTH ,   //we use the highest bitwidth for better resolution
        },
            {
                .atten = ADC_ATTEN,
                .channel = ADC_CH_V_BOOST, // GPIO 33
                .unit =ADC_UNIT,
                .bit_width =ADC_BITWIDTH , //we use the highest bitwidth for better resolution
            },

            {
                .atten = ADC_ATTEN,
                .channel =  ADC_CH_I_INDUCTOR, // GPIO 34
                .unit =ADC_UNIT,
                .bit_width =ADC_BITWIDTH , //we use the highest bitwidth for better resolution
            },
};


adc_continuous_config_t continuous_config = {
    .sample_freq_hz = SAMPLE_FREQUENCY_KZ ,  //Sampling frequency in Hz ..REMEBER THAT SAMPLING FREQUENCY IS SHARED BETWEEN THE CHANNELS 
                                            //Hence if we have 2 channels and we set 20 kHz sampling frequency that means each channel is sampled at 10 kHz SO FOR MORE SAME THING 
    .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
    .pattern_num = NUMBER_OF_PATTERNS, // Number of patterns
    .adc_pattern =  adc_patterns,     //No need to use & as its already an array and it decays to pointer (first element address to be more precise)
};


 //setting calibtration scheme for VIN_PWE_handle
     adc_cali_handle_t cali_handle_Unit1 ;
     adc_cali_line_fitting_config_t cali_config_Unit1 = {
        .unit_id =  ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };
    




/*
adc_digi_pattern_config_t adc_pattern_config_1 = {
    .atten = ADC_ATTEN_DB_12,
    .channel = ADC_CHANNEL_7,            //VDET_2 for pin name 
    .unit = ADC_UNIT_1,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};



adc_digi_pattern_config_t adc_pattern_config_2= {
    .atten = ADC_ATTEN_DB_12,
    .channel = ADC_CHANNEL_4,            // 32K_XP 
    .unit = ADC_UNIT_1,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};
*/


//For making the setings take change 

void Initialise_and_measure_ADC(void)
{

esp_err_t ret;
ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));
adc_continuous_config(handle, &continuous_config) ;
adc_continuous_start(handle) ;  //REMEBER TO HAVE STOP CASE WHEN SOMETHING BAD HAPPENS OR FAULT DETECTED IN THE SYSTEM 

 //error checking and calling out different calibration schemes for different channels
 ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config_Unit1, &cali_handle_Unit1));

int channels_num[3]= {0};
adc_continuous_channel_to_io(ADC_UNIT_1, ADC_CH_V_PV, &channels_num[0] );  //Mapping the channel to the respective IO pins
adc_continuous_channel_to_io(ADC_UNIT_1, ADC_CH_V_BOOST ,&channels_num[1]);
adc_continuous_channel_to_io(ADC_UNIT_1,  ADC_CH_I_INDUCTOR ,&channels_num[2]);

printf("ADC Channel %d is mapped to GPIO %d \n", ADC_CH_V_PV,  channels_num[0] );
printf("ADC Channel %d is mapped to GPIO %d \n", ADC_CH_V_BOOST , channels_num[1] );
printf("ADC Channel %d is mapped to GPIO %d \n",  ADC_CH_I_INDUCTOR , channels_num[2] );



for (int i =0; i<2; i++)
{
    int current_gpio = channels_num[i];
ret = gpio_pulldown_en(current_gpio);   //Enabling pulldown resistors on the respective GPIO pins
if(ret == ESP_OK)
{
    ESP_LOGI(TAG, "Pulldown enabled on GPIO %d", ( current_gpio) );
}
else
{
    ESP_LOGE(TAG, "Could not enable pulldown on GPIO %d",( current_gpio ));
}

}

while (1)

{
    
//Always remember that  an array decays to a pointer to its first element when passed to functions
//Hence no need to use & when passing arrays to functions for example adc_read_buf is already a pointer to the first element of the array
ret = adc_continuous_read(handle, adc_read_buf, EXAMPLE_READ_LEN , &ret_num_bytes_read, TIMEOUT );  //read function to read the data from the buffer

if (ret == ESP_OK) {
    
  // ESP_LOGI("ADC", "Read %lu bytes from ADC", ret_num_bytes_read);   JUST TO LESSEN THE NUMBER OF LOGS COMING OUT 
} else if (ret == ESP_ERR_TIMEOUT) {
   
    ESP_LOGE("ADC", "Timeout occurred while reading ADC data");
}
else if (ret == ESP_ERR_INVALID_STATE) {
    
ESP_LOGE("ADC", "ADC read invalid state");
}
else {
    
    ESP_LOGE("ADC","ADC read error" );  //Can readout the error codes from esp_err.h
}

//static uint32_t last_time = 0;
//uint32_t now = esp_timer_get_time(); // microseconds
//printf("Period: %d us\n", now - last_time);
//last_time = now;


//for (int i = 0; i < ret_num_bytes_read; i += SOC_ADC_DIGI_RESULT_BYTES) {
 for (int i = ret_num_bytes_read-(SOC_ADC_DIGI_RESULT_BYTES*3); i <= ret_num_bytes_read-1 ; i += SOC_ADC_DIGI_RESULT_BYTES) {
                    adc_digi_output_data_t *p = (adc_digi_output_data_t*)&adc_read_buf[i];  //Casting the read buffer to the output data type struct pointer which includes the channel number and data as members
                    uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
                    uint32_t data = EXAMPLE_ADC_GET_DATA(p);
                    
                   //float  float_data = data;
                     //scaled_data =(float_data/4095.0)*3.3  ;         //For 12 bit ADC with Vref of 3.3V

                    ret = adc_cali_raw_to_voltage(cali_handle_Unit1, data , &adc_measurement);  //Getting the calibrated voltage value in mV
                    if (ret == ESP_OK) {
                        //ESP_LOGI(TAG, "ADC Calibration Successful");
                    }
                    else {
                        ESP_LOGE(TAG, "ADC Calibration Failed");
                    }


                    /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                    //if (chan_num < SOC_ADC_CHANNEL_NUM(ADC_UNIT)) {
                      //  ESP_LOGI(TAG, " Channel: %"PRIu32", Value: %d mv", chan_num,adc_measurement );
                    //} else {
                        //ESP_LOGW(TAG, "Invalid data [%"PRIu32"_%d""]", chan_num, adc_measurement );
                    //}
             // }


             float measured_voltage = adc_measurement * MV_TO_V;   // mV -> V

if (chan_num < SOC_ADC_CHANNEL_NUM(ADC_UNIT)) {
    switch (chan_num) {
        case ADC_CH_V_PV:
            V_PV = measured_voltage * V_PV_DIVIDER_GAIN;
           // ESP_LOGI(TAG, "V_PV = %.3f V", V_PV);
            break;

        case ADC_CH_V_BOOST:
            V_BOOST = measured_voltage * V_BOOST_DIVIDER_GAIN;
           // ESP_LOGI(TAG, "V_BOOST = %.3f V", V_BOOST);
            break;

         case ADC_CH_I_INDUCTOR:
          I_PV = (measured_voltage - I_SENSOR_OFFSET_V) / I_SENSOR_V_PER_A;
 
          if (I_PV < 0.0f) {
           I_PV = 0.0f;
          }

    //ESP_LOGI(TAG, "I_PV = %.3f A", I_PV);
    break;

        default:
            ESP_LOGW(TAG, "Unhandled channel: %" PRIu32, chan_num);
            break;
    }
} else {
    ESP_LOGW(TAG, "Invalid data [%" PRIu32 "_%d]", chan_num, adc_measurement);
}

 }

vTaskDelay(1000/portTICK_PERIOD_MS);  //Delay of 1 second between readings  later i will cahgethis intoa task with semaphores to make it non blocking
                                    //we shall set the delay according to the sampling frequency we want for example if we want 20 kHz sampling frequency that means we need to sample every 50 microseconds and hence we can set the delay to 50 microseconds but at the same time we have to make sure that the processing of the data is done within this time frame and this is where the use of DMA buffering can help as it allows us to read the data from the buffer without blocking the CPU and hence we can process the data while new data is being sampled and stored in the buffer without losing any samples and this is one of the main advantages of using continuous mode with DMA buffering as it allows us to achieve high sampling frequencies with low latency and efficient processing of the data without losing any samples due to blocking or delays in processing.

}


}



/*

//One shot mode 

    adc_oneshot_unit_handle_t ADC7_handle;                    //You get the habdle to the unit once but one unit has many channels 
    adc_oneshot_unit_init_cfg_t init_config7 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };



    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config7, &ADC7_handle));    //instance created and installed now we move to setting upt the settings for the channel remember for a unit you just change  mke the hanle once n use it on multiple c channels
    
    
    //configuring the channels
    
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,   //bitwidth isa simply how much time it takes to send one bit ...and baud rate=1/Tb is how many buts per second.
        .atten = ADC_ATTEN_DB_12,             //Best Possible attenuation we can have from the Chip
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(ADC7_handle, ADC_CHANNEL_7, &config));  //  now we configured our channel we move on to calibration to get value on mV
    ESP_ERROR_CHECK(adc_oneshot_config_channel(ADC7_handle, ADC_CHANNEL_4, &config)); 

     //setting calibtration scheme for VIN_PWE_handle
     adc_cali_handle_t cali_handle_Unit1 ;
     adc_cali_line_fitting_config_t cali_config_Unit1 = {
        .unit_id =  ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    //error checking and calling out different calibration schemes for different channels
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config_Unit1, &cali_handle_Unit1));
    

    while(1){  //we can use tasks and semaphores here to make it non blocking  and make this a task ..but at the same time we can just try continous mode of the adc 



    adc_oneshot_read(ADC7_handle,ADC_CHANNEL_7, &raw_Pulse_Width_in7);   //notice how i use the adrerss of and the pointer//....ref from top of code 
    adc_oneshot_read(ADC7_handle,ADC_CHANNEL_4, &raw_Pulse_Width_in4); 
    adc_cali_raw_to_voltage(cali_handle_Unit1, raw_Pulse_Width_in7 , &Pulse_width_7);
    adc_cali_raw_to_voltage(cali_handle_Unit1, raw_Pulse_Width_in4 , &Pulse_width_4);  //This is used to measure the maximum e

    vTaskDelay(250/portTICK_PERIOD_MS);

    }





void vOtherFunction( void )   //fucntion with the configuration and  parameters for the created task
{
static uint8_t ucParameterToPass;
TaskHandle_t xHandle = NULL;

  xTaskCreate( vTaskCode, "NAME",8192, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
  configASSERT( xHandle );
  
}


















void Initialise_ADC(void)
{



















}
*/