# LVTN
## ROS package for simulator
1. Copy to src folder of catkin_ws.
2. In catkin_ws run: ` catkin_make install` to build and install package
3. Run: ` roslaunch team211 team211.launch` to launch ros package
4. Run CDS 2018 simulator 
## Google drive folder 
Link: [link](https://drive.google.com/open?id=1JvJ-h9TVdwQssJfrebwz1lI_tdiAgkTT)
### Include
Source code for LVTN <br />
1. opencv_root.tar.gz for intergrated opencv to ultra96 
To copy to SD card:` sudo dd if=opencv_root.tar.gz of=/dev/sdX` with /dev/sdX is your ext4 partition
2. CDS 2018 simulator
* Extract and run <br>
```bash
cd linux
chmod +x test.x86_64
./test.x86_64
```
* In team and ws type: team211 - ws://localhost:9090 <br>
### Performance Report
1. Hardware.log time report of aplication run on ultra96 with FPGA
2. Sofware.log time report of Aplication run on Ultra96 ARM
3. ROS_report.log time report of Aplication run on PC in ROS.
