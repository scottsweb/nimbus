// TO-DO: Consider MDNS - https://github.com/krvarma/MDNS_SparkCore

// include the math library
#include <math.h>

// pins
int led = D6;
int intled = D7;
int dht22 = D3;
int ga1a = A0;

// general vars
#define MAXTIMINGS 85 // timings for DHT22
uint8_t data[5];
float rawRange = 4095; // analogRead returns values up to 4095 (http://docs.spark.io/#/firmware/i-o-analogread)
float logRange = 5.0; //  3.3v = 10^5 lux (https://learn.adafruit.com/adafruit-ga1a12s202-log-scale-analog-light-sensor/use-it)
int warningTempLow = 6; // enable warning when temperature reaches this value
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000) // a day in in millis
unsigned long lastSync = millis(); // last sync time for time sync
char json[64]; // to store JSON string

// cloud vars
char ip[24];
double temp = 0;
double humidity = 0;
int rawlight = 0;
double lux = 0;

// set to true to debug over serial
bool debug = false;

// setup
void setup() {

    // debug over serial
    if (debug) {
        Serial.begin(9600);
	    while(!Serial.available()) {
	       Serial.println("Press any key to start...");
	       delay(2000);
	    }
    	Serial.println("Booting nimbus v1.0");
    }

	// grab local IP
    IPAddress nim_ip = Network.localIP();
    sprintf(ip, "%d.%d.%d.%d", nim_ip[0], nim_ip[1], nim_ip[2], nim_ip[3]);

    // output IP over serial
	if (debug) {
        Serial.println(ip);
	    Serial.println("----------");
	}

    // set led as output (http://docs.spark.io/#/firmware/setup-pinmode)
    pinMode(led, OUTPUT);
    pinMode(intled, OUTPUT);

    // set dh22 as input
    pinMode(dht22, INPUT);

    // setup cloud vars - currently disabled in favour of publish
    // Spark.variable("temperature", &temp, DOUBLE);
    // Spark.variable("humidity", &humidity, DOUBLE);
    // Spark.variable("lux", &lux, DOUBLE);
    // Spark.variable("ip", ip, STRING);

    // turn off the bright led (save power and use it for low temp warning)
    if (!debug) {
        RGB.control(true);
        RGB.color(0, 0, 0);
    }

    // delay
    delay(2000);

    // read sensors
    readAllSensors();

    // power throttle the wifi (https://community.spark.io/t/wifi-power-throttling/4486)?

}

// loop
void loop() {

    // what minute is it? - only do this 4 times per hour
    if ( Time.minute() == 00 || Time.minute() == 15 || Time.minute() == 30 || Time.minute() == 45) {

        // read sensors
        readAllSensors();

        // publish as single JSON even (http://docs.spark.io/#/firmware/data-and-control-spark-publish)
        // there is a limit to the number of bytes (63) so shorter variable names are important
        sprintf(json,"{\"t\": %.2f, \"h\": %.2f, \"l\": %.2f}", temp, humidity, lux);
        Spark.publish("nimbus", json, 60, PRIVATE);

        // old method of publishing seperate events for each variable
        //Spark.publish("nimbus/temperature", String(temp), 60, PRIVATE);
        //Spark.publish("nimbus/humidity", String(humidity), 60, PRIVATE);
        //Spark.publish("nimbus/lux", String(lux), 60, PRIVATE);
        //Spark.publish("nimbus/ip", ip, 60, PRIVATE);

        // TO-DO. Make a local request to log data
        // https://gist.github.com/wgbartley/0724c8def5c671c797bf

        // output vars over serial when debuging
        if (debug) {
            Serial.println(temp);
            Serial.println(humidity);
            Serial.println(rawlight);
            Serial.println(lux);
            Serial.println("----------");
        }

        //  warn when low temperature
        if (temp <= warningTempLow) {

            // set on board LED to red & switch LED pin high (at night 5pm to 11:59pm)
            if (Time.hour() >= 17) {
                RGB.color(255, 0, 0);
                digitalWrite(led, HIGH);
            }

            // publish (http://docs.spark.io/#/firmware/data-and-control-spark-publish)
            Spark.publish("nimbus/low", String(temp), 60, PRIVATE);

            // print to serial if debugging
            if (debug) {
                Serial.println("Low temperature warning!");
                Serial.println("----------");
            }

        } else {

            // turn off on board LED and switch LED pin low
            RGB.color(0, 0, 0);
            digitalWrite(led, LOW);
        }

        // indicate the core is working using the D7 LED
        digitalWrite(intled, HIGH);
        delay(200);
        digitalWrite(intled, LOW);
        delay(200);
        digitalWrite(intled, HIGH);
        delay(200);
        digitalWrite(intled, LOW);
        delay(200);
        digitalWrite(intled, HIGH);
        delay(200);
        digitalWrite(intled, LOW);
        delay(200);
        digitalWrite(intled, HIGH);
        delay(200);
        digitalWrite(intled, LOW);
        delay(200);
        digitalWrite(intled, HIGH);
        delay(200);
        digitalWrite(intled, LOW);
        delay(200);

        // sync the time from Spark Cloud ?
        if (millis() - lastSync > ONE_DAY_MILLIS) {
            Spark.syncTime();
            lastSync = millis();
        }

        // delay for the rest of this minute as we only want one reading
        // this also gives us a whole minute to flash the core if necessary
        delay(59000); // 59000

        // sleep the core to save power when not debugging
        if (!debug) {

            // deep sleep the core for 14 minutes - we need to give it time to re-boot & re-connect
            Spark.sleep(SLEEP_MODE_DEEP, 840);

            // if any sensors are attached in future that need constant monitoring (PIR?) then deep sleep needs to be disabled
            // to do this nimbus will need to use WiFi.on() and WiFi.off to get the data sent asap
            // standard Spark.sleep will not work althogh the loop keeps running during a standard sleep
            // Spark.sleep(780);
        }
    }
}

/**
 * readAllSensors()
 *
 * Wrapper function to read all sensors attached to nimbus
 *
 * @return void
 */
void readAllSensors() {

    // grab the lux value
    rawlight = analogRead(ga1a);
    lux = readGA1A(rawlight);

    // read the dht22 (temp & humidity)
    readDHT22();
}

/**
 * readGA1A()
 *
 * Convert the raw reading from the GA1A (http://www.adafruit.com/products/1384) to a lux value
 *
 * @return float
 */
double readGA1A(int raw) {
    double logLux = raw * logRange / rawRange;
    return pow(10, logLux);
}

/**
 * readDHT22()
 *
 * Read both the temperature and humidity with each read of the DHT22
 *
 * @return void
 */
void readDHT22(void) {

    if (read()) {
        temp = readTemperature();
        humidity = readHumidity();
    }
}

/**
 * readTemperature()
 *
 * Calculate temperature based on data returned from read()
 *
 * @return double
 */
double readTemperature(void) {

    double _t;

    _t = data[2] & 0x7F;
    _t *= 256;
    _t += data[3];
    _t /= 10;

    if (data[2] & 0x80)
        _t *= -1;

    return _t;
}

/**
 * readHumdity()
 *
 * Calculate humidity based on data returned from read()
 *
 * @return double
 */
double readHumidity(void) {

    double _h;

    _h = data[0];
    _h *= 256;
    _h += data[1];
    _h /= 10; // ?

    return _h;
}

/**
 * read()
 *
 * Read data from the DH22 as discussed here: https://community.spark.io/t/get-dht22-pulse-timing-with-micros/2689/3
 *
 * @return bool
 */
bool read(void) {

    uint8_t i, j=0;
    uint8_t lastState = HIGH;
    uint8_t newState = HIGH;
    unsigned long results[85];
    uint8_t state[85];
    unsigned long t0 = 0;
    unsigned long t1 = 0;

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    // send start sequence to the DHT22
    pinMode(dht22, OUTPUT);
    digitalWrite(dht22, LOW);
    delay(20);
    digitalWrite(dht22, HIGH);
    delayMicroseconds(40);
    pinMode(dht22, INPUT); // switch to input to read from the DHT22

    for (i=0; i< MAXTIMINGS; i++) {

        t0 = micros();
        while (newState == lastState) {
            t1 = micros();
            newState = digitalRead(dht22);
            if (t1 - t0 >= 500) break;
        }

        lastState = newState;

        if ((t1 - t0) > 500) break;

        // ignore first 4 transitions
        if (i >= 4  &&  lastState == 0) {
            // shove each bit into the storage bytes
            data[j/8] <<= 1;

            if ((t1 - t0) > 40) // in tests, low values were 22 to 26, and high values were 67 to 73
                data[j/8] |= 1;

            j++;
        }
    }

    // check we read 40 bits and that the checksum matches
    if ((j >= 40) &&  (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)))
        return true;

    return false;
}
