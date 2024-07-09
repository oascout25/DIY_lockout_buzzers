#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Define the radio configuration
RF24 radio(D0, D8); // CE, CSN (D0, D8)

enum BuzzerColor { RED = 0, YELLOW, GREEN, BLUE };
BuzzerColor buzzerColor = RED; // Change this for each buzzer

const char* addressLookup[] = {"1BUZZ", "2BUZZ", "3BUZZ", "4BUZZ"};
const byte* address = (const byte*)addressLookup[buzzerColor];

// Define the pins for the button and LED
const int buttonPin = A0; // A0 for the Call/Clear button
const int ledPin = D4; // D4 for the external LED

// Variable to store the state of the button
bool buttonPressed = false;
bool waitingForResponse = false;
bool waitingForclear = false;
bool confirmationReceived = false;
bool lockoutReceived = false;
int lastButtonState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long lastBuzzTime = 0;
unsigned long retryDelay = 1000; // Retry sending BUZZ every 1 second
int retryCount = 0;
const int maxRetries = 5; // Limit retries to 5 attempts

const int threshold = 100; // Threshold for analog read

void setup() {
  Serial.begin(9600);
  Serial.println("Welcome to JEOPARDY!");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Initialize the radio
  if (!radio.begin()) {
    Serial.println("Radio hardware is not responding!");
    while (1) {}
  } else {
    Serial.println("Radio hardware initialized successfully");
  }

  radio.setChannel(90); // Set the channel to 90
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address); // Controller address
  radio.setPALevel(RF24_PA_LOW);

  // Debugging information
  Serial.print("Buzzer Color: ");
  Serial.println(buzzerColor == RED ? "Red" : buzzerColor == YELLOW ? "Yellow" : buzzerColor == GREEN ? "Green" : "Blue");
  Serial.print("Address: ");
  Serial.println((char*)address);

  radio.startListening(); // Start listening for incoming messages
  Serial.println("Buzzer setup complete");
  Serial.println("Ready to play! Buzz in when you know the question!");
}

void loop() {
  int analogValue = analogRead(buttonPin);
  int reading = (analogValue < threshold) ? LOW : HIGH;

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonPressed) {
      buttonPressed = reading;

      if (buttonPressed == LOW && !waitingForResponse && !waitingForclear && !confirmationReceived && !lockoutReceived) {
        Serial.print("Buzzer activeted, sending BUZZ signal...");
        sendBuzzSignal();
      }
    }
  }

  lastButtonState = reading;

  if (radio.available()) {
    handleRadioResponse();
  }

  delay(10); // Small delay to prevent WDT clear and reduce spamming of the Serial Monitor
}

void sendBuzzSignal() {
  radio.stopListening(); // Stop listening to send message
  char text[] = "BUZZ";
  bool report = radio.write(&text, strlen(text) + 1); // Send null-terminated string
  Serial.println(report ? "Success" : "Fail");
  if (report) {
    lastBuzzTime = millis();
    retryCount = 0;
    waitingForResponse = true;
  } else {
    retryCount++;
    delay(retryDelay); // Wait before retrying
    if (retryCount < maxRetries) {
      sendBuzzSignal(); // Retry sending BUZZ signal
    } else {
      Serial.println("Failed to send BUZZ after max retries");
      waitingForResponse = false;
    }
  }
  radio.startListening(); // Start listening for confirmation
  Serial.println("Waiting for response from the CONTROLLER... GOOD LUCK!");
}

void handleRadioResponse() {
  char response[32] = {0};
  radio.read(&response, sizeof(response));
  Serial.print("The CONTROLLER says: ");
  Serial.println(response);

  if (strncmp(response, "CONFIRM", 7) == 0) {
    digitalWrite(ledPin, HIGH); // Turn on the LED
    Serial.println("You're up! What's the question???");
    buttonPressed = false;
    confirmationReceived = true;
    waitingForclear = true;
    waitingForResponse = false;
  }
  if (strncmp(response, "LOCKOUT", 7) == 0) {
    buttonPressed = false;
    waitingForclear = true;
    lockoutReceived = true;
    waitingForResponse = false;
    Serial.println("Guess you'll need to be quicker next round!");
  }
  if (strncmp(response, "CLEAR", 5) == 0) {
    digitalWrite(ledPin, LOW); // Turn off the LED
    buttonPressed = false; // Clear the button state
    waitingForResponse = false;
    waitingForclear = false;
    confirmationReceived = false;
    lockoutReceived = false;
    retryCount = 0; // Clear retry count
    Serial.println("Next round, keep your finger on that buzzer!");
  }
}
