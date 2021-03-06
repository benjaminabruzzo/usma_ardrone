# Set up the lab:
The Ar.Drone is configured to connect to the Linksys e2500 router (bottom in this image)<br />
![Open Project](https://github.com/westpoint-robotics/usma_ardrone/blob/master/media/routers_1.jpg)

Make sure that the green ethernet cable is connected to the e2500 router, this connects the computer running Optitrack Motive to the network that will be used for ROS and the AR.Drone.  This must happen before Optitrack Motive is launched in windows.  Open Motive Now. <br />

Open the project "abruzzo_face_tracking".  This should load in the opbjects "Ardrone" and "blockhead".
![Open Project](https://github.com/westpoint-robotics/usma_ardrone/blob/master/media/open_project.jpg)

To configure the Motive workspace, from the drop down in the top right corner, choose "abruzzo_uavface".  Make sure Optitrack Tracker is publishing on the Linksys network, which should be something like 192.168.0.xx.  This number must match the 
![Open Project](https://github.com/westpoint-robotics/usma_ardrone/blob/master/media/optitrack_IP.jpg)  

The IP address here must match the server_ip in the launch files mentioned below:
![Open Project](https://github.com/westpoint-robotics/usma_ardrone/blob/master/media/ip_addresses.png)  

## Ar.Drone Yolo

The AR.Drone should automatically connect to the linksys router, but it takes about 30-60 seconds.  After that, to test whether the ardrone is connected to the same network:

	ping 192.168.0.25

If the ping is successful, everything should be configured to work. **To run the face tracking demo**, the following command file will launch the control and tracking nodes for the ardrone

	roslaunch optitrack_controller pistol.launch


## Ar.Drone Face Tracking demo with optitrack feedback

<!-- Set AR.Drone to connect to router, using a laptop or PC, connect to the ssid "ardrone2_<######>", in terminal type 

	roscd wpa_support 
	. script/connect_linksys

"connect_linksys" must be called each time the drone is powered down, such as when changing the battery. Now connect the laptop to the linksys router network ARDRONE250024ghz and test whether the ardrone is connected to the same network:

	ping 192.168.0.25

if the ping returns a response time, then the ardrone is connected to the same network on the correct ip address



Assuming the optitrack software and cameras are booted and running, to launch the demo, first launch the vrpn service. This will stream the optitrack pose data as a ros message. (note this assumes the Ethernet cable for the optitrack pc has been switched from EECSDS3 to the linksys router, it has not been tested on EECSDS3)

	roslaunch optitrack_controller vrpn.launch

'rostopic list' should populate with the rigid bodies in the optitrack field of view. If you do not see rigid bodies on the 'rostopic list' output, check to make sure that the optitrack software is registering the rigid bodies in its software.  sometimes the too many reflective markers are out of view of the optitrack cameras. Try moving the drone around until it is seen by the cameras. 'ctrl-c' to close the vrpn server. -->

The AR.Drone should automatically connect to the linksys router, but it takes about 30-60 seconds.  After that, to test whether the ardrone is connected to the same network:

	ping 192.168.0.25

If the ping is successful, everything should be configured to work. **To run the face tracking demo**, the following command file will launch the control and tracking nodes for the ardrone

	roslaunch optitrack_controller track_face.launch network:=linksys

Finally to have the drone takeoff :

	roslaunch optitrack_controller liftoff.launch

To land : hit enter in either window used to launch the previous two commands

To give the drone permission t track faces:

	rostopic pub -1 /ardrone/face/face_permission_topic std_msgs/Empty


---

If there is an issue during takeoff, or if you need to do a hard abort for some reason, you may need to reset the drone before taking off a second time:
	
	rostopic pub -1 /ardrone/reset std_msgs/Empty

If there is an issue, you can always e-stop the Ar.Drone by putting a hand above and below the body of the UAV, grabbing it and flipping it over.  This will put the drone into an e-stop state and **will require** a reset before taking off again.


---
If you plan on logging data, run :
z
	. ~/ros/src/usma_ardrone/scripts/makedirs.sh 20181121 1 5

	. ~/ros/src/usma_ardrone/scripts/makedirs.sh <yyyymmdd> <run first index> <run last index>


	roslaunch optitrack_controller track_face.launch network:=linksys logging:=true date:=20181121 run:=001

---
Connect to the AR.Drone directly
=======
If you are directly connected to the ardrone network, the following launch file will launch the ardrone controller
	roslaunch optitrack_controller ardrone_direct.launch


---
Launch the face_tracker when directly connected to the ardrone
=======
roslaunch optitrack_controller ardrone_direct.launch <br />
roslaunch optitrack_controller track_face.launch <br />


---
Connect to ARDrone using linksys router and running the whole face tracking pipeline
=======
roslaunch optitrack_controller track_face.launch network:=ardrone #(directly connect to uav network) <br />
roslaunch optitrack_controller track_face.launch network:=linksys <br />
roslaunch optitrack_controller track_face.launch network:=EECSDS3 <br />

roslaunch optitrack_controller track_face.launch network:=linksys logging:=true run:=004<br />







---
Finding a MAC address
=======
	>> telnet 192.168.1.1
	>> ifconfig

ardrone2_234879  ::  90:03:B7:38:0D:72 <br />
ardrone2_065412  ::  90:03:B7:31:18:D5 <br />
ardrone2_049677  ::  90:03:B7:E8:3C:D8 <br />


script/install_linksys
"install_linksys" needs only to be called once, but 



rostopic pub -1 /ardrone/takeoff std_msgs/Empty
