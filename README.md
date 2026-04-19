# 🛸 AeroBalance | Modular Control Center

![AeroBalance Banner](aerobalance_banner_1776607596457.png)

AeroBalance is a premium, high-performance web interface designed for real-time control of ESP32-powered robotics, drones, and балансировочные платформы. Built with a focus on aesthetics and low-latency communication, it provides a unified command center for various input methods.

## ✨ Features

-   **Dual-Mode Connectivity**: Seamlessly switch between **Wi-Fi (WebSockets)** and **Bluetooth Low Energy (BLE)** to maintain a stable tether with your hardware.
-   **Multi-Input Systems**:
    -   🕹️ **Interactive Joystick**: Precision 2D joystick with real-time X/Y tracking.
    -   📱 **Tile/Gyroscope Mode**: Control your device by tilting your phone. Includes a **SET CENTER** calibration feature for personalized comfort.
    -   ⌨️ **Arrow Grid**: Familiar 4-way button interface for discrete movements.
    -   🏎️ **Speed Slider**: Vertical throttle control with forward/reverse indicators.
    -   🤖 **AI Pilot Mode**: Dedicated interface for autonomous operation.
-   **Rich Aesthetics**: Stunning glassmorphism UI with dynamic background orbs that react to your device's orientation.
-   **Mobile First**: Optimized for a fullscreen, app-like experience on iOS and Android.

## 🚀 Getting Started

### 1. Connection
Click the **DISCONNECTED** status box in the top-left to open the Connection Overlay.
-   **WI-FI**: Enter your ESP32's IP address (default: `192.168.4.1`) and tap **CONNECT**.
-   **Bluetooth**: Tap **PAIR DEVICE** and select your `AeroBalance` compatible hardware.

### 2. Operation
1.  Toggle the **BALANCE ON / OFF** button to initialize the system.
2.  Select your preferred control mode from the top bar.
3.  Use the generated controls to send real-time commands.

### 3. Gyroscope Calibration
When using **GYROSCOPE** mode:
1.  Hold your device in your most comfortable position.
2.  Tap **SET CENTER**. The UI will flash green and snap the center point to your current orientation.

## 🛠️ Technical Stack

-   **Frontend**: Vanilla HTML5, CSS3 (Glassmorphism), and JavaScript (ES6+).
-   **Communication**: 
    -   `WebSockets` for high-speed Wi-Fi data transmission.
    -   `Web Bluetooth API` for low-energy proximity control.
-   **Visuals**: Dynamic CSS animations and absolute-positioned glowing orbs for a premium "Cyberpunk" feel.

## 📡 Protocol Reference

Commands are sent as plain-text strings:
-   `BAL:ON` / `BAL:OFF`
-   `JOY:X,Y` (Range -100 to 100)
-   `GYRO:X,Y` (Range -100 to 100)
-   `SPD:Value` (Range -100 to 100)

---

Developed with ❤️ for high-performance robotics.
