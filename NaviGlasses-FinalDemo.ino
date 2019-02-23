#include <math.h>
// Made by Mois Bourla, Akhil Burle, and Nikhil Nakka
String str = "";
String current_location_str = "";
double current_location[2]; // present location as per phone's GPS 
double coordinates[75]; // coordinates that arrive over bluetooth
int is_available = 1;
int last_node = 0;
double pv[2]; // Past vector (heading)
double fv[2]; // What your future heading will be
int latitude_index = 0;
int longitude_index = 1;
int location_counter = 0;
// Pinouts
const int voltagePin = A0;
const int led = 5;
const int right = 4;
const int left = 2;
class endpoint
{
  public:
    double lat;
    double lon;
};
endpoint ep[27];

void setup() {
  /* 
   *  Define IO for hardware
   *  
   */
  pinMode(right, OUTPUT);
  pinMode(left, OUTPUT);
  pinMode(led, OUTPUT);
/*
 * Establish Serial communication with phone over bluetooth and parse input
 */
  Serial.begin(9600);
  const String endChar = ")";
  char inChar; // current char being read


  str += "(1 1 1 2 2 2 2 1 3 2 3 3 3 2)";
  Serial.println(str + " end of input string");
  int spaces[100] = {0};
  int k = 0;
  for (int i = 0; i < str.length(); i++)
  {
    if (str[i] == '(' || str[i] == ')' || str[i] == ' ')
    {
      spaces[k] = i;
      k++;
    }
  }
  int l = 1;
  while (spaces[l] != 0)
  {
    coordinates[l - 1] = str.substring(spaces[l - 1] + 1, spaces[l]).toDouble(); // cutting string up and conveting to doubles and storing into an array
    l++;
  }

  for (int i = 0; i < 10; i++)
  {
    Serial.println(coordinates[i], 5);
  }
  Serial.println("end of input string");


  int z = 0;

  while (coordinates[longitude_index] != 0)
  {
    ep[z].lat = coordinates[latitude_index];
    ep[z].lon = coordinates[longitude_index];
    latitude_index = latitude_index + 2;
    longitude_index = longitude_index + 2;
    z++;
  }
  for (int i = 1; i <= 3; i++)
  {
    Serial.println(ep[i].lat, 5);
    Serial.println(ep[i].lon, 5);
  }
  Serial.println("node data");


  pv[0] = ep[1].lat - ep[0].lat;// past vector
  pv[1] = ep[1].lon - ep[0].lon;// future vector


}

void loop()
{
/*
 * Determine if it is dark out and set the safety LED on.
 * Print statements only for demo
 */
  int sensorValue = analogRead(voltagePin);
  float voltage= sensorValue * (5.0 / 1023.0);
  Serial.print("my voltage is: ");
  Serial.println(voltage);
  if(voltage<1.6)
  {
    digitalWrite(led,HIGH);
    
    Serial.println("on");
  }
  else
  {
    digitalWrite(led,LOW);
  }
/*
 * Determine current location from bluetooth sensor
 */
  int delimiter;
  for (int i = 0; i < current_location_str.length(); i++)
  {
    if (str[i] == ' ' || str[i] == ',')
    {
      delimiter = i;
    }
  }

/*
 * add current location to an array
 */
  current_location[0] = coordinates[location_counter];//current_location_str.substring(0, delimiter).toDouble();
  current_location[1] = coordinates[location_counter + 1]; //current_location_str.substring((delimiter + 1), current_location_str.length() - 1).toDouble();
  location_counter += 2;
// Wait 1 second before updating
  delay(1000);
  Serial.println(current_location_str + "<-- This is the location string ");
  Serial.println(current_location[0]);
  Serial.println(current_location[1]);



// Set the future vectors by looking at the difference between google maps endpoint 

  fv[0] = ep[last_node + 1].lat - ep[last_node].lat;
  fv[1] = ep[last_node + 1].lon - ep[last_node].lon;



/* 
 *  This code only runs for the final leg of the trip.
 *  Determine if the user is within ~2m of the  final endpoint
 *  If the user is at an endpoint direct them towards the next endpoint using the correct output.
 */

  /*input final destination latitude and longitude*/
  if (sqrt(sq(current_location[0] - ep[last_node].lat) + sq(current_location[1] - ep[last_node].lon)) <= pow(10, -4)) // distance between current loc and waypoint
  {
    if (last_node == 0) // first time
    {
      if ((abs(ep[last_node + 1].lon - ep[last_node].lon)) > (abs(ep[last_node + 1].lat - ep[last_node].lat)))
      {
        if (ep[last_node + 1].lon > ep[last_node].lon)
        {
          digitalWrite(right, HIGH);
          Serial.println("Right");
          delay(3000);
          digitalWrite(right, LOW);
        }
        if (ep[last_node + 1].lon < ep[last_node].lon)
        {
          digitalWrite(left, HIGH);
          Serial.println("Left");
          delay(3000);
          digitalWrite(left, LOW);
        }
      }
      /*
       * If the user has overshoot their endpoint, tell them to go back.
       */
      if ((abs(ep[last_node + 1].lon - ep[last_node].lon)) < (abs(ep[last_node + 1].lat - ep[last_node].lat)))
      {

        if (ep[last_node + 1].lat < ep[last_node].lat)
        {
          digitalWrite(right, HIGH);
          digitalWrite(left, HIGH);
          Serial.println("Right");
          Serial.println("Left");
          delay(3000);
          digitalWrite(right, LOW);
          digitalWrite(left, LOW);
        }
      }
    }
    /*
     * If the trip is still in progress, we compute the angle between our present vector and our future vector. 
     * If we need to turn left, the left motor is signalled. If we need to turn right, we signal the right motor.
     * We turn on both motors if we need to turn around, and this triggers hardware logic.
     */
    else
    {


      if (atan2(fv[0], fv[1]) > atan2(pv[0], pv[1]))
      {
        if (atan2(fv[0], fv[1]) - atan2(pv[0], pv[1]) == PI)
        {
          digitalWrite(left, HIGH);
          digitalWrite(right, HIGH);
          Serial.println("Left");
          Serial.println("Right");
          delay(3000);
          digitalWrite(left, LOW);
          digitalWrite(right, LOW);
        }
        digitalWrite(left, HIGH);
        Serial.println("Left");
        delay(3000);
        digitalWrite(left, LOW);
      }
      else if (atan2(fv[0], fv[1]) < atan2(pv[0], pv[1]))
      {
        digitalWrite(right, HIGH);
        Serial.println("Right");
        delay(3000);
        digitalWrite(right, LOW);
      }


    }

  }
  /*
   * This step prepares for the next leg of the journey. 
   * We set our future vector to our past vector
   * and 
   */
  pv[0] = fv[0];
  pv[1] = fv[1];
  last_node++;
  current_location_str = "";
}
