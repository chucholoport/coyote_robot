#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
coyote_robot.py
=======================================
Educational ROS node for publishing Twist commands to control a two-wheel Arduino robot.

This node publishes geometry_msgs/Twist messages to /cmd_vel at a fixed rate,
simulating forward motion without rotation.

Developed for ROS Melodic (Python 2.7)
Author: Coyote Robotics Lab
"""

import rospy
from geometry_msgs.msg import Twist

# ===================== Global Variables =====================
cmd_pub = None

# ===================== ROS Node Initialization =====================
def init_ros_node():
    """
    Initializes ROS node and publisher.
    """
    global cmd_pub
    rospy.init_node('coyote_robot_controller', anonymous=True)
    cmd_pub = rospy.Publisher('/cmd_vel', Twist, queue_size=10)
    rospy.loginfo("Coyote Robot Node initialized. Publishing Twist commands...")

# ===================== Main Loop =====================
def control_loop():
    """
    Publishes forward commands at 2 Hz until shutdown.
    """
    rate = rospy.Rate(2)  # 2 Hz
    while not rospy.is_shutdown():
        send_forward_command()
        rate.sleep()

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
        # Stop robot when exiting
        if cmd_pub is not None:
            cmd = Twist()
            cmd.linear.x = 0.0
            cmd.angular.z = 0.0
            cmd_pub.publish(cmd)
            rospy.loginfo("Robot stopped. Exiting coyote_robot_controller...")
