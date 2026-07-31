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

# Open Challenge Strategy

Open Challenge
For the Open Challenge, we use only color sensors, without any other type of sensor. It works as follows: the robot moves forward and counts each line it passes over, allowing it to determine its location along the course.

The main issue we face is that the counting sometimes fails, causing it to miscount the lines. To compensate, we always program the robot to account for a few extra lines; this ensures that even if it misses a reading during the run, it still reaches the destination.

Another important point is that, upon reaching the roundabout, the robot must recalibrate its color readings, as the ambient lighting can make the colors appear lighter or darker than they actually are.

During practice sessions, we spend most of our time adjusting the lines on the track. This is because if the robot moves backward slightly, it might re-read the same line and miscount; we try to minimize this risk by carefully positioning the lines.









### Structural design

Our robot's design consists of a purchased chassis base, but most of the structure is 3D printed. We chose this minimalist and futuristic style because it makes it look more aesthetically pleasing at first glance. We created various structures to better organize the cables and sensors. Although the robot looks large, it's quite agile and weighs only 1 kg.
Initially, the robot's appearance was different because it originally contained a lidar unit, but we encountered several problems. At first, everything seemed fine; we tested the lidar outside the robot, and it worked perfectly. However, once we installed it, the problems began. The first issue was the lidar's placement. If we put it on top, it read beyond the black walls, so we placed it between the two panels. We knew there would be some blind spots, but at least it wasn't reading more than we wanted. After testing it three times in the field, we realized that the lidar "didn't see" black, but it did see other colors. After much discussion with the program and the robot, we decided that the best course of action was to remove the lidar.
With its removal, we decided to install three laser sensors on the front of the robot. We custom-made the brackets and later positioned them at a specific angle to prevent them from damaging the walls.


# Obstacle Challenge

Para el reto Obstacle Challenge utilizamos los sensores de color, igual que en el Open Challenge. La diferencia principal es que en este caso contamos además con una cámara, que se encarga de detectar los bloques verdes y rojos del recorrido.

Esa cámara la incorporamos al robot después de la prueba de Denia, así que todavía es una incorporación reciente. Se controla con un ESP32, que es el encargado de procesar las imágenes y comunicarse con el resto del robot.

Al ser una novedad y llevar poco tiempo montada, todavía estamos probándola en las prácticas. Esperamos que funcione bastante bien, pero sabemos que probablemente le queden algunos ajustes pendientes, sobre todo en lo que respecta a la detección de los bloques según la iluminación y la distancia.





## ELECTRONICS AND SENSORS










### List of components

* ESP32-DEVKIT-V1
* LM2596
* AMS1117-3.3
* TB6612FNG
* VL53L1X
* TCS34725
* HUSKYLENS V2
* SG90
* DC Motor
* 2S LiPo
* Resistance 4.7k
* Capacitor
* Button


### Sensor list

* VL53L1X
* TCS34725

* Camera
