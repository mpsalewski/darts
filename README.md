# Automated Dart Detection and Scoring System

This project was developed as part of the Digital Image / Video Processing module at HAW Hamburg under Prof. Dr. Marc Hensel (https://github.com/MarcOnTheMoon,  http://www.haw-hamburg.de/marc-hensel).

## Authors:
- Mika Paul Salewski (<mika.paul.salewski@gmail.com>)
- Lukas Grose (<lukas@grose.de>)

## Created On:
- 2025-01-04

## Last Revision:
- 2025-01-16

## Copyright (c) 2025:
- Mika Paul Salewski, Lukas Grose

## License:
- CC BY-NC-SA 4.0
- See: [https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en](https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en)

---

## Further Information

This project is divided into three main components (Threads):
- **Dart Scoreboard**
- **Image Processing**
- **Command Line Interface (CLI)**

### 1. **Dart Scoreboard**
The scoreboard is a simple frame that automatically updates after three darts are thrown. It can also be controlled via the command line. The number of players is configurable, and both legs and sets are tracked. Additionally, the logic for calculating the **precise double checkout** in darts is implemented.

### 2. **Image Processing**
The image processing component utilizes **differential images** and applies a series of steps to detect the barrel of the dart. A straight line is then drawn through the dart's tip. By using three camera perspectives, an optimal intersection point is identified. This point can then be converted into a sector on the dartboard through **angle and distance calculation**.

### 3. **Command Line Interface (CLI)**
The command line interface features a command parser that enables the registration of custom commands and links them to specific functions within the system. This allows for flexible control and interaction with the system, including manual updates and adjustments to gameplay or scoring.

### 4. **Design Pattern and Structure**
This project is implemented in **C++** (with Visual Studio 2022) with the use of **OpenCV** for image processing. The code follows an object-oriented C-style approach, where each module (e.g., image processing) is realized using a **`*.c/*.cpp`** and **`.h`** file. The **`*.c/*.cpp`** file contains a **static structure** with the module's variables, while the functions act as **member functions** (similar to object-oriented methods).

### 5. **Thread Safety**
To ensure the project operates in a multi-threaded environment, **mutexes** are used to make the system **thread-safe**. This allows different threads to access shared resources without causing data races or inconsistencies.

### 6. **Challenges and Limitations**

It is important to note that due to numerous variables such as **light conditions**, **dart position on the board**, **camera distance**, **camera fisheye distortion**, **obstruction by other darts**, **black darts on a black background**, and **dart patterns**, ... the problem is **non-trivial** and hence prone to errors. However, a dart that is positioned centrally in a single sector is very likely to be detected correctly. Despite these challenges, the system is also able to accurately detect **triple and double sectors**, as well as **close calls**.

Currently, the system is not yet robust due to the **differential image** approach, which can cause issues when the previous dart's position is altered by the subsequent throw. This leads to inconsistencies in the detection when darts are thrown in quick succession.

---

The combination of these components provides an **automated dart detection and scoring system** that enhances the accuracy and ease of tracking gameplay.

