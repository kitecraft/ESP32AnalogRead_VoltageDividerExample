#include <driver/adc.h>
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   50          //Multisampling

// Voltage divider example.
// These are the actual values as measured by Multimeter
#define RESISTOR_R1 10004
#define RESISTOR_R2 996

// This is the number we multiple the 'analog' value by to get the actual voltage
double MULTIPLIER = ((RESISTOR_R1 + RESISTOR_R2)/RESISTOR_R2);

// Or, simply use your own muliplier instead
//double MULTIPLIER = 10.6;


// This is what makes the analog reads accurate
esp_adc_cal_characteristics_t *adc_characteristics;

// You have choices here.
// If you don't know what they are, are why you want to use them
// then just stick with these. All though, you'll need to learn 
// these at some point.
adc_bits_width_t width = ADC_WIDTH_BIT_12;
adc_atten_t atten = ADC_ATTEN_DB_11;
adc_unit_t unit = ADC_UNIT_1;


// This next line is just setting the 'pin' number.
// Instead of using the Arduino way of 'int pin = 34'
// We're going to use the ESP32 way of doing things.
adc1_channel_t channel = ADC1_CHANNEL_6;     // GPIO34

// *** End of all that stuff *** //

// Function:  check_efuse(void)
// This is just for informational purposes.
// You can lookup what this means if you want to
// go ma bit more advanced.
// But you don't really need to.
void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        Serial.printf("eFuse Two Point: Supported\n");
    } else {
        Serial.printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        Serial.printf("eFuse Vref: Supported\n");
    } else {
        Serial.printf("eFuse Vref: NOT supported\n");
    }
}

// This is also just for informational purpose.
void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("Characterized using eFuse Vref\n");
    } else {
        Serial.printf("Characterized using Default Vref\n");
    }
}

void setup() 
{
  Serial.begin(115200);

  check_efuse();
  
  adc1_config_width(width);
  adc1_config_channel_atten(channel, atten);

  // Create the magic structure that makes it all work
  adc_characteristics = new esp_adc_cal_characteristics_t(); 
  
  // Get the ESP32 to do the math and build the structure
  // that makes it all work.
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_characteristics);
  print_char_val_type(val_type);


  // Use a voltmeter on pin 25 to read
  // the true VREF for your ESP32.
  // Use the number (in millivolts) as the 
  // DEFAULT_VREF value at the top of this file.
  if(adc2_vref_to_gpio(GPIO_NUM_25) == ESP_OK)
  {
    Serial.println("routed to pin 25");
  } else {
    Serial.println("Failed to route to pin 25");
  }

  Serial.printf("Multiplier: %.3d\n",MULTIPLIER);

  Serial.println("End Setup\n\n");
}

void loop()
{
  uint32_t adc_reading = 0;
  
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    adc_reading += adc1_get_raw(channel);
  }
  
  adc_reading /= NO_OF_SAMPLES;
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_characteristics);

  if(adc_reading < 65)
  {
    voltage = 0;
  }

  Serial.printf("Raw: %.3d \tVoltage: %.3d mV\n", adc_reading, voltage);
  
  double inVoltage = ((double)voltage * MULTIPLIER)/1000.0;
  Serial.print("Input voltage: ");
  Serial.print(inVoltage,3);
  Serial.println(" V\n\n");
  
  delay(500);
  
}
