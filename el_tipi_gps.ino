#include <SD.h>
#include <LiquidCrystal.h>
#include <TinyGPS.h>
#include<stdlib.h>

TinyGPS gps;
static char dtostrfbuffer[20];
int CS = 53;
int LED = 13;
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

String SD_date_time = "hata";
String SD_enlem = "hata";
String SD_boylam = "hata";
String dataString ="";

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec,int SD_val);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

void setup()
{
  pinMode(CS, OUTPUT);  //SD Card Chip Select
  pinMode(LED, OUTPUT);
  lcd.begin(16, 2);
  
  Serial.begin(115200);
  Serial3.begin(4800);
  
  //SD Kart bağlantı kontrolü
  if(!SD.begin(CS))
  {
    Serial.println("Hafiza Karti Bulunamadi")
    lcd.print("Hafiza Karti Bulunamadi");
    return;
  }
  Serial.println(TinyGPS::library_version());
  Serial.println("Omer Guvenir");
  Serial.println();
  Serial.println("Sats HDOP Latitude  Longitude  Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum");
  Serial.println("          (deg)     (deg)      Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail");
  Serial.println("-------------------------------------------------------------------------------------------------------------------------------------");
}

void loop()
{
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  
  dataString = SD_date_time + "," + SD_enlem + "," + SD_boylam;//Hafıza kartına yazdırılacak bilgi
  if(SD_date_time != "hata")
    digitalWrite(LED, HIGH);
  else
    digitalWrite(LED, LOW);
  
  //SD Kart içerisindeki Log.csv dosyasını açıyoruz
  File dataFile = SD.open("LOG.csv", FILE_WRITE);
  if (dataFile)
  {
    dataFile.println(dataString);//Hafıza kartına dataString yazdırılıyor
    Serial.println(dataString);
    //Ekranın ilk satırına Enlem ikinci satırına Boylam bilgileri yazdırılıyor
    lcd.print(SD_enlem);
    lcd.setCursor(0,1);
    lcd.print(SD_boylam);
    
    dataFile.close();
  }
  else
  {
    Serial.println("\nDosya Açilamadi");
    lcd.print("Dosya Açilamadi");
  }
  
  
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6 , 1);//Enlem
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6 , 2);//Boylam
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  
  print_date(gps);//Tarih-Saat
  
  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2,0);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2,0);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2,0);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
  print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2 , 0);
  print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);

  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
  
  smartdelay(1000);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial3.available())
      gps.encode(Serial3.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec,int SD_val)
{
  char sz[32];
  if (val == invalid)
  {
    strcpy(sz, "*******");
    sz[len] = 0;
        if (len > 0) 
          sz[len-1] = ' ';
    for (int i=7; i<len; ++i)
        sz[i] = ' ';
    Serial.print(sz);
    if(SD_val == 1) SD_enlem = sz;
    else if(SD_val == 2) SD_boylam = sz;
  }
  else
  {
    Serial.print(val, prec);
    if (SD_val == 1) SD_enlem = dtostrf(val,10,5,dtostrfbuffer);
    else if (SD_val == 2) SD_boylam = dtostrf(val,10,5,dtostrfbuffer);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(" ");
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
    SD_date_time = "hata";
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
     SD_date_time = sz;
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartdelay(0);
}
