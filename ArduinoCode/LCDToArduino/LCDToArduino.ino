
// global vars
//
//
// pin digital level readings
int level_COM[4] = {
  -1, -1, -1, -1};
// first four number of 14 is not used
int level_SEG[4][14];
int level_avg_SEG[4][14];

// short term statistic numbers
long count_SEG_high[4][14] = {
  0};
long count_SEG_low[4][14] = {
  0};


int digit_dictionary[11][7] = {
 {1, 1, 1, 0, 1, 1, 1},
 {0, 0, 1, 0, 0, 1, 0},
 {1, 0, 1, 1, 1, 0, 1},
 {1, 0, 1, 1, 0, 1, 1},
 {0, 1, 1, 1, 0, 1, 0},
 {1, 1, 0, 1, 0, 1, 1},
 {1, 1, 0, 1, 1, 1, 1},
 {1, 0, 1, 0, 0, 1, 0},
 {1, 1, 1, 1, 1, 1, 1},
 {1, 1, 1, 1, 0, 1, 1},
 {0, 0, 0, 0, 0, 0, 0}
};

// long term statistic numbers
long statistic_valid_reads = 0;
long statistic_skips_due_to_noise = 0;

// temp vars
int selected_COM_index = -1;

// functions declarations
//
//
// helper 
//
void myAssert(boolean assertion, char* reason);
void myAbort(char* reason);


// read all segments and store into level_SEG[][];
void readAllSegments(int selectedCOM);

// get the readings from level_SEG[][] 
//  and accumulate to count_SEG_high[][] and count_SEG_low[][]
void applySegmentsReadingToLocalCounts(int selectedCOM);

// print data from level_avg_SEG
void print_avg_readings();

// save avg of seg data to level_avg_SEG
void save_avg_readings();

// clear count_SEG_low and high
void clear_readings();

// given the digit index read the number from LCD
// 0 <= digit <= 4
int readNumberFromLCD(int digit);

void setup() 
{
  // initialize digital pins as input for reading segment4-13:
  for (int digitalPin = 4; digitalPin < 14; digitalPin ++)
  {
    pinMode(digitalPin, INPUT);
  }

  // initialize analog pin as digital input to read level_COM0-3
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  // initialize serial level_COMmunication:
  Serial.begin(57600);

  Serial.println("Begin");
}

void loop() 
{

  // read the pushbutton input pin:
  {
    level_COM[0] = digitalRead(A0);
    level_COM[1] = digitalRead(A1);
    level_COM[2] = digitalRead(A2);
    level_COM[3] = digitalRead(A3);

    int sum_of_COM_levels = level_COM[0] + level_COM[1] + level_COM[2] + level_COM[3];
    if (sum_of_COM_levels < 3)
    {
      statistic_skips_due_to_noise ++;
      Serial.println("Warning: two COMs are low (might because of noise), skip this reading.");
      return;
    }
    else if (sum_of_COM_levels == 4)
    {
      // no signal
      return;
    }
    else if (sum_of_COM_levels == 3)
    {
      // great, this is what we need. we will read the segment levels later.
    }
    else
    {
      myAbort("never bigger than 4.");
    }
  }

  // we are here because one COM level is low (their sum is 3)
  // now setting selected_COM_index
  {
    statistic_valid_reads++;

    // here we set the selected COM index
    selected_COM_index = -1;
    for (int COM_Pin = 0; COM_Pin < 4; COM_Pin++)
    {
      if (!level_COM[COM_Pin])
      {
        selected_COM_index = COM_Pin;
        break;
      }
    }
    myAssert(selected_COM_index != -1, "one COM level must be low");
  }

  // now read all segments
  {
    readAllSegments(selected_COM_index);
    applySegmentsReadingToLocalCounts(selected_COM_index);

    if (statistic_valid_reads > 1000)
    {
      statistic_valid_reads = 0;
      save_avg_readings();
      print_avg_readings();
      clear_readings();
    }
  }
}













// functions implementations

void myAssert(boolean assertion, char* reason)
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

void readAllSegments(int selectedCOM)
{
  myAssert(selectedCOM < 4, "selected com must < 4 readAllSegments");
  // not using PORT reads because this is more readable.
  for (int digitalPin = 4; digitalPin < 14; digitalPin ++)
  {
    level_SEG[selectedCOM][digitalPin] = digitalRead(digitalPin);
  }
}

void applySegmentsReadingToLocalCounts(int selectedCOM)
{
  myAssert(selectedCOM < 4, "selected com must < 4 applySegmentsReadingToLocalCounts");
  for (int digitalPin = 4; digitalPin < 14; digitalPin ++)
  {
    if (level_SEG[selectedCOM][digitalPin])
    {
      count_SEG_high[selectedCOM][digitalPin] ++;
    }
    else
    {
      count_SEG_low[selectedCOM][digitalPin] ++;
    }
  }
}



void print_avg_readings()
{
  static long print_avg_readings_count = 0;

  Serial.println("-----"+String(print_avg_readings_count++)+"---------");
  for (int COMPin = 0; COMPin < 4; COMPin ++)
  {
    for (int SEGPIN = 4; SEGPIN < 14; SEGPIN ++)
    {
      Serial.print(level_avg_SEG[COMPin][SEGPIN]);
    }
    Serial.println("");
  }
  print_real_numbers();
  Serial.println("");
}

void save_avg_readings()
{
  for (int COMPin = 0; COMPin < 4; COMPin ++)
  {
    for (int SEGPIN = 4; SEGPIN < 14; SEGPIN ++)
    {
      if (count_SEG_low[COMPin][SEGPIN]<count_SEG_high[COMPin][SEGPIN])
      {
        level_avg_SEG[COMPin][SEGPIN] = 1;
      }
      else if (count_SEG_low[COMPin][SEGPIN] == count_SEG_high[COMPin][SEGPIN])
      {
        level_avg_SEG[COMPin][SEGPIN] = 9;
      }
      else
      {
        level_avg_SEG[COMPin][SEGPIN] = 0;
      }
    }
  }
}

void clear_readings()
{
  for (int COMPin = 0; COMPin < 4; COMPin ++)
  {
    for (int SEGPIN = 4; SEGPIN < 14; SEGPIN ++)
    {
      count_SEG_low[COMPin][SEGPIN] = 0;
      count_SEG_high[COMPin][SEGPIN] = 0;
    }
  }
}


void printDigitFromLED(int digit)
{
  int real_number = readNumberFromLCD(digit);
  if (real_number>= 0 && real_number<= 9)
  {
    Serial.print(real_number);
  }
  else if (real_number == 10)
  {
    // do nothing
  }
  else
  {
    Serial.print('x');
  }
}

void printDot()
{
  if (level_avg_SEG[0][7])
  {
    Serial.print('.');
  }
}

void print_real_numbers()
{
  Serial.print("weight: ");
  printDigitFromLED(4);
  printDigitFromLED(3);
  printDigitFromLED(2);
  printDigitFromLED(1);
  printDot();
  printDigitFromLED(0);
}

int readNumberFromLCD(int digit)
{
  if(digit == 0)
  {
    return readNumberFromLCDLOW(5);
  }
  else if (digit == 1)
  {
    return readNumberFromLCDLOW(7);
  }
  else if (digit == 2)
  {
    return readNumberFromLCDLOW(9);
  }
  else if (digit == 3)
  {
    return readNumberFromLCDLOW(11);
  }
  else if (digit == 4)
  {
    return readNumberFromLCDHIGH();
  }
  else
  {
    myAbort("digit should between 0~4");
    return -1;
  }
}


int readNumberFromLCDLOW(int startSEG)
{
  int digital_ordered[7];
  digital_ordered[0] = level_avg_SEG[3][startSEG];
  digital_ordered[1] = level_avg_SEG[3][startSEG+1];
  digital_ordered[2] = level_avg_SEG[2][startSEG];
  digital_ordered[3] = level_avg_SEG[2][startSEG+1];  
  digital_ordered[4] = level_avg_SEG[1][startSEG+1];  
  digital_ordered[5] = level_avg_SEG[1][startSEG];
  digital_ordered[6] = level_avg_SEG[0][startSEG+1];
  
  /*
  Serial.print('<');
  for (int i = 0; i<7; i++)
  {
    Serial.print(digital_ordered[i]);
  }
  Serial.println('>');
  */
  
  int result = 0;
  for (; result < 11; result++)
  {
    boolean same = true;
    for (int i = 0; i < 7; i++)
    {
      if (digital_ordered[i] != digit_dictionary[result][i])
      {
        same = false;
        break;
      }
    }
    if (same)
    {
      return result;
    }
  }
  return -1;
}

int readNumberFromLCDHIGH()
{
  int sum = level_avg_SEG[2][13] + level_avg_SEG[1][13];
  if (sum == 2)
  {
    return 1;
  }
  else if (sum == 0)
  {
    return 10;
  }
  return -1;
}







