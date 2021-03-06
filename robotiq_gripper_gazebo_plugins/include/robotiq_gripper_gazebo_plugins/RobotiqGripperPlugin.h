/*
 * Copyright 2014 Open Source Robotics Foundation
 * Copyright 2015 Clearpath Robotics
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef GAZEBO_ROBOTIQ_GRIPPER_PLUGIN_HH
#define GAZEBO_ROBOTIQ_GRIPPER_PLUGIN_HH

#include <string>
#include <vector>

// Gazebo
#include <gazebo/common/Plugin.hh>
#include <gazebo/common/Time.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo_ros/node.hpp>

#include <hrim_actuator_gripper_msgs/msg/state_gripper.hpp>
#include <hrim_actuator_gripper_msgs/msg/state_finger_gripper.hpp>
#include <hrim_actuator_gripper_srvs/srv/specs_finger_gripper.hpp>
#include <hrim_actuator_gripper_srvs/srv/control_finger.hpp>

// HRIM messages
#include "hrim_generic_srvs/srv/id.hpp"
#include "hrim_generic_msgs/msg/status.hpp"
#include "hrim_generic_msgs/msg/power.hpp"
#include "hrim_generic_srvs/srv/simulation3_d.hpp"
#include "hrim_generic_srvs/srv/simulation_urdf.hpp"
#include "hrim_generic_srvs/srv/specs_communication.hpp"
#include "hrim_generic_msgs/msg/state_communication.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>

#define MIN_FORCE 10
#define MAX_FORCE 125

#define MAX_PAYLOAD 2.5

#define MIN_SPEED 30
#define MAX_SPEED 250

#define MAX_ACCELERATION 0

#define MAX_LENGHT 209
#define MAX_ANGLE 0.87

#define REPEATABILITY 0.08

// Boost
#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace std::chrono_literals;

/// \brief A plugin that implements the Robotiq 2-Finger Adaptative Gripper.
namespace gazebo
  {
  class RobotiqGripperPlugin : public gazebo::ModelPlugin
  {

    void createGenericTopics(std::string node_name);

    /// \brief Constructor.
    public: RobotiqGripperPlugin();

    /// \brief Destructor.
    public: virtual ~RobotiqGripperPlugin();

    private: uint8_t GetCurrentPosition(const gazebo::physics::JointPtr &_joint);
    private: bool IsHandFullyOpen();

    private: void UpdateJointPIDs();

    private: void UpdatePIDControl();

    // Documentation inherited.
    public: void Load(gazebo::physics::ModelPtr _parent, sdf::ElementPtr _sdf);

    /// \brief World pointer.
    private: gazebo::physics::WorldPtr world;

    /// \brief Parent model of the hand.
    private: gazebo::physics::ModelPtr model;

    /// \brief Pointer to the SDF of this plugin.
    private: sdf::ElementPtr sdf;

    /// \brief gazebo world update connection.
    private: gazebo::event::ConnectionPtr updateConnection;

    private: std::vector<physics::JointPtr> joint_v_;
    private: std::map<std::string, double> joint_multipliers_;
    private: std::vector<physics::JointPtr> rotation_v_;
    private: std::map<std::string, double> rotation_multipliers_;

    /// \brief Min. joint speed (rad/s). Finger is 125mm and tip speed is 22mm/s.
    private: const double MinVelocity = 0.176;

    /// \brief Max. joint speed (rad/s). Finger is 125mm and tip speed is 110mm/s.
    private: const double MaxVelocity = 0.88;

    /// \brief Velocity tolerance. Below this value we assume that the joint is
    /// stopped (rad/s).
    private: const double VelTolerance = 0.002;

    /// \brief Position tolerance. If the difference between target position and
    /// current position is within this value we'll conclude that the joint
    /// reached its target (rad).
    private: const double PoseTolerance = 0.002;

    int sentido = 1;

    double kp = 1;
    double ki = 0.1;
    double kd = 0.01;
    double imin = 0.0;
    double imax = 0.0;
    double cmdmax = 10.0;
    double cmdmin = -10.0;

    double targetJoint  = 0.0;
    double targetRotation  = 0.0;

    /// A pointer to the GazeboROS node.
    gazebo_ros::Node::SharedPtr ros_node_;

    std::shared_ptr<rclcpp::Publisher<hrim_generic_msgs::msg::Status>> status_pub;
    std::shared_ptr<rclcpp::Publisher<hrim_generic_msgs::msg::Power>> power_pub;
    std::shared_ptr<rclcpp::Publisher<hrim_generic_msgs::msg::StateCommunication>> state_comm_pub;

    rclcpp::Service<hrim_generic_srvs::srv::ID>::SharedPtr id_srv_;
    rclcpp::Service<hrim_generic_srvs::srv::Simulation3D>::SharedPtr sim_3d_srv_;
    rclcpp::Service<hrim_generic_srvs::srv::SimulationURDF>::SharedPtr sim_urdf_srv_;
    rclcpp::Service<hrim_generic_srvs::srv::SpecsCommunication>::SharedPtr specs_comm_srv_;
    rclcpp::Service<hrim_actuator_gripper_srvs::srv::ControlFinger>::SharedPtr srv_;
    rclcpp::Service<hrim_actuator_gripper_srvs::srv::SpecsFingerGripper>::SharedPtr specs_srv_;

    std::shared_ptr<rclcpp::Publisher<hrim_actuator_gripper_msgs::msg::StateFingerGripper>> gripper_finger_state_pub;
    std::shared_ptr<rclcpp::Publisher<hrim_actuator_gripper_msgs::msg::StateGripper>> gripper_state_pub;

    void gripper_service(const std::shared_ptr<rmw_request_id_t> request_header,
          const std::shared_ptr<hrim_actuator_gripper_srvs::srv::ControlFinger::Request> request,
          std::shared_ptr<hrim_actuator_gripper_srvs::srv::ControlFinger::Response> response);

    void SpecsCommunicationService(
        const std::shared_ptr<rmw_request_id_t> request_header,
        const std::shared_ptr<hrim_generic_srvs::srv::SpecsCommunication::Request> req,
        std::shared_ptr<hrim_generic_srvs::srv::SpecsCommunication::Response> res);

    void SpecsFingerGripperService(
        const std::shared_ptr<rmw_request_id_t> request_header,
        const std::shared_ptr<hrim_actuator_gripper_srvs::srv::SpecsFingerGripper::Request> req,
        std::shared_ptr<hrim_actuator_gripper_srvs::srv::SpecsFingerGripper::Response> res);

    void IDService(
        const std::shared_ptr<rmw_request_id_t> request_header,
        const std::shared_ptr<hrim_generic_srvs::srv::ID::Request> req,
        std::shared_ptr<hrim_generic_srvs::srv::ID::Response> res);

    void Sim3DService(
        const std::shared_ptr<rmw_request_id_t> request_header,
        const std::shared_ptr<hrim_generic_srvs::srv::Simulation3D::Request> req,
        std::shared_ptr<hrim_generic_srvs::srv::Simulation3D::Response> res);

    void URDFService(
        const std::shared_ptr<rmw_request_id_t> request_header,
        const std::shared_ptr<hrim_generic_srvs::srv::SimulationURDF::Request> req,
        std::shared_ptr<hrim_generic_srvs::srv::SimulationURDF::Response> res);

    void timer_power_msgs();
    void timer_status_msgs();
    void timer_comm_msgs();
    void timer_gripper_status_msgs();

    // void readfullFile(std::string file_to_read, hrim_generic_msgs::msg::Simulation3D& msg_sim_3d);

    std::shared_ptr<rclcpp::TimerBase> timer_status_;
    std::shared_ptr<rclcpp::TimerBase> timer_gripper_status_;
    std::shared_ptr<rclcpp::TimerBase> timer_power_;
    std::shared_ptr<rclcpp::TimerBase> timer_comm_;

  };
}
#endif  // GAZEBO_ROBOTIQ_GRIPPER_PLUGIN_HH
