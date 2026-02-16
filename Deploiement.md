# Les Grandes Étapes du Déploiement d'Applications Web

> **Objectif** : Rendre votre application accessible à n'importe qui sur Internet via une URL

## 🎯 Définition Claire

**Déployer** = Passer de "ça marche sur mon ordinateur" à "ça marche pour tout le monde sur Internet"

---

## 📋 Les 4 Piliers Fondamentaux

### 1. **INFRASTRUCTURE** 
*Où va tourner votre application ?*

- **Serveur/VPS** : Une machine qui tourne 24/7
  - Exemples : DigitalOcean, AWS EC2, Hetzner, OVH
  - C'est comme louer un ordinateur dans un datacenter

- **Domaine** : L'adresse que les gens taperont
  - Exemple : `monapp.com` au lieu de `195.123.45.67`
  - Fournisseurs : Namecheap, OVH, Gandi

### 2. **SÉCURITÉ**
*Protéger votre application et vos données*

- **SSH sécurisé** : Connexion par clés (pas de mot de passe)
- **Firewall** : Bloquer tous les ports sauf 22 (SSH), 80 (HTTP), 443 (HTTPS)
- **Utilisateur non-root** : Ne jamais utiliser le compte root
- **SSL/HTTPS** : Certificat gratuit Let's Encrypt
- **Variables d'environnement** : Secrets jamais dans le code

### 3. **CONTENEURISATION**
*Empaqueter votre application*

- **Docker** : Votre app + toutes ses dépendances dans un "container"
  - Dockerfile : Recette pour construire l'image
  - docker-compose.yml : Orchestrer plusieurs services (app + db + nginx)
  
- **Pourquoi ?**
  - Ça marche pareil partout (local = production)
  - Facile à mettre à jour
  - Isolation des services

### 4. **ROUTAGE WEB**
*Diriger le trafic Internet vers votre app*

- **DNS** : Pointer votre domaine vers l'IP du serveur
  - Type A : `monapp.com` → `195.123.45.67`
  
- **Reverse Proxy (Nginx)** : Le "portier" de votre serveur
  - Reçoit les requêtes HTTP/HTTPS
  - Les envoie à votre app
  - Gère le SSL
  - Sert les fichiers statiques

---

## 🔄 Le Workflow Complet (Étape par Étape)

### **PHASE 1 : PRÉPARATION (En Local)**

```
1. Développer votre application
2. Tester en local
3. Créer un Dockerfile
4. Créer un docker-compose.yml
5. Tester avec Docker en local
6. Pousser le code sur Git (GitHub/GitLab)
```

**✅ Checkpoint** : Votre app tourne dans Docker sur votre machine

---

### **PHASE 2 : INFRASTRUCTURE**

```
1. Acheter un domaine (monapp.com)
2. Louer un VPS (serveur Linux)
3. Noter l'IP du serveur
4. Configurer le DNS :
   - Type A : @ → IP_DU_SERVEUR
   - Type A : www → IP_DU_SERVEUR
5. Attendre la propagation DNS (5 min - 48h)
```

**✅ Checkpoint** : `ping monapp.com` retourne l'IP de votre serveur

---

### **PHASE 3 : SÉCURISATION DU SERVEUR**

```bash
# Se connecter en SSH
ssh root@IP_DU_SERVEUR

# Créer un utilisateur non-root
adduser monuser

# Ajouter monuser au groupe wheel (pour lui donner les droits sudo)
sudo usermod -aG wheel monuser

# Configurer SSH avec clés 
#(sur votre machine locale)
# 1. Générer une paire de clés SSH
ssh-keygen -t ed25519 -C "votre@email.com"
# Appuyez sur Entrée 3 fois (emplacement par défaut, pas de passphrase)
# 2. Afficher votre clé publique
cat ~/.ssh/id_ed25519.pub
# Vous verrez quelque chose comme :
# ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAA... votre@email.com
# COPIEZ tout ce texte (Ctrl+Shift+C)


#Sur le serveur AWS (dans le terminal AWS Console) :
# Basculer vers monuser
sudo su - monuser
# Créer le dossier .ssh
mkdir -p ~/.ssh
chmod 700 ~/.ssh
# Créer et éditer le fichier authorized_keys
nano ~/.ssh/authorized_keys
# COLLEZ votre clé publique (clic droit → Paste)
# Ctrl+X, puis Y, puis Entrée pour sauvegarder
# Définir les bonnes permissions
chmod 600 ~/.ssh/authorized_keys
# Vérifier que c'est bon
cat ~/.ssh/authorized_keys
# Revenir à ec2-user
exit

#Plus tard, vous pourrez directement vous connecter à votre serveur via la commande
ssh monuser@opus-symmetry.fr


# Désactiver la connexion root + mot de passe; il faut que personne ne puisse se connecter avec un mot de passe
sudo nano /etc/ssh/sshd_config
# PermitRootLogin no
# PasswordAuthentication no

# Redémarrer SSH pour appliquer les changements
sudo systemctl restart sshd



# Installer firewalld
sudo yum install firewalld -y

# Démarrer firewalld
sudo systemctl start firewalld
sudo systemctl enable firewalld

# Autoriser SSH (port 22)
sudo firewall-cmd --permanent --add-service=ssh

# Autoriser HTTP (port 80)
sudo firewall-cmd --permanent --add-service=http

# Autoriser HTTPS (port 443)
sudo firewall-cmd --permanent --add-service=https

# Recharger la configuration
sudo firewall-cmd --reload

# Vérifier le statut
sudo firewall-cmd --list-all

# Mises à jour, pas obligatoire, et il faut installer apt avant
sudo apt update && sudo apt upgrade -y
```

**✅ Checkpoint** : Vous ne pouvez vous connecter qu'avec votre clé SSH

---

### **PHASE 4 : INSTALLATION DE DOCKER**

```bash
# Installer Docker via les dépôts Amazon Linux
sudo yum install docker -y

# Démarrer Docker
sudo systemctl start docker
sudo systemctl enable docker

# Ajouter votre utilisateur au groupe docker
sudo usermod -aG docker $USER


# Docker Compose plugin n'est pas disponible, dans les dépots amazon linux standards, donc on va l'installer 
#manuellement

# Créer le répertoire pour les plugins Docker
mkdir -p ~/.docker/cli-plugins/

# Télécharger Docker Compose v2
curl -SL https://github.com/docker/compose/releases/latest/download/docker-compose-linux-x86_64 -o ~/.docker/cli-plugins/docker-compose

# Rendre le fichier exécutable
chmod +x ~/.docker/cli-plugins/docker-compose

# Vérifier l'installation
docker compose version

#Après installation, déconnexion 
exit

#Puis reconnexion
ssh monuser@16.171.116.66

# Vérifier Docker
docker --version

# Vérifier Docker Compose
docker compose version

# Tester
docker run hello-world

```
**✅ Checkpoint** : `docker run hello-world` fonctionne

---

### **PHASE 5 : DÉPLOIEMENT DE L'APPLICATION**

```bash
# 1. Se placer dans le home
cd ~

# 2. Créer le dossier apps
mkdir -p apps && cd apps

# 3. Installer Git (si pas déjà fait)
sudo yum install git -y

# 4. Cloner votre projet
git clone https://github.com/fork7ienthropy/myblog71.git

# 5. Aller dans le dossier de l'application
cd myblog71/joel_blog

# 6. Vérifier qu'on est au bon endroit
pwd
# Devrait afficher : /home/monuser/apps/myblog71/joel_blog

# 7. Créer le Dockerfile
nano Dockerfile

# 8. Créer docker-compose.yml
nano docker-compose.yml

# 9. Créer le fichier .env
nano .env

# 10. Modifier settings.py
# - DEBUG = False
# - Supprimer SECRET_KEY et la mettre dans .env

# 11. Construire l'image Docker (BUILD CLASSIQUE)
docker build -t joel_blog-web .

# 12. Lancer les containers
docker compose up -d

# 13. Vérifier que ça tourne
docker compose ps
docker compose logs -f

# EN CAS D'ERREUR - Boucle de debug :
docker compose down
docker system prune -f
# Corriger le problème
docker build -t joel_blog-web .
docker compose up -d
docker compose logs -f web
```
**✅ Checkpoint** : Votre app tourne, accessible sur http://IP_DU_SERVEUR

---




### **PHASE 6 : CONFIGURATION NGINX**

```bash
#vérifier que vous êtes dans le dossier contenant manage.py
mkdir nginx ; cd nginx ; nano nginx.conf 

```

```nginx
# nginx/nginx.conf - Configuration initiale (HTTP seulement)

events {
    worker_connections 1024;
}

http {
    include /etc/nginx/mime.types;
    default_type application/octet-stream;

    upstream django {
        server web:8000;  # ⚠️ "web" doit correspondre au service dans docker-compose.yml
    }

    server {
        listen 80;
        server_name opus-symmetry.fr www.opus-symmetry.fr;

        client_max_body_size 100M;

        # Pour Let's Encrypt (Phase 8)
        location /.well-known/acme-challenge/ {
            root /var/www/certbot;
        }

        # Servir les fichiers statiques
        location /static/ {
            alias /app/staticfiles/;
            expires 30d;
            add_header Cache-Control "public, immutable";
        }

        # Servir les fichiers média
        location /media/ {
            alias /app/media/;
        }

        # Proxy vers Django
        location / {
            proxy_pass http://django;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
            proxy_redirect off;
        }
    }
}

```

**✅ Checkpoint** : `http://monapp.com` affiche votre application




### **PHASE 7 : Gestion des fichiers statiques **

```
Comment faire en sorte que django accepte de "présenter" les fichiers
statiques en production ? En local, django sert automatiquement les fichiers statiques,pas de configuration nécessaire, tout fonctionne "magiquement". En production, django refuse de servir les fichiers statiques (par design) pour des soucis de performance. Django n'est pas optimisé pour 
servir des fichiers statiques. Raison pour laquelle on utilisera un serveur web Nginx pour cela.

Nginx : Le portier/serveur (ultra rapide, léger, moderne, asynchrone)

Reçoit TOUTES les requêtes HTTP/HTTPS
Décide qui doit traiter quoi
Sert les fichiers statiques lui-même (ultra rapide)
Passe le reste à Django


Gunicorn : Le coordinateur

Interface entre Nginx et Django
Gère plusieurs processus Django en parallèle
Protocole WSGI (Web Server Gateway Interface)


Django : Le cerveau

Traite uniquement les requêtes dynamiques
Génère les pages HTML personnalisées
Communique avec la base de données

```

```python
# Pensez à effectuer des modifications de votre fichier settings.py, c'est l'étape la plus délicate du processus
import os # à ajouter
BASE_DIR = Path(__file__).resolve().parent.parent 

# SECURITY WARNING: keep the secret key used in production secret! Toujours garder cette variable, c'était ca la cause du bug !
SECRET_KEY = os.environ.get('SECRET_KEY')

# SECURITY WARNING: don't run with debug turned on in production!
DEBUG = False

#ALLOWED_HOSTS = "Qui peut accéder à mon site ?"Par quel nom de domaine les gens peuvent accéder à ton serveur ? 
ALLOWED_HOSTS = ['opus-symmetry.fr', 'www.opus-symmetry.fr','127.0.0.1']

#CSRF_TRUSTED_ORIGINS = "Qui peut soumettre des formulaires sur mon site ?"
CSRF_TRUSTED_ORIGINS = ["https://opus-symmetry.fr", "https://www.opus-symmetry.fr", "http://opus-symmetry.fr", "http://www.opus-symmetry.fr"]


STATIC_URL = 'static/'
STATIC_ROOT = BASE_DIR/'staticfiles'


STATICFILES_DIRS = [
    BASE_DIR/"opus_symmetry/static",
]

# Vous ne devriez pas avoir de problèmes par la suite
```

```bash
# Début boucle de relance (elle est partiellement correcte)

# arrêter et supprimer tous les conteneurs, puis toutes les images non utilisées (nettoyage)
docker compose down
docker container prune
docker docker image prune -a

# Voir TOUS les containers (y compris arrêtés)
docker ps -a
# Voir toutes les images
docker images

# Après avoir mis à jour les paramètres de sécurité et push le code local, on le récupère dans l'ec2
git fetch origin
git reset --hard origin/main

docker build -t joel_blog-web .
docker compose up -d
docker compose ps

# Fin boucle 

# Debuggage 
# 1. Voir les logs en temps réel
docker compose logs web -f
docker compose logs web --tail=30

```


---

### **PHASE 8 : CERTIFICAT SSL (HTTPS)**



```bash


# Création du fichier .gitignore (j'avais oublié)
nano .gitignore

# Ajouter ce contenu
```
```
# Fichiers secrets - NE JAMAIS COMMIT
.env
*.env
.env.local
.env.production

# Base de données
db.sqlite3
*.sqlite3

# Fichiers Python
__pycache__/
*.py[cod]
*$py.class
*.so
.Python

# Django
*.log
local_settings.py
staticfiles/
media/

# Certificats SSL
certbot/

# Docker
*.pid
*.seed
*.pid.lock

# IDE
.vscode/
.idea/
*.swp
*.swo
*~

# OS
.DS_Store
Thumbs.db

# Backups
*.bak
*.backup
```


```bash
# Arrêter nginx temporairement
docker compose stop nginx

# Obtenir le certificat
docker compose run --rm certbot certonly --standalone \
  -d monapp.com -d www.monapp.com \
  --email votre@email.com \
  --agree-tos

# Mettre à jour nginx.conf pour ajouter HTTPS
# (voir configuration complète dans le guide)

# Relancer
docker compose up -d

# Le renouvellement est automatique avec le service certbot
```

**✅ Checkpoint** : `https://monapp.com` fonctionne avec le cadenas vert

---

### **Dernière phase suggérée par Claude(HTTPS)**
```bash

#Etape 0 : Créer le fichier .gitignore pour des raisons de sécurité (je l'ai fait dans le step 8, à recopier ici)
#Étape 1 : Modifier docker-compose.yml
nano docker-compose.yml
```
```yaml
services:
  web:
    build: .
    expose:
      - 8000
    volumes:
      - static_volume:/app/staticfiles
      - media_volume:/app/media
    env_file:
      - .env
    command: gunicorn --bind 0.0.0.0:8000 --workers 3 --access-logfile - --error-logfile - joel_blog.wsgi:application

  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - static_volume:/app/staticfiles:ro
      - media_volume:/app/media:ro
      - ./nginx/nginx.conf:/etc/nginx/nginx.conf:ro
      - ./certbot/conf:/etc/letsencrypt:ro
      - ./certbot/www:/var/www/certbot:ro
    depends_on:
      - web

  certbot:
    image: certbot/certbot
    volumes:
      - ./certbot/conf:/etc/letsencrypt
      - ./certbot/www:/var/www/certbot
    entrypoint: "/bin/sh -c 'trap exit TERM; while :; do certbot renew; sleep 12h & wait $${!}; done;'"

volumes:
  static_volume:
  media_volume:
```
```bash 
#Étape 2 : Créer les dossiers certbot
mkdir -p certbot/conf certbot/www

#Étape 3 : Modifier nginx.conf (version temporaire)
nano nginx/nginx.conf
```
```nginx
events {
    worker_connections 1024;
}

http {
    server {
        listen 80;
        server_name opus-symmetry.fr www.opus-symmetry.fr;
        
        location /.well-known/acme-challenge/ {
            root /var/www/certbot;
        }
        
        location / {
            proxy_pass http://web:8000;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
        }
    }
}
```
```bash


# Etape 4. TOUT arrêter
docker compose down

# Etape 5. Supprimer BRUTALEMENT certbot (au cas où il y a des fichiers cachés)
sudo rm -rf certbot/
mkdir -p certbot/conf certbot/www

# Etape 6. Créer le certificat standalone (SANS nginx qui tourne)
docker run --rm -it \
  -v "$(pwd)/certbot/conf:/etc/letsencrypt" \
  -v "$(pwd)/certbot/www:/var/www/certbot" \
  -p 80:80 \
  certbot/certbot certonly --standalone \
  -d opus-symmetry.fr -d www.opus-symmetry.fr \
  --email julientelook@gmail.com \
  --agree-tos \
  --no-eff-email



```

**Remplace `email@example.com`** par ton vrai email.

**Résultat attendu** :
```
Congratulations! Your certificate has been saved at:
/etc/letsencrypt/live/opus-symmetry.fr/fullchain.pem

```
```bash
#Étape 7 : Modifier nginx.conf (version finale avec HTTPS)
nano nginx/nginx.conf
```
```nginx
events {
    worker_connections 1024;
}

http {
    # HTTP → Redirige vers HTTPS
    server {
        listen 80;
        server_name opus-symmetry.fr www.opus-symmetry.fr;
        
        location /.well-known/acme-challenge/ {
            root /var/www/certbot;
        }
        
        location / {
            return 301 https://$host$request_uri;
        }
    }
    
    # HTTPS
    server {
        listen 443 ssl;
        server_name opus-symmetry.fr www.opus-symmetry.fr;
        
        ssl_certificate /etc/letsencrypt/live/opus-symmetry.fr/fullchain.pem;
        ssl_certificate_key /etc/letsencrypt/live/opus-symmetry.fr/privkey.pem;
        
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_prefer_server_ciphers on;
        
        location / {
            proxy_pass http://web:8000;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto https;
        }
        
        location /static/ {
            alias /app/staticfiles/;
        }
        
        location /media/ {
            alias /app/media/;
        }
    }
}
```
```bash
#Étape 8 : Redémarrer nginx
docker compose restart nginx
#Étape 9 : Tester
docker compose restart web
#Ouvre ton navigateur : https://opus-symmetry.fr

#debugging
docker compose logs nginx
docker compose logs web


#J'obtiens une erreur : 
# => ERROR [internal] load build context                          0.0s
#ERROR: failed to solve: error from sender: open certbot/conf/accounts: permission denied
# 1. Donner les bonnes permissions au dossier certbot
sudo chown -R monuser:monuser certbot/
sudo chmod -R 755 certbot/

# 2. Vérifier
ls -la certbot/
```

## Un site moche pas mis en page (les fichiers statiques ne sont pas chargés) ... Comment y remédier ? 

### ✅ Vérifier et collecter les fichiers statiques
```bash
# 1. Collecter les fichiers statiques
docker compose exec web python manage.py collectstatic --noinput

# 2. Vérifier qu'ils sont bien là dans le container web
docker compose exec web ls -la /app/staticfiles/

# 3. Vérifier qu'ils sont bien là dans nginx
docker compose exec nginx ls -la /app/staticfiles/

# 4. Vider le cache du navigateur ctrl+R+shift

```

### Si à ce niveau vous n'avez pas un site mis en page avec rendu des fichiers statics, contactez moi directement






## 🎓 Les Bonnes Pratiques Essentielles

### **1. SÉPARATION DES ENVIRONNEMENTS**

```
Local (dev)    → Vous codez et testez
Staging (pré-prod) → Tests avant production (optionnel)
Production     → Les vrais utilisateurs
```

### **2. VARIABLES D'ENVIRONNEMENT**

```env
# .env - JAMAIS dans Git !
DB_PASSWORD=secret_complexe
SECRET_KEY=clé_aléatoire_longue
API_KEY=votre_clé_api
```

```bash
# .gitignore
.env
*.pyc
__pycache__/
```

### **3. BASE DE DONNÉES**

- Utiliser un volume Docker pour la persistance
- Backups automatiques quotidiens
- Mot de passe fort
- Connexion uniquement depuis l'app (pas exposée sur Internet)

### **4. LOGS ET MONITORING**

```bash
# Consulter les logs
docker compose logs -f app
docker compose logs -f nginx

# Monitoring d'uptime (gratuit)
# - UptimeRobot
# - Pingdom
```

### **5. MISES À JOUR**

```bash
# Workflow de mise à jour
git pull                        # Récupérer le nouveau code
docker compose down             # Arrêter
docker compose up -d --build    # Reconstruire et relancer
```

### **6. BACKUPS**

```bash
# Script de backup de la DB (à automatiser avec cron)
#!/bin/bash
docker exec db_container pg_dump -U user dbname > backup_$(date +%Y%m%d).sql
gzip backup_$(date +%Y%m%d).sql
```

---

## 🔥 Architecture Typique en Production

```
Internet
   ↓
[DNS] monapp.com → 195.123.45.67
   ↓
[VPS - Ubuntu Linux]
   ↓
[Firewall UFW] ports 22, 80, 443 ouverts
   ↓
[Docker Engine]
   ├── [Nginx Container] port 80, 443
   │       ↓ reverse proxy
   ├── [App Container] port 8000 (non exposé)
   │       ↓
   └── [PostgreSQL Container] port 5432 (non exposé)
         ↓
   [Docker Volume] données persistantes
```

---

## 📊 Checklist de Déploiement

**Avant le déploiement :**
- [ ] L'app fonctionne en local avec Docker
- [ ] Les secrets sont dans .env (pas dans le code)
- [ ] .env est dans .gitignore
- [ ] Le code est sur Git

**Infrastructure :**
- [ ] Domaine acheté
- [ ] VPS loué
- [ ] DNS configuré
- [ ] SSH sécurisé (clés uniquement)
- [ ] Firewall activé
- [ ] Docker installé

**Déploiement :**
- [ ] Code cloné sur le serveur
- [ ] .env créé avec les bonnes valeurs
- [ ] Containers lancés
- [ ] App accessible en HTTP
- [ ] SSL configuré
- [ ] App accessible en HTTPS

**Post-déploiement :**
- [ ] Backups automatiques configurés
- [ ] Monitoring en place
- [ ] Logs accessibles
- [ ] Procédure de mise à jour testée

---

## 🆘 Les Erreurs Courantes à Éviter

1. **Mot de passe dans le code** → Toujours utiliser .env
2. **Root en production** → Créer un utilisateur dédié
3. **Pas de firewall** → Activer UFW
4. **Pas de HTTPS** → Let's Encrypt est gratuit
5. **Pas de backups** → Automatiser les backups DB
6. **Ports exposés inutilement** → Seul nginx doit être exposé
7. **Tester directement en prod** → Tester en local avec Docker d'abord
8. **Oublier .gitignore** → Les secrets ne doivent jamais être sur Git

---

## 🎯 Résumé en Une Phrase par Étape

1. **Domaine** : J'achète `monapp.com`
2. **VPS** : Je loue un serveur Linux
3. **DNS** : Je pointe `monapp.com` vers l'IP de mon serveur
4. **Sécurité** : Je ferme toutes les portes sauf les essentielles
5. **Docker** : J'emballe mon app dans des containers
6. **Git** : Je clône mon code sur le serveur
7. **Nginx** : Je configure le routage web
8. **SSL** : J'active HTTPS avec Let's Encrypt
9. **Monitoring** : Je surveille que tout tourne
10. **Backups** : Je sauvegarde mes données

**Et voilà ! Votre application est accessible au monde entier ! 🚀**

---

## 📚 Pour Aller Plus Loin

- **CI/CD** : Automatiser le déploiement (GitHub Actions, GitLab CI)
- **Load Balancing** : Plusieurs serveurs pour gérer plus de trafic
- **CDN** : Servir les fichiers statiques plus rapidement
- **Kubernetes** : Orchestration avancée pour grandes apps
- **Serverless** : Vercel, Netlify pour le frontend

---

*Document créé pour comprendre les fondamentaux du déploiement web fullstack*
