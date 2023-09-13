/*
 * Author : Michael Rice 
 * Email : m.rice11@nuigalway.ie
 * Dual Axis Solar Tracking System with Night Detection, Temperature Sensing and LCD Display Screen
 */

#include<LiquidCrystal_I2C.h>//lcd library with i2c
LiquidCrystal_I2C lcd(0x27, 16, 2);//instantiates lcd display with i2c address, no. of columns and no. of rows

//defines the pins for our externally powered motor(ie horizontal axis))
#define forwards 10
#define backwards 11

//sets pins of input ldrs
int LDR1 = A0;//top ldr
int LDR2 = A1;//bottom
int LDR3 = A2;//left ldr
int LDR4 = A3;//right ldr

//vertical axis motor pin numbers
int vPin1 = 13;
int vPin2 = 12;

const int numReadings = 10;//size of arrays used for moving average filters, can be changed quickly and easily if desired

//creation of array for each LDR of size numReadings
int readingsTop[numReadings];
int readingsBottom[numReadings];
int readingsLeft[numReadings];
int readingsRight[numReadings];

int index = 0;//index variable to keep track of position in array
//total variable for each array/LDR
int totalTop;
int totalBottom;
int totalLeft;
int totalRight;

//average variable for each array/LDR
int averageTop;
int averageBottom;
int averageLeft;
int averageRight;
int nightVal = 200;//value of ldr below which the program considers it to be night time

void setup() 
{
  //begins serial communication
  Serial.begin(9600);
  
  lcd.init();//initialises the LCD object
  lcd.backlight();//turns on the backlight of the LCD
  lcd.setCursor(0,0);//sets the cursor of the LCD object to the first row and first column
  
  //sets ldr pins as inputs
  pinMode(LDR1, INPUT);
  pinMode(LDR2, INPUT);
  pinMode(LDR3, INPUT);
  pinMode(LDR4, INPUT);
  
  //sets motor pins as outputs
  pinMode(vPin1,OUTPUT);
  pinMode(vPin2,OUTPUT);  
  pinMode(forwards,OUTPUT);
  pinMode(backwards,OUTPUT);


//when device being setup, all arrays for moving average filters are cleared
  for(int value = 0; value<numReadings ; value++)
  {
    readingsTop[value] = 0;
    readingsBottom[value] = 0;
    readingsLeft[value] = 0;
    readingsRight[value] = 0;
  }
}

void loop() 
{
  //reads in all of the values from the LDRs into specifically named variables
  int top = analogRead(LDR1);
  int bottom = analogRead(LDR2);
  int left = analogRead(LDR3);
  int right = analogRead(LDR4);

  
  //method to detect whether or not it is night
  //this method clears the lcd and prints a message to show its night if the correct LDR values are observed
  //it also ensures all motors are turned off in this mode
  if(top<nightVal && right<nightVal && left<nightVal  && bottom<nightVal )
  {
      analogWrite(forwards,0);
      analogWrite(backwards,0);
      digitalWrite(vPin1,LOW);
      digitalWrite(vPin2,LOW);
      lcd.clear();
      lcd.print("NightMode Active");
  }

  //day time code
  else
  {
      //clears the lcd before each loop iteration   
      lcd.clear();
    //reads in sensor values from each of the LDRs and assigns the values to variables to variables
 
//subtract the last readings from each array
totalTop = totalTop - readingsTop[index];
totalBottom = totalBottom - readingsBottom[index];
totalLeft = totalLeft - readingsLeft[index];
totalRight = totalRight - readingsRight[index];

//input the new readings directly from each analogPin
readingsTop[index] = top;
readingsBottom[index] = bottom;
readingsLeft[index] = left;
readingsRight[index] = right;

//update the total value by adding the new value to it
totalTop = totalTop + readingsTop[index];
totalBottom = totalBottom + readingsBottom[index];
totalLeft = totalLeft + readingsLeft[index];
totalRight = totalRight + readingsRight[index];

index = index + 1;//increment the index variable to move to the next position in the array

//if loop to detect if the index variable is greater than or equal to the size of the array
//if it is, reset the value back to the start( back to 0 )
if(index>=numReadings)
{
  index = 0;
}

//calculates new average value for each array with updated array
averageTop = totalTop/numReadings;
averageBottom = totalBottom/numReadings;
averageLeft = totalLeft/numReadings;
averageRight = totalRight/numReadings;

//totalAvg is the average of the 4 other averages/print this to the serial monitor
//this avg is the total average of the last 40(10 for each ldr) readings
  int totalAvg = (averageTop+averageBottom+averageLeft+averageRight)/4;


  int verticalAvg = (top+bottom)/2;//avg of the two ldrs in the vertical plane
  int horizontalAvg = (left+right)/2;//avg of the two ldrs in the horizontal plane
  int avg = (verticalAvg+horizontalAvg)/2;//current instantaneous average of each of the 4 LDRs

  //tolerance value that changes how much difference there needs to be between the pairs of LDRs before movement occurs
  int moveTolerance = 60;



  
//if the value of the left ldr is greater than the right ldr plus the tolerance, turn the motor on in the correct direction
//turns on the motor in the correct direction in max motor speed setting
  if(left > right + moveTolerance)
  {
      analogWrite(backwards,255);
  }
 //if the value of the right ldr is greater than the left ldr plus the tolerance, turn the motor on in the correct direction
 //turns on the motor in the correct direction in max motor speed setting
 else if(right > left + moveTolerance)
  {
      analogWrite(forwards,255);
  }
//if the tolerance value has not been exceeded, turn both motors off
  else
  {
      analogWrite(forwards,0);
      analogWrite(backwards,0);
//if the value of the top ldr is greater than the bottom plus the tolerance, turn the motors on in the upwards direction
  if(top > bottom + moveTolerance)
  {
      digitalWrite(vPin1,LOW);
      digitalWrite(vPin2,HIGH);
  }
//if the value of the bottom ldr is greater than the top plus the tolerance, turn the motor in the downwards direction  
  else if(bottom > top + moveTolerance)
 {
     digitalWrite(vPin1,HIGH);
     digitalWrite(vPin2,LOW);
 }
 //if the tolerance value has not been exceeded, turn both motors off
  else
 {
      digitalWrite(vPin1,LOW);
     digitalWrite(vPin2,LOW);
  }



    //serial printing sequence that prints the results from the moving average filters to the serial monitor
    Serial.print("Top LDR value = ");
    Serial.println(top);
    Serial.print("Current Top Average(last 10 readings): ");
    Serial.println(averageTop);
    Serial.println();
    Serial.print("Bottom LDR value = ");
    Serial.println(bottom);
    Serial.print("Current Bottom Average(last 10 readings): ");
    Serial.println(averageBottom);
    Serial.println();
    Serial.print("Left LDR value = ");
    Serial.println(left);
    Serial.print("Current Left Average(last 10 readings): ");
    Serial.println(averageLeft);
    Serial.println();
    Serial.print("Right LDR value = ");
    Serial.println(right);
    Serial.print("Current Right Average(last 10 readings): ");
    Serial.println(averageRight);
    Serial.println();
    delay(2000);

    

//lcd printing sequence that prints the axes averages and the current instantaneous average to the lcd screen 
lcd.setCursor(0,0);//sets lcd cursor to first row, first column
lcd.print("Avg Light:");
lcd.print(avg);//prints the current average light level
lcd.setCursor(0,1);//sets the cursor to the second column, first row
lcd.print("Y:");//
lcd.print(verticalAvg);//prints average of 2 ldrs in y plane
lcd.print(" ");
lcd.print("X:");
lcd.print(horizontalAvg);//prints average of 2 ldrs in x plane

//code to read in the temperature sensor value and print to the monitor/lcd screen (whichever is preferred)
float temp = analogRead(A4);//reads the temp sensor value in from an analog pin to a float variable
temp = temp * 0.38828125;//conversion of voltage reading to degrees celsius

//temperature output can be printed to serial monitor if desired but commented out until then 
//Serial.print("Temp : ");
//Serial.print(temp);
//Serial.println("*C");

//if the temperature is over 25 degrees print a statement to say it is hot
if(temp>25)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("It's really hot!");
  lcd.setCursor(0,1);
  lcd.print("Temp : ");
  lcd.print(temp);
  lcd.print("*C");
  //Serial.println("It's really hot!");
}
}

  delay(50);//this value determines the delay between each sensor reading/the lower this value is determines how sensitive the device itself is 

}

  

  
  









 

  
  
