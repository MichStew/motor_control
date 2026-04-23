// need to send rotational velocity and direction to LEDC
// PCNT to measure encoder signal

// RTOS for reading motor state and computing new PWM duty cycle

// quadrature encodes speed and phase (for direction), see EE. 

#include<stdio.h> 
#include<stdint.h>
#include<Arduino.h> 
#include<math.h>
#include "driver/pulse_cnt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
using namespace std;

#define BRAKEPIN = 21;
#define ENCODER_A = 6;
#define ENCODER_B = 7;
#define DIRECTION = 4;
#define PWM = 5;


// stuff from the last lab 
#define // how to talk to motor

#define LEDC_TIMER0_CONF_REG 0x600190A0 // set reference clock, divider, and duty cycle width register address purpose
#define LEDC_CONF_REG 0x600190D0 //Enables and selects LEDC clock


// initialize PCNT periphereal (unit & channel) 
pcnt_unit_handle_t mypcnt = 0; 
pcnt_channel_handle_t mychannel = 0;

// configuration struct 
pcnt_unit_config_t myunitconfig = {
  .low_limit = -1,
  .high_limit  = (1 << 14), 
  .intr_priority = 0, 
  .flags = {
  .accum_count = 0
  }
};

pcnt_chan_config_t mychannelconfig = {
  .edge_gpio_num = <pin>, 
  .level_gpio_num = -1,
  .flags = {
  .invert_edge_input = 0,
  .invert_level_input = 0,
  .virt_edge_io_level = 0,
  .io_loop_back = 0;
}};

// initialize structs of type (struct) 
pctn_new_unit(&myunitconfig, &mypcnt);
pcnt_new_channel(mypcnt, &mychannelconfig, &mychannel);

// defines what happens on positive and negtive signal
// this can be modified by passing PCTN_CHAN...INCREASE as the latter parameter to get more fine tuned reading
pcnt_channel_set_edge_action(mychannel, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);

pcnt_unit_clear(mypcnt); // clear
pcnt_unit_enable(mypcnt); // ensure its on
pcnt_unit_start(mypcnt);  // start it up 

// i suppose there will have to be a loop to read, this is how we would do that 
pcnt_unit_get_count(mypcnt, &val); 
pcnt_unit_clear_count(mypcnt);

// docs say we will need 2 tasks, 1 to monitor motor speed, and one to control PWM signal. (loop task to monitor cpu)
// refer to doc for my structure here


// I think there are just so RTOS has a copy? since we define the function behavior below 

// initialize obj motor speed handle of taskhandle type 
TaskHandle_t motor_speed_handle; 
// create motor_speed task with 64kb mem on stack, highest priority, and address of obj of handle. 
xTaskCreate(
  motor_speed, // function
  "monitoring the motor speed", // name 
  1<<16, // amt of data on stack
  0, // parameters
  configMAX_PRIORITIES -1, // priority 
  &motor_speed // handle
  );

xTaskCreate (
  monitor_cpu, 
  "monitor cpu usage",
  1<<16, 
  0,
  2, // I guess this is less priority than driving motor? 
  &monitor_cpu
  );
 

void monitor_cpu () {

}

// the motor speed needs to 
//  1.) set the speed of motor via PWM
//  2.) calculate rpm from given params, read actual rpm
//  3.) based on error we adjust via PID
void motor_speed () {
  uint64_t start_task; 
  TickType_t xLastWakeTime = xTaskGetTickCount(); 
  TickType_t loop_time = pdMS_TO_TICKS(10); // 10ms loop

  // PID Variables
  float Kp = 1.0f, Ki = 01.f ,Kd = 0.01f;
  float integral = 0.0f;
  float prevError = 0.0f;
  int f = 800; //default rpm. PID will mut this

  for(;;) { // infinite loop 
  // start task, make sure to get time 
  start_task = esp_time_get_time();

  // send PWM to motor
  digitalWrite(5, f); // register 5.
  REG_READ(ENCODER_B
  //

  error = setpoint - motor_speed; // where from ?
  derivative_error = error - prevError; 
  integral_error = error + integral;
    // motor speed code here 
    REG_WRITE(REG_ADDRESS, DATA);  // define above if possible, data is mutable for speed?  
    
    
    // ** hint ** taskATotal can be used to monitor cpu utilization for a task.
     vTaskDelayUntil(&xLastWakeTime,xTimeIncrement);
     return taskATotal += esp_timer_get_time() - start_task; // return to main loop so we can monitor 
    
    }
   }
void setup_LEDC(){
  REG_WRITE(0x600c0018, HIGH);

}


void setup() {
  Serial.begin(115200); 
  delay(500);
  pinMode(LED_PIN, OUTPUT); // is this right for motor? 
  setup_LEDC(); 
  REG_SET_BIT(DIRECTION, 0);// drive direction low  
  

  bool system_on = true; 
  float start_time = esp_timer_get_time(); // I think this should be a float no 
  cout << "Welcome to hell" << "We hope you enjoy your stay" << endl;

// loop for pwm 
    while(system_on) {
      // track cpu utilization 
      float cpu_utilization = taskATotal / (esp_timer_get_time-start_time); 
      int rpm = (60 * 100) / f; // we need to set/get f here somehow
      REG_CLR_BIT(R, address[3]) // pwn bit on board (maybe)
      
      for(i  
      REG_SET_BIT(21, HIGH) // clear -> set.
       
      if () {
        system_on = false; // break loop
        break; 
    }}
}
