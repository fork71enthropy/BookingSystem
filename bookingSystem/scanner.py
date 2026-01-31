#!/usr/bin/env python3
"""
Scanner de vulnérabilités web éducatif - Top 10 OWASP
ATTENTION: À utiliser UNIQUEMENT sur vos propres applications ou avec autorisation écrite!
"""

import requests
import argparse
from urllib.parse import urljoin, urlparse
import re
from bs4 import BeautifulSoup
import time
from typing import List, Dict
import warnings
warnings.filterwarnings('ignore', message='Unverified HTTPS request')

class OWASPScanner:
    def __init__(self, target_url: str, verbose: bool = False):
        self.target_url = target_url
        self.verbose = verbose
        self.vulnerabilities = []
        self.session = requests.Session()
        self.session.headers.update({
            'User-Agent': 'OWASP-Educational-Scanner/1.0'
        })
        
    def log(self, message: str, level: str = "INFO"):
        """Affiche les logs si verbose est activé"""
        if self.verbose:
            print(f"[{level}] {message}")
    
    def add_vulnerability(self, vuln_type: str, severity: str, description: str, details: str = ""):
        """Ajoute une vulnérabilité détectée"""
        self.vulnerabilities.append({
            'type': vuln_type,
            'severity': severity,
            'description': description,
            'details': details
        })
        print(f"[{severity}] {vuln_type}: {description}")
    
    # 1. Injection SQL (A03:2021)
    def test_sql_injection(self):
        """Test basique d'injection SQL"""
        print("\n[*] Test d'injection SQL...")
        
        # Payloads SQL classiques
        sql_payloads = [
            "' OR '1'='1",
            "' OR '1'='1' --",
            "' OR '1'='1' /*",
            "admin' --",
            "1' AND '1'='2",
            "' UNION SELECT NULL--"
        ]
        
        try:
            response = self.session.get(self.target_url, verify=False, timeout=10)
            
            # Recherche de formulaires
            soup = BeautifulSoup(response.text, 'html.parser')
            forms = soup.find_all('form')
            
            for form in forms:
                action = form.get('action', '')
                method = form.get('method', 'get').lower()
                inputs = form.find_all('input')
                
                for payload in sql_payloads:
                    data = {}
                    for inp in inputs:
                        name = inp.get('name')
                        if name:
                            data[name] = payload
                    
                    try:
                        url = urljoin(self.target_url, action)
                        if method == 'post':
                            test_response = self.session.post(url, data=data, verify=False, timeout=10)
                        else:
                            test_response = self.session.get(url, params=data, verify=False, timeout=10)
                        
                        # Détection d'erreurs SQL
                        sql_errors = [
                            'sql syntax', 'mysql', 'postgresql', 'oracle',
                            'syntax error', 'unclosed quotation', 'quoted string'
                        ]
                        
                        for error in sql_errors:
                            if error in test_response.text.lower():
                                self.add_vulnerability(
                                    "SQL Injection",
                                    "CRITICAL",
                                    f"Possible injection SQL détectée dans le formulaire {action}",
                                    f"Payload: {payload}"
                                )
                                break
                        
                        time.sleep(0.5)  # Rate limiting
                    except Exception as e:
                        self.log(f"Erreur lors du test SQL: {e}", "ERROR")
                        
        except Exception as e:
            self.log(f"Erreur lors du test d'injection SQL: {e}", "ERROR")
    
    # 2. Broken Authentication (A07:2021)
    def test_weak_authentication(self):
        """Test d'authentification faible"""
        print("\n[*] Test d'authentification faible...")
        
        try:
            response = self.session.get(self.target_url, verify=False, timeout=10)
            
            # Vérification de l'absence de HTTPS
            if urlparse(self.target_url).scheme == 'http':
                self.add_vulnerability(
                    "Broken Authentication",
                    "HIGH",
                    "Le site n'utilise pas HTTPS - les credentials peuvent être interceptés"
                )
            
            # Vérification des cookies de session
            cookies = response.cookies
            for cookie in cookies:
                if not cookie.secure:
                    self.add_vulnerability(
                        "Broken Authentication",
                        "MEDIUM",
                        f"Cookie '{cookie.name}' sans flag Secure",
                        "Les cookies devraient avoir le flag Secure en HTTPS"
                    )
                if not cookie.has_nonstandard_attr('HttpOnly'):
                    self.add_vulnerability(
                        "Broken Authentication",
                        "MEDIUM",
                        f"Cookie '{cookie.name}' sans flag HttpOnly",
                        "Vulnérable aux attaques XSS"
                    )
                    
        except Exception as e:
            self.log(f"Erreur lors du test d'authentification: {e}", "ERROR")
    
    # 3. Sensitive Data Exposure (A02:2021)
    def test_sensitive_data_exposure(self):
        """Test d'exposition de données sensibles"""
        print("\n[*] Test d'exposition de données sensibles...")
        
        sensitive_paths = [
            '/.git/config',
            '/.env',
            '/config.php',
            '/web.config',
            '/backup.sql',
            '/database.sql',
            '/.DS_Store',
            '/phpinfo.php',
            '/admin',
            '/backup',
            '/config'
        ]
        
        try:
            for path in sensitive_paths:
                url = urljoin(self.target_url, path)
                try:
                    response = self.session.get(url, verify=False, timeout=5)
                    if response.status_code == 200:
                        self.add_vulnerability(
                            "Sensitive Data Exposure",
                            "HIGH",
                            f"Fichier/dossier sensible accessible: {path}",
                            f"Status code: {response.status_code}"
                        )
                    time.sleep(0.3)
                except:
                    pass
                    
        except Exception as e:
            self.log(f"Erreur lors du test d'exposition de données: {e}", "ERROR")
    
    # 4. XSS - Cross-Site Scripting (A03:2021)
    def test_xss(self):
        """Test basique de XSS"""
        print("\n[*] Test de Cross-Site Scripting (XSS)...")
        
        xss_payloads = [
            "<script>alert('XSS')</script>",
            "<img src=x onerror=alert('XSS')>",
            "<svg/onload=alert('XSS')>",
            "javascript:alert('XSS')"
        ]
        
        try:
            response = self.session.get(self.target_url, verify=False, timeout=10)
            soup = BeautifulSoup(response.text, 'html.parser')
            forms = soup.find_all('form')
            
            for form in forms:
                action = form.get('action', '')
                method = form.get('method', 'get').lower()
                inputs = form.find_all('input')
                
                for payload in xss_payloads:
                    data = {}
                    for inp in inputs:
                        name = inp.get('name')
                        if name and inp.get('type') != 'hidden':
                            data[name] = payload
                    
                    try:
                        url = urljoin(self.target_url, action)
                        if method == 'post':
                            test_response = self.session.post(url, data=data, verify=False, timeout=10)
                        else:
                            test_response = self.session.get(url, params=data, verify=False, timeout=10)
                        
                        # Vérification si le payload est reflété sans encodage
                        if payload in test_response.text:
                            self.add_vulnerability(
                                "Cross-Site Scripting (XSS)",
                                "HIGH",
                                f"Possible XSS reflété détecté dans {action}",
                                f"Payload reflété: {payload[:50]}"
                            )
                            break
                        
                        time.sleep(0.5)
                    except Exception as e:
                        self.log(f"Erreur lors du test XSS: {e}", "ERROR")
                        
        except Exception as e:
            self.log(f"Erreur lors du test XSS: {e}", "ERROR")
    
    # 5. Security Misconfiguration (A05:2021)
    def test_security_misconfiguration(self):
        """Test de mauvaise configuration de sécurité"""
        print("\n[*] Test de mauvaise configuration de sécurité...")
        
        try:
            response = self.session.get(self.target_url, verify=False, timeout=10)
            
            # Vérification des headers de sécurité
            security_headers = {
                'X-Frame-Options': 'Protection contre le clickjacking',
                'X-Content-Type-Options': 'Protection contre le MIME sniffing',
                'Strict-Transport-Security': 'Force HTTPS',
                'Content-Security-Policy': 'Protection XSS',
                'X-XSS-Protection': 'Protection XSS (legacy)'
            }
            
            for header, description in security_headers.items():
                if header not in response.headers:
                    self.add_vulnerability(
                        "Security Misconfiguration",
                        "MEDIUM",
                        f"Header de sécurité manquant: {header}",
                        description
                    )
            
            # Vérification du Server header
            if 'Server' in response.headers:
                server = response.headers['Server']
                if any(tech in server.lower() for tech in ['apache', 'nginx', 'iis']):
                    self.add_vulnerability(
                        "Security Misconfiguration",
                        "LOW",
                        "Le header Server révèle la technologie utilisée",
                        f"Server: {server}"
                    )
                    
        except Exception as e:
            self.log(f"Erreur lors du test de configuration: {e}", "ERROR")
    
    # 6. Identification and Authentication Failures
    def test_directory_listing(self):
        """Test de directory listing"""
        print("\n[*] Test de directory listing...")
        
        test_paths = ['/', '/images/', '/js/', '/css/', '/uploads/']
        
        try:
            for path in test_paths:
                url = urljoin(self.target_url, path)
                try:
                    response = self.session.get(url, verify=False, timeout=5)
                    
                    # Indices de directory listing
                    if any(indicator in response.text.lower() for indicator in 
                          ['index of', 'parent directory', 'directory listing']):
                        self.add_vulnerability(
                            "Security Misconfiguration",
                            "MEDIUM",
                            f"Directory listing activé: {path}",
                            "Les attaquants peuvent voir la structure des fichiers"
                        )
                    time.sleep(0.3)
                except:
                    pass
                    
        except Exception as e:
            self.log(f"Erreur lors du test de directory listing: {e}", "ERROR")
    
    def generate_report(self):
        """Génère un rapport des vulnérabilités trouvées"""
        print("\n" + "="*60)
        print("RAPPORT DE SCAN - TOP 10 OWASP")
        print("="*60)
        print(f"Cible: {self.target_url}")
        print(f"Date: {time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"\nNombre total de vulnérabilités: {len(self.vulnerabilities)}")
        
        # Comptage par sévérité
        severity_count = {'CRITICAL': 0, 'HIGH': 0, 'MEDIUM': 0, 'LOW': 0}
        for vuln in self.vulnerabilities:
            severity_count[vuln['severity']] += 1
        
        print("\nRépartition par sévérité:")
        for severity, count in severity_count.items():
            if count > 0:
                print(f"  {severity}: {count}")
        
        print("\n" + "="*60)
        print("DÉTAILS DES VULNÉRABILITÉS")
        print("="*60)
        
        if not self.vulnerabilities:
            print("\nAucune vulnérabilité détectée (ou cible non accessible)")
        else:
            for i, vuln in enumerate(self.vulnerabilities, 1):
                print(f"\n[{i}] {vuln['type']}")
                print(f"Sévérité: {vuln['severity']}")
                print(f"Description: {vuln['description']}")
                if vuln['details']:
                    print(f"Détails: {vuln['details']}")
        
        print("\n" + "="*60)
        print("RECOMMANDATIONS OWASP")
        print("="*60)
        print("""
1. SQL Injection: Utilisez des requêtes préparées (prepared statements)
2. Broken Auth: Implémentez MFA, sessions sécurisées, HTTPS
3. Sensitive Data: Chiffrez les données au repos et en transit
4. XSS: Encodez/échappez toutes les entrées utilisateur
5. Security Misc: Configurez correctement les headers de sécurité
6. Directory Listing: Désactivez le listing dans la config serveur

Pour plus d'informations: https://owasp.org/Top10/
        """)

def main():
    banner = """
    ╔═══════════════════════════════════════════════╗
    ║   OWASP Top 10 Educational Scanner v1.0       ║
    ║   ⚠️  USAGE ÉDUCATIF UNIQUEMENT ⚠️            ║
    ╚═══════════════════════════════════════════════╝
    """
    print(banner)
    
    parser = argparse.ArgumentParser(
        description='Scanner de vulnérabilités web éducatif - Top 10 OWASP'
    )
    parser.add_argument('url', help='URL cible (ex: http://localhost:8080)')
    parser.add_argument('-v', '--verbose', action='store_true', 
                       help='Mode verbeux')
    parser.add_argument('--all', action='store_true', 
                       help='Exécuter tous les tests')
    parser.add_argument('--sql', action='store_true', 
                       help='Test d\'injection SQL')
    parser.add_argument('--xss', action='store_true', 
                       help='Test XSS')
    parser.add_argument('--auth', action='store_true', 
                       help='Test d\'authentification')
    parser.add_argument('--config', action='store_true', 
                       help='Test de configuration')
    parser.add_argument('--sensitive', action='store_true', 
                       help='Test d\'exposition de données')
    
    args = parser.parse_args()
    
    # Avertissement légal
    print("\n⚠️  AVERTISSEMENT LÉGAL ⚠️")
    print("Cet outil doit être utilisé UNIQUEMENT sur:")
    print("  - Vos propres applications")
    print("  - Des environnements de test (DVWA, WebGoat, etc.)")
    print("  - Avec une autorisation écrite explicite")
    print("\nTester des applications sans permission est ILLÉGAL!")
    
    confirmation = input("\nConfirmez-vous avoir l'autorisation de tester cette URL? (oui/non): ")
    if confirmation.lower() not in ['oui', 'yes', 'o', 'y']:
        print("Scan annulé.")
        return
    
    scanner = OWASPScanner(args.url, args.verbose)
    
    print(f"\n[*] Début du scan de {args.url}...")
    
    try:
        # Exécution des tests
        if args.all or not any([args.sql, args.xss, args.auth, args.config, args.sensitive]):
            scanner.test_sql_injection()
            scanner.test_xss()
            scanner.test_weak_authentication()
            scanner.test_security_misconfiguration()
            scanner.test_sensitive_data_exposure()
            scanner.test_directory_listing()
        else:
            if args.sql:
                scanner.test_sql_injection()
            if args.xss:
                scanner.test_xss()
            if args.auth:
                scanner.test_weak_authentication()
            if args.config:
                scanner.test_security_misconfiguration()
            if args.sensitive:
                scanner.test_sensitive_data_exposure()
        
        # Génération du rapport
        scanner.generate_report()
        
    except KeyboardInterrupt:
        print("\n\n[!] Scan interrompu par l'utilisateur")
    except Exception as e:
        print(f"\n[ERROR] Erreur lors du scan: {e}")

if __name__ == "__main__":
    main()
