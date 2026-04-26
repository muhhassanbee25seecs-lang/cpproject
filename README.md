Setup Guide for Windows Users

To get the app running on Windows, follow these steps:

    Step 1: Start by installing Docker Desktop and VcXsrv. When you're setting up VcXsrv, make sure to check the box for "Disable access control"—this is important for the display to work.

    Step 2: Ensure your system is ready for Linux containers. Go to "Turn Windows features on or off" and enable both Virtual Machine Platform and Windows Subsystem for Linux. Then, open PowerShell as an admin and run wsl --update.

    Step 3: Pull the latest version of the app by running:
    docker pull hackson12/attendance-app:latest

    Step 4: Now, find your local IP address by typing ipconfig in PowerShell. Copy your IPv4 address.

    Step 5: With Docker and VcXsrv both running, launch the app using this command (just swap in your IP address where it says <IPv4>):
  
    docker run -it --rm `
      -e DISPLAY=<IPv4>:0.0 `
      -v /tmp/.X11-unix:/tmp/.X11-unix `
      hackson12/attendance-app:latest

    Note on Camera Usage: If the camera isn't working, you'll need to reach out to the admin for permissions at medu142a@gmail.com.
  
