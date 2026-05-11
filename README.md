# WRO 2026 Vila-Stem 8 repository
**This is our repository for the 2026 season of the WRO Future Engineers Challenge**




## TABLE OF CONTENTS

* [Hardware](#hardware)
    * [Car movement](#car-movement)
    * [Structural design](#structural-design)
 
* [Electronics and sensors](#electronics-and-sensors)
    * [List of components](#list-of-components)
    * [Sensor list](#sensor-list)
    * [Power Management](#power-management)
    * [PCB](#pcb)
   
* [Strategy and operation of the code](#strategy-and-operation-of-the-code)
    * [Slave code](#how-the-slave-code-works)
    * [Location of the robot](#location-of-the-robot-on-the-board)
    * [Open Challenge Strategy](#open-challenge-strategy)
    * [Obstacle Challenge Strategy](#obstacle-challenge-strategy)

* [V-photos](#V-photos)

* [T-photos](#T-photos)

* [Demostration videos](#demonstration-videos)

* [License](#license)

* [Contact and Social Media](#contact-and-social-media)

## HARDWARE



### Car movement











### Structural design

Our robot's design consists of a purchased chassis base, but most of the structure is 3D printed. We chose this minimalist and futuristic style because it makes it look more aesthetically pleasing at first glance. We created various structures to better organize the cables and sensors. Although the robot looks large, it's quite agile and weighs only 1 kg.
Initially, the robot's appearance was different because it originally contained a lidar unit, but we encountered several problems. At first, everything seemed fine; we tested the lidar outside the robot, and it worked perfectly. However, once we installed it, the problems began. The first issue was the lidar's placement. If we put it on top, it read beyond the black walls, so we placed it between the two panels. We knew there would be some blind spots, but at least it wasn't reading more than we wanted. After testing it three times in the field, we realized that the lidar "didn't see" black, but it did see other colors. After much discussion with the program and the robot, we decided that the best course of action was to remove the lidar.
With its removal, we decided to install three laser sensors on the front of the robot. We custom-made the brackets and later positioned them at a specific angle to prevent them from damaging the walls.








## ELECTRONICS AND SENSORS

ESP32








### List of components

* HuskyLens 2 (DFRobot)
* Battery 20.999mAh
* Chassis Kit
* 3D printed Chassis


### Sensor list

* Camera
