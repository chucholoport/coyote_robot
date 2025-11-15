#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
coyote_robot.py
=======================================
ROS node that listens to /cmd_vel and forwards Twist commands to the robot.
Also receives wheel RPMs from Arduino and publishes odometry and TF transform.

Developed for ROS Melodic (Python 2.7)
Author: Coyote Robotics Lab
"""

import rospy
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from std_msgs.msg import Float32MultiArray
import tf
import math

# ===================== Global Variables =====================
cmd_pub = None
odom_pub = None
odom_broadcaster = None

x = 0.0
y = 0.0
theta = 0.0
last_time = None

# ===================== Callback: Forward cmd_vel =====================
def cmd_vel_callback(msg):
    global cmd_pub
    rospy.loginfo("Forwarding Twist command: linear=%.2f angular=%.2f", msg.linear.x, msg.angular.z)
    cmd_pub.publish(msg)

# ===================== Callback: Compute Odometry =====================
def rpm_callback(msg):
    global x, y, theta, last_time

    now = rospy.Time.now()
    if last_time is None:
        last_time = now
        return

    dt = (now - last_time).to_sec()
    last_time = now

    # Extract RPMs
    left_rpm = msg.data[0]
    right_rpm = msg.data[1]

    # Robot parameters
    wheel_radius = 0.03  # meters
    wheel_base = 0.12    # meters

    # Convert RPM to m/s
    left_vel = (2 * math.pi * wheel_radius * left_rpm) / 60.0
    right_vel = (2 * math.pi * wheel_radius * right_rpm) / 60.0

    # Differential drive kinematics
    linear = (right_vel + left_vel) / 2.0
    angular = (right_vel - left_vel) / wheel_base

    # Integrate position
    delta_x = linear * math.cos(theta) * dt
    delta_y = linear * math.sin(theta) * dt
    delta_theta = angular * dt

    x += delta_x
    y += delta_y
    theta += delta_theta

    # Publish TF transform
    odom_quat = tf.transformations.quaternion_from_euler(0, 0, theta)
    odom_broadcaster.sendTransform(
        (x, y, 0.0),
        odom_quat,
        now,
        "base_link",
        "odom"
    )

    # Publish Odometry message
    odom = Odometry()
    odom.header.stamp = now
    odom.header.frame_id = "odom"
    odom.child_frame_id = "base_link"

    odom.pose.pose.position.x = x
    odom.pose.pose.position.y = y
    odom.pose.pose.position.z = 0.0
    odom.pose.pose.orientation.x = odom_quat[0]
    odom.pose.pose.orientation.y = odom_quat[1]
    odom.pose.pose.orientation.z = odom_quat[2]
    odom.pose.pose.orientation.w = odom_quat[3]

    odom.twist.twist.linear.x = linear
    odom.twist.twist.angular.z = angular

    odom_pub.publish(odom)

# ===================== ROS Node Initialization =====================
def init_ros_node():
    global cmd_pub, odom_pub, odom_broadcaster, last_time
    rospy.init_node('coyote_robot_controller', anonymous=True)

    last_time = rospy.Time.now()

    cmd_pub = rospy.Publisher('/robot_cmd_vel', Twist, queue_size=10)
    odom_pub = rospy.Publisher('/odom', Odometry, queue_size=10)
    odom_broadcaster = tf.TransformBroadcaster()

    rospy.Subscriber('/cmd_vel', Twist, cmd_vel_callback)
    rospy.Subscriber('/wheel_rpm', Float32MultiArray, rpm_callback)

    rospy.loginfo("Coyote Robot Node initialized. Listening to /cmd_vel and computing odometry...")

# ===================== Main Entry Point =====================
if __name__ == '__main__':
    try:
        init_ros_node()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass
    except KeyboardInterrupt:
        rospy.loginfo("User interrupted execution.")
    finally:
        if cmd_pub is not None:
            stop_cmd = Twist()
            cmd_pub.publish(stop_cmd)
            rospy.loginfo("Robot stopped. Exiting coyote_robot_controller...")
