#include "SD_MMC.h"
#include "TFT_eSPI.h"
#include "help.hpp"
#include "WiFi.h"
#include <HTTPClient.h>
#include <time.h> 

TFT_eSPI tft = TFT_eSPI();
char ssid[50];
char password[50];
char hostserver[50];
char token[50];
char ss[50];
char uuid[50];

#define PRINT_STR_LN(str, x, y)                                                                                                                      \
  do {                                                                                                                                               \
    if (y > 80) {y=0; x=0; tft.fillScreen(TFT_BLACK);}                                                                                               \
    Serial.println(str);                                                                                                                             \
    tft.drawString(str, x, y);                                                                                                                       \
    y += 8;                                                                                                                                          \
  } while (0);

#define PRINT_STR(str, x, y)                                                                                                                         \
  do {                                                                                                                                               \
    tft.drawString(str, x, y);                                                                                                                       \
  } while (0);

int32_t x, y;

void print_tft (String msg)
{
  PRINT_STR_LN(msg, x, y)
}

void read_wifi_config () {
          File file;
          file = SD_MMC.open("/SETUP.INI", FILE_READ);
          char s1[50], s2[50];
          String str1, str2;
          if (file) {
             for (int i = 0; i < 5; i++)
          {
            if (ReadLine(&file, ss, 50) != 0)
            { 
              PRINT_STR_LN("Error Lectura", x, y)
              return;
            }
            DivideStr(ss, s1, s2, '=');
            str1 = String(s1);
            str2 = String(s2);
            str1.toUpperCase();
            if (str1 == "SSID") {
              str2.toCharArray(ssid, str2.length() + 1);
            }
            if (str1 == "PASSWORD") {
              str2.toCharArray(password, str2.length() + 1);
            }
            if (str1 == "SERVER") {
              str2.toCharArray(hostserver, str2.length() + 1);
            }
            if (str1 == "TOKEN") {
              str2.toCharArray(token, str2.length() + 1);
            }
            if (str1 == "UUID") {
              str2.toCharArray(uuid, str2.length() + 1);
            }
          }
          file.close();
          }
          else PRINT_STR_LN("Error open file!!", x, y)
        
          PRINT_STR_LN(ssid, x, y)
          PRINT_STR_LN(password, x, y)
          PRINT_STR_LN(token, x, y)
          PRINT_STR_LN(uuid, x, y)
          PRINT_STR_LN(hostserver, x, y)
  }

  void init_wifi () {
          
          WiFi.hostname("Dongle-esp32S3");
          // Reduce startup surge current
          WiFi.setAutoConnect(false);
          WiFi.mode(WIFI_STA);
          WiFi.begin(ssid, password);
          delay (1000);
          // Wait for connection
          int countW = 1;
          PRINT_STR_LN("Iniciando Wifi", x, y)
          while (WiFi.status() != WL_CONNECTED) {
            if (countW == 20) break;
            PRINT_STR(".", x + 1, y)
            countW++;
            delay (500);
          }
}

String get_list_file (String nombrePlaca, String id) {
    String payload;
    String message = " ";
    HTTPClient http;
    WiFiClient client;
    
    if (http.begin(client,"http://"+String (hostserver)+"/api/file_server/obtener_archivos/")) {
    
    http.addHeader("Content-Type", "multipart/form-data");
    http.addHeader("token",nombrePlaca);
    http.addHeader("uuid", id);
    http.addHeader("accept", "multipart/form-data");
    int httpCode = http.POST(message);
    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
        payload = http.getString();
       }
    }
    else {
      //Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      PRINT_STR_LN ("error http",x,y)
      payload = String (httpCode);
    }
    http.end();
    return (payload);
    }
    else {
      PRINT_STR_LN ("error Conexion al servidor",x,y)
      }
    
}

void list_file (String dato)
{
String MyS = dato;
int MyP = 0;
int MyI = 0;
   MyS = dato; 
   MyP=0;MyI=0;
   while(MyI>=0){
          MyI = MyS.indexOf("\n",MyP); //busco cada linea
         if (MyI >=0)
         { 
          String s = MyS.substring(MyP,MyI);
          PRINT_STR_LN (s,x,y);
        MyP = MyI + 1;
         }
      }
}

//funcion para realizar el pedido de archivos
void post_download (String nombrePlaca, String id, File *uploadFile, String urlDown) {
    PRINT_STR_LN ("Inicio descarga",x,y);
    HTTPClient http;
    WiFiClient client;
    String message = "";
    http.setReuse (true);
    if (http.begin(client,"http://"+String (hostserver)+"/api/file_server/descargar_archivo/")) {

      http.addHeader("Content-Type", "multipart/form-data");
      http.addHeader("Accept", "*/*");
      http.addHeader("Accept-Encoding", "gzip");
      http.addHeader("Accept-Encoding","deflate");
      http.addHeader("Accept-Encoding","br");
      http.addHeader("token",nombrePlaca);
      http.addHeader("uuid", id);
      http.addHeader("url", urlDown);
    
      delay (50);
  
      int httpCode = http.POST(" ");
      delay (10);
      if(httpCode > 0) {
            int len = http.getSize();
             // create buffer for read
              uint8_t buff[5120] = {0};
             // get tcp stream
             WiFiClient *stream = http.getStreamPtr();
          // read all data from server
             while (http.connected() && (len > 0 || len == -1))
                 {
                    // get available data size
                    size_t size = stream->available();
                    if (size)
                    { // read up to 128 byte
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        uploadFile->write(buff, c);
                        // close file.
                        if (len > 0)
                        {
                          len -= c;
                        }
                    }
                    delayMicroseconds(1500);
                }
          PRINT_STR_LN ("Fin",x,y);
        }
      else {
        String msg = "[HTTP] POST... failed, error: " + String(httpCode);
        PRINT_STR_LN (msg,x,y);
      }
      http.end();
   
    }
    else 
    {PRINT_STR_LN ("Error en conectarse al servidor",x,y);
    }
    
    
   
}

void deleteFile (String nombrePlaca, String id, String urlDelete)
{
   // Prepara el formulario para eliminar el archivo
    PRINT_STR_LN ("Borro",x,y);
    String deleteFileForm = "uuid="+id+"&token="+nombrePlaca+"&url=" + urlDelete;
    HTTPClient http;
    WiFiClient client;
    if (http.begin(client,"http://"+String (hostserver)+"/api/file_server/borrar_archivo/")) {
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
          // Envía la solicitud POST para eliminar el archivo
          int httpResponseCode = http.POST(deleteFileForm);
      
          if (httpResponseCode > 0) {
            String msg;
            msg = "Codigo: " + String (httpResponseCode);
            PRINT_STR_LN (msg,x,y);
          } else {
            String msg;
            msg = "Error en la eliminación: " + String (httpResponseCode);
            PRINT_STR_LN (msg,x,y);
          }
          http.end();
    }
    else 
    {
     PRINT_STR_LN ("Error en conectarse al servidor",x,y);
    }
}

void postUpload (String nombrePlaca, String id, File *uploadFile, String urlUpload)
{ // Abrir el archivo en la tarjeta SD
 
    WiFiClient client;
     char* serverHost = "www.ursulagis.com";
     int serverPort = 80;
    String token = nombrePlaca;
    String uuid = id;
    String url = "/api/file_server/upload_file/";
    String urlToUpload;
    if (urlUpload == "/")
      urlToUpload = "";
    else 
      urlToUpload = urlUpload;
    
  if (!client.connect(serverHost, serverPort)) {
    Serial.println("Error al conectar al servidor");
    return;
  }
    String nameFile = uploadFile->name();
     // Construye la solicitud HTTP POST
    String requestBody = "";
    requestBody += "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
    requestBody += "Content-Disposition: form-data; name=\"file\"; filename=\""+ nameFile +"\"\r\n";
    requestBody += "Content-Type: application/pdf\r\n\r\n";

    String finalBoundary = "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

  // Envía el encabezado de la solicitud
  client.println("POST /api/file_server/upload_file/ HTTP/1.1");
  client.print("Host: ");
  client.println(serverHost);
  client.println("token: "+ token);
  client.println("uuid: " + uuid);
  client.println("url:" + urlToUpload);
  client.println("Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");
  client.print("Content-Length: ");
  client.println(requestBody.length() + uploadFile->size() + finalBoundary.length());
  client.println();
 // Envía el cuerpo de la solicitud
  client.print(requestBody);
  // enviamos el archivo con buffuer
  uint8_t fileBuffer[1024];
  size_t bytesTotal=0;
  int punto = 0;
  while (uploadFile->available()) {
    size_t bytesRead = uploadFile->read(fileBuffer, 1024);
    client.write(fileBuffer, bytesRead);
    bytesTotal = bytesTotal + bytesRead;
   }
  String msg = String (bytesTotal/1024);
  PRINT_STR_LN (msg,x,y);
  // Envía el final del cuerpo de la solicitud
  client.println(finalBoundary);
  
     if (client.available()) {
          String responseLine = client.readStringUntil('\r');
          Serial.println(responseLine);
        }
      
    Serial.println("Client Stop");
    client.stop();
}

void downloadFiles (String list_files)
{
  String MyS = list_files;
  int MyP = 0;
  int MyI = 0;
  String array[10];
  PRINT_STR_LN ("Iniciando Descargas",x,y);
  while (MyI >= 0) {
    MyI = MyS.indexOf("\n", MyP); //busco cada linea
    if (MyI >= 0)
    {
      String s = MyS.substring(MyP, MyI);
      //proceso cada linea buscando directorios y extenciones
      String MyS2 = s;
      int MyP2 = 0;
      int MyI2 = 0;
      String dir = "/";
      bool isROOT = true;
      while (MyI2 >= 0) {
        MyI2 = MyS2.indexOf("/", MyP2);
        if (MyI2 == 0)
        {
          delay (2);
        }
        else if (MyP2 != MyI2)
        {
          String Ss = MyS2.substring(MyP2, MyI2);
          String MyS3 = Ss;
          int MyP3 = 0;
          int MyI3 = 0;
          while (MyI3 >= 0) {
            String temp;
            if (isROOT) temp = dir;
            else temp = dir + "/";
            MyI3 = MyS3.indexOf(".", MyP3);
            if (MyI3 == -1) //aca proceso directorios
            {
              dir = temp + Ss;
              if (SD_MMC.exists(dir.c_str()))
              {
                String msg = "la carpeta: " + dir + " existe";
                PRINT_STR_LN (msg,x,y)
              }
              else
              {
                if (SD_MMC.mkdir(dir.c_str())) Serial.println("Se creo la carpeta: " + dir);
              }
              isROOT = false;
            }
            else if (MyP3 != MyI3)  //aca se procesa los archivos
            { String nFile;
              if (isROOT)
              {
                nFile = dir + Ss;
              }
              else
              {
                nFile = dir + "/" + Ss;
              }
              File FileT = SD_MMC.open(nFile, FILE_READ);
              if (FileT)
              { unsigned long tam = FileT.size();
                PRINT_STR_LN ("El archivo existe",x,y);
                FileT.close();
                delay (10);
                if (tam > 0)
                { PRINT_STR_LN ("Tiene tamaño se deja",x,y);
                }
                else
                { PRINT_STR_LN ("No tiene tamaño se borra",x,y);
                  SD_MMC.remove (nFile.c_str());
                  delay (5);
                  File downFile = SD_MMC.open(nFile, FILE_WRITE);
                  if (!downFile) {
                    PRINT_STR_LN ("open fail",x,y);
                    return;
                  }
                  delay (10);
                  post_download (String(token), String (uuid), &downFile, nFile);
                  delay (10);
                  downFile.close();
                  PRINT_STR_LN ("Archivo Bajado",x,y);
                  }
              }
              else
              { PRINT_STR_LN ("Archivo no existe",x,y);
                File downFile = SD_MMC.open(nFile, FILE_WRITE);
                if (!downFile) {
                  PRINT_STR_LN ("not open",x,y);
                  return;
                }
                delay (10);
                post_download (String(token), String (uuid), &downFile, nFile);
                delay (10);
                downFile.close();
             }
              break;
            }
            MyP3 = MyI3 + 1;
          }
        }
        MyP2 = MyI2 + 1;
      }
    }
 MyP = MyI + 1;
  }
}

void sendLog (){
            deleteFile (String(token), String (uuid), "/log.txt");
            delay(10);
              File nfile = SD_MMC.open("/log.txt", FILE_APPEND);
              delay (50);
              if (nfile) {
                time_t now = time(nullptr);
                nfile.println("Log de ejecucion");
                nfile.print("Fecha: ");
                nfile.print(ctime(&now));
                nfile.print("\t");
                nfile.print("Mac: ");
                nfile.print(WiFi.macAddress());
                delay (10);
                nfile.close();
                PRINT_STR_LN("Se agrego linea al log", x, y)
                delay (50);
              }

               File efile = SD_MMC.open("/log.txt", FILE_READ);
               delay(10);
               if (efile) {
               postUpload (String(token), String (uuid), &efile, "/");
              } else {
                PRINT_STR_LN("Error Abrir log", x, y)
              }
  
               efile.close();
               }

void uploadFromSd(File dir, String currentPath, String &ListServer) {
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      // No hay más archivos
      PRINT_STR_LN("No hay que mostar", x, y)
      break;
    }
    String fullPath = currentPath + "/" + entry.name();   
    if (entry.isDirectory()) {
      // Si es un directorio, llama recursivamente a la función
      String fullPath = currentPath + "/" + entry.name();
      uploadFromSd(entry, fullPath, ListServer);
    } else {
      // Si es un archivo, añade su tamaño a la cadena
      String fileInSd= currentPath + "/" + entry.name();
      if (ListServer.indexOf(fileInSd) == -1) {
                PRINT_STR_LN("File in sd upload", x, y)
               File efile = SD_MMC.open(fileInSd, FILE_READ);
               delay(10);
               if (efile) {
               postUpload (String(token), String (uuid), &efile, currentPath);
              } else {
                PRINT_STR_LN("Error Open file", x, y)
              }
              efile.close();
      }
    }
    entry.close();
  }
}

void sd_init_usd(void) {
  // Initialise TFT pantalla 
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  digitalWrite(TFT_LEDA_PIN, 0);
  tft.setTextFont(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextWrap(true); 
  
  //inicio la SD_CARD
  SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
  if (!SD_MMC.begin()) {
    PRINT_STR_LN("Card Mount Failed", x, y)
    return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    PRINT_STR_LN("No SD_MMC card attached", x, y)
    return;
  }
  String str;
  str = "SD_MMC Card Type: ";
  if (cardType == CARD_MMC) {
    str += "MMC";
  } else if (cardType == CARD_SD) {
    str += "SD_MMCSC";
  } else if (cardType == CARD_SDHC) {
    str += "SD_MMCHC";
  } else {
    str += "UNKNOWN";
  }

  PRINT_STR_LN(str, x, y)
  uint32_t cardSize = SD_MMC.cardSize() / (1024 * 1024);

  str = "SD_MMC Card Size: ";
  str += cardSize;
  PRINT_STR_LN(str, x, y)

  str = "Total space: ";
  str += uint32_t(SD_MMC.totalBytes() / (1024 * 1024));
  str += "MB";
  PRINT_STR_LN(str, x, y)

  str = "Used space: ";
  str += uint32_t(SD_MMC.usedBytes() / (1024 * 1024));
  str += "MB";
  PRINT_STR_LN(str, x, y)

  //busco archivo de configuraciones
  read_wifi_config ();
  delay (4000);
    tft.fillScreen(TFT_BLACK);
    x=0;
    y=0;
  init_wifi (); // inicio wifi
    tft.fillScreen(TFT_BLACK);
    x=0;
    y=0;
  if (WiFi.status() == WL_CONNECTED) {
      //actualizo hora
          PRINT_STR_LN("Configurando Fecha y Hora", x, y)
          long timezone = -4; 
          byte daysavetime = 1;
          configTime(3600*timezone, daysavetime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
          delay (5000); 
            // Muestro fecha y hora
             time_t now = time(nullptr);
             String printNow = ctime(&now);
             PRINT_STR_LN(printNow, x, y)
             delay (2000);
          PRINT_STR_LN("Obtener Archivos de la placa", x, y)
          
          String listedFiles;
          listedFiles = get_list_file (String(token), String (uuid));
          //reviso si el archivo del listado esta en la sd y bajo  list (Listado);
          downloadFiles (listedFiles);
          delay (1000);
          tft.fillScreen(TFT_BLACK);
          x=0;
          y=0;
            
          PRINT_STR_LN("Busco y envio archios de sd", x, y)
          File root;
          root = SD_MMC.open("/");
          uploadFromSd(root, "", listedFiles);//compruevo los archivos que hay en la Sd y se deben enviar
          sendLog ();
  }
  else PRINT_STR_LN("No hay WIFI", x, y) 
  //tft.fillScreen(TFT_BLACK);
  //x=0;
  //y=0;

  
  SD_MMC.end ();
  delay (500);
  WiFi.disconnect ();
  delay (500);
}
