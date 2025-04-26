//
#include "ros/ros.h"
#include "sri_force_sensor_driver/sriCommDefine.h"
#include "sri_force_sensor_driver/sriCommManager.h"
int main(int argc, char *argv[])
{
	ros::init(argc, argv, "sri_driver_demo_node");
	ros::NodeHandle nodeHandle;
	printf("SRI Serial Demo.\n");
	CSRICommManager commManager(nodeHandle,115200);
	if (commManager.Init("COM1") == true) //Init system
	{
		if (commManager.Run() == true)  //running system
		{
			while (ros::ok())
			{
				getchar();
				std::vector<double> sensorData;
				sensorData.resize(6);
				sensorData = commManager.readSensorData();
				std::cout << "[force sensor]: " << sensorData[0] << ", " << sensorData[1] << ", " << sensorData[2] << ", " 
						  << sensorData[3] << ", " << sensorData[4] << ", " << sensorData[5] << std::endl;
			}
		}
	}
	printf("Demo done!\nPress ENTER to close.\n");
	getchar();
    return 0;
}

