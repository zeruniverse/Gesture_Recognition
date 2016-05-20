# Gesture_Recognition  
[![Build status](https://ci.appveyor.com/api/projects/status/ixkg1f06acmxdd26?svg=true)](https://ci.appveyor.com/project/zeruniverse/gesture-recognition)  
Recognize Rock, Scissors and Paper from video captured by camera. And play the game rock-paper-scissors with your computer!  
Course Project FOR SFU COMPUTER VISION  
  
##DEMO  
[YouTube Video](https://www.youtube.com/watch?v=2Sp7oNF_qP0)  
  
<img width="622" alt="cc1" src="https://cloud.githubusercontent.com/assets/4648756/10240677/f30f9d5e-6893-11e5-937f-f82abc74b322.PNG">
<img width="619" alt="cc2" src="https://cloud.githubusercontent.com/assets/4648756/10240698/41de56f0-6894-11e5-81b4-6ccf11856239.PNG">
<img width="619" alt="cc3" src="https://cloud.githubusercontent.com/assets/4648756/10240699/479e199a-6894-11e5-8fc8-4e99aaac580a.PNG">  
  
##Download  
You don't need to build the project yourself if you only want to try it. Download the binary EXE from one of the following links:    
+ [Stable Release](https://github.com/zeruniverse/Gesture_Recognition/releases/download/V1.3/Gesture_Recognition.exe)
    
The binary above is built with the assuption that the computer has at least one webcam and the front webcam is the default one. If you want to change this setting, you need to modify function `CGesture_RecognitionDlg::OnCreate` in file `Gesture_Recognition/Gesture_RecognitionDlg.cpp`  
  
##How to use  
+ Start the program. Make yourself in the video stream captured by the camera.  
+ Don't put your hand in the scene for now. Click the button 'Set Background'. This will make the program take a snapshot of the current scene and it will be used in the future for background removal.  
+ Put your hand in the scene, test if you can see your hand and if your gesture is detected.  
+ Start game by clicking 'Start Game' button.   
+ Reset score board by clicking 'Reset Score Board' button.    
+ If you see the program detects something in the background on the detection window, you might need to set background again.   
    
##Build Environment  
+ [OpenCV 2.4.11](https://github.com/zeruniverse/Gesture_Recognition/releases/download/V1.0/opencv.zip)  
+ MFC  
+ Microsoft Visual Studio 2013  
   
##Build Steps  
+ Download `OpenCV 2.4.11` from the link above, unzip all files to `C:\opencv` (If you don't want to unzip files into `C:\opencv`, you need to edit `/Gesture_Recognition/Gesture_Recognition.vcxproj` with some plain text editor)    
+ Build the project with MS VS2013  
+ Set platform `WIN32(x86)` and use `Release` configuration  
+ Please use OpenCV with version 2.4.11 exactly  
+ If you want to configure project dependency yourself, use `staticlib`  
  
##Dependency  
Please refer to the file `Gesture_Recognition/Gesture_Recognition.vcxproj`  
  
##Implementation Overview  
###Face Detection  
Haar-like features (algorithm implemented in OpenCV).  
###Skin Color Modeling  
Use face color to build skin color model   
###Apply Skin Color Detection  
Detect hand region and remove face  
###Background Removing  
Remove background (noise)  
###Find Largest Contour  
This makes the shape of the hand.  
###Get Convex Hull and Convexity Defects of Hand  
###Find Palm Center  
Will be used to determine Fingertips
###Find Convexity Defects Representing Gaps Between Fingers
Convexity Defects is feature 1. 
###Find Fingertips  
Number of fingertips is feature 2.  
###Build Decision Tree  
With feature 1 & 2  
###Use History to Avoid Noise  
Use a queue to record history  
###Game and UI Design  
Use MFC to implement the UI and randomize the gesture of the computer  
  
##Reference  
1.	Hand tracking and gesture recognition system for human-computer interaction using low-cost hardware, Hui-Shyong Yeo & Byung-Gook Lee & Hyotaek Lim, Dongseo University, Multimedia Tools Application, May 2013  
2.	Visual Processing of Rock, Scissors, Paper Game for Human Robot Interaction, Ho-sub Yoon & Su-Young Chi, Division of Intellgent Robot, SICE-ICASE International Joint Conference 2006  
3.	Chai D, Ngan KN (1999) Face segmentation using skin-color map in videophone applications. IEEE Trans Circ Syst Video Technol 9(4):551-564   
4.	Real-Time Palm Tracking and Hand Gesture Estimation Based on Fore-Arm Contour, Weishao Chen, National Taiwan University of Science and Technology, July 2011.  
5.	Vision-Based Finger Action Recognition by Angle Detection and Contour Analysis, Daeho Lee & SeungGwan Lee, ETRI Journal, vol. 33, no. 3, June. 2011, pp. 415-422.  
6.	Vision based gesture recognition system with single camera, Wei Du & Hua Li, CAD Lab, Institute of Computing Technology, Chinese Academy of Science, Signal Processing Proceedings, 2000. WCCC-ICSP 2000. 5th International Conference on Volume:2  
  
##License  
GNU GPL 3.0  
**You must give appropriate credit (link to this repo) if you use (part of) this project IN YOUR WORK. Appropriate copyright info is also required by GNU GPL 3.0**
