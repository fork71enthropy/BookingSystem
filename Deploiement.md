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

# Configurer SSH avec clés (sur votre machine locale)
ssh-keygen -t ed25519
ssh-copy-id monuser@IP_DU_SERVEUR

# Désactiver la connexion root + mot de passe
sudo nano /etc/ssh/sshd_config
# PermitRootLogin no
# PasswordAuthentication no

# Configurer le firewall
sudo ufw allow 22/tcp    # SSH
sudo ufw allow 80/tcp    # HTTP
sudo ufw allow 443/tcp   # HTTPS
sudo ufw enable

# Mises à jour
sudo apt update && sudo apt upgrade -y
```

**✅ Checkpoint** : Vous ne pouvez vous connecter qu'avec votre clé SSH

---

### **PHASE 4 : INSTALLATION DE DOCKER**

```bash
# Installer Docker + Docker Compose
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker $USER

# Tester
docker --version
docker compose version
```

**✅ Checkpoint** : `docker run hello-world` fonctionne

---

### **PHASE 5 : DÉPLOIEMENT DE L'APPLICATION**

```bash
# Cloner votre projet
cd ~
mkdir apps && cd apps
git clone https://github.com/vous/votre-projet.git
cd votre-projet

# Créer le fichier .env avec les vraies valeurs
nano .env

# Structure typique :
# - app/ (votre code)
# - nginx/ (config nginx)
# - Dockerfile
# - docker-compose.yml
# - .env

# Lancer l'application
docker compose up -d --build

# Vérifier
docker compose ps
docker compose logs -f
```

**✅ Checkpoint** : Votre app tourne, accessible sur http://IP_DU_SERVEUR

---

### **PHASE 6 : CONFIGURATION NGINX**

```nginx
# nginx/nginx.conf - Configuration initiale (HTTP seulement)

events {
    worker_connections 1024;
}

http {
    server {
        listen 80;
        server_name monapp.com www.monapp.com;
        
        # Pour Let's Encrypt
        location /.well-known/acme-challenge/ {
            root /var/www/certbot;
        }
        
        # Proxy vers votre app
        location / {
            proxy_pass http://app:8000;  # app = nom du service dans docker-compose
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        }
    }
}
```

**✅ Checkpoint** : `http://monapp.com` affiche votre application

---

### **PHASE 7 : CERTIFICAT SSL (HTTPS)**

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
