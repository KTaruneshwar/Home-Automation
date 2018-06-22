#include "mbed.h"
#include "ESP8266.h"
#include <string> 
#include "DHT.h"

#define   DHT_DATA_PIN  D7
 
DHT sensor(DHT_DATA_PIN, DHT11);                    //DHT(PinName pin, eType DHTtype)

Serial pc(USBTX,USBRX);

DigitalOut led1(D3);
DigitalOut led2(D4);
DigitalOut led3(D5);



//POT sensor 

//wifi UART port and baud rate
ESP8266 wifi(PA_9, PA_10, 115200); 

//buffers for wifi library
char resp[5000];
char http_cmd[300], comm[300];
char a[500]="@";

int timeout = 8000; //timeout for wifi commands

float h = 0.0f, c = 0.0f, k = 0.0f;

//SSID and password for connection
#define SSID "RobinHood" 
#define PASS "RobinHood123"  

//Remote IP
#define IP "api.thingspeak.com"

//Update key for thingspeak
char* Update_Key = "8R7KDJY54C5Q81RM";
 
//Wifi init function
void wifi_initialize(void){
    
    pc.printf("******** Resetting wifi module ********\r\n");
    wifi.Reset();
    
    //wait for 5 seconds for response, else display no response receiveed
    if (wifi.RcvReply(resp, 5000))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response");
    
    pc.printf("******** Setting Station mode of wifi with AP ********\r\n");
    wifi.SetMode(1);    // set transparent  mode
    if (wifi.RcvReply(resp, timeout))    //receive a response from ESP
        pc.printf("%s",resp);    //Print the response onscreen
    else
        pc.printf("No response while setting mode. \r\n");
    
    pc.printf("******** Joining network with SSID and PASS ********\r\n");
    wifi.Join(SSID, PASS);     
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);   
    else
        pc.printf("No response while connecting to network \r\n");
        
    pc.printf("******** Getting IP and MAC of module ********\r\n");
    wifi.GetIP(resp);     
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response while getting IP \r\n");
    
    pc.printf("******** Setting WIFI UART passthrough ********\r\n");
    wifi.setTransparent();          
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response while setting wifi passthrough. \r\n");
    wait(1);    
    
    pc.printf("******** Setting single connection mode ********\r\n");
    wifi.SetSingle();             
    wifi.RcvReply(resp, timeout);
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response while setting single connection \r\n");
    wait(1);
}


void wifi_send(void){
    
    pc.printf("******** Starting TCP connection on IP and port ********\r\n");
    wifi.startTCPConn(IP,80);    //cipstart
    wifi.RcvReply(resp, timeout);
    if (wifi.RcvReply(resp, timeout)) {   
        pc.printf("%s",resp);    
            //create link 
    sprintf(http_cmd,"/update?api_key=%s&field1=%f&field2=%f",Update_Key,c,h); 
    pc.printf(http_cmd);
    }
    pc.printf("******** Sending URL to wifi ********\r\n");
    wifi.sendURL(http_cmd, comm);   //cipsend and get command
    if(wifi.RcvReply(resp,timeout)){
        //pc.printf("%s",resp);  
    }
}
void wifi_recv(void){
    
    pc.printf("******** Starting TCP connection on IP and port ********\r\n");
    wifi.startTCPConn(IP,80);    //cipstart
    wifi.RcvReply(resp, timeout);
    if (wifi.RcvReply(resp, timeout)) {   
        pc.printf("%s",resp);    
            //create link 
    sprintf(http_cmd,"/talkbacks/26856/commands/execute?api_key=BP4K65A69ISRP8HD HTTP/1.1\r\nHost: api.thingspeak.com\r\n\r\n"); 
    pc.printf(http_cmd);
    }
    pc.printf("******** Sending URL to wifi ********\r\n");
    wifi.sendURL(http_cmd, comm);   //cipsend and get command
    char *x;
    if(wifi.RcvReply(resp,timeout)){
        

      char *token;
      char *action = "";
      if(strstr(resp, "FAN"))
      {
          token = strstr(resp, "FAN");
          action = strtok(token, "C");
          }
          else if(strstr(resp, "TV"))
      {
          token = strstr(resp, "TV");
          action = strtok(token, "C");
          }
          else if(strstr(resp, "LIGHT"))
      {
          token = strstr(resp, "LIGHT");
          action = strtok(token, "C");
          }
    else
    {
        pc.printf(resp);
        }   
        x=action;
    } 
    pc.printf("..............talkback command........\n");
  
  if(strcmp(x,"LIGHTON")==0)
    {
    pc.printf("......LIGHT ON......\n");
    led1=0;
    }
  if(strcmp(x,"LIGHTOFF")==0)
    {
    pc.printf("......LIGHT OFF......\n");
    led1=1;
    }
      if(strcmp(x,"FANON")==0)
    {
    pc.printf("......FAN ON......\n");
    led2=0;
    }
      if(strcmp(x,"FANOFF")==0)
    {
    pc.printf("......FAN OFF......\n");
    led2=1;
    }
      if(strcmp(x,"TVON")==0)
    {
    pc.printf("......TV ON......\n");
    led3=0;
    }
      if(strcmp(x,"TVOFF")==0)
    {
    pc.printf("......TV OFF......\n");
    led3=1;
    }
    
}

int main () {
    led1=1;led2=1;led3=1;
    wifi_initialize();
    int error = 0;
    while (1) {
        wait(2.0f);                                 //wait 2 second
        error = sensor.readData();                  //read error value
        if (error == 0)                             //case: no error 
        {
            c   = sensor.ReadTemperature(CELCIUS);
            k   = sensor.ReadTemperature(KELVIN);
            h   = sensor.ReadHumidity();
            printf("Temperature in Kelvin: %4.2f, Celcius: %4.2f\n", k, c);
            printf("Humidity is %4.2f\n", h);
        } 
        else                                        //case: error
        {
            printf("Error: %d\n", error);
        }
        if(c>30)
        {
            pc.printf("......FAN ON......\n");
            led2=0;
        }
        if(c<30)
        {
            pc.printf("......FAN OFF......\n");
            led2=1;
        }
        wifi_send();
        wait(20);
        wifi_recv();
        wait(20);
    }
}