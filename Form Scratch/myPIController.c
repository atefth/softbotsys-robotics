#define setpoint 0;
#define Kp 5;
#define Kd 2;
#define power 100;

double gyroReading;

double currentGyroOffset;
double lastGyroOffset;
double currentGyroDeviation;
double lastGyroDeviation;

// double inputForGyro;
// double outputForGyro;
// double setpointForGyro;

double currentPower;
double lastPower;

void initiate(){
	lastGyroOffset = gyroReading;
	currentGyroOffset = gyroReading;
}

double getCurrentGyroOffset(){
  lastGyroOffset = currentGyroOffset;
  currentGyroOffset = (gyroReading - lastGyroOffset);
  return currentGyroOffset;
}

double getCurrentGyroDeviation(){
  lastGyroDeviation = currentGyroDeviation;
  currentGyroDeviation = (currentGyroOffset - lastGyroOffset);
  return currentGyroDeviation;
}

void adjustCurrentPower(){		
	int current = getCurrentGyroDeviation();
        if(current < setpoint && current > setpoint){
                int last = lastGyroOffset;
		current = getCurrentGyroDeviation();
		if(current > 0 && current < last){
			if(currentPower < lastPower)
			{
				currentPower = currentPower - Kp;
			}else{
				currentPower = currentPower + Kp;
			}
		}else if (current < 0 && current > last){
			if(currentPower > lastPower)
			{
				currentPower = currentPower - Kp;
			}else{
				currentPower = currentPower + Kp;
			}
		}
	}
	power = currentPower;
}
