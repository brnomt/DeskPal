# DeskPal 🤠

DeskPal is a small ESP32-based interactive desktop companion inspired by Tamagotchi.
It displays animated facial expressions 😃 and reacts to user interaction by showing a motivational phrase 📜 or the currently playing Spotify song 🎵.

## Features ✨
- Displays a random animated face with different expressions 😊😆😴.
- Shows the current time ⏰ on a small OLED screen.
- Reacts to user input via a touch module 🔘:
  - **Single Press:** Fetches and displays a motivational quote 💡.
  - **Double Press:** Shows the currently playing song on Spotify 🎶.
- Connects to WiFi 📡 and communicates via MQTT 🔗.

## Hardware Requirements 🛠️
- ESP32C3 microcontroller ⚡
- OLED display (SSD1306) 📺
- TTP233 Module (A button should work as well) 🔲
- WiFi connectivity 🌐
- Some Backend, i used Node-RED

## Setup Instructions ⚙️
1. Flash the provided firmware to your ESP32 🖥️.
2. Configure WiFi and MQTT settings in the code 🔧.
3. Ensure an MQTT broker is running 🖧.
4. Deploy a backend service to handle motivational quotes and Spotify data 🎤.
5. Power on DeskPal and interact with it by tapping it! 🚀

## Usage 🎭
- DeskPal will display a random face 😐.
- Press the button to receive motivational messages 💪.
- Double press the button to display the currently playing song 🎼.
- The device updates its time via MQTT ⏳.

## License 📜
This project is open-source and available for modification and personal use 🛠️.
---

