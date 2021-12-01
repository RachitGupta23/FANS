# airborne_networks

## [A] Setting up PX4 SITL with ROS Melodic (Ubuntu 18.04) in a simulated environment in Gazebo.

### 0) Resources used
* PX4 Dev official documentation : https://docs.px4.io/master/en/
* Gitbook from GAAS : https://gaas.gitbook.io/guide/software-realization-build-your-own-autonomous-drone/build-your-own-autonomous-drone-e01-offboard-control-and-gazebo-simulation 

### 1) Install ROS Melodic from the official [ROSWiki documentation](http://wiki.ros.org/melodic/Installation/Ubuntu)
Make sure you install the correct ROS Distribution corresponding to your Ubuntu version,

### 2) Install dependencies

* Execute the following commands to install dependencies : 
```
sudo apt-get update -y
sudo apt-get install git zip cmake build-essential genromfs ninja-build exiftool astyle python-argparse python-empy python-toml python-numpy python-dev python-pip python3-pip gstreamer1.0-plugins-bad gstreamer1.0-libav gstreamer1.0-gl python-catkin-tools python-rosinstall-generator ros-melodic-gazebo-* -y
sudo -H pip install --upgrade pip
sudo -H pip install pandas jinja2 pyserial pyyaml
sudo -H pip3 install pyulog
sudo usermod -a -G dialout $USER
sudo apt-get remove modemmanager -y

sudo apt install -y \
	ninja-build
	exiftool
	python-argparse
	python-empy
	python-toml
	python-numpy
	python-yaml
	python-dev
	python-pip
	ninja-build
	protobuf-compiler
	libeigen3-dev
	genromfs
pip install \
	pandas \
	jinja2 \
	pyserial \
	cerberus \
	pyulog \
	numpy \
	toml \
	pyquaternion
```

### 3) Download MAVROS, MAVLink packages and build them
* After ROS installation is complete, set up and initialize a ROS workspace with any name (for e.g. `sop_ws`)
```
mkdir -p sop_ws/src
catkin init && wstool init src
```
* Add the .rosinstall files for MAVROS and MAVLink and build them (Note that the below steps are to be done in the root of the workspace)
```
rosinstall_generator --rosdistro melodic mavlink | tee /tmp/mavros.rosinstall
rosinstall_generator --upstream mavros | tee -a /tmp/mavros.rosinstall
wstool merge -t src /tmp/mavros.rosinstall
wstool update -t src -j4
rosdep install --from-paths src --ignore-src -y
./src/mavros/mavros/scripts/install_geographiclib_datasets.sh

```
* Build your workspace
```
catkin build 
```

### 4) Download the PX4 Firmware package and build it
```
cd ~/sop_ws/src
git clone https://github.com/PX4/PX4-Autopilot.git --recursive
make px4_sitl_default gazebo 
```

### 5) Add the Gazebo model and ROS package paths in your `.bashrc` file
* Add the following lines to your `.bashrc` folder so Gazebo can find the sdf models and ROS can find the packages.
```
source ~/sop_ws/src/PX4-Autopilot/Tools/setup_gazebo.bash ~/sop_ws/src/PX4-Autopilot/ ~/sop_ws/src/PX4-Autopilot/build/px4_sitl_default
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:~/sop_ws/src/PX4-Autopilot
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:~/sop_ws/src/PX4-Autopilot/Tools/sitl_gazebo
```
* If you don't want the Gazebo paths to be printed in your terminal every time, comment out the print (`echo`) statements in the `sop_ws/src/PX4-Autopilot/Tools/setup_gazebo.bash` file.
```
#echo -e "GAZEBO_PLUGIN_PATH $GAZEBO_PLUGIN_PATH"
#echo -e "GAZEBO_MODEL_PATH $GAZEBO_MODEL_PATH"
#echo -e "LD_LIBRARY_PATH $LD_LIBRARY_PATH"
```
### 6) [Optional] Add downward pointing camera to your drones in Gazebo
* Inside the file `iris.sdf.jinja` in `PX4-Autopilot/Tools/sitl_gazebo/models/iris`, add the following code snippet at the end after the `</plugin>` tag and before the `</model>` 
```xml
<include>
	<uri>model://fpv_cam</uri>
	<pose>0.1 0 0 0 1.5708 0</pose>
</include>
<joint name="fpv_cam_joint" type="fixed">
	<child>fpv_cam::link</child>
	<parent>base_link</parent>
	<axis>
		<xyz>0 0 1</xyz>
		<limit>
			<upper>0</upper>
			<lower>0</lower>
		</limit>
	</axis>
</joint>
```
### 7) Testing the setup
* Source your workspace in the current working terminal
```
source ~/sop_ws/devel.setup.bash
```
* Launch PX4 and MAVROS nodes using the following command, you must see an iris drone in a Gazebo world environment.
```
roslaunch px4 mavros_posix_sitl.launch
```
* In a new terminal, run the following command
```
rostopic echo /mavros/state
```
If it shows the `connected` parameter to be `True` as shown below, then you may assume that your setup works fine. 
```
---
header: 
  seq: 29
  stamp: 
    secs: 29
    nsecs: 336000000
  frame_id: ''
connected: True
armed: False
guided: False
manual_input: True
mode: "AUTO.LOITER"
system_status: 3
---
```

### n) Tips for fast execution
* Create an alias in your `.bashrc` file with the name of your workspace to source your workspace from any terminal super fast. For e.g. if the name of your workspace is `sop_ws` (assumed to be there in the `~` directory), add the following line in your `.bashrc` file (present in the `~` folder).
```
alias sop_ws='source ~/sop_ws/devel/setup.bash'
```

## [B] Installing and setting up NS3 with cmake