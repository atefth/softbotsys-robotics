long revolutionTime = 305480;

int minDistance = 5;

int steering = 1;

long last_micros = 0;
long last_lift_micros = 0;
long movementTime = 0;

long turningTime = 655480;

void printTimeData(){
  Serial.print("last micros : ");
  Serial.print(last_micros);
  Serial.print(" ---- current micros : ");
  Serial.print(micros());
  Serial.print(" ---- time moved : ");
  Serial.print(micros() - last_micros);  
  Serial.print(" ---- distance moved : ");
  Serial.print(getDistanceMoved());
  Serial.println();  
}

long timeToMove(int distanceToMove){
  return ((distanceToMove/minDistance) * revolutionTime);
}

int getDistanceMoved(){
  return (micros()/revolutionTime) * minDistance;
}

boolean isTime(){
  if (micros() - last_micros < movementTime){
    return false; 
  }
  else{
    return true; 
  }
}

boolean isTurnTime(){
  if (micros() - last_micros < turningTime){
    return false; 
  }
  else{
    return true; 
  }
}

boolean isTimeSpan(long span){
  if(micros() - last_lift_micros < span){
    return false;
  }
  else{
    return true;
  } 
}

void goForward(){
  while(!isTime()){      
    moveForward();
    printTimeData();
  }
  brake();
}

void goForward(long span){
  while(!isTimeSpan(span)){      
    moveForward();
    printTimeData();
  }
  brake();
}

void goBackward(){
  while(!isTime()){  
    moveBackward();
    printTimeData();
  }
  brake();
}

void goBackward(long span){
  while(!isTimeSpan(span)){      
    moveBackward();
    printTimeData();
  }
  brake();
}

void turnLeft(){
  while(!isTurnTime()){      
    moveLeft();
    printTimeData();
  }
  brake();
}

void turnLeft(long span){
  while(!isTimeSpan(span)){      
    moveLeft();
    printTimeData();
  }
  brake();
}

void turnRight(){
  while(!isTurnTime()){      
    moveRight();
    printTimeData();
  }
  brake();
}

void turnRight(long span){
  while(!isTimeSpan(span)){      
    moveRight();
    printTimeData();
  }
  brake();
}