# speedwire-lib
Code implementing a SMA Speedwire(TM) access library. It implements a full parser for the sma header and the emeter datagram structure, including obis filtering. In addition, it implements some parsing functionality for inverter query and response datagrams.

The overall speedwire packet format is:
- The packets start with a 4 byte SMA Signature containing the ascii encoded string "SMA\0".
- After the signature follows a sequence of tag packets, where each tag packet starts with a tag header followed a sequence of tag payload bytes.
- The last tag packet is an end-of-data packet with 0 bytes of tag payload and a tag id of 0.

Emeter and inverter speedwire packets follow a standard format consisting of a tag0 packet holding the group id, a data2 packet holding the payload and an end-of-data packet.

    +---------------------------------------------------------------------------------+
    +      4 Bytes   | SMA Signature "SMA\0"                                          +
    +---------------------------------------------------------------------------------+
    +  Tag Packet 0                                                                   +
    +      2 Bytes   | Tag0 Length            | 4                                     +
    +      2 Bytes   | Tag0 ID                | 0x02a0                                +
    +      4 Bytes   | Group ID               | 0x00000001                            +
    +---------------------------------------------------------------------------------+
    +  Tag Packet 1                                                                   +
    +      2 Bytes   | Data2 Tag Length       | # of bytes following Data2 Tag ID     +
    +      2 Bytes   | Data2 Tag ID           | 0x0010                                +
    +      2 Bytes   | Protocol ID            | always encoded for Data2 tag packets  +
    +        Bytes   | Data                   | Emeter or inverter data               +
    +---------------------------------------------------------------------------------+
    +  Tag Packet 2                                                                   +
    +      2 Bytes   | End-of-Data Tag Length | 0x0000                                +
    +      2 Bytes   | End-of-Data Tag ID     | 0x0000                                +
    +---------------------------------------------------------------------------------+

The data part for emeter packets is documented in the official SMA-Emeter(TM) protocol specification:

   https://developer.sma.de/fileadmin/content/global/Partner/Documents/SMA_Labs/EMETER-Protokoll-TI-en-10.pdf

The data part for inverter packets is not officially documented. The protocol is based on register IDs that can be read from or written to. libspeedwire provides an implementation for reading register information, writing register information is not implemented. SMA recommends to rely on the documented Modbus API.

    +---------------------------------------------------------------------------------+
    +  Destination Address                                                            +
    +      2 Bytes   | Susy ID                | Destination devices susy id           +
    +      4 Bytes   | Serial Number          | Destination devices serial number     +
    +      2 Bytes   | Control                | unknown semantics                     +
    +---------------------------------------------------------------------------------+
    +  Source Address                                                                 +
    +      2 Bytes   | Susy ID                | Source devices susy id                +
    +      4 Bytes   | Serial Number          | Source devices serial number          +
    +      2 Bytes   | Control                | unknown semantics                     +
    +---------------------------------------------------------------------------------+
    +  Inverter Protocol                                                              +
    +      2 Bytes   | Error Code             | 0x0000 if no error                    +
    +      2 Bytes   | Fragment Counter       | Count down counter for # of packets   +
    +      2 Bytes   | Packet ID              | Unique packet identification          +
    +      4 Bytes   | Command ID             | see enum Command                      +
    +      4 Bytes   | First Register ID      | First Register ID                     +
    +      4 Bytes   | Last Register ID       | Last Register ID                      +
    +---------------------------------------------------------------------------------+
    +  Register Data                                                                  +
    +      n Bytes   | Register Data #0 First | see class SpeedwireRawData            +
    +      n Bytes   | ...                    |                                       +
    +      n Bytes   | Register Data #n Last  |                                       +
    +---------------------------------------------------------------------------------+

Useful examples on how to use this library can be found in the accompanying repositories:
- https://github.com/RalfOGit/sma-emeter-and-inverter-to-influxdb-cpp,
- https://github.com/RalfOGit/sma-emeter-simulator,
- https://github.com/RalfOGit/speedwire-router.

The implementation comes with Doxygen comments. Thus you can use Doxygen to create the documentation.

The software comes as is. No warrantees whatsoever are given and no responsibility is assumed in case of failure or damage being caused to your equipment.

The simplest way to build this library together with your code is to checkout this library into a separate folder and use unix symbolic links (ln -s ...) or ntfs junctions (mklink /J ...) to integrate it as a sub-folder within your projects folder.

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

   https://developer.sma.de/fileadmin/content/global/Partner/Documents/SMA_Labs/EMETER-Protokoll-TI-en-10.pdf

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
