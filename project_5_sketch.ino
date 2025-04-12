/* 
iPoduino v2.0

author: Fernando Navarrete

date of modification: FN 3/28/2025 finished the program for using the play/pause and skip/Next buttons
*/

#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

/* Use pins 20 and 21 with HardwareSerial to communicate with DFPlayer Mini +  Play and Skip button pins 8 */
const int PIN_MP3_RX = 20;        // Connects to module's RX
const int PIN_MP3_TX = 21;        // Connects to module's TX
const int playPauseButtonPin = 3; // Play and Pause button pin
const int skipButtonPin = 4;      // Skip/Next button pin

/* Define serial port objects */
HardwareSerial mySerial1(1);
/* Create the player object which will serve as an interface for our DFPlayer commands */
DFRobotDFPlayerMini player;

/* these varibles will track our states */
bool playing = false;
bool playPauseButtonPressed = false;
bool skipButtonPressed = false;

void setup() {
  /* Initialize USB serial port for debugging */
  Serial.begin(115200);
  
  /* Initialize the pins for HardwareSerial. No pinMode calls needed. */
  mySerial1.begin(9600, SERIAL_8N1, PIN_MP3_RX, PIN_MP3_TX);

  delay(1000); // DO NOT REMOVE THIS DELAY. It is needed to properly initialize HardwareSerial.

  /* Start communication with DFPlayer Mini using player.begin
  player.begin returns true if the connection was a success
  and false if it failed */
  if (!player.begin(mySerial1)) 
  {
    Serial.println("Connecting to DFPlayer Mini failed!");
    while (true); 
  }

  /* Set the player volume to maximum (0 to 30). */
	player.volume(30);

  /* Setup buttons using the internal pull-up resistor */
  pinMode(playPauseButtonPin, INPUT_PULLUP);
  pinMode(skipButtonPin, INPUT_PULLUP);
}

void loop() {
  /* will handle our Play/Pause button case  */
  if (digitalRead(playPauseButtonPin) == LOW) 
  {
    if (!playPauseButtonPressed) 
    {
      playPauseButtonPressed = true;

      if (playing) 
      {
        player.pause();
        Serial.println("Paused");
      } 
      else 
      {
        player.start();
        Serial.println("Playing");
      }
      playing = !playing;
    }
  } 
  else 
  {
    playPauseButtonPressed = false;
  }

  /* will handle our skip case */
  if (digitalRead(skipButtonPin) == LOW) 
  {
    if (!skipButtonPressed) 
    {
      skipButtonPressed = true;

      player.next();
      Serial.println("Next Track");
      playing = true; // will resume if it was previously paused
    }
  } 
  else 
  {
    skipButtonPressed = false;
  }

  delay(50); // debounce correction
}
