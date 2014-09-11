#define LW 0
#define RW 1

unsigned long count[2] = {
  0,0};

void setup()
{
  Serial.begin(57600);
  attachInterrupt(0, attachLeft, CHANGE);
  attachInterrupt(1, attachRight, CHANGE);
}

void loop()
{
  Serial.print("Speed:");
  Serial.print(count[LW]);
  Serial.print("\t");
  Serial.println(count[RW]);
  
//  count[LW] = 0;
//  count[RW] = 0;
  delay(200);
}

void attachLeft()
{
  count[LW] ++;
}
void attachRight()
{
  count[RW] ++;
}
