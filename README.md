# l'Otomate
Voici l'**Otomate**, un outil développé par et pour les membres de la CAPÉ (Coopérative pour l'agriculture de proximité écologique) pour permettre aux petites, moyennes et grosses fermes d'automatiser leurs serres à une fraction du coût du marché.

Il dispose de fonctionalités permettant :
- de définir jusqu'à 5 périodes horaires permettant de faire évoluer la température normale au courant de la journée
- de contrôler l'ouverture et la fermeture de moteurs de côtés ouvrants, avec possibilité d'utiliser un programme distinct pour chaque côté de la serre
- d'actionner un ou deux systèmes de ventilation
- d'actionner une ou deux fournaises

Le programme supporte la sonde de température DS18B20 et le module d'horloge DS3231 pour le moment.

Le code, les schémas, les plans nécessaires à la conception de ce contrôleur sont du domaine public.Le code, les schémas et les plans nécessaires à la conception de ce contrôleur sont certifiés open-source, donc appartiennent au domaine public. Ils peuvent être utilisés, modifiés, hackés, par n'importe qui, pour n'importe quel usage, tant que les termes de la license GPL sont respectés.

Pour des détails quant  l'utilisation du présent code, consultez la section [wiki](https://github.com/LoupHC/controleur-CAPE/wiki).
## Librairies
Pour exécuter le programme, vous devez préalablement déplacer toutes les fichiers du dossier "librairies" dans la banque de librairie de votre IDE.

Aperçu des librairies nécessaires: 
- GreenhouseLib : fonctions liées au contrôle du climat
- Timelord : pour calculer le lever/coucher du soleil et les changements d'heure
- OneWire : pour communiquer via le protocole oneWire
- Dallas Temperature : version MODIFIÉE de la librairie originale, pour communiquer avec la sonde DS18B20
- DS3231 : pour communiquer avec le module d'horloge DS3231
- LiquidCrystal_I2C : pour communiquer avec le backpack I2C de l'écran LCD
- Keypad : pour interprèter le clavier matriciel
- Keypad_I2C : pour interprèter le clavier matriciel via la chip PCF8574
- elapsedMillis : optimiser les délais

Si vous n'avez jamais téléchargé une librairie, suivez les instructions à partir de l'onglet Manual Installation :
https://www.arduino.cc/en/Guide/Libraries

