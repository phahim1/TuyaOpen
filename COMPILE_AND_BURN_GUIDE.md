# TuyaOpen Compilation and Burning Guide

This guide will help you compile and burn the `your_chat_bot` project.

## Prerequisites

1. **Python 3.6+** (You have Python 3.11.9 ✓)
2. **Git** (for cloning submodules if needed)
3. **Hardware**: Tuya T5AI Board or compatible device
4. **USB Cable** to connect your device

## Step 1: Environment Setup

### Option A: Using Git Bash or WSL (Recommended for Windows)

1. Open **Git Bash** or **WSL** terminal
2. Navigate to the project directory:
   ```bash
   cd /e/TuyaO/TuyaOpen
   ```
3. Source the export script:
   ```bash
   . ./export.sh
   ```
   This will:
   - Create a Python virtual environment
   - Install all required dependencies
   - Set up environment variables

### Option B: Manual Setup (If Option A fails)

1. **Install Python dependencies manually:**
   ```powershell
   cd E:\TuyaO\TuyaOpen
   python -m pip install --user -r requirements.txt
   ```
   
   If you encounter permission errors, you may need to:
   - Run PowerShell as Administrator, OR
   - Use a virtual environment:
     ```powershell
     python -m venv venv
     .\venv\Scripts\activate.ps1
     pip install -r requirements.txt
     ```

2. **Set environment variables:**
   ```powershell
   $env:OPEN_SDK_ROOT = "E:\TuyaO\TuyaOpen"
   $env:PATH = "$env:PATH;$env:OPEN_SDK_ROOT"
   ```

## Step 2: Navigate to Project Directory

```powershell
cd E:\TuyaO\TuyaOpen\apps\tuya.ai\your_chat_bot
```

## Step 3: Configure the Project

Run the configuration command to select your hardware platform:

```powershell
python ..\..\..\tos.py config choice
```

You will see a list of available configurations. Based on the README, select one of:
- **T5AI.config** (for TUYA T5AI_Board Development Board)
- **TUYA_T5AI_BOARD_LCD_3.5.config** (if you have the LCD version)
- **TUYA_T5AI_EVB.config** (for TUYA T5AI_EVB Board)

**Example output:**
```
--------------------
1. LN882H.config
2. EWT103-W15.config
3. Ubuntu.config
4. ESP32-C3.config
5. ESP32-S3.config
6. ESP32.config
7. T3.config
8. T5AI.config
9. T2.config
10. BK7231X.config
--------------------
Input "q" to exit.
Choice config file: 8
```

## Step 4: Compile the Project

After configuration, compile the project:

```powershell
python ..\..\..\tos.py build
```

This will:
- Build the project for your selected platform
- Generate firmware binary files in `.build/bin/` directory

**Expected output:**
```
[INFO]: ******************************
[INFO]: /xxx/TuyaOpen/apps/tuya.ai/your_chat_bot/.build/bin/your_chat_bot_QIO_1.0.0.bin
[INFO]: ******************************
[INFO]: ******* Build Success ********
[INFO]: ******************************
```

## Step 5: Connect Your Device

1. Connect your Tuya T5AI board to your computer via USB
2. **For Windows**: Check Device Manager to find the COM port
   - Open Device Manager (Win+X → Device Manager)
   - Look for "Ports (COM & LPT)" section
   - Note the COM port number (e.g., COM3, COM4)
   - **Note**: T5 series boards may show TWO COM ports:
     - One with "A" in the name = Download/Flash port
     - One with "B" in the name = Log/Monitor port

3. **For Linux/Mac**: List serial ports:
   ```bash
   ls /dev/tty*
   # or
   ls /dev/ttyACM*
   ```

## Step 6: Flash/Burn the Firmware

**Important**: Make sure you're in the project directory where you ran `tos.py build`.

```powershell
python ..\..\..\tos.py flash
```

The tool will:
- Detect available serial ports
- Ask you to select the flash port
- Erase flash memory
- Write the firmware
- Verify the CRC

**Example output:**
```
[INFO]: Run Tuya Uart Tool.
[INFO]: Use default baudrate: [921600]
[INFO]: Use default start address: [0x00]
--------------------
1. COM3
2. COM4
--------------------
Select serial port: 1
[INFO]: Waiting Reset ...
[INFO]: unprotect flash OK.
[INFO]: sync baudrate 921600 success
Erasing: ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 100%
[INFO]: Erase flash success
Writing: ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 100%
[INFO]: Write flash success
[INFO]: CRC check success
[INFO]: Reboot done
[INFO]: Flash write success.
```

**If you see "Port [xxx] may be busy":**
- Wait 1-2 minutes and try again
- Close any other programs using the serial port
- Unplug and replug the USB cable

## Step 7: Monitor Logs (Optional)

To view device logs:

```powershell
python ..\..\..\tos.py monitor
```

Select the **log port** (usually the port with "B" in Windows or the higher number in Linux).

Press `Ctrl+C` to exit monitoring.

## Troubleshooting

### Windows Compilation Slow
- Close `MSPCManagerService` process in Task Manager
- Add TuyaOpen directory to Windows Defender exclusions
- Move project to non-system drive (D: drive)

### Permission Errors
- Run PowerShell/Command Prompt as Administrator
- Or use Git Bash/WSL instead

### Port Not Found
- Install USB drivers for your board
- Check Device Manager for unrecognized devices
- Try different USB cable/port

### Build Fails
- Ensure all dependencies are installed: `pip install -r requirements.txt`
- Check that you've selected the correct config file
- Clean build cache: `python ..\..\..\tos.py clean -f`

### Flash Fails
- Ensure device is in download mode (may need to hold a button)
- Try different baudrate (some boards use 115200 instead of 921600)
- Check USB cable connection
- Wait 1-2 minutes if port is busy

## Additional Resources

- **Environment Setup**: https://tuyaopen.ai/zh/docs/quick-start/environment-setup
- **Project Compilation**: https://tuyaopen.ai/zh/docs/quick-start/project-compilation  
- **Firmware Burning**: https://tuyaopen.ai/zh/docs/quick-start/firmware-burning
- **TuyaOpen Documentation**: https://tuyaopen.ai/docs/about-tuyaopen

## Quick Reference Commands

```powershell
# Navigate to project
cd E:\TuyaO\TuyaOpen\apps\tuya.ai\your_chat_bot

# Configure
python ..\..\..\tos.py config choice

# Build
python ..\..\..\tos.py build

# Flash
python ..\..\..\tos.py flash

# Monitor
python ..\..\..\tos.py monitor

# Clean
python ..\..\..\tos.py clean -f
```
