#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN D8
#define RST_PIN D0
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

byte uids[][4] = {
  {10, 115, 204, 36},
  {10, 66, 118, 36},
  {186, 120, 6, 25},
  {186, 104, 31, 25},
  {224, 87, 84, 25},
};
void setup() {
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}
void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  for (byte i = 0; i < 5; i++) {
    if (rfid.uid.uidByte[0] == uids[i][0] && rfid.uid.uidByte[1] == uids[i][1] && rfid.uid.uidByte[2] == uids[i][2] && rfid.uid.uidByte[3] == uids[i][3]) {
      Serial.println("IT IS " + String(i));
    }
  }
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
