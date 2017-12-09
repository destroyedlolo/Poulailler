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
* **GPIO0** : enable water warming (*low*)
* **GPIO2** : 1-wire
* **GPIO4** : water low (*low*)
* **GPIO15** : Door in stop position (*high* depend on GPIO12/13)
* **GPIO013** : Door up (*high*)
* **GPIO012** : Door Down (*high*)

## DS28EA00 PIOs

* **PIO.A** : Auxiliary power enable (*low*)
* **PIO.B** : sun light (*low*)

## Door control

- MX1508 H-Bridge
