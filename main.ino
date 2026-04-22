// need to send rotational velocity and direction to LEDC
// PCNT to measure encoder signal

// RTOS for reading motor state and computing new PWM duty cycle

// quadrature encodes speed and phase (for direction), see EE. 

#include<stdio>; 
using namespace std;
#include "driver/pulse_cnt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
xTaskCreate(motor_speed,"monitoring the motor speed", 1<<16, 0, configMAX_PRIORITIES -1, &motor_speed_handle);

// task functons can be setup like so
void motor_speed ( void *args ) {
  uint64_t start_task; 
  TickType_t xLastWakeTime = xTaskGetTickCount(); 
  TickType_t xTimeIncrement = pdMS_TO_TICKS(TASK_PERIOD_IN_MS);

  for(;;) { // infinite loop, you learn something new every day. why not just while (!bool) and set bool = 0?
    start_task = esp_time_get_time(); 

    // motor speed code here 
    
    // ** hint ** taskATotal can be used to monitor cpu utilization for a task.
    taskATotal += esp_timer_get_time() - start_task; 
    vTaskDelayUntil(&xLastWakeTime,xTimeIncrement);
    }
   }

void setup() {

bool system_on = true; 
float start_time = esp_timer_get_time(); // I think this should be a float no?
float cpu_utilization; 
int rpm; 
cout << "Welcome to hell" << "We hope you enjoy your stay" << endl; 
while(system_on) {
  // track cpu utilization 
  float cpu_utilization = taskATotal / (esp_timer_get_time-start_time); 
  int rpm = (60 * 100) / f; // we need to set/get f here somehow
  if (button_pressed) {

}

rpm += rpm / f;

}


}
