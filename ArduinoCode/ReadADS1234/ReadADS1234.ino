
#define DATA_PIN       (12)
#define CLOCK_PIN      (11)
#define POWERUP_PIN    (10)
#define SPEED_PIN      (9)

#define GAIN1_PIN      (8)
#define GAIN0_PIN      (7)

#define A0_PIN         (4)
#define A1_PIN         (5)

#define CLOCK_PERIOD   (0.001)

#define ZERO_LEFT      (4769)
#define ZERO_RIGHT     (-4110)


#define AVG_ROUND            (10)
#define ONCE_READ_ROUND      (1)

long number2_24 = 1;

long init_data[4];
long cur_data[4];
float RATIO = 29200/85.2;

void setup() {
  Serial.begin(115200);

  pinMode(DATA_PIN, INPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(POWERUP_PIN, OUTPUT);
  pinMode(A0_PIN, OUTPUT);
  pinMode(A1_PIN,OUTPUT);
  pinMode(SPEED_PIN,OUTPUT);
  pinMode(GAIN0_PIN,OUTPUT);      
  pinMode(GAIN1_PIN,OUTPUT);

  setGain(128);
  setDataSpeed(1);
  digitalWrite(POWERUP_PIN, LOW);

  delay(1000);
  digitalWrite(POWERUP_PIN, HIGH);
  digitalWrite(CLOCK_PIN, LOW);


  for (int i=1; i<24; i++)
  {
    number2_24 *= 2;
  }
  setChannel(0);
  init_data[0] = ZERO_LEFT;
  init_data[1] = ZERO_RIGHT;
}

long result0;
long result1;
// the loop routine runs over and over again forever:
#define AVG_SIZE  40
long record[AVG_SIZE] = {0};
long bufferSum = 0;
int currentBufferIndex = 0;
long sum;
void loop() 
{ 
  setChannel(0);
  result0 = read24digit() - init_data[0];
  setChannel(1);
  result1 = read24digit() - init_data[1];
  sum = result0 + result1;
  bufferSum -= record[currentBufferIndex];
  bufferSum += sum;
  record[currentBufferIndex] = sum;
  currentBufferIndex++;
  if (currentBufferIndex == AVG_SIZE)
  {
    currentBufferIndex = 0;
  }
  Serial.print(result0);
  Serial.print('\t');
  Serial.print(result1);
  Serial.print('\t');
  Serial.print(sum);
  Serial.print('\t');
  Serial.println(bufferSum/RATIO/AVG_SIZE);
}

void myAssert(boolean assertion, char* reason)
{
  if (!assertion)
  {
    Serial.println("Assertion Failed");
    Serial.println(reason);
    while (1);
  }
}

void myAssert(boolean assertion, String reason)
{
  if (!assertion)
  {
    Serial.println("Assertion Failed");
    Serial.println(reason);
    while (1);
  }
}

void myAbort(char* reason)
{
  myAssert(0, reason);
}

void myAbort(String reason)
{
  myAssert(0, reason);
}

void waitUntilDataReady()
{
  int isReady;
  do
  {
    isReady = digitalRead(DATA_PIN);
  }
  while(isReady);
}

void setChannel (int in_channel)
{
  myAssert(in_channel >= 0 && in_channel < 4, "channel should be 0~3");
  digitalWrite(A0_PIN, 0);
  digitalWrite(A1_PIN, in_channel>0);
}

void setDataSpeed(boolean in_speed)
{
  digitalWrite(SPEED_PIN, in_speed);
}

// gain can be 1, 2, 64, 128
void setGain(int gain)
{
  if (gain == 1)
  {
    digitalWrite(GAIN0_PIN, 0);
    digitalWrite(GAIN1_PIN, 0);
  }
  else if (gain == 2)
  {
    digitalWrite(GAIN0_PIN, 1);
    digitalWrite(GAIN1_PIN, 0);
  }
  else if (gain == 64)
  {
    digitalWrite(GAIN0_PIN, 0);
    digitalWrite(GAIN1_PIN, 1);
  }
  else if (gain == 128)
  {
    digitalWrite(GAIN0_PIN, 1);
    digitalWrite(GAIN1_PIN, 1);
  }
  else
  {
    myAbort(String("wrong gain: ")+String(gain)+ String("should be 1, 2, 8, 128"));
  }
}

boolean read_one_digit()
{
  digitalWrite(CLOCK_PIN, HIGH);
  delay(CLOCK_PERIOD);
  digitalWrite(CLOCK_PIN, LOW);
  delay(CLOCK_PERIOD);
  int cur_digit = digitalRead(DATA_PIN);
  return (cur_digit == HIGH);
}

long read24digit ()
{
  waitUntilDataReady();
  long number = 0;
  for (int i=0; i<24; i++)
  {
    number = number*2;
    if(read_one_digit())
    {
      number += 1;
    }
  }
  // dummy read
  read_one_digit();

  if (number > 0x7FFFFF)
  {
    number = number - 0x1000000;
  }
  return number/100;
}

long read24digit_avg()
{
  long l_min;
  long l_max;
  long current;
  long sum = 0;
  for (int i = 0; i < AVG_ROUND; i++)
  {
    current = read24digit ();
    sum += current;
    if (i == 0)
    {
      l_min = current;
      l_max = current;
    }
    else
    {
      l_min = min(l_min, current);
      l_max = max(l_max, current);
    }
  }
  return (sum - l_min - l_max)/(AVG_ROUND-2);
}









