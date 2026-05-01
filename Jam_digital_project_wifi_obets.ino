//code_with_obets
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h> 
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

//config_panel
#define PANEL_RES_X 64      
#define PANEL_RES_Y 32     
#define PANEL_CHAIN 1      

MatrixPanel_I2S_DMA *dma_display = nullptr;

//config_network_and_time
char ssid[] = ""; 
char pass[] = ""; 
#define TZ (+7*60*60) //configurasi WIB (GMT+7)

bool flasher = false;
int h, m, s, d, yr;
uint8_t month, dow;
unsigned int NewRTCh = 24, NewRTCm = 60, NewRTCs = 10;
time_t t;

//static_value_minggu_and_jam
const char* wd[7] = {"Minggu", "Senin", "Selasa", "Rabu","Kamis", "Jumat", "Sabtu"};
const char* months[] = {"Jan", "Feb", "Mar", "Apr", "Mei", "Jun","Jul", "Agu", "Sep", "Okt", "Nov", "Des"};

//define_defaul_color
uint16_t myWHITE;
uint16_t myBLACK;
uint16_t myRED;
uint16_t myBLUE;

void Date_text() {
  dma_display->fillRect(0, 24, 64, 8, myBLACK); 
  dma_display->setTextSize(1);
  dma_display->setTextColor(myWHITE);


  int x_start = 3; 
  
  //print_tanggal
  dma_display->setCursor(x_start, 24);
  dma_display->printf("%02d", d);
  
  //print_bulan
  dma_display->setCursor(x_start + 15, 24); 
  dma_display->print(months[month]);
  
  //print_tahun
  dma_display->setCursor(x_start + 36, 24);
  dma_display->printf("%04d", yr);
}

//function_day_text
void dofw_text() {
  dma_display->fillRect(0, 0, 64, 8, myBLACK); 
  
  String dayText = wd[dow];
  int xpo = (64 - (dayText.length() * 6)) / 2; 
  
  dma_display->setCursor(xpo, 0);
  dma_display->setTextSize(1);
  dma_display->setTextColor(myBLUE);
  dma_display->print(dayText);
}

//function_time_text
void getTim() {
  if (NewRTCh != h) {
    dma_display->fillRect(0, 8, 22, 14, myBLACK); 
    dma_display->setFont(&FreeMonoBold9pt7b);
    dma_display->setCursor(0, 19); 
    dma_display->setTextColor(myWHITE);
    dma_display->printf("%02d", h);
    dma_display->setFont(); 
    NewRTCh = h;
    dofw_text(); 
  }

  //funch_detik_indikator
  dma_display->fillRect(21, 8, 5, 14, myBLACK); 
  if (flasher) {
    dma_display->setFont(&FreeMonoBold9pt7b);
    dma_display->setCursor(20, 18);
    dma_display->setTextColor(myWHITE);
    dma_display->print(":");
    dma_display->setFont();
  }

  if (NewRTCm != m) {
    dma_display->fillRect(26, 8, 23, 14, myBLACK); 
    dma_display->setFont(&FreeMonoBold9pt7b);
    dma_display->setCursor(26, 19); 
    dma_display->setTextColor(myWHITE);
    dma_display->printf("%02d", m);
    dma_display->setFont();
    NewRTCm = m;
  }

  dma_display->fillRect(49, 12, 15, 8, myBLACK);
  dma_display->setCursor(50, 12); 
  dma_display->setTextSize(1);
  dma_display->setTextColor(myRED);
  dma_display->printf("%02d", s);
}

void updateTime() {
  struct tm *tm;
  tm = localtime(&t);
  h = tm->tm_hour;
  m = tm->tm_min;
  s = tm->tm_sec;
  
  if (d != tm->tm_mday) {
    d = tm->tm_mday;
    dow = tm->tm_wday;
    month = tm->tm_mon;
    yr = tm->tm_year + 1900;
    Date_text();
  }
}

void setup() {
  //print_log_serial_monitor(baudrate 115200) 
  Serial.begin(115200);

  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi Terhubung!");
  } else {
    Serial.println("\n[ERROR] Gagal konek WiFi. Cek SSID/Password!");
  }

  Serial.println("Sinkronisasi Jam via NTP...");
  configTime(TZ, 0, "pool.ntp.org", "id.pool.ntp.org");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println("\n[OK] konfigurasi Jam sudah Berhasil Sinkron!");

  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
  mxconfig.gpio.e = 18; 
  
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(70); 
  dma_display->clearScreen();

  myWHITE = dma_display->color565(255, 255, 255);
  myBLACK = dma_display->color565(0, 0, 0);
  myRED   = dma_display->color565(255, 0, 0);
  myBLUE  = dma_display->color565(0, 100, 255);
  
  updateTime();
  Date_text();
  dofw_text();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) ESP.restart();

  t = time(NULL);
  static time_t last_t;
  if (last_t != t) {
    updateTime();
    getTim();
    flasher = !flasher;
    last_t = t;
  }
}