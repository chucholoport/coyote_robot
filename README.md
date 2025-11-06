# 🐺 Coyote Robot — Motor Test Package

Este paquete forma parte del proyecto **Coyote Robot**, una versión educativa del JetBot que utiliza **Arduino UNO**, un **puente H L298N** y un **motor JGB37-520** con encoder.  
El objetivo de este módulo es **controlar una sola rueda** desde ROS para entender el flujo básico entre **ROS ↔ Arduino** antes de integrar el LiDAR A1M8 y la cámara.

---

## Contenido del paquete

```bash
coyote_robot/
├── arduino/
│   └── motor_test/
│       └── motor_test.ino # Código Arduino con ROSSerial
├── scripts/
│ └── motor_test.py # Nodo ROS en Python para control del motor
├── launch/
│ └── motor_test.launch # Archivo launch para ejecutar el nodo
├── CMakeLists.txt
└── package.xml
```

---

## Dependencias

Asegúrate de tener instalados los siguientes paquetes ROS en tu sistema:

```bash
sudo apt update
sudo apt install ros-melodic-rosserial-arduino ros-melodic-rosserial-python
sudo apt install ros-melodic-geometry-msgs ros-melodic-std-msgs
```

También necesitas tener **Arduino IDE** instalado y configurado para usar `ros_lib`:

```bash
# Copiar la librería ros_lib a Arduino
cd ~/Arduino/libraries
rosrun rosserial_arduino make_libraries.py .
```

---

## Clonación y compilación

Clona el repositorio directamente en tu espacio de trabajo de catkin:

```bash
cd ~/catkin_ws/src
git clone https://github.com/chucholoport/coyote_robot.git
cd ~/catkin_ws
catkin_make
source devel/setup.bash
```
---

## Carga del código en Arduino

Conecta tu **Arduino UNO** al puerto USB y abre el sketch:

```bash
cd ~/catkin_ws/src/coyote_robot/arduino/motor_test
arduino motor_test.ino
```

Selecciona:

- **Placa:** Arduino UNO
- **Puerto:** el que corresponda (ej. `/dev/ttyACM0` o `/dev/ttyUSB0`)
- Luego **Sube** el sketch.

---

## Conexiones eléctricas

### Tabla de conexiones Arduino ↔ L298N ↔ Motor JGB37-520

| Componente          | Pin | Conectar a  | Descripción                             |
| ------------------- | --- | ----------- | --------------------------------------- |
| **L298N**           | IN1 | D8          | Dirección motor A (sentido horario)     |
| **L298N**           | IN2 | D9          | Dirección motor A (sentido antihorario) |
| **L298N**           | ENA | D5          | PWM de velocidad (usa `analogWrite`)    |
| **L298N**           | 12V | Fuente 12 V | Alimentación del motor                  |
| **L298N**           | GND | GND Arduino | Tierra común                            |
| **Motor JGB37-520** | M1  | OUT1 L298N  | Terminal del motor                      |
| **Motor JGB37-520** | M2  | OUT2 L298N  | Terminal del motor                      |
| **Encoder**         | C1  | D2          | Canal A del encoder (interrupción 0)    |
| **Encoder**         | C2  | D3          | Canal B del encoder (interrupción 1)    |
| **Encoder**         | VCC | 5V Arduino  | Alimentación encoder                    |
| **Encoder**         | GND | GND Arduino | Tierra encoder                          |

> **⚠️ Importante:** si usas una fuente de laboratorio o eliminador, asegúrate de que el GND esté compartido con el Arduino.

---

## Ejecución en ROS

1. **Sube el código Arduino** (`motor_test.ino`)

2. **Lanza el nodo ROS** desde tu workspace:

    ```bash
    roslaunch coyote_robot motor_test.launch
    ```

    Deberias ver algo similar a esto:
    ```bash
    jetbot@ubuntu:~/catkin_ws/src/coyote_robot$ roslaunch coyote_robot motor_test.launch
    ... logging to /home/jetbot/.ros/log/c7f49a10-bb36-11f0-aad8-000c2924d42f/roslaunch-ubuntu-5028.log
    Checking log directory for disk usage. This may take a while.
    Press Ctrl-C to interrupt
    Done checking log file disk usage. Usage is <1GB.

    started roslaunch server http://ubuntu:36055/

    SUMMARY
    ========

    PARAMETERS
    * /rosdistro: melodic
    * /rosversion: 1.14.13

    NODES
    /
        coyote_arduino_bridge (rosserial_python/serial_node.py)
        coyote_motor_test (coyote_robot/motor_test.py)

    ROS_MASTER_URI=http://ubuntu:11311

    process[coyote_arduino_bridge-1]: started with pid [5043]
    process[coyote_motor_test-2]: started with pid [5044]
    [INFO] [1762450758.837378]: ROS Serial Python Node
    [INFO] [1762450758.855112]: Connecting to /dev/ttyACM0 at 57600 baud
    [INFO] [1762450758.905416]: Coyote Motor Test Node started.
    [INFO] [1762450758.910242]: Use keyboard input to control motor speed.
    Enter speed (-1.0 to 1.0) or q to quit: 1
    [INFO] [1762450759.798873]: Command sent: linear.x = 1.00
    Enter speed (-1.0 to 1.0) or q to quit: [INFO] [1762450760.976748]: Requesting topics...
    [ERROR] [1762450775.982514]: Unable to sync with device; possible link problem or link software version mismatch such as hydro rosserial_python with groovy Arduino
    [INFO] [1762450775.987437]: Requesting topics...
    [ERROR] [1762450790.995956]: Unable to sync with device; possible link problem or link software version mismatch such as hydro rosserial_python with groovy Arduino
    [INFO] [1762450791.001341]: Requesting topics...
    [ERROR] [1762450806.005873]: Unable to sync with device; possible link problem or link software version mismatch such as hydro rosserial_python with groovy Arduino
    [INFO] [1762450806.010288]: Requesting topics...
    [INFO] [1762450806.046172]: Note: publish buffer size is 280 bytes
    [INFO] [1762450806.050837]: Setup publisher on wheel_rpm [std_msgs/Float32]
    [INFO] [1762450806.163709]: Note: subscribe buffer size is 280 bytes
    [INFO] [1762450806.173286]: Setup subscriber on cmd_vel [geometry_msgs/Twist]
    [INFO] [1762450806.633804]: Measured RPM: 0.00
    [INFO] [1762450807.637277]: Measured RPM: 0.00
    ```

3. En el entorno de **ROS Melodic (Python 2.7)**, instala el paquete `teleop_twist_keyboard`:

    ```bash
    sudo apt install ros-melodic-teleop-twist-keyboard
    ```

4. En otra terminal, ejecuta el nodo de teleoperación:

    ```bash
    rosrun teleop_twist_keyboard teleop_twist_keyboard.py
    ```

    Aparecerán las instrucciones de control:

    ```bash
    Reading from the keyboard  and Publishing to Twist!
    ---------------------------
    Moving around:
            i    
    j    k    l
            ,    

    i: avanzar hacia adelante
    ,: retroceder
    j/l: girar (izquierda/derecha)
    k: detener movimiento
    ```

5. Puedes observar los mensajes que se están publicando hacia el Arduino con:
    
    ```bash
    rostopic echo /cmd_vel
    ```

    Y comprobar el feedback de velocidad publicado por el Arduino:
    
    ```bash
    rostopic echo /wheel_rpm
    ```