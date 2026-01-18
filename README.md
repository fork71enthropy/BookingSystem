# Principal Title (H1)
## Subtitle (H2)
### second Subtitle (H3)

**bold text**
*Italic text*

- Liste à puces
- Second element
  - indented sub element

1. ordered list
2. second element

[Google link](https://google.com)

![Image Describtion](url-de-image.jpg)

`code inline`
```python
# Bloc of code
def hello():
    print("Hello")
```

3. Commands for starting a new django project
`code inline`
```bash
#Bloc of code
mkdir directory_name
python3 -m venv virtual_env_name
source virtual_env_name/bin/activate
pip3 install django
python3 -m django version
django-admin startproject project_name
cd project_name
python3 manage.py runserver #test if you have the website available on localhost

#next step is to push this version
git init
git add .
git commit -m "first commit"
git branch -M main
git remote add origin https://github.com/fork71enthropy/BookingSystem.git
git push -u origin main
```
4. Commands for packages installation for api development and app creation
`code inline`
```bash
#Bloc of bash code
python3 manage.py startapp LittleLemonAPI
#REST best practices : KISS,filter && order && paginate,versionning,caching,rate limiting,monitoring (status code and 
#network bandwith)
#Security and authentication in rest api : 
#SSL(for https),signed urls(via hmac),password based authentication < token based authentication (via json web token (JWT))
#http codes during authentication process(401,403),Cross-Origin Ressource Sharing(CORS policy and firewalls),
#firewall application on your server

#Access control: Roles(Collection of privileges) and privileges(authority to do a task)






















