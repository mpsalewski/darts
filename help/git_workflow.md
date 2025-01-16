# GitHub Workflow for Beginners (Windows + Git Bash)

This guide explains step-by-step how to use Git and GitHub on a Windows system with Git Bash. It covers the basic workflow from cloning a repository to working with branches.

## Prerequisites
1. Install Git from [https://git-scm.com/](https://git-scm.com/).
2. Create a GitHub account at [https://github.com/](https://github.com/).
3. Ensure you have access to the repository (e.g., via invitation or public repositories).

---

## 1. Cloning a Repository
### Step 1: Open Git Bash
- Right-click on your desktop or in any folder and select **"Git Bash Here"**.

### Step 2: Clone the Repository
- Copy the URL of the GitHub repository (e.g., `https://github.com/username/repo.git`).
- In Git Bash, run the following:
  ```bash
  git clone https://github.com/username/repo.git
  ```
- Press Enter. The repository will be downloaded into a new folder named `repo`.

---

## 2. Initial Configuration
### Step 1: Configure Git User Information
- Ensure your name and email address are correctly configured:
  ```bash
  git config --global user.name "Your Name"
  git config --global user.email "your.email@example.com"
  ```
- Verify the settings with:
  ```bash
  git config --list
  ```

### Step 2: Navigate to the Project Folder
- Switch to the directory of the cloned repository:
  ```bash
  cd repo
  ```

---

## 3. Making Changes and Committing
### Step 1: Check the Status
- Check for any changes in the repository:
  ```bash
  git status
  ```

### Step 2: Make Changes
- Edit or add files in the project folder.

### Step 3: Stage the Changes
- Add the changes to the staging area:
  ```bash
  git add filename
  ```
- To add all changes:
  ```bash
  git add .
  ```

### Step 4: Commit the Changes
- Create a commit with a message:
  ```bash
  git commit -m "Describe your change here"
  ```

---

## 4. Pull Before Push
Before pushing your changes, ensure you have the latest updates from the remote repository to avoid conflicts:
```bash
git pull origin main
```

---

## 5. Pushing Changes
- Push your commits to the remote repository:
  ```bash
  git push origin main
  ```
- If prompted, enter your GitHub username and token. Note that GitHub requires a [Personal Access Token](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token) instead of a password.

---

## 6. Reviewing Your Changes
### Step 1: View Commit History
- To view a simplified commit history:
  ```bash
  git log --oneline --graph
  ```

### Step 2: Check Differences
- To see the differences between your changes and the latest commit:
  ```bash
  git diff
  ```
  
  
## 7. Handling Merge Conflicts
If conflicts arise when merging branches or pulling changes, resolve them manually:
1. Open the conflicted files and edit them to resolve the conflicts.
2. Stage the resolved files:
   ```bash
   git add conflicted_file
   ```
3. Commit the resolution:
   ```bash
   git commit
   ```

---

## 8. Create `.gitignore`
To prevent specific files or folders from being tracked by Git, create a `.gitignore` file:
```bash
# Example .gitignore
*.log
*.tmp
node_modules/
.env
```
- Add and commit the `.gitignore` file:
  ```bash
  git add .gitignore
  git commit -m "Add .gitignore"
  ```

---

## 9. Check Remote Repositories
To check which remote repositories are connected to your project:
```bash
git remote -v
```

---

# Usefull Git Commands
View current Repository status (changes, staged files,...)
```bash
git status
```

View list of last Commits with Hash, Author, Date and Message 
```bash
git log
```

Short Version of git log 
```bash
git log --oneline
```

Grafic Version of git log 
```bash
git log --oneline --graph
```
 
View Differences 
```bash
git diff
```

Tag a special Commit (e.g. Release-Version)
```bash
git tag -a v1.0 -m "Release version 1.0"
```

---




# Git Workflow: Feature Branch Development and Integration

This guide walks you through creating a feature branch, developing a feature, handling updates in the `main` branch, merging changes, and cleaning up after the process.

```bash
# 1. Create and Switch to a New Feature Branch
git checkout -b new_feature_branch
git checkout new_feature_branch

# 2. Add Your Feature
# Make changes, then stage and commit them
git add .
git commit -m "Implement a new feature in new_feature_branch"

# 3. Handle Possible Changes in the main Branch
# Switch to the feature branch (ensure you're on it)
git checkout new_feature_branch

# Merge the latest changes from the main branch into the feature branch
git merge main

# If there are conflicts, Git will output something like this:
# Auto-merging src/example/example/conflict_file.cpp
# CONFLICT (content): Merge conflict in src/example/example/conflict_file.cpp
# Automatic merge failed; fix conflicts and then commit the result.

# Open conflicted files and resolve the conflicts manually.
# Then, stage the resolved files:
git add src/example/example/conflict_file.cpp

# Finalize the merge
git commit
< git add .; git commit -m "...">

# 4. Push the Feature Branch to Remote
git push origin new_feature_branch

# 5. Merge the Feature into the main Branch
# Switch to the main branch
git checkout main

# Optional: Pull the latest updates from the remote repository
git pull

# Merge the feature branch into the main branch
git merge new_feature_branch

# 6. Optional: Delete the Feature Branch
# If the feature is fully implemented and you do not want to keep it, delete the feature branch.

# Delete the branch locally
git branch -d new_feature_branch

# Delete the branch remotely
git push origin --delete new_feature_branch
```
