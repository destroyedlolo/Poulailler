# Poulailler

My **chicken coop** automation.

Automatisation de mon poulailler.
*Ce projet est majoritairement en anglais, non pas pour faire le mec qui s'y croit, mais simplement pour pouvoir partager sur les forums internationaux.*

## Goals

Due to several issues with the chicken coop itself I had to manage, I missed my deadline and had to buy a commercial door automation.
Consequently, despite it was close to be finished, I gave up with v1 which is now stalling and this v2's goals are simpler :

* temperature and humidity monitoring
* check the door is open at sunset and open at sunrise
* ensure there is remaining water
* still connected to my home automation : expose critical figures on my dashboards, raise alarm in case of trouble ...

## ESP8266 GPIOs

The material is recycled from v1: some parts of the material could have been simplified and some GPIOs are currently unused.

* **ADC** : power supply monitoring (*internal*)
* **GPIO0** : enable aux power (*low*)
* **GPIO2** : 1-wire
* **GPIO4** : Water warming (*unused*)
* **GPIO5** : DHT22
* **GPIO15** : Door open (*high* depend on GPIO13)
* **GPIO013** : Must be *high* to measure the door is open.
* **GPIO012** : (*unused* - connected to the H bridge)

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
* ~~28ff8fbf711703c3 : DS18B20 - Water temperature~~

## Test Bat

ADC = 5v * 25.1 / (100 + 25.1) = 1.0032v
I = 5 / 125.1K = ~ 40uA

## Connector

| Power | DHT  22 | Water |  1-wire | Closing Door | Opening Door |
| ----- | ------- | ----- | ------- | ------------ | ------------ |
| +5 G  | G Dt +3 | Dt G  | G Dt +3 | H  Com  End  |  H Com End   |
                                      V   O    M      J  B   O
									  i   r    a      a  l   r
									  o   a    r      u  e
									  l   n    r      n  u

## Dependencies

* [SimpleDHT](https://github.com/winlinvip/SimpleDHT) - **DHT** connectivity
* [PubSubClient](https://github.com/knolleary/pubsubclient) - **MQTT** connectivity
* [KeepInRTC](https://github.com/destroyedlolo/KeepInRTC) - ESP8266's RTC memory management
* [LFUtilities](https://github.com/destroyedlolo/LFUtilities) - Various helpers
