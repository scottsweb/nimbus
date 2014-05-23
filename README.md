## nimbus

A WiFi weather station built upon a Spark Core.

## About

nimbus is a WiFi weather station with a variety of sensors. At the moment it can read light, temperature and humidity. In future I am hoping to add a nitrogen dioxide and carbon monoxide sensor to keep an eye on local pollution levels. 

Issues/Problems/Questions? [Open a GitHub issue](https://github.com/scottsweb/nimbus/issues). You can also contact me via [scott.ee](http://scott.ee) or [twitter (@scottsweb)](http://twitter.com/scottsweb).

## Hardware

* [Spark Core](https://www.spark.io/)
* [DHT22 (AM2302)](https://learn.adafruit.com/dht)
* [Adafruit GA1A12S202 Log-scale Analog Light Sensor](https://learn.adafruit.com/adafruit-ga1a12s202-log-scale-analog-light-sensor)
* Single LED & resistor (see http://docs.spark.io/#/examples/blink-an-led for an intro into calculating resistors)
* 10kΩ resistor

## Wiring


nimbus.fzz can be opened in [Fritzing](http://fritzing.org/). The 10kΩ resistor is needed for the DHT22.

## Installation

Copy the contents of nimbus.ino into the [Spark Build IDE](https://www.spark.io/build/) and flash your core. There are no external libraries required at the moment.

## Changelog 

### 1.0
* Initial release



