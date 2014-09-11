void adjustCurrentPower(){    
  calculateDeviations();
  int current = getCurrentGyroDeviation();
  lastPower = currentPower;
  if(current < setpoint || current > setpoint){
    int last = lastGyroOffset;
    current = getCurrentGyroDeviation();
    currentPower = currentPower + current;
    }
  }

double getCurrentPower(){
  return currentPower;
}

double getLastPower(){
  return lastPower;
}

void initiate(){
  lastGyroOffset = 0;
  currentGyroOffset = 0;
  currentGyroDeviation = 0;
  lastGyroDeviation = 0;
}

double getCurrentGyroOffset(){      
  return currentGyroOffset;
}

double getLastGyroOffset(){  
  return lastGyroOffset;
}

void calculateOffsets(){
  lastGyroOffset = abs(gyroReading - setpoint);
  updateIMUSensors();
  currentGyroOffset = abs(gyroReading - setpoint);
}

void calculateDeviations(){
  calculateOffsets();
  lastGyroDeviation = currentGyroDeviation;
  currentGyroDeviation = abs(getCurrentGyroOffset() - getLastGyroOffset());  
}

double getCurrentGyroDeviation(){    
  return currentGyroDeviation;
}

double getLastGyroDeviation(){
  return lastGyroDeviation;
}

