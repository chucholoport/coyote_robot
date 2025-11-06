/*
 * motor_test.ino
 * 
 * Educational example for students using ROS Melodic + Arduino UNO.
 * The sketch controls a single DC motor with L298N driver and reads an encoder.
 * Communication with ROS is handled via rosserial_arduino.
 *
 * Connections:
 *  - L298N ENA  -> PWM pin 5
 *  - L298N IN1  -> pin 8
 *  - L298N IN2  -> pin 9
 *  - Encoder C1 -> pin 2 (interrupt 0)
 *  - Encoder C2 -> pin 4 (optional for direction detection)
 *  - GNDs of L298N, encoder and Arduino must be common.
 *
 * Author: Coyote Robotics Lab
 */

#include <ros.h>
#include <std_msgs/Float32.h>
#include <geometry_msgs/Twist.h>

// ===================== Pin Definitions =====================
const int ENA_PIN = 5;   // PWM output to L298N enable pin
const int IN1_PIN = 8;   // Direction pin 1
const int IN2_PIN = 9;   // Direction pin 2
const int ENC_A_PIN = 2; // Encoder channel A (interrupt)
const int ENC_B_PIN = 4; // Encoder channel B (optional)

// ===================== Global Variables =====================
volatile long encoder_ticks = 0;
unsigned long last_time = 0;
float current_rpm = 0.0f;
float target_speed = 0.0f; // received from ROS cmd_vel
const int PULSES_PER_REV = 11; // typical JGB37-520 encoder (check yours)

// ===================== Fallback for digitalPinToInterrupt =====================
#ifndef digitalPinToInterrupt
  #define digitalPinToInterrupt(p) ((p) == 2 ? 0 : ((p) == 3 ? 1 : -1))
#endif

// ===================== ROS Setup =====================
ros::NodeHandle nh;
std_msgs::Float32 rpm_msg;
ros::Publisher rpm_pub("wheel_rpm", &rpm_msg);

// Forward declaration
void cmdVelCallback(const geometry_msgs::Twist &msg);
ros::Subscriber<geometry_msgs::Twist> cmd_sub("cmd_vel", cmdVelCallback);

// ===================== Encoder ISR =====================
void encoderISR()
{
  encoder_ticks++;
}

// ===================== Motor Control =====================
void setMotorSpeed(float speed)
{
  // Clamp speed between -255 and 255
  if (speed > 255.0f) speed = 255.0f;
  if (speed < -255.0f) speed = -255.0f;

  if (speed > 0)
  {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    analogWrite(ENA_PIN, (int)speed);
  }
  else if (speed < 0)
  {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
    analogWrite(ENA_PIN, (int)(-speed));
  }
  else
  {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    analogWrite(ENA_PIN, 0);
  }
}

// ===================== ROS Callback =====================
void cmdVelCallback(const geometry_msgs::Twist &msg)
{
  // Use only linear.x for this test
  target_speed = msg.linear.x * 255.0f; // scale for PWM
  setMotorSpeed(target_speed);
}

// ===================== Setup =====================
void setup()
{
  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);

  // Attach interrupt safely even if digitalPinToInterrupt is undefined
  int encInterrupt = digitalPinToInterrupt(ENC_A_PIN);
  if (encInterrupt >= 0)
  {
    attachInterrupt(encInterrupt, encoderISR, RISING);
  }

  nh.initNode();
  nh.advertise(rpm_pub);
  nh.subscribe(cmd_sub);

  last_time = millis();
}

// ===================== Loop =====================
void loop()
{
  nh.spinOnce();

  unsigned long now = millis();
  if (now - last_time >= 1000UL) // every second
  {
    noInterrupts();
    long ticks = encoder_ticks;
    encoder_ticks = 0;
    interrupts();

    current_rpm = (ticks / (float)PULSES_PER_REV) * 60.0f; // 1s interval
    rpm_msg.data = current_rpm;
    rpm_pub.publish(&rpm_msg);

    last_time = now;
  }

  delay(5);
}

