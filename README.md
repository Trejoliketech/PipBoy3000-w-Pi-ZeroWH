## Project Overview
* Modularity is a topic of discussion that holds considerable weight within the realm of electronics and technology. Modularity refers to the breaking down of a system into individual, swappable modules that continue to work as one single system. Tech giants like Apple and Samsung tend to house their platforms within closed "ecosystems" that operate best with their propietary architecture and systems, 

## System/Project Scope
* The project includes an accurate internal clock synced through NTP to atomic clock servers, temperature and humidity readings from an OpenWeatherMap API key, load balancing between the ESP32 and a Raspberry Pi Zero 2. The Pi Zero 2 sends time and weather information to the ESP32 for displaying onto the UI. 
* This project will not deliver a watch that completely functionally replaces a traditional Apple or Samsung watch. Due to propietary architecture, it is not entirely possible to include deep IoS and Samsung integration as it is "walled" to work within their ecosystem and not with third party manufacturers. Instead, this project aims to demonstrate concepts of modularity, custom system design, and system communications with considerable room for improvement.

## Architecture/Workflow Overview
* The pipboy case is 3D printed, utilizing [SurvivorGrim's](https://github.com/SurvivorGrim/PipBoy-Pi5/tree/main) STL files. Since these files are designed specifically for a Raspberry Pi 5, it would be a great avenue to expand this projects overall strucutural integrity by modifying the files to better fit the components.
* The system is designed around a split architecture: the ESP32 handles the user interface and controls while the Pi 0 Two handles retrieval of live data which is then communicated to the ESP32 over UART.

### Raspberry Pi Zero 2 (Backend)
* Connects to Wi-Fi
* Syncs time using NTP
* Fetches weather data from external APIs
* Sends processed data to the ESP32 via UART
### ESP32 (Frontend / UI Controller)
* Handles user input (rotary encoder, buttons)
* Renders the graphical interface on the TFT display
* Receives and parses data from the Pi
* Updates the UI accordingly

This separation allows each component to focus on specific responsibilities, improving performance and simplifying debugging.
* The parts list can be found below:
1. [Aokin ESP32](https://www.amazon.com/Aokin-ESP-WROOM-32-Development-Bluetooth-Microcontroller/dp/B08NW77465?th=1) 
2. [Raspberry Pi 0 Two](https://www.amazon.com/Zero-Pre-Soldered-Color-Coded-Quad-Core-Bluetooth/dp/B0DS68NPGF/ref=sr_1_2?crid=19AGDV51G78GR&dib=eyJ2IjoiMSJ9.YW8mQbgF1VmhkLsXpN_K-hgvqO0EXNef2XYx1c0g7lm1JSjDhY8TZ7qZVbrjDFT3d7mP05ouV-JFk01jroHZWnXfvzqw3HnuqAGmHnavgojX6UJZdWC7q2_rx3HyjMstsEjvqBGlcM7cUTmw_zRQdJlF6xB7ImhMYT_77iyU9n3eowaSSYuYtyuI_IIaJL_qBEU3ZvXjljpE_mT0uRhiI_ZN09ao_F4j8bAeTlS_sLw-Kjt0-ILZJ1gOkt_QZy5shYXKHehLVI6EvY1emR7v5ViUusYKPQ2sFmtvR1AP9rk.sfu6S2wWBXiqDxjMaXfaKyysvgikIG0aIhJsGn0Xf-w&dib_tag=se&keywords=pi+zero+2+w&qid=1777406392&s=electronics&sprefix=pi+%2Celectronics%2C133&sr=1-2)
3. [TP4057 Charging Board](https://www.amazon.com/dp/B0CDWZ9MDC?ref=ppx_yo2ov_dt_b_fed_asin_title)
4. [WAGO Connectors](https://www.amazon.com/dp/B06XGYXVXR?ref=ppx_yo2ov_dt_b_fed_asin_title)
5. [Multimeter](https://www.amazon.com/dp/B0BYD32JZV?ref=ppx_yo2ov_dt_b_fed_asin_title)
6. [3.5 in TFT Display](https://www.amazon.com/dp/B0DFWL9M9B?ref=ppx_yo2ov_dt_b_fed_asin_title)

## Setup/Usage Instructions
### Hardware Setup
* For more detailed instructions regarding the construction of the pipboy itself, I heavily encourage checking out [SurvivorGrim's](https://github.com/SurvivorGrim/PipBoy-Pi5/tree/main) repository for instructions and a more indepth instruction list.

* Connect the ESP32, TFT display, and input devices (rotary encoder/buttons)
* Power the system using a LiPo battery routed through a charging module (TP4057) and a boost converter (MT3608 set to ~5V)
* Connect the Raspberry Pi Zero 2 to the same power rail
* Establish UART communication:
Pi TX → ESP32 RX
Pi RX → ESP32 TX
* Common ground between all components
### Software Setup (Raspberry Pi)
Configure Wi-Fi access
Install required Python libraries (e.g., requests, time, serial)
Set up scripts to:
Sync time via NTP
Fetch weather data from OpenWeatherMap
Send formatted data over UART
### Software Setup (ESP32)
Upload firmware handling:
UART data parsing
UI rendering on the TFT display
Input handling with proper debounce logic
Usage
Power on the device
The Pi retrieves and sends updated data
The ESP32 displays information and allows navigation through menus using the encoder

## Ethical, Security, Risk Considerations
* The integration of multiple hardware components introduces considerations related to system reliability, data handling, and physical safety.
* This project aims to serve as a reminder to the importance of keeping systems recognizable, serviceable, and understandable. Consumers should have a right to repair and service their systems without needing to rely exclusively on manufacturers.

### Security
* API keys (e.g., OpenWeatherMap) must be stored securely on the Raspberry Pi and not exposed in public repositories 
* Network communication should be limited to trusted connections
### System Reliability
* Clear separation between backend (Pi) and frontend (ESP32) reduces the likelihood of total system failure
* UART communication must be validated to prevent corrupted or partial data from affecting the UI
### Power and Hardware Safety
* Proper voltage regulation is critical to prevent damage to components. It's advised to use a multimeter to set the MT3068
* The LiPo battery system must be handled carefully to avoid overheating, overcharging, or short circuits
### Ethical Considerations
* This project emphasizes open design and learning rather than replicating proprietary systems. It avoids bypassing or exploiting restricted ecosystems and instead demonstrates alternative approaches to wearable technology


### Pin Configuration
```
Pi Zero Two:
- RX: GPIO16
- TX: GPIO17

TFT Display
- SCL: GPIO18
- SDA: GPIO23
- RST: GPIO4
- DC: GPIO2
- CS: GPIO15
- SDA-O: GPIO19

Main Menu Encoder:
- ENC-CK: GPIO25
- ENC-DT: GPIO26
- ENC-SW: GPIO27

Sub Menu Encoder:
- ENC-CLK: GPIO21
- ENC-DT: GPIO22
- ENC-SW: GPIO32

Power and Ground wires should be grouped into separate WAGO terminals.
```

## Software Dependencies

- TFT_eSPI library
- AnimatedGIF

## Contributing

Feel free to contribute to this project:
1. Fork the repository
2. Create your feature branch
3. Submit a pull request

## Acknowledgments

- Inspired by Fallout's Pip-Boy 3000
- Built using the Arduino ecosystem for ESP32
- Utilizes various open-source libraries
- Special thanks to the Fallout modding community for inspiration
- Credit to SurvivorGrim! [https://ytec3d.com/pip-boy-3000-mark-iv/](https://github.com/SurvivorGrim/PipBoy-Pi5/tree/main)
- Credit to the original repository from robegamesios! [https://www.youtube.com/@jejelinge5978](https://github.com/robegamesios/PipBoy3000?tab=readme-ov-file#pin-configuration)

## License

This project is licensed under the MIT License - see the LICENSE file for details.
