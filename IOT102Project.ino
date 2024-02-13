#include <Keypad.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <Servo.h>

const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

Servo myservo;
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

const int motorPin = 10;

const int buzzerPin = 13;

const int TRIG_PIN = 11;
const int ECHO_PIN = 12;

int inputCount = 0;
int timeLock = 0;

bool doorOpen = false;
const int distanceThreshold = 20; // 20cm

String D2003 = "1234";
String D2004 = "5678";
String D2005 = "1357";

void setup()
{
	Serial.begin(9600);
	lcd.begin(16, 2);
	myservo.attach(motorPin);
	myservo.write(0);
	pinMode(TRIG_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);
	if (readStringFromEEPROM(0) == "")
	{
		writeStringToEEPROM(0, D2003);
	}
	else
	{
		D2003 = readStringFromEEPROM(0);
	}
	if (readStringFromEEPROM(5) == "")
	{
		writeStringToEEPROM(5, D2004);
	}
	else
	{
		D2004 = readStringFromEEPROM(5);
	}
	if (readStringFromEEPROM(10) == "")
	{
		writeStringToEEPROM(10, D2005);
	}
	else
	{
		D2005 = readStringFromEEPROM(10);
	}
}

void writeStringToEEPROM(int addrOffset, String strToWrite)
{
	byte len = strToWrite.length();
	EEPROM.write(addrOffset, len);
	for (int i = 0; i < len; i++)
	{
		EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
	}
}

String readStringFromEEPROM(int addrOffset)
{
	int newStrLen = EEPROM.read(addrOffset);
	char data[newStrLen + 1];
	for (int i = 0; i < newStrLen; i++)
	{
		data[i] = EEPROM.read(addrOffset + 1 + i);
	}
	data[newStrLen] = '\0';
	return String(data);
}

boolean verifyPassword(String enteredPassword, String password)
{
	return enteredPassword.indexOf(password) != -1;
}

void enterDisplay()
{
	lcd.setCursor(0, 0);
	lcd.print("Enter password :");
	Serial.println("Enter password :");
}

String getPasswordInput(boolean changePass)
{
	lcd.clear();
	unsigned long startTime = millis();
	unsigned long timeout = 20000;
	if (changePass == false)
	{
		lcd.print("Enter password :");
	}
	else
	{
		lcd.print("New password: ");
	}
	String lcdPass = "";
	String input = "";
	char key;
	while (true)
	{
		key = customKeypad.getKey();
		if (key)
		{
			if (key == '#')
			{
				break;
			}
			else if (key == 'C')
			{
				boolean success = changePassword();
				if (success)
				{
					return "change";
				}
				else
				{
					return "not change";
				}
			}
			else if (key == 'D')
			{
				lcdPass = "";
				input = "";
				lcd.clear();
				lcd.print("Enter password :");
			}
			else if (key == 'B')
			{
				writeStringToEEPROM(0, "1234");
				writeStringToEEPROM(5, "5678");
				writeStringToEEPROM(10, "1357");
			}
			else
			{
				lcd.setCursor(0, 1);
				lcdPass += "*";
				lcd.print(lcdPass);
				input += key;
			}
		}
		if (millis() - startTime > timeout)
		{
			return "Timeout! No input.";
		}
	}
	return input;
}

boolean changePassword()
{
	lcd.clear();
	lcd.print("Before password:");

	String lcdPass = "";
	String input = "";
	char key;

	while (true)
	{
		key = customKeypad.getKey();
		if (key)
		{
			if (key == '#')
			{
				break;
			}
			else
			{
				lcd.setCursor(0, 1);
				lcdPass += "*";
				lcd.print(lcdPass);
				input += key;
			}
		}
	}
	String passEEPROM = readStringFromEEPROM(0);
	String newPass = "";
	if (input == D2003 || input == passEEPROM)
	{
		newPass = getPasswordInput(true);
		D2003 = newPass;
		writeStringToEEPROM(0, D2003);
		return true;
	}
	else if (input == D2004)
	{
		newPass = getPasswordInput(true);
		D2004 = newPass;
		writeStringToEEPROM(5, D2004);
		return true;
	}
	else if (input == D2005)
	{
		newPass = getPasswordInput(true);
		D2005 = newPass;
		writeStringToEEPROM(10, D2005);
		return true;
	}
	else
	{
		lcd.clear();
		lcd.print("Invalid password");
		delay(2000);
	}
	return false;
}

void incorrectPass()
{
	lcd.print("Lock surcurity!");
	tone(buzzerPin, 1000, 3000);
	delay(3000);
	noTone(buzzerPin);
	lcd.setCursor(0, 0);
}

void openDoor()
{
	myservo.write(90);
	doorOpen = true;
}

void closeDoor()
{
	myservo.write(0);
	doorOpen = false;
}

void lockKeypad()
{
	lcd.clear();
	lcd.print("Keypad locked!");
	String countDown = "";
	tone(buzzerPin, 1000, 3000);

	lcd.setCursor(0, 1);
	timeLock += 3000;

	for (int i = timeLock / 1000; i > 0; i--)
	{
		lcd.setCursor(0, 1);
		if (i < 10)
		{
			lcd.print("0");
		}
		lcd.print(i);
		delay(1000);
	}
	lcd.clear();
	lcd.print("Unlocking...");
	delay(2000);
}

void loop()
{
	digitalWrite(TRIG_PIN, LOW);
	delayMicroseconds(2);
	digitalWrite(TRIG_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG_PIN, LOW);

	const long duration = pulseIn(ECHO_PIN, HIGH);
	int distance = duration / 29 / 2;

	if (distance < distanceThreshold && !doorOpen)
	{
		String enteredPassword = getPasswordInput(false);
		String passEEPROM1 = readStringFromEEPROM(0);
		String passEEPROM2 = readStringFromEEPROM(5);
		String passEEPROM3 = readStringFromEEPROM(10);
		if (verifyPassword(enteredPassword, passEEPROM1) ||
			verifyPassword(enteredPassword, passEEPROM2) ||
			verifyPassword(enteredPassword, passEEPROM3))
		{
			lcd.clear();
			lcd.print("Door Opened!");
			inputCount = 0;
			openDoor();
			delay(5000);
			closeDoor();
		}
		else if (enteredPassword == "change")
		{
			lcd.clear();
			lcd.print("Change ok!");
			delay(3000);
		}
		else if (enteredPassword == "not change")
		{
			lcd.clear();
			lcd.print("Change not ok!");
			delay(3000);
		}
		else if (enteredPassword == "Timeout! No input.")
		{
			lcd.clear();
			lcd.print("Timeout!");
			delay(2000);
		}
		else
		{
			inputCount++;
			if (inputCount >= 3)
			{
				lockKeypad();
			}
			else
			{
				lcd.clear();
				lcd.print("Wrong password!");
				delay(2000);
				lcd.clear();
			}
		}
	}
	else if (distance > distanceThreshold)
	{
		lcd.clear();
	}
	delay(100);
}
