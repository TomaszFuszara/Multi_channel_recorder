#include <SPI.h>  // Biblioteka umożliwiająca komunikację Arduino z urządzeniami za pomocą protokołu SPI
#include <SD.h>  // Biblioteka umożliwiająca zapis i odczyt danych z karty SD
#include <OneWire.h>  // Biblioteka umożliwiająca komunikację Arduino z urządzeniami za pomocą protokołu 1-Wire
#include <DallasTemperature.h> // Biblioteka odpowiedzialna za obsługę czujników
#include <Wire.h>  //  Biblioteka umożliwiająca komunikację Arduino z urządzeniami za pomocą protokołu I2C
#include <RTClib.h> // Biblioteka odpowiedzialna za obsługę zegarów czasu rzeczywistego
#include <LiquidCrystal_I2C.h>  // Biblioteka odpowiedzialna za obsługę wyświetlaczy lcd z dedeykowanym modułem I2C
  
LiquidCrystal_I2C lcd(0x27, 16, 2); //Podłączenie ekranu (GND, VCC SDA, SCL)

RTC_DS1307 RTC;       //Podłączenie zegara (GND, VCC SDA, SCL)
 
OneWire oneWire(A0); //Podłączenie Dallasów (A0)
DallasTemperature sensors(&oneWire); //Przekazania informacji do biblioteki

float Czas = 0; //Przygotowanie czasomierza

DeviceAddress T1 = {0x28, 0xF9, 0xCD, 0x95, 0xF0, 0x1, 0x3C, 0x0}; //Określenie adresu czujnika 1
DeviceAddress T2 = {0x28, 0x6B, 0xB5, 0x9595, 0xF0, 0x1, 0x3C, 0x8C}; //Określenie adresu czujnika 2
DeviceAddress T3 = {0x28, 0x94, 0x47, 0x95, 0xF0, 0x1, 0x3C, 0xFF}; //Określenie adresu czujnika 3
DeviceAddress T4 = {0x28, 0xDF, 0x88, 0x95, 0xF0, 0x1, 0x3C, 0x3F}; //Określenie adresu czujnika 4

File plik; // Przygotowanie pliku do stworzenia

int Akcja = 0;             //Zmienne przygotowane do prowadzenia operacji matemattycznych                    
int liczbaCzujnikow = 0;
int interwal = 0;
int czujnik = 0;
int podswietlenie = 0;
int czasS = 0;
int czasM = 0;
int czasH = 0;
unsigned long czas = 0;
unsigned long uplyw = 0;
unsigned long uplyw1 = 0;
unsigned long czasZapisu = 0;

void setup() {
  SD.begin(3); // Inicjalizacja pracy czytnika kart pamięci
  sensors.begin(); //Inicjalizacja czujnikow
  Wire.begin(); //Inicjalizacja komunikacji I2C
  RTC.begin(); // Inicjalizacja zegara
  lcd.init(); // inicjalizacja ekranu
  
  pinMode(4, INPUT_PULLUP); // Przygotowanie pinów do pracy, wraz z podpiętymi do nich rezystorami                                                                                     
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  
   plik = SD.open("dane.txt", FILE_WRITE);                                                                      // Otworzenie pliku, zapis fragmentu jeszcze przed rozpoczęciem pomiarów i zamknięcie pliku
   plik.println("");
   plik.println("................................................................");
   plik.println("Godzina:   T1:     T2:     T3:     T4:");     
   plik.close();
}

void loop() { 
sensors.requestTemperatures(); // Rozpoczęcie zbierania danych przez czujniki

DateTime now = RTC.now(); // Rozpoczęcie pracy przez zegar

if(digitalRead(7) == LOW){  // Włączanie i wyłaczanie podświetlenia dedykowanym przyciskiem                      
  delay(20);
  podswietlenie++;
}
if(podswietlenie == 1){
  lcd.backlight();
}else{
  lcd.noBacklight();
}
if(podswietlenie >= 2){
  podswietlenie = 0;
}
  
if(digitalRead(4) == LOW){  // Przełączanie urzytkownika między ekranami dedykowanym przyciskiem
  delay(20);
  Akcja++;
    lcd.setCursor(0,0);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print("                ");
  if(Akcja > 5){
    Akcja = 5;}
  }
switch(Akcja){
  case 0:                         //Wyświetlane komendy po przełączeniu urzytkownika
  lcd.setCursor(0,0);
  lcd.print("Wcisnij Ok by   ");
  lcd.setCursor(0,1);
  lcd.print("zadac parametry ");
  break;
  
  case 1:  
  lcd.setCursor(0,0);              //Wyświetlanie liczby czujników, podczas ich ustawiania
  lcd.print("Liczba czujnikow");
  lcd.setCursor(0,1);
  lcd.print(liczbaCzujnikow);

  if(digitalRead(5) == LOW){ //Ustawianie liczby czujników
    delay(20);
    liczbaCzujnikow++;
    if(liczbaCzujnikow >= 5){
      liczbaCzujnikow = 0;
    }
  }
    if(digitalRead(6) == LOW){
      delay(20);
      liczbaCzujnikow--;
      if(liczbaCzujnikow < 0){
      liczbaCzujnikow = 4;
    }
  }
  break;
  
  case 2:
  
  lcd.setCursor(0,0);              //Wyświetlanie częstotliwości zapisu podczas jej ustawiania
  lcd.print("Zapis danych co");
  if(interwal <10){
    lcd.setCursor(0,1);
    lcd.print(interwal);
    lcd.setCursor(1,1);
    lcd.print(" minut ");
  }
  else{
    lcd.setCursor(0,1);
    lcd.print(interwal);
    lcd.setCursor(2,1);
    lcd.print(" minut");
  }
  
  if(digitalRead(5) == LOW){   //Ustawianie czętotliwości zapisu
    delay(20);
    interwal++;
  }
  if(digitalRead(6) == LOW){
      delay(20);
      interwal--;
      if(interwal < 0){
      interwal = 0;
    }
  }
  break;
  
  case 3:
  
  lcd.setCursor(0,0);              //Komend wyświetlana po zakończeniu ustawiania parametrów
  lcd.print("Wcisnij Ok by   ");
  lcd.setCursor(0,1);
  lcd.print("zaczac pomiary  ");
  delay(20);
  break;
  
  case 4:  
  
  if((millis() - uplyw1) >=1000){
    uplyw1 = millis();
    czas++;
  }

if((millis() - uplyw) >=564){
    uplyw = millis();
    czasS++;
  }
if(czasS >= 60){
  czasS = 0;
  czasM++;
}
if(czasM >= 60){
  czasM = 0;
  czasH++;
}

  if(interwal > 0){                                   // Rozpoczęcie zapisu danych na karcie co określoną liczbę minut
  if((millis() - czasZapisu) >= interwal*60000){
    czasZapisu = millis();
    //File
    plik = SD.open("dane.txt", FILE_WRITE);  
    if (plik){
      plik.print(now.hour(), DEC);  //Wyświetlanie godziny
      plik.print(':');  //Wyświetla przerwy (:)
      plik.print(now.minute(), DEC);  //Wyświetlanie minuty
      plik.print(':');  //Wyświetlanie przerwy (:)
      plik.print(now.second(), DEC);  //Wyświetlanie sekundy
      
      if(liczbaCzujnikow >= 1){
        plik.print("   ");
        plik.print(sensors.getTempC(T1));  //Wyswietlenie informacji z czujnika 1
      }
      
      if(liczbaCzujnikow >= 2){
        plik.print("   ");
        plik.print(sensors.getTempC(T2));  //Wyswietlenie informacji z czujnika 2
      }
      
      if(liczbaCzujnikow >= 3){
        plik.print("   ");
        plik.print(sensors.getTempC(T3));  //Wyswietlenie informacji z czujnika 3 
      }
      
      if(liczbaCzujnikow >= 4){
        plik.print("   ");
        plik.print(sensors.getTempC(T4));  //Wyswietlenie informacji z czujnika 4
      }
      
      }
      plik.println("   ");
    plik.close();
    }
}

  lcd.setCursor(0,0);
  lcd.print("Czas(s):");   // Wyświetlanie czasu od rozpoczęcia pomiarów na ekranie
 
  if(czasH < 10){
    lcd.setCursor(0,1);
    lcd.print("0");
    lcd.setCursor(1,1);
    lcd.print(czasH);
  }else{
    lcd.setCursor(0,1);
    lcd.print(czasH);
  }
  lcd.setCursor(2,1);
  lcd.print(":");
  if(czasM < 10){
    lcd.setCursor(3,1);
    lcd.print("0");
    lcd.setCursor(4,1);
    lcd.print(czasM);
  }else{
    lcd.setCursor(3,1);
    lcd.print(czasM);
  }
  lcd.setCursor(5,1);
  lcd.print(":");
  if(czasS < 10){
    lcd.setCursor(6,1);
    lcd.print("0");
    lcd.setCursor(7,1);
    lcd.print(czasS);
  }else{
    lcd.setCursor(6,1);
    lcd.print(czasS);
  }

 if(digitalRead(5) == LOW){  // Przełączanie między pokazywaniem tempratury danego czujnika na wskazannie z innego
    delay(20);
    czujnik++;
    if(czujnik > liczbaCzujnikow){
      czujnik = 0;
    }
  }
    if(digitalRead(6) == LOW){
      delay(20);
      czujnik--;
      if(czujnik < 0){
      czujnik = liczbaCzujnikow;
    }
  }
 
 if(czujnik == 1 && liczbaCzujnikow >= 1){
      lcd.setCursor(10,0);       //Wyświetlenie Temperatury 1
      lcd.print("Temp1:");
      lcd.setCursor(11,1);
      lcd.print(sensors.getTempC(T1));
    }
if(czujnik == 2 && liczbaCzujnikow >= 2){
      lcd.setCursor(10,0);       //Wyświetlenie Temperatury 2
      lcd.print("Temp2:");
      lcd.setCursor(11,1);
      lcd.print(sensors.getTempC(T2));
    }
if(czujnik == 3 && liczbaCzujnikow >= 3){
      lcd.setCursor(10,0);       //Wyświetlenie Temperatury 3
      lcd.print("Temp3:");
      lcd.setCursor(11,1);
      lcd.print(sensors.getTempC(T3));
    }
if(czujnik == 4 && liczbaCzujnikow >= 4){
      lcd.setCursor(10,0);       //Wyświetlenie Temperatury 4
      lcd.print("Temp4:");
      lcd.setCursor(11,1);
      lcd.print(sensors.getTempC(T4));
    }
if(czujnik == 0){
      lcd.setCursor(10,0);       //Koniec wyświetlenia Temperatury 
      lcd.print("       ");
      lcd.setCursor(11,1);
      lcd.print("       ");
    }
  break;

 case 5:  
  lcd.setCursor(0,0);              //Informacja wyświetlana po zakończeniu pomiarów
  lcd.print("Pomiary         ");
  lcd.setCursor(0,1);
  lcd.print("zakonczone      ");
  break; 
}
}
