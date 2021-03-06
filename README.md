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
* **GPIO4** : Water warming
* **GPIO5** : DHT22
* **GPIO15** : Door in stop position (*high* depend on GPIO12/13)
* **GPIO013** : Door up (*high*)
* **GPIO012** : water warming (*high*)

## DS2413 PIOs

* **PIO.A** : sun light (*low*)
* **PIO.B** : water low (*low*)

## Topics

* **Poulailler/Perchoir/Temperature** : as the name said
* **Poulailler/Perchoir/Humidite** : as the name said

* **Poulailler/Eau/Niveau** : Suffisant / Vide

* **Poulailler/Luminosite** : Jour / Nuit

* **Poulailler/Alim** : Power supply
* **Poulailler/Memoire** : available memory
* **Poulailler/Reseau** : Network we are connected to
* **Poulailler/Reseau/Change** : when we switch to another network
* **Poulailler/Wifi** : time to connect to the WiFi network
* **Poulailler/MQTT** : reconnect duration

* **Poulailler/Error** : Error messages
* **Poulailler/Message** : various messages, output, ...
* **Poulailler/Command** : MQTT commands topic ...

## Hardware

* MX1508 H-Bridge
* 3a77553800000091 : DS2413 - Day light probe
* 28ff8fbf711703c3 : DS18B20 - Watter temperature

## Test Bat

ADC = 5v * 25.1 / (100 + 25.1) = 1.0032v
I = 5 / 125.1K = ~ 40uA

## Scenarii

### Startup

The goal is the reach a stable and known door position. MQTT data has to be ignored has we don't know if they are valid or outdated.

1. Day or Night ?
	1. Power Aux
	1. Reading Day (publish **water** as well)

1. Door position ?
	* if **Day**
		- Door open
	* if **Night**
		- Door close

### Running
WIP : to be ignored.
* ~~Perchoir : every 5 minutes. No action~~
* ~~Aux : every 15 minutes~~
	* Water : if too low, error at smart home dashboard level, prevent warming
	* ~~light :~~ 
		* ~~If Day -> Night close door after 5'~~
		* ~~If Night -> Day open the door~~

## Connector

| Power | DHT  22 | Water |  1-wire | Closing Door | Opening Door |
| ----- | ------- | ----- | ------- | ------------ | ------------ |
| +5 G  | G Dt +3 | Dt G  | G Dt +3 | H  Com  End  |  H Com End   |
                                      V   O    M      J  B   O
									  i   r    a      a  l   r
									  o   a    r      u  e
									  l   n    r      n  u
