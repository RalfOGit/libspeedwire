# speedwire-lib
Code implementing a SMA Speedwire(TM) access library. It implements a full parser for the sma header and the emeter datagram structure, including obis filtering. In addition, it implements some parsing functionality for inverter query and response datagrams.

Useful examples on how to use this library can be found in the accompanying repositories:

    https://github.com/RalfOGit/sma-emeter-and-inverter-to-influxdb-cpp,
    https://github.com/RalfOGit/sma-emeter-simulator,
    https://github.com/RalfOGit/speedwire-router.

The simplest way to build this library together with your code is to checkout this library into a separate folder and use unix symbolic links (ln -s ...) or ntfs junctions (mklink /J ...) to integrate it as a sub-folder within your projects folder. This will let you use the included CMakeLists.txt without modifications.

For example, if you are developing on a Windows host and your projects reside in C:\workspaces:

    cd C:\workspaces
    mkdir libspeedwire
    git clone https://github.com/RalfOGit/libspeedwire
    cd ..\YOUR_PROJECT_FOLDER
    mklink /J libspeedwire ..\libspeedwire
    Now you can start Visual Studio
    And in Visual Studio open folder YOUR_PROJECT_FOLDER

And if you are developing on a Linux host and your projects reside in /home/YOU/workspaces:

    cd /home/YOU/workspaces
    mkdir libspeedwire
    git clone https://github.com/RalfOGit/libspeedwire
    cd ../YOUR_PROJECT_FOLDER
    ln -s ../libspeedwire
    Now you can start VSCode
    And in VSCode open folder YOUR_PROJECT_FOLDER

The official SMA-Emeter(TM) protocol specification is available here:

    https://www.sma.de/fileadmin/content/global/Partner/Documents/SMA_Labs/EMETER-Protokoll-TI-en-10.pdf

Further information regarding the SMA-Inverter(TM) datagrams can be found in various places on the internet:

    https://github.com/SBFspot/SBFspot
    https://github.com/Rincewind76/SMAInverter
    https://github.com/ardexa/sma-rs485-inverters
    https://github.com/dgibson/python-smadata2/blob/master/doc/protocol.txt
    https://github.com/peterbarker/python-smadata2

The code has been tested against the following environment:

    OS: CentOS 8(TM), IDE: VSCode (TM)
    OS: Windows 10(TM), IDE: Visual Studio Community Edition 2019 (TM)

You will need to open your local firewall for udp packets on port 9522.
