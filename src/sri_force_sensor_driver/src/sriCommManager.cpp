#include "sri_force_sensor_driver/sriCommManager.h"

CSRICommManager::CSRICommManager(const ros::NodeHandle& node_handle, const unsigned int buad_rate)
	: nodeHandle_(node_handle),
    buad_rate_(buad_rate),
    update_rate(100){
	zeroCount_ = 0;
	for (int i = 0; i < 6; ++i) {
		lpf_.emplace_back(70.0f, 1/333.0f);
	}
}


CSRICommManager::~CSRICommManager()
{
}
//initialization
bool CSRICommManager::Init(std::string  com)
{

	#ifdef  IS_WINDOWS_OS
		mSerial.OpenPort(com);
		mSerial.AddCommParser(&mATParser);
		mSerial.AddCommParser(&mM8218Parser);
		//bind Network communication failure processing function
		SRICommNetworkFailureCallbackFunction networkFailureCallback = std::bind(&CSRICommManager::OnNetworkFailure, this, std::placeholders::_1);
		mSerial.SetNetworkFailureCallbackFunction(networkFailureCallback);
	#else
		mSerial_linux.OpenSerialPort(ttyUSB0);
		mSerial_linux.AddCommParser(&mATParser);
		mSerial_linux.AddCommParser(&mM8218Parser);
		//bind Network communication failure processing function
		SRICommNetworkFailureCallbackFunction networkFailureCallback = std::bind(&CSRICommManager::OnNetworkFailure, this, std::placeholders::_1);
		mSerial_linux.SetNetworkFailureCallbackFunction(networkFailureCallback);
		sixForceSensorPublisher = nodeHandle_.advertise<geometry_msgs::Wrench>("six_force_sensor",10);
		sixForceSensorPublisherFiltered = nodeHandle_.advertise<geometry_msgs::Wrench>("six_force_sensor_filtered",10);
	#endif




	//bind ACK command Processing function
	SRICommATCallbackFunction atCallback = std::bind(&CSRICommManager::OnCommACK, this, std::placeholders::_1);
	mATParser.SetATCallbackFunction(atCallback);

	//bind M8128 Data processing function
	SRICommM8218CallbackFunction m8218Callback = std::bind(&CSRICommManager::OnCommM8218, this, 
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, 
		std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
	mM8218Parser.SetM8218CallbackFunction(m8218Callback);

	return true;
}
//run
bool CSRICommManager::Run()
{
	#ifdef  IS_WINDOWS_OS
		mSerial.run(buad_rate_);
	#else
	mSerial_linux.SerialRun(buad_rate_);
	#endif

	//Send command to M8128 to set sampling rate
	if (SendCommand("SMPF", "1500") == false)
	{
		if (SendCommand("GSD", "STOP") == false)
		{
		}
		if (SendCommand("SMPF", "1000") == false)
		{
			return false;
		}
	}

	//Send command to M8128 to set data check mode
	if (SendCommand("DCKMD", "SUM") == false)
	{
		return false;
	}
	//Send command to M8128 Start to upload sensor data continuously锟斤�?
	if (SendCommand("GSD", "") == false)
	{
		//GSD has no ACK
		//return false;
	}
	return true;
}

std::vector<double> CSRICommManager::readSensorData(){
	std::vector<double> ftData;
	ftData.resize(6);
	lockcircleQueue_.pop_wait(ftData);
	return ftData;
}


bool CSRICommManager::Stop()
{
	#ifdef  IS_WINDOWS_OS
		mSerial.Close_Com();
	#else
		mSerial_linux.ClosePort();
	#endif	
	return true;
}

bool CSRICommManager::OnNetworkFailure(std::string infor)
{
	printf("OnNetworkFailure = %s\n", infor.data());
	return true;
}


//send command 
bool CSRICommManager::SendCommand(std::string command, std::string parames)
{
	int CommandLenght;
	mIsGetACK = false;
	mCommandACK = "";
	mParamesACK = "";

	//Combination command
	std::string atCommand = "AT+" + command + "=" + parames + "\r\n";
	CommandLenght = atCommand.length();

#ifdef  IS_WINDOWS_OS
	mSerial.ComWrite((BYTE*)atCommand.data(), CommandLenght);
	std::clock_t start = clock();
	while (true)
	{
		if (mIsGetACK == true)
		{
			break;
		}
		std::clock_t end = clock();
		long span = end - start;
		if (span >= 3000)//10s
		{
			return false;
		}
	}
#else
	mSerial_linux.Write((char*)atCommand.data(), CommandLenght);
	timeval start, end;
	gettimeofday(&start, NULL);
	while (true)
	{
		if (mIsGetACK == true)
		{
			break;
		}
		gettimeofday(&end, NULL);
		long span = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000;
		if (span >= 10000)//10s
		{
			return false;
		}
	}
#endif
	//
	if (mCommandACK != command)
	{
		//ACK command error
		return false;
	}
	//
	printf("ACK+%s=%s", mCommandACK.data(), mParamesACK.data());
	//
	return true;
}

//ACK command processing
bool CSRICommManager::OnCommACK(std::string command)
{
	int index = (int)command.find('=');
	if (index == -1)
	{
		mCommandACK = command;
		mParamesACK = "";
	}
	else
	{
		mCommandACK = command.substr(0, index);
		mParamesACK = command.substr(index+1);
	}
	mCommandACK = mCommandACK.substr(4);	
	
	mIsGetACK = true;

	return true;
}
//M8128 data
bool CSRICommManager::OnCommM8218(float fx, float fy, float fz, float mx, float my, float mz)
{
	// printf("M8218 = %f, %f, %f,   %f, %f, %f\n", fx, fy, fz, mx, my, mz);
	float sensorDataTmp[6] = {fx, fy, fz, mx, my, mz};
	float sensorDataFilter[6] = {0};


	if(zeroCount_ < 1000){
		zeroData_[0] += sensorDataTmp[0];
		zeroData_[1] += sensorDataTmp[1];
		zeroData_[2] += sensorDataTmp[2];
		zeroData_[3] += sensorDataTmp[3];
		zeroData_[4] += sensorDataTmp[4];
		zeroData_[5] += sensorDataTmp[5];
		zeroCount_++;
	} else if(zeroCount_ == 1000){
		zeroData_[0] /= 1000;
		zeroData_[1] /= 1000;
		zeroData_[2] /= 1000;
		zeroData_[3] /= 1000;
		zeroData_[4] /= 1000;
		zeroData_[5] /= 1000;
		zeroCount_++;
		std::cout << "[force sensor]: set zeros complete!" << std::endl;
	} else{
		std::vector<double> sensorData;
		sensorData.resize(6);
		for(int i=0; i<6; i++){
			sensorDataTmp[i] -= zeroData_[i];	
		}

		// lpf_fx.input(sensorDataTmp[2]);
		// sensorDataTmp[3] = lpf_fx.output();
		int dataIndex = 0;
		for (auto& filter : lpf_) {
			filter.input(sensorDataTmp[dataIndex]);  
			sensorDataFilter[dataIndex] = filter.output();
			sensorData[dataIndex] = sensorDataFilter[dataIndex];
			dataIndex++;
		}

		lockcircleQueue_.push(sensorData);
		SensorDataPublish(sensorDataTmp, sensorDataFilter);
		return true;
	}
}

void CSRICommManager::SensorDataPublish(float *data_, float *dataFiltered_){
	geometry_msgs::Wrench sixForceData_;
	sixForceData_.force.x = data_[0];
	sixForceData_.force.y = data_[1];
	sixForceData_.force.z = data_[2];
	sixForceData_.torque.x = data_[3];
	sixForceData_.torque.y = data_[4];
	sixForceData_.torque.z = data_[5];
	sixForceSensorPublisher.publish(sixForceData_);

	geometry_msgs::Wrench sixForceDataFiltered_;
	sixForceDataFiltered_.force.x = dataFiltered_[0];
	sixForceDataFiltered_.force.y = dataFiltered_[1];
	sixForceDataFiltered_.force.z = dataFiltered_[2];
	sixForceDataFiltered_.torque.x = dataFiltered_[3];
	sixForceDataFiltered_.torque.y = dataFiltered_[4];
	sixForceDataFiltered_.torque.z = dataFiltered_[5];
	sixForceSensorPublisherFiltered.publish(sixForceDataFiltered_);
}