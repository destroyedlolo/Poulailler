# Poulailler

My **chicken coop** automation.

Automatisation de mon poulailler.
*Ce projet est majoritairement en anglais, non pas pour faire le mec qui s'y croit, mais simplement pour pouvoir partager sur les forums internationaux.*

## Goals

* Automatic open/close door as per sun light and/or consign
* Autonomous but ...
* ... connected to my home dashboard : wakeup consign, display critical figures on my dashboard and raise alarm in case of trouble.

## ESP8266 GPIOs

* **ADC** : power supply monitoring (*internal*)
* **GPIO0** : enable aux power (*low*)
* **GPIO2** : 1-wire
* **GPIO4** : water low (*low*)
* **GPIO5** : DHT22
* **GPIO15** : Door in stop position (*high* depend on GPIO12/13)
* **GPIO013** : Door up (*high*)
* **GPIO012** : water warming (*high*)

## DS2413 PIOs

* **PIO.A** : Force water warming
* **PIO.B** : sun light (*low*)

## Topics

* **Poulailler/Perchoir/Temperature** : as the name said
* **Poulailler/Perchoir/Humidite** : as the name said

* **Poulailler/Alim** : Power supply
* **Poulailler/Memory** : available memory
* **Poulailler/Network** : Network we are connected to
* **Poulailler/Wifi** : time to connect to the WiFi network
* **Poulailler/MQTT/Connection** : reconnect duration

* **Poulailler/Message** : various messages (errors, ...)

## Hardware

* MX1508 H-Bridge
* 3a77553800000091 : DS2413 - Day light probe

## Scenarii

* Perchoir : every 5 miniutes. No action
* Aux : every 15 minutes
	* Water : if too low, error at smart home dashboard level, prevent warming
	* light : 
		* If Day -> Night close door after 5'
		* If Night -> Day open the door

