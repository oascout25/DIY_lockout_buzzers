#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Define the radio configuration
RF24 radio(D0, D8);  // CE, CSN (D0, D8)

const byte address[4][6] = { "1BUZZ", "2BUZZ", "3BUZZ", "4BUZZ" };  // Addresses for the buzzers

const int clearButton = A0;                // Pin for the clear button
const int ledPins[] = { D1, D2, D3, D4 };  // Correct order: D1 (Red for Team 1), D2 (Yellow for Team 2), D3 (Green for Team 3), D4 (Blue for Team 4)

bool roundActive = true;  // To track if a round is active

const int threshold = 100;  // Threshold for analog read

void setup() {
  Serial.begin(9600);
  Serial.println("Welcome to JEOPARDY!");
  pinMode(clearButton, INPUT);  // Button as input

  // Initialize LED pins
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);  // Ensure all LEDs are off initially
  }

  // Initialize the radio
  if (!radio.begin()) {
    Serial.println("Radio hardware is not responding!");
    while (1) {}
  } else {
    Serial.println("Radio hardware initialized successfully");
  }

  radio.setChannel(90);  // Set the channel to 90
  for (int i = 0; i < 4; i++) {
    radio.openReadingPipe(i + 1, address[i]);
    Serial.print("Opened reading pipe ");
    Serial.print(i + 1);
    Serial.print(" with address ");
    Serial.println((char*)address[i]);
  }
  radio.setPALevel(RF24_PA_LOW);

  radio.startListening();  // Start listening for incoming data
  Serial.println("Controller setup complete");
  Serial.println("Ready to play! Start Round!");
}

bool sendWithRetry(const byte* address, const char* message) {
  bool success = false;
  for (int i = 0; i < 5; i++) {  // Retry up to 5 times
    radio.openWritingPipe(address);
    success = radio.write(message, strlen(message) + 1);  // Send null-terminated string
    if (success) break;
    delay(10);  // Small delay before retrying
  }
  return success;
}

void loop() {
  int clearReading = analogRead(clearButton) < threshold ? LOW : HIGH;  // Use analogRead for clear button

  if (clearReading == LOW) {
    Serial.println("Cleaning up this round!");
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[i], LOW);  // Turn off all LEDs
    }
    radio.stopListening();  // Stop listening to send message
    for (int i = 0; i < 4; i++) {
      bool clearStatus = sendWithRetry(address[i], "CLEAR");
      Serial.print("Sent round clear to buzzer ");
      Serial.print(i + 1);
      Serial.print("clear send status: ");
      Serial.println(clearStatus ? "Success" : "Fail");
    }
    radio.startListening();  // Start listening again
    roundActive = true;      // Set roundActive flag to true
    delay(500); // Delay to prevent rapid sends when button is held down
    Serial.println("Round cleared, ready for the next round!");
  }

  uint8_t pipeNum;
  if (radio.available(&pipeNum)) {
    char text[32] = { 0 };
    radio.read(&text, sizeof(text));
    Serial.print("Received ");
    Serial.print(text);
    Serial.print(" From TEAM ");
    Serial.println(pipeNum);  // This should change to look up team name/color

    if (strcmp(text, "BUZZ") == 0 && roundActive) {
      radio.stopListening();  // Stop listening to send message

      int buzzerIndex = pipeNum - 1;
      if (buzzerIndex >= 0 && buzzerIndex < 4) {

        // Send CONFIRM to the first buzzer
        bool confirmStatus = sendWithRetry(address[buzzerIndex], "CONFIRM");
        Serial.print("Sent CONFIRM to team "); 
        Serial.print(buzzerIndex + 1);  // This should change to look up team name/color
        Serial.print(": ");
        Serial.println(confirmStatus ? "Success" : "Fail");
        digitalWrite(ledPins[buzzerIndex], HIGH);  // Turn on the LED for the first buzzer

        // Send LOCKOUT to other buzzers
        for (int j = 0; j < 4; j++) {
          if (j != buzzerIndex) {
            bool lockoutStatus = sendWithRetry(address[j], "LOCKOUT");
            Serial.print("Sent LOCKOUT to Team ");
            Serial.print(j + 1);
            Serial.print(": ");
            Serial.println(lockoutStatus ? "Success" : "Fail");
          }
        }
      } else {
        Serial.println("Invalid buzzer index");
      }

      Serial.print("Lockout initiated. It's Team ");
      Serial.print(buzzerIndex + 1);  // This should change to look up team name/color
      Serial.println("'s prompt!");
      roundActive = false;     // Mark round as inactive after sending confirmation and lockout messages
      radio.startListening();  // Start listening again
    }
  }

  delay(10);  // Small delay to prevent WDT clear and reduce spamming of the Serial Monitor
}
