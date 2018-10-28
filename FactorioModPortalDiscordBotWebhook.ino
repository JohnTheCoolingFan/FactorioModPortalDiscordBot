//mods.factorio.com discord bot for ESP32 by John The Cooling Fan

//LCD Screen for IRL debug
//#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x3F, 20, 4);

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "ssid";
const char* password = "password";
const String mod_name = "mod_name";
const String factorio_server = "https://mods.factorio.com/api/mods/" + mod_name;
const String discord_webhook = "discord_webhook";
const String discord_server = "https://discordapp.com/api";

unsigned long lastConnectionTime = 0;

String httpData;

struct mod_structure {
	const char* download_url;
	const char* file_name;
	const char* released_at;
	const char* version;
	const char* name;
	const char* owner;
	const char* downloads_count;
};

struct webhook_strcuture {
	const char* id;
	const char* guild_id;
	const char* token;
	const char* channel_id;
	const char* name;
};

mod_structure mod_data;
webhook_strcuture webhook_data;

void setup() {
	/*lcd.init();
	lcd.backlight();
	lcd.setCursor(0, 0);
	lcd.print("Connecting to:");
	lcd.setCursor(0, 1);
	lcd.print(ssid);
	lcd.setCursor(0, 3);
	lcd.print("Please, wait...");*/

	Serial.begin(115200);
	Serial.println("\nBot Started.");

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(250);
	}

	httpRequest(factorio_server);
	delay(250);
	parseData();
	//delay(250);
	//httpRequest(discord_webhook);
	//delay(250);
	//parse_webhook();
  
	postData(discord_webhook);
}

void loop() {
	// not used

}

bool httpRequest(String url) {
	HTTPClient client;
	bool find = false;
	Serial.print("Connecting ");
	client.begin(url);
	int httpCode = client.GET();

	if (httpCode > 0) {
		Serial.printf("successfully, code: %d\n", httpCode);
		if (httpCode == HTTP_CODE_OK) {
			httpData = client.getString();
			lastConnectionTime = millis();
			find = true;
		}
	}
	else Serial.printf("failed, error: %s\n", client.errorToString(httpCode).c_str());

	client.end();

	return find;
}

bool postData(String post_url) {
	StaticJsonBuffer<300> JSONbuffer;
	JsonObject& JSONencoder = JSONbuffer.createObject();
	
	Serial.println("Forming message. Content:");
	Serial.println("Mod name: " + String(mod_data.name));
	Serial.println("Owner: " + String(mod_data.owner));
	Serial.println("Latest version: " + String(mod_data.version));
	Serial.println("Download: https://mods.factorio.com" + String(mod_data.download_url));
	Serial.println("Downloaded " + String(mod_data.downloads_count) + " times");
	
	JSONencoder["content"] = "Mod name: " + String(mod_data.name) + "\nOwner: " + String(mod_data.owner) + "\nLatest version: " + String(mod_data.version) + "\nDownload: https://mods.factorio.com" + String(mod_data.download_url) + "\nDownloaded " + String(mod_data.downloads_count) + " times.";
	
	char JSONmessageBuffer[300];
	JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
	
	HTTPClient post_client;
	post_client.begin(post_url);
	post_client.addHeader("Content-Type", "application/json"); //Specify content-type header
	int httpCode = post_client.POST(JSONmessageBuffer); //Send the request
	String payload = post_client.getString(); //Get the response payload
	Serial.println(httpCode); //Print HTTP return code
	Serial.println(payload); //Print request response payload
	post_client.end(); //Close connection
}

bool parseData() {
	Serial.println("Parsing started, httpData:");
	Serial.println(httpData);
	Serial.print("\n");

	DynamicJsonBuffer jsonBuffer_mod;
	JsonObject& mod_root = jsonBuffer_mod.parseObject(httpData);

	if (!mod_root.success()) {
		Serial.println("Json parsing failed!");
		httpData = "";
		return false;
	}

	int latest_release_num = 0;
	bool flag = true;
	for (int i = 0; flag; i++) {
		if (mod_root["releases"][i] == NULL) {
			flag = false;
			latest_release_num = i - 1;
		}
	}
	
	Serial.println("Forming mod_data. JSON:");
	mod_root.prettyPrintTo(Serial);
	Serial.println();
  
	mod_data.download_url    = mod_root["releases"][latest_release_num]["download_url"];
	mod_data.file_name       = mod_root["releases"][latest_release_num]["file_name"];
	mod_data.released_at     = mod_root["releases"][latest_release_num]["released_at"];
	mod_data.version         = mod_root["releases"][latest_release_num]["version"];
	mod_data.name            = mod_root["name"];
	mod_data.owner           = mod_root["owner"];
	mod_data.downloads_count = mod_root["downloads_count"];
	
	httpData = "";
	return true;
}

bool parse_webhook() {
	Serial.println("Webhook parsing started, httpData:");
	Serial.println(httpData);
	Serial.print("\n");
	
	DynamicJsonBuffer jsonBuffer_webhook;
	JsonObject& webhook_root = jsonBuffer_webhook.parseObject(httpData);
	
	if (!webhook_root.success()) {
		Serial.println("Webhook Json parsing failed!");
		httpData = "";
		return false;
	}
	
	Serial.println("Forming webhook_data. JSON:");
	webhook_root.prettyPrintTo(Serial);
	Serial.println();
	
	webhook_data.id         = webhook_root["id"];
	webhook_data.guild_id   = webhook_root["guild_id"];
	webhook_data.token      = webhook_root["token"];
	webhook_data.channel_id = webhook_root["channel_id"];
	webhook_data.name       = webhook_root["name"];
	
	httpData = "";
	return true;
}