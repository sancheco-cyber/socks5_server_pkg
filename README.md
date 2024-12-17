# socks5_server_pkg

Ce projet permet de lancer un serveur SOCKS V5 qui répond aux requêtes utilisant la commande CONNECT (cf. [RFC1928](https://datatracker.ietf.org/doc/html/rfc1928)).

## Environement de développement

Ubuntu 20.04.6 LTS

IDE: Qt Creator 4.11.0 (Based on Qt 5.12.8)

Installer `sudo apt install libclang-common-8-dev` pour l'IDE.

## Exécution du projet

Les options utilisées à la compilation se trouvent dans le fichier CMakeLists.txt

### Librairies utilisées

Utilisation de la librairie `pthread` pour le traitement des connections clients (threads).

### Portabilité 32-bit

Compilation en 32-bit en activant le flag `-m32`.

Installer les packages necessaires à la compilation :
~~~sh
sudo apt install gcc-multilib g++-multilib
sudo apt install libc6-dev-i386
~~~
### Compilation & lancement

Utilisation de cmake avec le fichier CMakeLists.txt :

~~~sh
cmake --version             # Verifier si CMake est installé
cd socKs5_server_pkg        # Aller dans le répertoire du projet
mkdir build                 # Créer un répertoire de build
cd build                    # Aller dans le répertoire de build
cmake ../socks5_srv         # Générer les fichiers de compilation avec CMake
make                        # Compiler le projet
ldd ./socksv5_pkg           # Affiche les librairies utilisées.

./socksv5_pkg               # Lancer le serveur (sans argument = serveur s'arête au bout de 5min)
./socksv5_pkg x             # Le serveur s'arête au bout de x min
~~~

## Tests

Navigateur : Chrome Version 131.0.6778.139

Paramètre du proxy (Socks Host): localhost 1080

Navigation libre sur Internet (Youtube, www.lemonde.fr, etc.)

Lancer dans un terminal. Le serveur s'arrète automatiquement au bout de 5min si non spécifié en argument.

## Todo
- Activation des log avec la librairie `<iostream>` uniquement en mode `DEBUG`
- Utiliser un pool de Thread pour la gestion des clients afin de pouvoir les maitriser.
- Le serveur s'arrète automatiquement au bout de 5min. Adapter l'arrêt du serveur à l'usage
- ...
