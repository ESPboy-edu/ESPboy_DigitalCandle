# ESPboy Digital Candle 🕯️❄️

A highly atmospheric, realistic, and smooth interactive digital candle simulation designed for the [ESPboy](https://www.espboy.com/) platform. Built with a focus on immersive lighting, fluid physics, and hardware optimization, this project transforms your ESPboy into a cozy, interactive night lamp with a beautiful snowy backdrop.

Beyond just a technical showcase, this digital candle is explicitly designed as a tool for mindfulness and stress relief. The hypnotic, mathematically smooth dance of the flame, combined with the soft falling snow and the warm ambient glow of the screen and LED, creates a deeply calming atmosphere. It is perfect for late-night relaxation, meditation sessions, visual breathwork focus, or simply adding a touch of cozy warmth to a dark room without the fire hazards or smoke of a real candle.

## ✨ Features

* **Smooth Fire Physics (Pull Model):** Uses a custom, heavily optimized cellular automaton. Unlike the classic, noisy "Doom Fire", this algorithm uses a "Pull Model" that reads data against the wind direction. This guarantees a monolithic, fluid, and tear-free flame plasma without diagonal slicing artifacts.
* **Multi-Layered Parallax Snow:** A mesmerizing background featuring 4 depth layers of snow. Snowflakes range in speed and color from deep, slow-moving distant purples to soft, fast-moving icy blues in the foreground. The snow dynamically reacts to the same wind forces as the flame.
* **Organic Micro-Drafts:** Simulates ambient room air using a Low-Pass Filtered target wind. The flame features natural inertia, lazy flickering, and micro-tongues that randomly detach from the tip.
* **Procedural Wax & Wick:** Renders a 3D-shaded cylindrical wax body, a concave molten wax pool that reflects the fire, and an elongated exposed wick with a glowing, randomly flickering red-hot ember.
* **Smart Particles:** Emits a mixture of bright flying sparks and soft gray smoke. Each particle has an individual decay rate and reacts to the ambient wind. Sparks organically cool down and turn into smoke as they rise.
* **Immersive Lighting (Hardware DAC):** Modulates the TFT screen backlight in real-time via the MCP4725 I2C DAC. The ambient light naturally flickers and reacts with deep voltage drops during strong wind gusts, perfectly mimicking real firelight.
* **Ambient RGB LED:** The rear WS2812B LED syncs its color and intensity with the flame's core temperature. Brightness is strictly capped (0-5) to ensure it acts as a soft nightlight without blinding the user.
* **Multiple Palettes:** Choose between Classic Warm Amber, Magic Blue, or Alchemy Green. The selected palette is automatically saved to EEPROM.

## 🎮 Controls

| Button | Action |
| :--- | :--- |
| **ACT** | Cycle through flame color palettes (Saved to EEPROM). |
| **UP / DOWN** | Adjust the height and power of the flame (Levels 1 to 5). |
| **LEFT / RIGHT** | Apply strong, manual directional wind to the flame and snow. |
| **ESC** | Toggle the rear RGB LED On/Off. |


## 📄 License
This project is open-source. Feel free to modify, optimize, and share!