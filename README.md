# speedwire-lib
Code implementing a SMA Speedwire(TM) access library. It implements a full parser for the sma header and the emeter datagram structure, including obis filtering. In addition, it implements some parsing functionality for inverter query and response datagrams.

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
