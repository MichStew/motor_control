#include <Arduino.h>

#include "driver/pulse_cnt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// constexpr ensures these wont change at runtime
constexpr uint8_t BRAKE_PIN = 21;
constexpr uint8_t DIRECTION_PIN = 4;
constexpr uint8_t PWM_PIN = 5;
constexpr uint8_t ENCODER_A_PIN = 6;
constexpr uint8_t ENCODER_B_PIN = 7;

constexpr uint8_t BRAKE_MODE = HIGH;
constexpr uint8_t DIRECTION_LEVEL = LOW;

constexpr uint8_t PWM_CHANNEL = 0; //
constexpr uint32_t PWM_FREQ_HZ = 8000;
constexpr uint8_t PWM_RES_BITS = 12; // 0 - 4095 into LEDC 
constexpr uint32_t PWM_MAX_DUTY = (1 << PWM_RES_BITS) - 1;

constexpr float TARGET_RPM = 200.0f;
constexpr float PULSES_PER_REV = 100.0f;

constexpr uint32_t SPEED_PERIOD_MS = 10;
constexpr uint32_t CONTROL_PERIOD_MS = 20;
constexpr float SPEED_PERIOD_S = SPEED_PERIOD_MS / 1000.0f;
constexpr float CONTROL_PERIOD_S = CONTROL_PERIOD_MS / 1000.0f;

// correction values
constexpr float KP = 0.12f; // dont cause oscillation here 
constexpr float KI = 0.00f; // adds a 'slow' buildup to the RPM
constexpr float KD = 0.0000f; // causes a damping effect
constexpr float INTEGRAL_LIMIT = 4000.0f;

constexpr uint32_t TASK_STACK_SIZE = 4096; // smaller stack allocation

// instantiate
pcnt_unit_handle_t pcntUnit;
pcnt_channel_handle_t pcntChannel;

volatile float motorRpm = 0.0f;
volatile uint32_t pwmDuty = 0;
volatile uint64_t speedTaskUs = 0;
volatile uint64_t controlTaskUs = 0;

uint64_t startUs = 0; // default time = 0 (obv)so

void setupMotorPins() {
  digitalWrite(BRAKE_PIN, BRAKE_MODE);
  // gonna have some fun with a second motor
  digitalWrite(DIRECTION_PIN, DIRECTION_LEVEL);
}

void setupPwm() {
  ledcAttachChannel(PWM_PIN, PWM_FREQ_HZ, PWM_RES_BITS, PWM_CHANNEL);
  ledcWriteChannel(PWM_CHANNEL, 0);
}

void setupPulseCounter() {

 // init according to 
  pcnt_unit_config_t unitConfig = {};
  unitConfig.low_limit = -1;
  unitConfig.high_limit = 1 << 14;
  unitConfig.intr_priority = 0;
  unitConfig.flags = { .accum_count=0 };

  pcnt_chan_config_t channelConfig = {};
  channelConfig.edge_gpio_num = ENCODER_A_PIN;
  channelConfig.level_gpio_num = -1;
  channelConfig.flags = { .invert_edge_input = 0, .invert_level_input = 0, .virt_edge_io_level = 0, .io_loop_back =0 };

  pcnt_new_unit(&unitConfig, &pcntUnit);
  pcnt_new_channel(pcntUnit, &channelConfig, &pcntChannel);
  
  pcnt_channel_set_edge_action(
      pcntChannel,
      PCNT_CHANNEL_EDGE_ACTION_INCREASE,
      PCNT_CHANNEL_EDGE_ACTION_HOLD);

  pcnt_unit_clear_count(pcntUnit);
  pcnt_unit_enable(pcntUnit);
  pcnt_unit_start(pcntUnit);
}

void speedTask(void *args) {
  (void)args;

  TickType_t lastWakeTime = xTaskGetTickCount();
  TickType_t period = pdMS_TO_TICKS(SPEED_PERIOD_MS);

  for (;;) {
    uint64_t taskStartUs = esp_timer_get_time();
    int pulses = 0;

    pcnt_unit_get_count(pcntUnit, &pulses);
    pcnt_unit_clear_count(pcntUnit);

    float pulsesPerSecond = pulses / SPEED_PERIOD_S;
    motorRpm = (pulsesPerSecond * 60.0f) / PULSES_PER_REV;
    speedTaskUs += esp_timer_get_time() - taskStartUs;

    vTaskDelayUntil(&lastWakeTime, period);
  }
}

float duty = 0;

void controlTask(void *args) {
  (void)args;

  TickType_t lastWakeTime = xTaskGetTickCount();
  TickType_t period = pdMS_TO_TICKS(CONTROL_PERIOD_MS);

  float integral = 0.0f; 
  float previousError = 0.0f;

  for (;;) {
    uint64_t taskStartUs = esp_timer_get_time();
    float error = motorRpm - TARGET_RPM; // can't produce positive error now 

    integral += error * CONTROL_PERIOD_S;
    integral = constrain(integral, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);

    float derivative = (error - previousError) / CONTROL_PERIOD_S;
    duty += KP * error + KI * integral + KD * derivative; // accumulate over time 
    if (duty > PWM_MAX_DUTY ) {
      duty =  PWM_MAX_DUTY;
    } else if (duty < 0) {
        duty = 0;            
    }
    ledcWriteChannel(PWM_CHANNEL, (uint32_t)duty); // good ole casting - alex mccleod 2026 

    previousError = error;
    controlTaskUs += esp_timer_get_time() - taskStartUs;

    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void setup() {
  Serial.begin(115200);

  setupMotorPins();
  setupPwm();
  setupPulseCounter();

  startUs = esp_timer_get_time();

  xTaskCreate(speedTask, "speed", TASK_STACK_SIZE, nullptr, configMAX_PRIORITIES - 1, nullptr);
  xTaskCreate(controlTask, "control", TASK_STACK_SIZE, nullptr, configMAX_PRIORITIES - 2, nullptr);
}

void loop() {
  uint64_t elapsedUs = esp_timer_get_time() - startUs;
  uint64_t speedUs = speedTaskUs;
  uint64_t controlUs = controlTaskUs;
  float controlCpu = 100.0f * controlUs / elapsedUs;
  Serial.printf("duty is %f\n", duty);

  Serial.printf(
      "rpm=%.1f speed_us=%llu control_us=%llu control_cpu=%.2f%%\n",
      motorRpm,
      speedUs, // amt of time it took to execute a speedtask
      controlUs,
      controlCpu);

  delay(1000);
}
