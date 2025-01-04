# Darts
## This project relates to HAW Hamburg Module "Bildverabreitung" (WiSe2024/25, Prof. Dr. Hensel) 
## Image-processing-based automated darts scoreboard
## Authors: M. Salewski, L. Grose 
## Created on: 04.01.2025


# Workflow
- Visual Studio 2022 installieren 
- opencv herunterladen (version 4100) und umgebungsvariablen hinzufügen
- github account erstellen 

# GitHub Workflow für Anfänger (Windows + Git Bash)

Dieser Leitfaden beschreibt Schritt für Schritt, wie man mit Git und GitHub auf einem Windows-System mit der Git Bash arbeitet. Er deckt die grundlegenden Arbeitsabläufe von der Repository-Klonung bis zur Arbeit mit Branches ab.

## Voraussetzungen
1. Installiere Git von [https://git-scm.com/](https://git-scm.com/).
2. Erstelle ein GitHub-Konto unter [https://github.com/](https://github.com/).
3. Stelle sicher, dass du Zugriff auf das Repository hast (z. B. durch Einladung oder öffentliche Repositories).

---

## 1. Repository klonen
### Schritt 1: Öffne die Git Bash
- Klicke mit der rechten Maustaste auf den Desktop oder einen beliebigen Ordner und wähle "Git Bash Here".

### Schritt 2: Klone das Repository
- Kopiere die URL des GitHub-Repositories (z. B. https://github.com/username/repo.git).
- Gib in der Git Bash ein:
  ```bash
  git clone https://github.com/username/repo.git
  ```
- Drücke Enter. Das Repository wird in einen neuen Ordner mit dem Namen `repo` heruntergeladen.

---

## 2. Erste Konfiguration
### Schritt 1: Git-Benutzerdaten einstellen
- Stelle sicher, dass dein Name und deine E-Mail-Adresse korrekt konfiguriert sind:
  ```bash
  git config --global user.name "Dein Name"
  git config --global user.email "deine.email@example.com"
  ```
- Überprüfe die Einstellungen mit:
  ```bash
  git config --list
  ```

### Schritt 2: In den Projektordner wechseln
- Wechsle in den Ordner des geklonten Repositories:
  ```bash
  cd repo
  ```

---

## 3. Änderungen vornehmen und committen
### Schritt 1: Status überprüfen
- Prüfe, ob Änderungen vorhanden sind:
  ```bash
  git status
  ```

### Schritt 2: Änderungen vornehmen
- Bearbeite oder füge Dateien im Projektordner hinzu.

### Schritt 3: Dateien zum Commit hinzufügen
- Füge die Änderungen zur Staging-Area hinzu:
  ```bash
  git add dateiname
  ```
- Um alle Änderungen hinzuzufügen:
  ```bash
  git add .
  ```

### Schritt 4: Commit erstellen
- Erstelle einen Commit mit einer Nachricht:
  ```bash
  git commit -m "Beschreibe die Änderung hier"
  ```

---

## 4. Änderungen pushen
- Sende deine Commits zum Remote-Repository:
  ```bash
  git push origin main
  ```
- Falls du nach deinen GitHub-Anmeldedaten gefragt wirst, gib deinen Benutzernamen und dein Token ein (statt eines Passworts wird bei GitHub ein [Personal Access Token](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token) verwendet).

---

## 5. Mit Branches arbeiten
### Schritt 1: Neuen Branch erstellen
- Erstelle und wechsle zu einem neuen Branch:
  ```bash
  git checkout -b neuer-branch
  ```

### Schritt 2: Änderungen vornehmen und committen
- Nimm Änderungen vor, füge sie hinzu und committe wie oben beschrieben.

### Schritt 3: Änderungen pushen
- Push den neuen Branch zum Remote-Repository:
  ```bash
  git push origin neuer-branch
  ```

### Schritt 4: Branch wechseln
- Wechsel zurück zum `main` Branch:
  ```bash
  git checkout main
  ```

---

## 6. Änderungen von anderen abrufen
### Schritt 1: Repository aktualisieren
- Hole die neuesten Änderungen vom Remote-Repository:
  ```bash
  git pull origin main
  ```

### Schritt 2: Branches synchronisieren
- Wenn du in einem Branch arbeitest, stelle sicher, dass er aktuell ist:
  ```bash
  git pull origin branch-name
  ```

---

## 7. Konflikte lösen
- Wenn Git Konflikte meldet:
  1. Öffne die betroffenen Dateien.
  2. Suche nach den Konfliktmarkierungen (`<<<<<<<`, `=======`, `>>>>>>>`).
  3. Bearbeite die Datei, um die Konflikte zu lösen.
  4. Füge die Datei erneut zur Staging-Area hinzu:
     ```bash
     git add dateiname
     ```
  5. Erstelle einen neuen Commit:
     ```bash
     git commit -m "Konflikte gelöst"
     ```

---

## 8. Pull Requests erstellen
- Gehe auf die GitHub-Seite des Repositories.
- Wechsle zum Branch, den du gepusht hast.
- Klicke auf "Compare & pull request".
- Schreibe eine Beschreibung und erstelle den Pull Request.

---

Mit diesem Workflow kannst du effektiv mit Git und GitHub arbeiten. Viel Erfolg!
