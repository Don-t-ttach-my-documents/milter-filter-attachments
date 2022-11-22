# Connexion à Postfix via l'API Milter
## Installer l'environnement de dev
```bash
sudo apt install libmilter-dev
sudo apt install libcurl4-openssl-dev
sudo apt install miltertest
```
## Compiler
```bash
make
```
## Utilisation de services tiers
Ce filtre utilise la librairie [Milter](https://en.wikipedia.org/wiki/Milter) pour développer un filtre mail sur [Postfix](https://www.postfix.org/) ainsi que [libcurl](https://curl.se/libcurl/) pour effectuer des requêtes HTTP.

Ce filtre utilise [le service de parsing smtp](https://github.com/Don-t-ttach-my-documents/smtp-add-on) spécialement implémenté pour ce projet. Le filtre essaie actuellement de se connecter à l'adresse http://localhost:3201/ via une requête POST en envoyant en donnée le corps du mail, et en recevant le nouveau corps du mail avec la nouvelle pièce jointe.

## Tester le filtre 
Après avoir compiler, lancer miltertest via la ligne de commande
```bash
cd test
miltertest -s test1.lua
```
Des tests supplémentaires peuvent être rédigés en lua via [miltertest](https://manpages.ubuntu.com/manpages/trusty/man8/miltertest.8.html).


