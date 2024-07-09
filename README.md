# Lockout Buzzer System

This project is a Jeopardy-style buzzer system using ESP8266 and nRF24L01 modules. It allows multiple teams to buzz in and locks out other buzzers once a team has buzzed in.

## Features
- Multiple team support with unique buzzer colors
- Lockout functionality to prevent multiple buzz-ins
- Clear round functionality to reset the system for the next round
- LED indicators for visual feedback
- Serial monitor output for debugging

## Hardware Components
- ESP8266 (NodeMCU) boards
- nRF24L01 modules
- LEDs (Red, Yellow, Green, Blue)
- Push buttons
- Resistors and capacitors for stabilization
- Breadboard and jumper wires (Custom PCB design in progress)

## Setup
1. **Wiring**: Follow the schematic to connect the components.
2. **Upload Code**: Upload the provided code to the ESP8266 boards.
3. **Power Up**: Connect the boards to a power source.

## Usage
1. **Start Round**: Press the clear button on the hub to start a new round.
2. **Buzz In**: Teams can buzz in using their buttons. The first to buzz in will be acknowledged, and others will be locked out.
3. **Clear Round**: Press the clear button again to reset the system for the next round.

## Schematics and Diagrams
- Included in PCB folder.

## License
This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE.txt) file for details.

## Contributing
Contributions are welcome! Please fork the repository and submit pull requests.

## Acknowledgments
- Inspiration from Jeopardy-style quiz games.
