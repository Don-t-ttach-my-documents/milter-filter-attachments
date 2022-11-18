# Connexion à Postfix via l'API Milter
## Installer l'environnement de dev
```bash
sudo apt install libmilter-dev
sudo apt install miltertest
```
## Compiler
```bash
make
```

## Tester le filtre 
Après avoir compiler, lancer miltertest via la ligne de commande
```bash
cd test
miltertest -s test1.lua
```
