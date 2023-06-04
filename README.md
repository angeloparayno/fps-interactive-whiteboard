# FPS-Interactive-Whiteboard
### Thesis / Capstone Project (2016) - University of Santo Tomas
#### Led a team of 4, as the main developer of an interactive learning platform on a Windows Environment acquiring 100% accuracy on body movement and color tracking
* FPS Interactive Whiteboard was named after the  proponent's last name: *Fajardo*, *Parayno*, *Sy* (FPS)
* Sub-Project for this could be found here -> [RGB LED Flashlights](../RGB-LED-Flashlights)
---
## Repository Contents
* FPS Interactive Whiteboard Project Folder
  * Source code: [./FPS Interactive Whiteboard/FPS Interactive Whiteboard/**main.cpp**](https://github.com/angeloparayno/FPS-Interactive-Whiteboard/blob/main/FPS%20Interactive%20Whiteboard/FPS%20Interactive%20Whiteboard/main.cpp)
* [FPS Interactive Whiteboard Thesis Paper](FPS-Interactive-Whiteboard-Thesis-Paper.pdf)
* [FPS Interactive Whiteboard Defense Slide Presentation](Thesis-Defense-Slide-Presentation.pdf)
* Thresholding Program folder
  * Additional program used calibrating the correct values of a color
  * Source code : [./ThresholdingProgram/main.cpp](ThresholdingProgram/main.cpp)
---
## Abstract

&emsp; The FPS Interactive White-Board is a system created in C++ using the Kinect motion capture sensor to allow users interaction with a computer environment using the their hand movements as the input for cursor manipulation and the LED pens as the input for action identification. 

&emsp; The main objective of this project is to improve upon the technologies and functionality of the previous capstone project entitled **“Interactive Whiteboard using Raspberry Pi”**, also improve on certain existing functionalities such as a more accurate way of tracking and translating of the users’ hand movements with a faster response time when it comes to color detection for action identification.

&emsp;The proposed system will benefit schools and other learning institutions that can incorporate interactive applications such as drawing applications, video applications and even educational games with their curriculum. The system can allow students a more hands on experience with certain applications as they can interact with those applications using the movement of their hands, this kind of interaction is also not limited with only one user but instead has a maximum capacity of 2 users.

&emsp;The system is recommended to be used in an open space, a typical classroom setting or as specified in the latter parts of the documentation.

---
## Developer's Inside Context (2023)

(work in progress..)

---
## Diagrams

### Overview
![](assets/figure-3-5-overview.png)
### Use Case Diagram
![](assets/figure-4-1-use-case-diagram.png)

---
## Tools, Specifications and Requirements
* IDE : Visual Studio 2010 Express C++
* OS : Windows 8
* Programming language: C++
* Open Source Libraries 
  * Open CV 2.4.11
  * OpenNi 2.2
  * Nite 2.2
* Windows Library: 
  * Touch injection API on windows 8
  * WM_Paint Message
* Drivers Used:
  * Kinect SDK 1.8 (Official Microsoft SDK)
* Hardware:
  * Xbox 360 Kinect
  * Xbox 360 AC Adapter/Power Supply
  * LCD Projector
 
