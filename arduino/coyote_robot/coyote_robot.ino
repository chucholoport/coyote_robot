/*
 * coyote_robot.ino
 * 
 * Educational example for students using ROS Melodic + Arduino UNO.
 * This sketch controls a full two-wheel differential drive robot using 
 * an L293D (or L298N) motor driver and reads encoders from both motors.
 * Communication with ROS is handled via rosserial_arduino.
 *
 * Connections:
 *  - Left Motor:
 *      ENA  -> PWM pin 5
 *      IN1  -> pin 8
 *      IN2  -> pin 9
 *      Encoder A -> pin 2 (interrupt 0)
 *      Encoder B -> pin 4 (optional)
 *
 *  - Right Motor:
 *      ENB  -> PWM pin 6
 *      IN3  -> pin 10
 *      IN4  -> pin 11
 *      Encoder A -> pin 3 (interrupt 1)
 *      Encoder B -> pin 12 (optional)
 *
 *  - Power:
 *      - External motor power (6–12 V) to L293D +5 V logic via Arduino.
 *      - All GNDs (Arduino, L293D, encoders, and power supply) must be common.
 *
 * ROS Topics:
 *  - Subscriber: /cmd_vel [geometry_msgs/Twist]
 *  - Publisher:  /wheel_rpm_left  [std_msgs/Float32]
 *                /wheel_rpm_right [std_msgs/Float32]
 *
 * Author: Coyote Robotics Lab
 */

#include <ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Float32MultiArray.h>

// ============= Pin Configuration =============
#define ENA 5      // PWM Left Motor
#define IN1 6
#define IN2 7
#define ENB 9      // PWM Right Motor
#define IN3 10
#define IN4 11

#define ENC_L_A 2
#define ENC_L_B 4
#define ENC_R_A 3
#define ENC_R_B 8

// ===================== Fallback for digitalPinToInterrupt =====================
#ifndef digitalPinToInterrupt
  #define digitalPinToInterrupt(p) ((p) == 2 ? 0 : ((p) == 3 ? 1 : -1))
#endif

// ============= Constants =============
#define PULSES_PER_REV 330.0f
#define WHEEL_RADIUS 0.03f   // 3 cm
#define WHEEL_BASE   0.12f   // 12 cm between wheels

// ============= Variables =============
volatile long left_ticks = 0;
volatile long right_ticks = 0;

float left_rpm = 0.0f;
float right_rpm = 0.0f;
unsigned long last_time = 0;

// ============= ROS Setup =============
ros::NodeHandle nh;
std_msgs::Float32MultiArray rpm_msg;
ros::Publisher rpm_pub("wheel_rpm", &rpm_msg);

// ============= Encoder ISRs =============
void leftEncoderISR()  { left_ticks++; }
void rightEncoderISR() { right_ticks++; }

// ============= Motor Control =============
void setMotor(int en, int in1, int in2, float speed)
{
  int pwm = constrain((int)(fabs(speed) * 255.0f), 0, 255);

  if (speed > 0)
  {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
  else if (speed < 0)
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }
  else
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  analogWrite(en, pwm);
}

// ============= Twist Callback =============
void cmdVelCallback(const geometry_msgs::Twist &msg)
{
  float linear = msg.linear.x;   // m/s
  float angular = msg.angular.z; // rad/s

  float left_speed  = linear - (angular * WHEEL_BASE / 2.0f);
  float right_speed = linear + (angular * WHEEL_BASE / 2.0f);

  setMotor(ENA, IN1, IN2, left_speed);
  setMotor(ENB, IN3, IN4, right_speed);
}

// ============= ROS Sub =============
ros::Subscriber<geometry_msgs::Twist> cmd_sub("cmd_vel", cmdVelCallback);

// ============= Setup =============
void setup()
{
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENC_L_A, INPUT_PULLUP);
  pinMode(ENC_L_B, INPUT_PULLUP);
  pinMode(ENC_R_A, INPUT_PULLUP);
  pinMode(ENC_R_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_L_A), leftEncoderISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), rightEncoderISR, RISING);

  nh.initNode();
  nh.advertise(rpm_pub);
  nh.subscribe(cmd_sub);

  rpm_msg.data_length = 2;
  rpm_msg.data = (float*)malloc(sizeof(float) * 2);

  last_time = millis();
}

// ============= Main Loop =============
void loop()
{
  nh.spinOnce();

  unsigned long now = millis();
  if (now - last_time >= 1000UL)
  {
    noInterrupts();
    long lt = left_ticks;
    long rt = right_ticks;
    left_ticks = 0;
    right_ticks = 0;
    interrupts();

    left_rpm  = (lt / PULSES_PER_REV) * 60.0f;
    right_rpm = (rt / PULSES_PER_REV) * 60.0f;

    rpm_msg.data[0] = left_rpm;
    rpm_msg.data[1] = right_rpm;
    rpm_pub.publish(&rpm_msg);

    last_time = now;
  }

  delay(5);
}
