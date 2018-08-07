//ROS Communications
#include <ros/ros.h>
	// Messages
	#include <ardrone_autonomy/Navdata.h>
	#include <std_msgs/Empty.h>
	#include <geometry_msgs/Twist.h>
	#include <geometry_msgs/Pose.h>
	#include <nav_msgs/Odometry.h>
	#include <tf/transform_listener.h>
	#include <opencv/cv.h>

	// #include <hast/flag.h>
	#include <usma_plugins/flag.h>
	#include <usma_plugins/ardrone_pose.h>

class optitrackAutopilot
{
	private:
		// shutdown manager
		ros::Subscriber ShutDown_sub;
			std_msgs::Empty Null_msg;

		// uav commands
		ros::Publisher uav_cmd_pub;
			geometry_msgs::Twist uav_cmd_msg;
			double YawRateCommand;
			std::string s_uav_cmd_topic;

		// mocap subscriber
		ros::Subscriber mocap_pose_sub;
			std::string s_mocap_pose_topic;
			geometry_msgs::Pose uav_mocap_pose_msg;
			usma_plugins::ardrone_pose uav_mocap_ardrone_pose;
			tf::Quaternion uav_qt;
			tf::Vector3 uav_vt;
			tf::Transform uav_TF;
			double uav_RPY[3];
		    tf::Matrix3x3 uav_R;
		    cv::Mat R_from_yaw;

		// desired pose subscriber
		ros::Subscriber desired_pose_sub;
			std::string s_desired_pose_topic;
			// geometry_msgs::Pose uav_desired_pose_global_msg, uav_desired_pose_body_msg;
			// # position.x/.y/.z : meters
			// # heading : radians
			usma_plugins::ardrone_pose uav_desired_pose_global_msg, uav_desired_pose_body_msg;

		// PV variables
		double Kp, Kv;

	public:
		ros::NodeHandle n;

	optitrackAutopilot()
	{
		/*--------- Initialize ROS Communication & Variables ------------- */
		/*-----  Publishers and Subscribers */

		ros::param::get("~uav_cmd_topic", s_uav_cmd_topic);
			uav_cmd_pub = n.advertise<geometry_msgs::Twist>	(s_uav_cmd_topic, 1);

		ros::param::get("~uav_desired_pose_topic", s_desired_pose_topic);
			desired_pose_sub = n.subscribe(s_desired_pose_topic,   10,  &optitrackAutopilot::updateDesiredPose, this);

		ros::param::get("~mocap_pose_topic", s_mocap_pose_topic);
			mocap_pose_sub = n.subscribe(s_mocap_pose_topic,   10,  &optitrackAutopilot::updatePose, this);

		ros::param::get("~Kp", Kp);
		ros::param::get("~Kv", Kv);


		initDesiredPose();

		ROS_INFO("optitrackAutopilot Constructed");
	}

	void updatePose(const geometry_msgs::Pose::ConstPtr& mocap_pose_msg)
	{
		uav_mocap_pose_msg.position = mocap_pose_msg->position;

		uav_mocap_pose_msg.orientation = mocap_pose_msg->orientation;
        
        tf::Quaternion qt ( 
        	uav_mocap_pose_msg.orientation.x, 
        	uav_mocap_pose_msg.orientation.y, 
        	uav_mocap_pose_msg.orientation.z, 
        	uav_mocap_pose_msg.orientation.w);
        tf::Vector3 vt ( 
        	uav_mocap_pose_msg.position.x, 
        	uav_mocap_pose_msg.position.y, 
        	uav_mocap_pose_msg.position.z);
        tf::Transform baseTF ( uav_qt, uav_vt );
        uav_vt = vt;
        uav_qt = qt;
        uav_TF = baseTF;
        tf::Matrix3x3 m(qt);
        uav_R = m;
	    m.getRPY(uav_RPY[0], uav_RPY[1], uav_RPY[2]);

		uav_mocap_ardrone_pose.position.x = mocap_pose_msg->position.x;
		uav_mocap_ardrone_pose.position.y = mocap_pose_msg->position.y;
		uav_mocap_ardrone_pose.position.z = mocap_pose_msg->position.z;
		uav_mocap_ardrone_pose.heading = uav_RPY[2];

		// now compute UAV commands
			uav_Kp();

	}

	void initDesiredPose()
	{
		uav_desired_pose_body_msg.position.x = uav_desired_pose_body_msg.position.y = 0; // meters
		uav_desired_pose_global_msg.position.x = uav_desired_pose_global_msg.position.y = 0; // meters
		uav_desired_pose_body_msg.position.z = uav_desired_pose_global_msg.position.z = 1.0; // meters
		uav_desired_pose_body_msg.heading = uav_desired_pose_global_msg.heading = 0; // radians
		R_from_yaw = (cv::Mat_<double>(3, 3) << 1, 0, 0,
												0, 1, 0,
												0, 0, 1);

	}

	void updateDesiredPose(const usma_plugins::ardrone_pose::ConstPtr& desired_pose_msg)
	{
		uav_desired_pose_global_msg.heading = desired_pose_msg->heading; // radians
		uav_desired_pose_global_msg.position = desired_pose_msg->position; // radians
	}

	void uav_Kp()
	{
		cv::Mat global_position_error = (cv::Mat_<double>(3, 1) <<  
			uav_desired_pose_global_msg.position.x - uav_mocap_ardrone_pose.position.x,
			uav_desired_pose_global_msg.position.y - uav_mocap_ardrone_pose.position.y,
			uav_desired_pose_global_msg.position.z - uav_mocap_ardrone_pose.position.z);
		double global_heading_error = uav_desired_pose_global_msg.heading - uav_mocap_ardrone_pose.heading;
        // ROS_INFO("uav_Kp global position and heading errors [x, y, z, Y] : [%2.3f, %2.3f, %2.3f, %2.4f]", 
        // 	global_position_error.at<double>(0,0),
        // 	global_position_error.at<double>(1,0),
        // 	global_position_error.at<double>(2,0),
        // 	global_heading_error);

		// convert global frame to uav body frame
		double cosyaw = cos(uav_RPY[2]);
		double sinyaw = sin(uav_RPY[2]);
		R_from_yaw = (cv::Mat_<double>(3, 3) <<  cosyaw, sinyaw, 0,
												-sinyaw, cosyaw, 0,
												 0, 0, 1);
		cv::Mat body_position_error = R_from_yaw * global_position_error;
        // ROS_INFO("uav_Kp body position and heading errors [x, y, z, Y] : [%2.3f, %2.3f, %2.3f, %2.4f]", 
        // 	body_position_error.at<double>(0,0),
        // 	body_position_error.at<double>(1,0),
        // 	body_position_error.at<double>(2,0),
        // 	global_heading_error);


		uav_cmd_msg.linear.x = Kp*body_position_error.at<double>(0,0);
		uav_cmd_msg.linear.y = Kp*body_position_error.at<double>(1,0);
		uav_cmd_msg.linear.z = Kp*body_position_error.at<double>(2,0);
		uav_cmd_msg.angular.x = 0;
		uav_cmd_msg.angular.y = 0;
		uav_cmd_msg.angular.z = Kp*global_heading_error;

		cmdUAV(uav_cmd_msg);
	}

	void cmdUAV(geometry_msgs::Twist cmd_vel)
	{
		uav_cmd_pub.publish(cmd_vel);
		// cmd_count += 1;
		// fprintf (pFile,"\nuavCon.cmd.time(%d,:) = % -6.8f;\n", cmd_count, ros::Time::now().toSec());
		// fprintf (pFile,"uavCon.cmd.linear(%d,:)  = [% -6.8f, % -6.8f, % -6.8f];\n", cmd_count, cmd_vel.linear.x, cmd_vel.linear.y, cmd_vel.linear.z);
		// fprintf (pFile,"uavCon.cmd.angular(%d,:) = [% -6.8f, % -6.8f, % -6.8f];\n", cmd_count, cmd_vel.angular.x, cmd_vel.angular.y, cmd_vel.angular.z);
	}



	void nodeShutDown(const usma_plugins::flag::ConstPtr& ShutDown)
	{
		if(ShutDown->flag)
			{ros::shutdown();}
	}

};



int main(int argc, char **argv)
{
	ros::init(argc, argv, "optitrackAutopilot");
	optitrackAutopilot oA;
	ros::spin();
	return 0;
}
