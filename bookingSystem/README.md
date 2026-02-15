# Etapes de réalisation d'un projet django/Étapes de développement

## 1. Préparation de l'environnement
```
Installer python(3.8+)
Créer un environnement virtuel Python pour isoler les dépendances
Installer Python (3.8+), pip, virtualenv, django
Installer PostgreSQL sur votre machine
```
`code inline`
```bash
sudo apt update
sudo apt upgrade -y
sudo apt install python3 python3-pip python3-venv -y
sudo apt install postgresql postgresql-contrib -y
#vérification de postgresql
sudo systemctl status postgresql
psql --version
#création du dossier projet
mkdir mon_api_django
cd mon_api_django
#création de l'environnement virtuel et activation
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip3
pip install django djangorestframework psycopg2-binary python-decouple django-cors-headers
```

## 2. Initialisation du projet
```
Créer et activer l'environnement virtuel
Installer Django et psycopg2 (driver PostgreSQL)
Créer le projet Django avec django-admin startproject
Créer votre première application avec python manage.py startapp
```

## 3. Configuration de la base de données
```
Créer une base PostgreSQL dédiée
Configurer les paramètres de connexion dans settings.py
Exécuter les migrations initiales avec python manage.py migrate
```

## 4. Développement backend
```
Définir vos modèles de données (models.py)
Créer les migrations et les appliquer
Développer les vues (views.py) et URLs (urls.py)
Configurer l'admin Django si besoin
Créer les API REST (avec Django REST Framework si nécessaire)
```

## 5. Développement frontend
```
Créer les templates HTML dans le dossier templates/
Intégrer CSS/JavaScript (statiques dans static/)
Ou utiliser un framework JS moderne (React, Vue) avec Django en backend API
```

## 6. Tests et débogage
```
Écrire des tests unitaires
Tester avec le serveur de développement (python manage.py runserver)
Déboguer avec Django Debug Toolbar
```

## 7. Déploiement
```
Configurer les variables d'environnement (secrets, DEBUG=False)
Collecter les fichiers statiques
Choisir une plateforme (Heroku, AWS, DigitalOcean, etc.)
Configurer un serveur WSGI (Gunicorn) et un reverse proxy (Nginx)
```
`code inline`
```bash
#1)buy a domain name (easy on ovh)
#2)launch a virtual private server (I launch an amazon ec2)
#name : server's name
#select amazon machine linux for OS images
# amazon machine image (AMI) : Amazon Linux 2023 kernel-6.1 AMI
#Architecture : 64-bit(x86)
#Instance type : t3.micro
#key pair login : create a new key pair, key pair type RSA, private key file format .pem for use of openssh
#register your .pem file localy and secure it
#Network settings : firewall security groups : create security group, activate ssh,https and http traffic from 
#anywhere 0.0.0.0/0 for the moment. We will see it more in details next time
#Configure storage : 1x8 GIB gp3 Root volume, 3000 IOPS, Not encrypted
#launch instance
#click on the link, now the vps is online
#connect to the vps
#3)point the domain name on the vps (configure it on the domain name provider)
#I have already done it, I will come back later to complete this part, I promise
#4)Docker installation (containerize an app using docker)
