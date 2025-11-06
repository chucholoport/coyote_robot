#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
motor_test.py
---------------------------------------
Educational ROS node for controlling a single motor through Arduino (rosserial).

This node publishes geometry_msgs/Twist messages to /cmd_vel,
and subscribes to /wheel_rpm to display measured speed feedback.

Developed for ROS Melodic (Python 2.7)
Author: Coyote Robotics Lab
"""

import rospy
from geometry_msgs.msg import Twist
from std_msgs.msg import Float32

# ===================== Global Variables =====================
current_rpm = 0.0
cmd_pub = None

# ===================== Callback Function =====================
def rpm_callback(msg):
    """Called whenever Arduino publishes a new RPM value."""
    global current_rpm
    current_rpm = msg.data
    rospy.loginfo("Measured RPM: %.2f" % current_rpm)

# ===================== Motor Command Function =====================
def send_motor_command(speed):
    """
    Publishes a Twist message to control the motor.
    :param speed: Desired motor speed in range [-1.0, 1.0]
    """
    global cmd_pub
    cmd = Twist()
    cmd.linear.x = speed
    cmd.angular.z = 0.0
    cmd_pub.publish(cmd)
    rospy.loginfo("Command sent: linear.x = %.2f" % speed)

# ===================== Interactive Control Loop =====================
def control_loop():
    """
    Interactive loop for user input to control motor speed.
    Allows keyboard input between -1.0 and 1.0.
    """
    rate = rospy.Rate(2)  # 2 Hz
    while not rospy.is_shutdown():
        try:
            val = raw_input("Enter speed (-1.0 to 1.0) or q to quit: ")
            if val.lower() == 'q':
                break
            speed = float(val)
            if speed > 1.0:
                speed = 1.0
            elif speed < -1.0:
                speed = -1.0

            send_motor_command(speed)
            rate.sleep()

        except ValueError:
            rospy.logwarn("Invalid input, please enter a number between -1.0 and 1.0.")

# ===================== ROS Node Initialization =====================
def init_ros_node():
    """Initializes ROS node, publisher, and subscriber."""
    global cmd_pub
    rospy.init_node('motor_test_node', anonymous=True)
    cmd_pub = rospy.Publisher('/cmd_vel', Twist, queue_size=10)
    rospy.Subscriber('/wheel_rpm', Float32, rpm_callback)
    rospy.loginfo("Coyote Motor Test Node started.")
    rospy.loginfo("Use keyboard input to control motor speed.")

# ===================== Main Entry Point =====================
if __name__ == '__main__':
    try:
        init_ros_node()
        control_loop()
    except rospy.ROSInterruptException:
        pass
    except KeyboardInterrupt:
        rospy.loginfo("User interrupted execution.")
    finally:
        # Stop motor when exiting
        if cmd_pub is not None:
            send_motor_command(0.0)
        rospy.loginfo("Motor stopped. Exiting motor_test_node...")

