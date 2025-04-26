#ifndef SRI_COMM_MANAGER_H
#define SRI_COMM_MANAGER_H

#include "ros/ros.h"
#include "sriCommDefine.h"
#include "sriCommATParser.h"
#include "sriCommM8218Parser.h"

#include "sriSerialManager_linux.h"
#include "geometry_msgs/Wrench.h"
#include "LockCircleQueue.hpp"
#include "filters.h"
#include <vector>

class CSRICommManager
{
public:
	CSRICommManager(const ros::NodeHandle& node_handle, const unsigned int buad_rate);
	~CSRICommManager();

	bool Init(std::string  com);
	bool Run();
	bool Stop();
	std::vector<double> readSensorData();

	bool SendCommand(std::string command, std::string parames);
	
	bool OnNetworkFailure(std::string infor);//闂侇偅淇洪鍡樺緞闁秵鏅搁柡鍌樺€栫€氾�?
	bool OnCommACK(std::string command);//ACK閹煎瓨妫冮弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁瑰湱鏌夐幓顏堝箯閻戣姤鏅搁柡鍌樺€栫€氾�?
	bool OnCommM8218(float fx, float fy, float fz, float mx, float my, float mz);//GSD闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柟鍦焿閹活亪骞忛悜鑺ユ櫢闁哄倶鍊栫€氾拷
	void SensorDataPublish(float *data_, float *dataFiltered_);

private:
	#ifdef  IS_WINDOWS_OS
		CSRICommSerial mSerial;
	#else
		SerialManeger_linux mSerial_linux;
	#endif


	CSRICommATParser mATParser;//AT闁圭ǹ娲弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫€氬綊鏌ㄩ悤鍌涘
	CSRICommM8218Parser mM8218Parser;//GSD闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柟褰掓敱閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫€氬綊鏌ㄩ悢鍛婄伄闁瑰嚖鎷�?

	bool mIsGetACK;
	std::string mCommandACK;
	std::string mParamesACK;

	ros::NodeHandle nodeHandle_;
	double update_rate;
	const unsigned int buad_rate_;
	ros::Publisher sixForceSensorPublisher;
	ros::Publisher sixForceSensorPublisherFiltered;
	LockCircleQueue<std::vector<double>> lockcircleQueue_;

	//set zero
	double zeroData_[6]= {0};;
	double homeFtData_[6] = {0};
	int zeroCount_;

	//filters
	std::vector<digital_lp_filter<double>> lpf_;
	
};

#endif

